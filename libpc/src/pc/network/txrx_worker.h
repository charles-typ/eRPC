//
// Created by yupeng on 1/19/23.
//

#ifndef PC_LIBPC_SRC_PC_NETWORK_TXRX_WORKER_H_
#define PC_LIBPC_SRC_PC_NETWORK_TXRX_WORKER_H_

/**
 * @file txrx_worker.h
 * @brief txrx worker thread definition.  This is the parent / interface
 */

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "concurrentqueue.h"
#include "pc/utils/logger.h"
#include "pc/config.h"

namespace pc
{
  namespace network
  {

    enum WorkerType {
        RX_WORKER,
        TX_WORKER
    };

    class TxRxWorker
    {
    public:
      static constexpr bool kDebugTxMemory = false;
      WorkerType type_;

      TxRxWorker(Config* cfg, size_t core_offset, size_t tid,
                 moodycamel::ConcurrentQueue<Request> *request_q,
                 moodycamel::ConcurrentQueue<Response> *response_q,
                 WorkerType type);
      TxRxWorker() = delete;

      virtual ~TxRxWorker();

      virtual void Start();
      virtual void Stop();
      virtual void DoTxRx() = 0;

      inline size_t Id() const { return tid_; }
      inline bool Started() const { return started_; }
      inline bool Running() const { return running_; }
      
      moodycamel::ConcurrentQueue<Request> *request_q_;
      moodycamel::ConcurrentQueue<Response> *response_q_;

    protected:
      // void WaitSync();
      // inline Config* Configuration() { return cfg_; }
      bool NotifyComplete(const Request &req);
      // FIXME
      std::vector<Request> GetPendingTxRequests(size_t max_dequeue = 10);
      //RxPacket &GetRxPacket();
      //void ReturnRxPacket(RxPacket &unused_packet);
      //Packet *GetTxPacket(size_t frame, size_t symbol, size_t ant);
      //Packet *GetUlTxPacket(size_t frame, size_t symbol, size_t ant);

      const size_t tid_;
      bool running_;
      Config *cfg_;

      /// Owned by the parent TxRx object for sync
      // std::mutex& mutex_;
      // std::condition_variable& cond_;
      // std::atomic<bool>& can_proceed_;
      
      // This used to be private
      bool started_;

    private:
      std::thread thread_;


    };

  }
}

#endif // PC_LIBPC_SRC_PC_NETWORK_TXRX_WORKER_H_
