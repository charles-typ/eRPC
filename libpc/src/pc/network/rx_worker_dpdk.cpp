
#include "rx_worker_dpdk.h"

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

    RxWorkerDpdk::RxWorkerDpdk(Config* cfg,
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

      //char buf[1024];
      //sprintf(buf,
      //        "Adding steering rule for src IP %s, dest IP %s, src port: "
      //        "%d, dst port: %d, DPDK dev %d, queue: %d\n",
      //        config->BsRruAddr().c_str(), config->BsServerAddr().c_str(), src_port,
      //        dest_port, port_id, queue_id);

      //LOG(log_level::info) << std::string(buf);
      // FIXME check if need to install flow rules
      // DpdkTransport::InstallFlowRule(port_id, queue_id, bs_rru_addr_,
      //                               bs_server_addr_, src_port, dest_port);
    }

    RxWorkerDpdk::~RxWorkerDpdk() { Stop(); };

    // Todo: remove this
    template <void (RxWorkerDpdk::*run_thread)()>
    static void ClassFunctioWrapper(RxWorkerDpdk *context)
    {
      return (context->*run_thread)();
    }

    // DPDK doesn't use c++ std threads but the class still has a 1:1 mapping
    // worker:lcore
    void RxWorkerDpdk::Start()
    {
      // Wait until an lcore is free
      rte_eal_wait_lcore(tid_);
      char buf[1024];
      sprintf(buf, "RxWorkerDpdk[%zu]: starting\n", tid_);
      LOG(log_level::info) << std::string(buf);
      // Launch a function on another lcore
      const int status = rte_eal_remote_launch(
          (lcore_function_t *)(ClassFunctioWrapper<&RxWorkerDpdk::DoTxRx>), this,
          tid_);
      sprintf(buf, "RxWorkerDpdk[%zu]: started on dpdk managed l_core\n", tid_);
      LOG(log_level::info) << std::string(buf);
      RtAssert(status == 0, "Lcore cannot launch TxRx function");
    }

    void RxWorkerDpdk::Stop()
    {
      // Configuration()->Running(false);
      //  Wait until the lcore finishes its job (join)
      running_ = false;
      rte_eal_wait_lcore(tid_);
    }

    void RxWorkerDpdk::DoTxRx()
    {
      // This is originally used by Agora to synchronize between different workers to start at the same time
      started_ = true;
      size_t prev_frame_id = SIZE_MAX;
      size_t rx_index = 0;
      const unsigned int thread_socket = rte_socket_id();

      char buf[1024];
      sprintf(buf, "RxWorkerDpdk[%zu]: running on socket %u\n", tid_,
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
      //       "RxWorkerDpdk[%zu]: running on socket %u but the ethernet device "
      //       "is on socket %u\n",
      //       tid_, thread_socket, dev_socket);
      // }
      dev_id = current_dev_id;
      //}
      running_ = true;

      while (running_)
      {
        // const auto &port_queue_id = dpdk_phy_port_queues_.at(rx_index);
        // const auto &port_queue_id = 0;
        // FIXME
        auto rx_result = RecvEnqueue(0, 0);
        for (auto &rx_packet : rx_result)
        {
          bool ret = response_q_->enqueue(*rx_packet);
          //LOG(log_level::info) << "Receiving this request: " << rx_packet->key;
          //// Could move this to the Recv function
          // if (kIsWorkerTimingEnabled)
          //{
          //   const size_t &rx_frame_id = rx_packet->frame_id_;
          //   if ((prev_frame_id == SIZE_MAX) || (rx_frame_id > prev_frame_id))
          //   {
          //     rx_frame_start_[rx_frame_id % kNumStatsFrames] = GetTime::Rdtsc();
          //     prev_frame_id = rx_frame_id;
          //   }
          // } // end kIsWorkerTimingEnabled
        }
        // Cycle through all ports / queues.  Don't wait for successful rx on any given config
        //rx_index++;
        //if (rx_index == dpdk_phy_port_queues_.size())
        //{
        //  rx_index = 0;
        //}
      }   // running
      running_ = false;
    }

    std::vector<Response*> RxWorkerDpdk::RecvEnqueue(uint16_t port_id,
                                                      uint16_t queue_id)
    {
      std::vector<Response*> rx_packets;
      std::array<rte_mbuf *, kRxBatchSize> rx_bufs;
      //LOG(log_level::info) << "Trying to receive packet from here";
      uint16_t nb_rx =
          rte_eth_rx_burst(port_id, queue_id, rx_bufs.data(), kRxBatchSize);
      //LOG(log_level::info) << "Reading from the port: " << port_id << " queue id: " << queue_id << " number of packets received: " << nb_rx;
      //LOG(log_level::info) << "Finish to receive packet from here";

      for (size_t i = 0; i < nb_rx; i++)
      {
        //LOG(log_level::info) << "Processing packet here";
        rte_mbuf *dpdk_pkt = rx_bufs.at(i);

        auto *eth_hdr = rte_pktmbuf_mtod(dpdk_pkt, rte_ether_hdr *);
        auto *ip_hdr = reinterpret_cast<rte_ipv4_hdr *>(
            reinterpret_cast<uint8_t *>(eth_hdr) + sizeof(rte_ether_hdr));

        const uint16_t eth_type = rte_be_to_cpu_16(eth_hdr->ether_type);
        /// \todo Add support / detection of fragmented packets

        if (kDebugDPDK)
        {
          auto *udp_h = reinterpret_cast<rte_udp_hdr *>(
              reinterpret_cast<uint8_t *>(ip_hdr) + sizeof(rte_ipv4_hdr));

          DpdkTransport::PrintPkt(ip_hdr->src_addr, ip_hdr->dst_addr,
                                  udp_h->src_port, udp_h->dst_port,
                                  dpdk_pkt->data_len, tid_);
          std::printf(
              "pkt_len: %d, datagram len %d, data len %d, nb_segs: %d, data "
              "offset %d, Header type: %d, IPv4: %d on dpdk dev %u and queue id "
              "%u\n",
              dpdk_pkt->pkt_len, rte_be_to_cpu_16(udp_h->dgram_len),
              dpdk_pkt->data_len, dpdk_pkt->nb_segs, dpdk_pkt->data_off,
              rte_be_to_cpu_16(eth_hdr->ether_type), RTE_ETHER_TYPE_IPV4, port_id,
              queue_id);
        }

        auto *payload = reinterpret_cast<uint8_t *>(eth_hdr) + kPayloadOffset;
        // FIXME
        //auto &rx = GetRxPacket();
        //Packet *pkt = rx.RawPacket();

//#if defined(USE_DPDK_MEMORY)
//        rx.Set(dpdk_pkt, reinterpret_cast<Packet *>(payload));
//#else
        // FIXME
        Response *pkt = new Response;
        rte_memcpy(reinterpret_cast<uint8_t *>(pkt), payload,
                   cfg_->packet_length);
        rte_pktmbuf_free(dpdk_pkt);
//#endif

        //char buf[1024];
        //sprintf(buf,
        //        "RxWorkerDpdk[%zu]::RecvEnqueue received pkt (frame %d, symbol "
        //        "%d, ant %d) on port %u queue %u\n",
        //        tid_, pkt->frame_id_, pkt->symbol_id_, pkt->ant_id_, port_id,
        //        queue_id);
        //LOG(log_level::info) << std::string(buf);

        // Push kPacketRX event into the queue.
        //LOG(log_level::info) << "Receiving this request: " << reinterpret_cast<Request*>(pkt)->ht << " at time: " << get_time();
        rx_packets.push_back(pkt);
      }
      return rx_packets;
    }


  }
}
