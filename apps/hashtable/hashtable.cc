#include "util/latency.h"
#include <gflags/gflags.h>
#include <signal.h>
#include <cstring>
#include "../apps_common.h"
#include "HdrHistogram_c/src/hdr_histogram.h"
#include "rpc.h"
#include "util/autorun_helpers.h"
#include "util/numautils.h"
#include "pc/parser/rpc_parse.h"
#include "pc/config.h"
#include "pc/utils/logger.h"
#include "pc/datastructure/hashtable.h"
#include "pc/worker/rpc_worker.h"
#include "concurrentqueue.h"

using namespace pc;
using namespace pc::utils;
using namespace pc::datastructure;
using namespace pc::worker;
using namespace pc::parser;


static constexpr size_t kAppEvLoopMs = 1;  // Duration of event loop
static constexpr bool kAppVerbose = false;    // Print debug info on datapath
static constexpr size_t kAppReqType = 1;      // eRPC request type
static constexpr size_t kAppMaxWindowSize = 32;  // Max pending reqs per client
static constexpr size_t kAppReqSize = 40;
static constexpr size_t kAppRespSize = 40;
static constexpr size_t kAppMaxConcurrency = 32; // Set outstanding request per thread

static RpcParse input_parser;

// Precision factor for latency measurement
static constexpr double kAppLatFac = erpc::kIsAzure ? 1.0 : 10.0;

volatile sig_atomic_t ctrl_c_pressed = 0;
void ctrl_c_handler(int) { ctrl_c_pressed = 1; }

DEFINE_uint64(num_server_processes, 1, "Number of server processes");
DEFINE_uint64(resp_size, 8, "Size of the server's RPC response in bytes");
DEFINE_uint64(req_size, 0, "Request data size");
DEFINE_uint64(concurrency, 0, "Concurrent requests per thread");
DEFINE_uint64(num_keys, 0, "Number of keys");
DEFINE_uint64(num_queries, 0, "Number of queries");

struct app_stats_t {
  double rx_gbps;
  double tx_gbps;
  size_t re_tx;
  double rtt_50_us;  // Median packet RTT
  double rtt_99_us;  // 99th percentile packet RTT
  double rpc_50_us;
  double rpc_99_us;
  double rpc_999_us;

  app_stats_t() { memset(this, 0, sizeof(app_stats_t)); }

  static std::string get_template_str() {
    return "rx_gbps tx_gbps re_tx rtt_50_us rtt_99_us rpc_50_us rpc_99_us "
           "rpc_999_us";
  }

  /// Return a space-separated string of all stats
  std::string to_string() {
    return std::to_string(rx_gbps) + " " + std::to_string(tx_gbps) + " " +
           std::to_string(re_tx) + " " + std::to_string(rtt_50_us) + " " +
           std::to_string(rtt_99_us) + " " + std::to_string(rpc_50_us) + " " +
           std::to_string(rpc_99_us) + " " + std::to_string(rpc_999_us);
  }

  /// Accumulate stats
  app_stats_t& operator+=(const app_stats_t& rhs) {
    this->rx_gbps += rhs.rx_gbps;
    this->tx_gbps += rhs.tx_gbps;
    this->re_tx += rhs.re_tx;
    this->rtt_50_us += rhs.rtt_50_us;
    this->rtt_99_us += rhs.rtt_99_us;
    this->rpc_50_us += rhs.rpc_50_us;
    this->rpc_99_us += rhs.rpc_99_us;
    this->rpc_999_us += rhs.rpc_999_us;
    return *this;
  }
};

class ServerContext : public BasicAppContext {
 public:
  // Data structure 
  hashtable::HashTable ht;
  // Workers
  moodycamel::ConcurrentQueue<Request> request_q;
  moodycamel::ConcurrentQueue<Response> response_q;
  std::vector<std::unique_ptr<RpcWorker>> worker_set;
  // Tracking
  size_t num_resps = 0;
  size_t stat_rx_bytes_tot = 0;
  size_t stat_tx_bytes_tot = 0;
  // Buffer
  erpc::MsgBuffer req_msgbuf_, resp_msgbuf_;
};

class ClientContext : public BasicAppContext {
  static constexpr int64_t kMinLatencyMicros = 1;
  static constexpr int64_t kMaxLatencyMicros = 1000 * 1000 * 100;  // 100 sec
  static constexpr int64_t kLatencyPrecision = 2;  // Two significant digits

