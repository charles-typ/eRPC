#include "packet_txrx_dpdk.h"

namespace pc
{
    namespace network
    {
        PacketTxRxDpdk::PacketTxRxDpdk(Config *const cfg, size_t core_offset,
                                       moodycamel::ConcurrentQueue<Request> *request_q,
                                       moodycamel::ConcurrentQueue<Response> *response_q)
        {
            request_q_ = request_q;
            response_q_ = response_q;
            cfg_ = cfg;
            // TODO for now only one network thread is supported
            const size_t worker_threads = 1;
            const size_t num_dpdk_eth_dev = 1;
            LOG(log_level::info) << "Initialize DPDK lcores with " << cfg_->num_network_threads << " network threads and " << cfg_->num_worker_threads << " worker threads";
            DpdkTransport::DpdkInit(0, cfg_->num_network_threads + cfg_->num_worker_threads);
            // Only one port for now
            mbuf_pool_ = DpdkTransport::CreateMempool(1);
            // Each dpdk dev (port_id) will have a tx/rx queue for each local port/interface that will be assigned to the port
            worker_dev_queue_assignment_.resize(1); // Only one network worker
            // Init on port 0 and with 1 thread (1 RX ring, 1 TX ring)
            const int init_status = DpdkTransport::NicInit(0, mbuf_pool_, 1);
            if (init_status != 0)
            {
                rte_exit(EXIT_FAILURE, "Cannot init nic with id %u\n", 0);
            }
        }

        PacketTxRxDpdk::~PacketTxRxDpdk()
        {
            StopTxRx();
            rte_flow_error flow_error;
            LOG(log_level::info) << "~PacketTxRxDpdk: dpdk eal cleanup";
            rte_mempool_free(mbuf_pool_);

            //uint16_t eth_port = UINT16_MAX;
            uint16_t eth_port = 0;
            auto ret_status = rte_flow_flush(eth_port, &flow_error);
            if (ret_status != 0)
            {
                LOG(log_level::info) << "Flow cannot be flushed " << flow_error.type << " message: " << flow_error.message;
            }

            ret_status = rte_eth_dev_stop(eth_port);
            if (ret_status < 0)
            {
                LOG(log_level::info) << "Failed to stop port " << eth_port << " " << rte_strerror(-ret_status);
            }
            ret_status = rte_eth_dev_close(eth_port);
            if (ret_status < 0)
            {
                LOG(log_level::info) << "Failed to close device " << eth_port << " " << rte_strerror(-ret_status);
            }
            LOG(log_level::info) << "PacketTxRxDpdk::Shutdown down dev port " << eth_port;
            rte_delay_ms(100);
            rte_eal_cleanup();
        }

        bool PacketTxRxDpdk::StartTxRx()
        {
            CreateWorker(0, WorkerType::RX_WORKER);
            CreateWorker(1, WorkerType::TX_WORKER);
            LOG(log_level::info) << "PacketTxRx: StartTxRx threads " << worker_threads_.size();
            for (auto &worker : worker_threads_)
            {
                worker->Start();
                size_t waited_ms = 0;
                while (worker->Started() == false)
                {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(kWorkerStartWaitMs));
                    waited_ms += kWorkerStartWaitMs;
                    if (waited_ms >= kWorkerStartWaitMsMax)
                    {
                        throw std::runtime_error(
                            "TxRx worker did not start in reasonable time");
                    }
                }
                if(worker->type_ == WorkerType::RX_WORKER) {
                    LOG(log_level::info) << "PacketTxRx: RX worker " << worker->Id() << " has started";
                } else {
                    LOG(log_level::info) << "PacketTxRx: TX worker " << worker->Id() << " has started";
                }
            }
            return true;
        }

        bool PacketTxRxDpdk::CreateWorker(size_t tid, WorkerType type)
        {
            //  lcore refers to a logical execution unit of the processor, sometimes called a hardware thread
            unsigned int thread_l_core = tid;
            for (size_t lcore_idx = 0; lcore_idx <= tid; lcore_idx++)
            {
                // 1 to skip main core, 0 to disable wrap
                thread_l_core = rte_get_next_lcore(thread_l_core, 1, 0);
            }

            // Verify the lcore id is enabled (should have be inited with proper id)
            const int enabled = rte_lcore_is_enabled(thread_l_core);
            if (enabled == false)
            {
                throw std::runtime_error("The lcore " + std::to_string(thread_l_core) +
                                         " tid passed to CreateWorker is not enabled");
            }

            // launch communication and task thread onto specific core
            if (type == WorkerType::RX_WORKER) {
                worker_threads_.emplace_back(std::make_unique<RxWorkerDpdk>(cfg_,
                                                                          0, thread_l_core, request_q_, response_q_, mbuf_pool_, type));

            } else {
                worker_threads_.emplace_back(std::make_unique<TxWorkerDpdk>(cfg_,
                                                                          0, thread_l_core, request_q_, response_q_, mbuf_pool_, type));
            }
            LOG(log_level::info) << "PacketTxRxDpdk: worker " << tid << " assigned to lcore " << thread_l_core;

            return true;
        }

        bool PacketTxRxDpdk::StopTxRx()
        {
            // cfg_->Running(false);
            for (auto &worker_threads : worker_threads_)
            {
                worker_threads->Stop();
            }
            return true;
        }
    }
}
