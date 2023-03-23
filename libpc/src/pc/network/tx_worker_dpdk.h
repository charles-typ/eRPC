#ifndef PC_LIBPC_SRC_PC_NETWORK_TX_WORKER_DPDK_H_
#define PC_LIBPC_SRC_PC_NETWORK_TX_WORKER_DPDK_H_
#include <cstddef>
#include <cstdint>
#include <vector>

#include "dpdk_transport.h"
#include "txrx_worker.h"
#include "pc/config.h"

namespace pc
{
  namespace network
  {

    class TxWorkerDpdk : public TxRxWorker
    {
    public:
      TxWorkerDpdk() = delete;
      TxWorkerDpdk(Config* cfg, size_t core_offset, size_t tid,
                     moodycamel::ConcurrentQueue<Request> *request_q,
                     moodycamel::ConcurrentQueue<Response> *response_q,
                     rte_mempool *mbuf_pool,
                     WorkerType type);
      ~TxWorkerDpdk() final;
      void DoTxRx() final;
      void Start() final;
      void Stop() final;

    private:
      size_t DequeueSend();
      uint32_t src_addr_;    // IPv4 address of the client
      uint32_t dst_addr_; // IPv4 address of the server
      // dpdk port_id / device : queue_id
      const std::vector<std::pair<uint16_t, uint16_t>> dpdk_phy_port_queues_;
      // Shared memory pool for rx and tx
      rte_mempool *mbuf_pool_;
      rte_ether_addr src_mac_;
      rte_ether_addr dest_mac_;
    };

  }
}
#endif // PC_LIBPC_SRC_PC_NETWORK_TX_WORKER_DPDK_H_