 public:
  // Tracking
  std::vector<double> lat_vec;
  size_t num_reqs = 0;
  size_t stat_rx_bytes_tot = 0;
  size_t stat_tx_bytes_tot = 0;
  erpc::ChronoTimer tput_t0;  // Start time for throughput measurement
  app_stats_t* app_stats;     // Common stats array for all threads

  //size_t start_tsc_; // This is for single request
  uint64_t req_ts[kAppMaxConcurrency];  // Per-request timestamps
  size_t req_size_;  // Between kAppMinReqSize and kAppMaxReqSize
  erpc::MsgBuffer req_msgbuf_[kAppMaxConcurrency], resp_msgbuf_[kAppMaxConcurrency];
  hdr_histogram *latency_hist_;
  size_t latency_samples_ = 0;
  size_t latency_samples_prev_ = 0;

  ClientContext() {
    int ret = hdr_init(kMinLatencyMicros, kMaxLatencyMicros, kLatencyPrecision,
                       &latency_hist_);
    erpc::rt_assert(ret == 0, "Failed to initialize latency histogram");
  }

  ~ClientContext() { hdr_close(latency_hist_); }
};

void req_handler(erpc::ReqHandle *req_handle, void *_context) {
  auto *c = static_cast<ServerContext *>(_context);
  erpc::Rpc<erpc::CTransport>::resize_msg_buffer(&req_handle->pre_resp_msgbuf_,
                                                 FLAGS_resp_size);

  const erpc::MsgBuffer *req_msgbuf = req_handle->get_req_msgbuf();
  //size_t req_size = req_msgbuf->get_data_size();
  const Request *req = reinterpret_cast<const Request*>(req_msgbuf->buf_);

  //uint8_t key_copy[sizeof(hashtable::key_type)];  // mti->get() modifies key
  hashtable::key_type key_copy;
  memcpy(&key_copy, &(req->key), sizeof(hashtable::key_type));

  auto *resp =
      reinterpret_cast<Response *>(req_handle->pre_resp_msgbuf_.buf_);
  auto result =  c->ht.find(key_copy); // TODO avoid data copy here
  //const bool success = c->ht.find_inline(key_copy, resp->result); // TODO avoid data copy here
  memcpy(&(resp->result), &result, sizeof(Response::result));
  c->stat_rx_bytes_tot += FLAGS_req_size;
  c->stat_tx_bytes_tot += FLAGS_resp_size;
  //LOG(log_level::info) << "Receving requests and Sending out response";
  c->rpc_->enqueue_response(req_handle, &req_handle->pre_resp_msgbuf_);
}

void server_func(erpc::Nexus *nexus) {
  printf("Hash table: Running server, process ID %zu\n", FLAGS_process_id);

  std::vector<size_t> port_vec = flags_get_numa_ports(FLAGS_numa_node);
  uint8_t phy_port = port_vec.at(0);

  ServerContext c;

  erpc::Rpc<erpc::CTransport> rpc(nexus, static_cast<void *>(&c), 0 /* tid */,
                                  basic_sm_handler, phy_port);

  // Setup hashtable
  std::ifstream data_stream(std::string("/var/data/ycsbc-key-test"));
  input_parser.read_all_keys(data_stream, FLAGS_num_keys); //FIXME num_keys
  LOG(log_level::info) << "Load all keys";
  for(size_t i = 0; i < FLAGS_num_keys; i++) {  //FIXME num_keys
    std::string value = gen_random(7); //FIXME check this size
    c.ht.insert(std::make_pair(input_parser.all_keys[i], value.c_str()));    
  }
  LOG(log_level::info) << "insert all keys";

  rpc.set_pre_resp_msgbuf_size(FLAGS_resp_size);
  c.rpc_ = &rpc;

  while (true) {
    rpc.run_event_loop(1000);
    if (ctrl_c_pressed == 1) break;
  }
}

void connect_sessions(ClientContext &c) {
  for (size_t i = 0; i < FLAGS_num_server_processes; i++) {
    const std::string server_uri = erpc::get_uri_for_process(i);
    printf("Process %zu: Creating session to %s.\n", FLAGS_process_id,
           server_uri.c_str());

    const int session_num =
        c.rpc_->create_session(server_uri, 0 /* tid at server */);
    erpc::rt_assert(session_num >= 0, "Failed to create session");
    c.session_num_vec_.push_back(session_num);

    while (c.num_sm_resps_ != (i + 1)) {
      //LOG(log_level::info) << "Into this event loop";
      c.rpc_->run_event_loop(kAppEvLoopMs);
      if (unlikely(ctrl_c_pressed == 1)) return;
    }
  }
}

