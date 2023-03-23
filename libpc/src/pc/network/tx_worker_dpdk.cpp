#include "tx_worker_dpdk.h"

#include <arpa/inet.h>

#include <cassert>
#include <utility>

#include "dpdk_transport.h"
#include "txrx_worker.h"
#include "pc/utils/logger.h"

using namespace pc::utils;

namespace pc
{
  namespace network
  {

    static constexpr bool kDebugDPDK = false;

    TxWorkerDpdk::TxWorkerDpdk(Config* cfg,
        size_t core_offset, size_t tid,
        moodycamel::ConcurrentQueue<Request> *request_q,
        moodycamel::ConcurrentQueue<Response> *response_q,
        rte_mempool *mbuf_pool,
        WorkerType type)
        : TxRxWorker(cfg, core_offset, tid,
                     request_q,
                     response_q, type),
          mbuf_pool_(mbuf_pool)
    {
      //int ret = inet_pton(AF_INET, config->BsRruAddr().c_str(), &bs_rru_addr_);
      //RtAssert(ret == 1, "Invalid sender IP address");
      //ret = inet_pton(AF_INET, config->BsServerAddr().c_str(), &bs_server_addr_);
      //RtAssert(ret == 1, "Invalid server IP address");

      // FIXME check this
      const uint16_t dest_port = 0;
      const uint16_t src_port = 0;

      // const auto &port_queue_id = dpdk_phy_port_queues_.at(interface);
      const auto &port_queue_id = 0;
      // const auto &port_id = port_queue_id.first;
      const auto &port_id = 0;
      // const auto &queue_id = port_queue_id.second;
      const auto &queue_id = 0;

      auto status = rte_eth_macaddr_get(port_id, &src_mac_);
      // addr.addr_bytes
      dest_mac_ = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
      RtAssert(status == 0, "Could not retreive mac address");

    }

    TxWorkerDpdk::~TxWorkerDpdk() { Stop(); };

    // Todo: remove this
    template <void (TxWorkerDpdk::*run_thread)()>
    static void ClassFunctioWrapper(TxWorkerDpdk *context)
    {
      return (context->*run_thread)();
    }

    // DPDK doesn't use c++ std threads but the class still has a 1:1 mapping
    // worker:lcore
    void TxWorkerDpdk::Start()
    {
      // Wait until an lcore is free
      rte_eal_wait_lcore(tid_);
      char buf[1024];
      sprintf(buf, "TxWorkerDpdk[%zu]: starting\n", tid_);
      LOG(log_level::info) << std::string(buf);
      // Launch a function on another lcore
      const int status = rte_eal_remote_launch(
          (lcore_function_t *)(ClassFunctioWrapper<&TxWorkerDpdk::DoTxRx>), this,
          tid_);
      sprintf(buf, "TxWorkerDpdk[%zu]: started on dpdk managed l_core\n", tid_);
      LOG(log_level::info) << std::string(buf);
      RtAssert(status == 0, "Lcore cannot launch TxRx function");
    }

    void TxWorkerDpdk::Stop()
    {
      // Configuration()->Running(false);
      //  Wait until the lcore finishes its job (join)
      running_ = false;
      rte_eal_wait_lcore(tid_);
    }

    void TxWorkerDpdk::DoTxRx()
    {
      // This is originally used by Agora to synchronize between different workers to start at the same time
      started_ = true;
      size_t prev_frame_id = SIZE_MAX;
      size_t rx_index = 0;
      const unsigned int thread_socket = rte_socket_id();

      char buf[1024];
      sprintf(buf, "TxWorkerDpdk[%zu]: running on socket %u\n", tid_,
              thread_socket);
      LOG(log_level::info) << std::string(buf);
      uint16_t dev_id = UINT16_MAX;
      // for (auto &device_queue : dpdk_phy_port_queues_)
      //{
      // const uint16_t current_dev_id = device_queue.first;
      const uint16_t current_dev_id = 0;
      const unsigned int dev_socket = rte_eth_dev_socket_id(current_dev_id);
      // if ((dev_id != current_dev_id) && (thread_socket != dev_socket))
      //{
      //   AGORA_LOG_WARN(
      //       "TxWorkerDpdk[%zu]: running on socket %u but the ethernet device "
      //       "is on socket %u\n",
      //       tid_, thread_socket, dev_socket);
      // }
      dev_id = current_dev_id;
      //}
      running_ = true;

      while (running_)
      {
        const size_t send_result = DequeueSend();
      }   // running
      running_ = false;
    }

    size_t TxWorkerDpdk::DequeueSend()
    {
      auto tx_requests = GetPendingTxRequests();

      // FIXME check tx event
      // Process each pending tx event
      for (const Request &current_request : tx_requests)
      {
        //auto *pkt = GetTxPacket(frame_id, symbol_id, ant_id);
        //new (pkt)
        //    Packet(frame_id, symbol_id, Configuration()->CellId().at(0), ant_id);
        auto *pkt = &current_request;

        rte_mbuf *tx_bufs __attribute__((aligned(64)));
        tx_bufs = DpdkTransport::AllocUdp(
            mbuf_pool_, cfg_->srcMac,
            cfg_->dstMac, cfg_->srcIp, cfg_->dstIp, cfg_->srcPort, cfg_->dstPort, cfg_->buffer_length, 1);

        static_assert(
            kTxBatchSize == 1,
            "kTxBatchSize must equal 1 - correct logic or set the value to 1");
        rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(tx_bufs, rte_ether_hdr *);
        auto *payload = reinterpret_cast<char *>(eth_hdr) + kPayloadOffset;

        rte_memcpy(payload, pkt, cfg_->packet_length);

        // Send data (one OFDM symbol)
        // Must send this out the correct port (dev) + queue that is assigned to this interface (convert global to local index)
        //const auto &tx_info = dpdk_phy_port_queues_.at(local_interface_idx);
        size_t nb_tx_new =
            rte_eth_tx_burst(0, 0, &tx_bufs, kTxBatchSize);
        //std::this_thread::sleep_for (std::chrono::milliseconds(1));
        //LOG(log_level::info) << "Sending out request: " << current_request.ht << " at time: " << get_time();
        if (unlikely(nb_tx_new != kTxBatchSize))
        {
          LOG(log_level::info) << "TxWorkerDpdk[" << tid_ << "]: rte_eth_tx_burst() failed";
          throw std::runtime_error("TxWorkerDpdk: rte_eth_tx_burst() failed");
        }
      }

      // FIXME
      //return tx_events.size();
      return 0;
    }

  }
}
