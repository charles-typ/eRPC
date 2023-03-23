#ifndef PC_LIBPC_SRC_PC_NETWORK_PACKET_TXRX_DPDK_H_
#define PC_LIBPC_SRC_PC_NETWORK_PACKET_TXRX_DPDK_H_

#include <vector>
#include "dpdk_transport.h"
#include "pc/config.h"
#include "concurrentqueue.h"
#include "pc/utils/logger.h"
#include "txrx_worker.h"
#include "txrx_worker_dpdk.h"
#include "tx_worker_dpdk.h"
#include "rx_worker_dpdk.h"

using namespace pc::utils;

namespace pc
{
    namespace network
    {
        static constexpr size_t kNotifyWaitMs = 100;
        static constexpr size_t kWorkerStartWaitMs = 10;
        static constexpr size_t kWorkerStartWaitMsMax = 5000;
        class PacketTxRxDpdk
        {
        public:
            PacketTxRxDpdk(Config *const cfg, size_t core_offset,
                           moodycamel::ConcurrentQueue<Request> *request_q,
                           moodycamel::ConcurrentQueue<Response> *response_q);
            ~PacketTxRxDpdk();
            bool StartTxRx();
            bool StopTxRx();

        private:
            void DoTxRx(size_t tid);

            bool CreateWorker(size_t tid, WorkerType type);

            uint32_t src_ip_addr_;
            uint32_t dst_ip_addr_;
            rte_mempool *mbuf_pool_;

        protected:
            std::vector<std::unique_ptr<TxRxWorker>> worker_threads_;
            std::vector<std::unique_ptr<TxRxWorker>> rx_worker_threads_;
            std::vector<std::unique_ptr<TxRxWorker>> tx_worker_threads_;
            Config *cfg_;
            moodycamel::ConcurrentQueue<Request> *request_q_;
            moodycamel::ConcurrentQueue<Response> *response_q_;
            // Worker x (dpdk dev: queueid)
            std::vector<std::vector<std::pair<uint16_t, uint16_t>>> worker_dev_queue_assignment_;
        };

    }
}

#endif // PC_LIBPC_SRC_PC_NETWORK_PACKET_TXRX_DPDK_H_