void app_cont_func(void *, void *);
inline void send_req(ClientContext &c, size_t msgbuf_idx) {
  c.rpc_->resize_msg_buffer(&c.req_msgbuf_[msgbuf_idx], FLAGS_req_size);
  c.rpc_->resize_msg_buffer(&c.resp_msgbuf_[msgbuf_idx], FLAGS_resp_size);
  //LOG(log_level::info) << "Resize buffers";
  c.req_ts[msgbuf_idx] = erpc::rdtsc();
  erpc::MsgBuffer &req_msgbuf = c.req_msgbuf_[msgbuf_idx];
  Request req;
  //LOG(log_level::info) << "Number of queries: " << input_parser.all_query.size();
  req.key = input_parser.all_query[c.num_reqs].key;
  *reinterpret_cast<Request *>(req_msgbuf.buf_) = req;

  const size_t server_id = c.fastrand_.next_u32() % FLAGS_num_server_processes;
  c.rpc_->enqueue_request(c.session_num_vec_[server_id], kAppReqType,
                          &c.req_msgbuf_[msgbuf_idx], &c.resp_msgbuf_[msgbuf_idx], app_cont_func,
                          reinterpret_cast<void*>(msgbuf_idx));
  //LOG(log_level::info) << "Request enqueued";
  if (kAppVerbose) {
    printf("Latency: Sending request of size %zu bytes to server #%zu\n",
           c.req_msgbuf_[msgbuf_idx].get_data_size(), server_id);
  }
  c.num_reqs++;
  if(c.num_reqs%1000 == 0) {
    LOG(log_level::info) << "Number of requests sent: " << c.num_reqs;
  }
  c.stat_tx_bytes_tot += FLAGS_req_size;
}

void app_cont_func(void *_context, void *_tag) {
  auto *c = static_cast<ClientContext *>(_context);
  auto msgbuf_idx = reinterpret_cast<size_t>(_tag);
  //const erpc::MsgBuffer &resp_msgbuf = c->req_msgbuf_[msgbuf_idx];
  //assert(c->resp_msgbuf_.get_data_size() == kAppRespSize);

  if (kAppVerbose) {
    printf("Latency: Received response of size %zu bytes\n",
           c->resp_msgbuf_[msgbuf_idx].get_data_size());
  }

  // Measure latency. 1 us granularity is sufficient for large RPC latency.
  double usec = erpc::to_usec(erpc::rdtsc() - c->req_ts[msgbuf_idx],
                              c->rpc_->get_freq_ghz());

  //const double req_lat_us =
  //    erpc::to_usec(erpc::rdtsc() - c->start_tsc_, c->rpc_->get_freq_ghz());
  //LOG(log_level::info) << "Latency is: " << usec << " microseconds";
  c->lat_vec.push_back(usec);
  c->stat_rx_bytes_tot += FLAGS_resp_size;
  //LOG(log_level::info) << "Sending out response";

  //hdr_record_value(c->latency_hist_,
  //                 static_cast<int64_t>(req_lat_us * kAppLatFac));
  //c->latency_samples_++;

  send_req(*c, msgbuf_idx);
}

void client_func(erpc::Nexus *nexus) {
  printf("Latency: Running client, process ID %zu\n", FLAGS_process_id);
  
  std::ifstream query_stream(std::string("/var/data/ycsbc-query-test"));
  input_parser.read_all_query(query_stream, FLAGS_num_queries); //FIXME num_queries
  LOG(log_level::info) << "Load all queries";

  std::vector<size_t> port_vec = flags_get_numa_ports(FLAGS_numa_node);
  uint8_t phy_port = port_vec.at(0);

  ClientContext c;
  erpc::Rpc<erpc::CTransport> rpc(nexus, static_cast<void *>(&c), 0,
                                  basic_sm_handler, phy_port);

  rpc.retry_connect_on_invalid_rpc_id_ = true;
  c.rpc_ = &rpc;
  c.app_stats = new struct app_stats_t;

  connect_sessions(c);
  LOG(log_level::info) << "Session connected ";

  for (size_t i = 0; i < FLAGS_concurrency; i++) {
    c.req_msgbuf_[i] = c.rpc_->alloc_msg_buffer_or_die(FLAGS_req_size);
    c.resp_msgbuf_[i] = c.rpc_->alloc_msg_buffer_or_die(FLAGS_resp_size);

    // Fill the request regardless of kAppMemset. This is a one-time thing.
    //memset(c.req_msgbuf_[i].buf_, kAppDataByte, FLAGS_req_size);
  }
  LOG(log_level::info) << "Finished initializing buffers";

  //c.req_msgbuf_ = rpc.alloc_msg_buffer_or_die(kAppReqSize);
  //c.resp_msgbuf_ = rpc.alloc_msg_buffer_or_die(kAppRespSize);


  //printf("Latency: Process %zu: Session connected. Starting work.\n",
  //       FLAGS_process_id);
  //printf(
  //    "req_size median_us 5th_us 99th_us 99.9th_us 99.99th_us 99.999th_us "
  //    "99.9999th_us max_us [new samples, total_samples in distribution, "
  //    "total_time]\n");

  for (size_t msgbuf_idx = 0; msgbuf_idx < FLAGS_concurrency; msgbuf_idx++) {
    send_req(c, msgbuf_idx);
  }

  //send_req(c);
  c.tput_t0.reset();
  LOG(log_level::info) << "Start event loop";

  for (size_t i = 0; i < FLAGS_test_ms; i += kAppEvLoopMs) {
    rpc.run_event_loop(kAppEvLoopMs);
    if (unlikely(ctrl_c_pressed == 1)) break;
    if (c.session_num_vec_.size() == 0) continue;  // No stats to print

    const double ns = c.tput_t0.get_ns();
    erpc::Timely *timely_0 = c.rpc_->get_timely(0);

    // Publish stats
    auto &stats = c.app_stats[c.thread_id_];
    stats.rx_gbps = c.stat_rx_bytes_tot * 8 / ns;
    stats.tx_gbps = c.stat_tx_bytes_tot * 8 / ns;
    stats.re_tx = c.rpc_->get_num_re_tx(c.session_num_vec_[0]);
    stats.rtt_50_us = timely_0->get_rtt_perc(0.50);
    stats.rtt_99_us = timely_0->get_rtt_perc(0.99);

    if (c.lat_vec.size() > 0) {
      //LOG(log_level::info) << "Sort here";
      std::sort(c.lat_vec.begin(), c.lat_vec.end());
      stats.rpc_50_us = c.lat_vec[c.lat_vec.size() * 0.50];
      stats.rpc_99_us = c.lat_vec[c.lat_vec.size() * 0.99];
      stats.rpc_999_us = c.lat_vec[c.lat_vec.size() * 0.999];
    } else {
      // Even if no RPCs completed, we need retransmission counter
      stats.rpc_50_us = kAppEvLoopMs * 1000;
      stats.rpc_99_us = kAppEvLoopMs * 1000;
      stats.rpc_999_us = kAppEvLoopMs * 1000;
    }

    printf(
        "large_rpc_tput: Thread %zu: Tput {RX %.2f (%zu), TX %.2f (%zu)} "
        "Gbps (IOPS). Retransmissions %zu. Packet RTTs: {%.1f, %.1f} us. "
        "RPC latency {%.1f 50th, %.1f 99th, %.1f 99.9th}. Timely rate %.1f "
        "Gbps. Credits %zu (best = 32).\n",
        c.thread_id_, stats.rx_gbps, c.stat_rx_bytes_tot / FLAGS_resp_size,
        stats.tx_gbps, c.stat_tx_bytes_tot / FLAGS_req_size, stats.re_tx,
        stats.rtt_50_us, stats.rtt_99_us, stats.rpc_50_us, stats.rpc_99_us,
        stats.rpc_999_us, timely_0->get_rate_gbps(), erpc::kSessionCredits);
    fflush(stdout);

    // Reset stats for next iteration
    c.stat_rx_bytes_tot = 0;
    c.stat_tx_bytes_tot = 0;
    c.rpc_->reset_num_re_tx(c.session_num_vec_[0]);
    c.lat_vec.clear();
    timely_0->reset_rtt_stats();

    //if (c.thread_id_ == 0) {
    //  app_stats_t accum_stats;
    //  for (size_t i = 0; i < FLAGS_num_proc_other_threads; i++) {
    //    accum_stats += c.app_stats[i];
    //  }

    //  // Compute averages for non-additive stats
    //  accum_stats.rtt_50_us /= FLAGS_num_proc_other_threads;
    //  accum_stats.rtt_99_us /= FLAGS_num_proc_other_threads;
    //  accum_stats.rpc_50_us /= FLAGS_num_proc_other_threads;
    //  accum_stats.rpc_99_us /= FLAGS_num_proc_other_threads;
    //  accum_stats.rpc_999_us /= FLAGS_num_proc_other_threads;
    //  c.tmp_stat_->write(accum_stats.to_string());
    //}

    c.tput_t0.reset();
  }

  //erpc::TimingWheel *wheel = rpc.get_wheel();
  //if (wheel != nullptr && !wheel->record_vec_.empty()) {
  //  const size_t num_to_print = 200;
  //  const size_t tot_entries = wheel->record_vec_.size();
  //  const size_t base_entry = tot_entries * .9;

  //  printf("Printing up to 200 entries toward the end of wheel record\n");
  //  size_t num_printed = 0;

  //  for (size_t i = base_entry; i < tot_entries; i++) {
  //    auto &rec = wheel->record_vec_.at(i);
  //    printf("wheel: %s\n",
  //           rec.to_string(console_ref_tsc, rpc.get_freq_ghz()).c_str());

  //    if (num_printed++ == num_to_print) break;
  //  }
  //}
  //for (size_t i = 0; i < FLAGS_test_ms; i += 1000) {
  //  rpc.run_event_loop(kAppEvLoopMs);  // 1 second
  //  if (ctrl_c_pressed == 1) break;

  //  //const double ns = c.tput_t0.get_ns();
  //  //erpc::Timely *timely_0 = c.rpc_->get_timely(0);

  //  if (c.latency_samples_ == c.latency_samples_prev_) {
  //    printf("No new responses in %.2f seconds\n", kAppEvLoopMs / 1000.0);
  //    fprintf(stderr, "No new responses in %.2f seconds\n",
  //            kAppEvLoopMs / 1000.0);
  //    fflush(stderr);
  //  } else {
  //    printf(
  //        "%zu %.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f "
  //        "[%zu new samples, %zu total samples, %zu seconds]\n",
  //        c.req_size_,
  //        hdr_value_at_percentile(c.latency_hist_, 50.0) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 5.0) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 99) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 99.9) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 99.99) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 99.999) / kAppLatFac,
  //        hdr_value_at_percentile(c.latency_hist_, 99.9999) / kAppLatFac,
  //        hdr_max(c.latency_hist_) / kAppLatFac,
  //        c.latency_samples_ - c.latency_samples_prev_, c.latency_samples_,
  //        i / 1000);
  //  }
  //  fflush(stdout);

  //  // Warmup for the first two seconds. Also, reset percentiles every minute.
  //  const size_t seconds = i / 1000;
  //  if (seconds < 2 || (seconds % 60 == 0)) {
  //    hdr_reset(c.latency_hist_);
  //    c.latency_samples_ = 0;
  //    c.latency_samples_prev_ = 0;
  //  }

  //  c.latency_samples_prev_ = c.latency_samples_;
  //}
}

int main(int argc, char **argv) {
  signal(SIGINT, ctrl_c_handler);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  erpc::rt_assert(FLAGS_numa_node <= 1, "Invalid NUMA node");

  erpc::Nexus nexus(erpc::get_uri_for_process(FLAGS_process_id),
                    FLAGS_numa_node, 0);
  nexus.register_req_func(kAppReqType, req_handler);

  //if(FLAGS_process_id < FLAGS_num_server_processes) {
  //  // Server function

  //} else {
  //  // Client function
  //  //std::ifstream data_stream(std::string("/var/data/ycsbc-key-1-blade"));
  //  //parser.read_all_keys(data_stream, num_keys);
  //  //std::ifstream query_stream(std::string("/var/data/ycsbc-query-1-blade"));
  //  //parser.read_all_queries(query_stream, num_queries);
  //}

  auto t = std::thread(
      FLAGS_process_id < FLAGS_num_server_processes ? server_func : client_func,
      &nexus);

  const size_t num_socket_cores =
      erpc::get_lcores_for_numa_node(FLAGS_numa_node).size();
  const size_t affinity_core = FLAGS_process_id % num_socket_cores;
  printf("Latency: Will run on CPU core %zu\n", affinity_core);
  if (FLAGS_process_id >= num_socket_cores) {
    fprintf(stderr,
            "Latency: Warning: The number of latency processes is close to "
            "this machine's core count. This could be fine, but to ensure good "
            "performance, please double-check for core collision.\n");
  }

  erpc::bind_to_core(t, FLAGS_numa_node, affinity_core);
  t.join();
}
