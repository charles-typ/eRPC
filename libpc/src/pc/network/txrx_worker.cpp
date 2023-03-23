//
// Created by yupeng on 1/19/23.
//

/**
 * @file txrx_worker.cc
 * @brief Implementation of PacketTxRx initialization functions, and datapath
 * functions for communicating with simulators.
 */

#include "txrx_worker.h"
#include "pc/utils/logger.h"

using namespace pc::utils;


namespace pc
{
  namespace network
  {

    TxRxWorker::TxRxWorker(Config *cfg,size_t core_offset, size_t tid,
                           moodycamel::ConcurrentQueue<Request> *request_q,
                           moodycamel::ConcurrentQueue<Response> *response_q,
                           WorkerType type)
        : tid_(tid),
          cfg_(cfg),
          running_(false),
          request_q_(request_q),
          response_q_(response_q),
          type_(type),
          started_(false) {}

    TxRxWorker::~TxRxWorker() { Stop(); }

    void TxRxWorker::Start()
    {
      LOG(log_level::info) << "TxRxWorker[ " << tid_ << "] starting";
      if (!thread_.joinable())
      {
        thread_ = std::thread(&TxRxWorker::DoTxRx, this);
      }
      else
      {
        throw std::runtime_error(
            "TxRxWorker::Start() called with thread already assigned.  Ensure you "
            "have called Stop() before calling Start() a second time.");
      }
    }

    void TxRxWorker::Stop()
    {
      running_ = false;
      if (thread_.joinable())
      {
        LOG(log_level::info) << "TxRxWorker[ " << tid_ << "] stopping";
        thread_.join();
      }
    }

    /////Using a latch might be better but adds c++20 requirement
    // void TxRxWorker::WaitSync() {
    //   // Use mutex to sychronize data receiving across threads
    //   {
    //     std::unique_lock<std::mutex> locker(mutex_);
    //     AGORA_LOG_TRACE("TxRxWorker[%zu]: waiting for sync\n", tid_);
    //     started_ = true;
    //     cond_.wait(locker, [this] { return can_proceed_.load(); });
    //   }
    //   AGORA_LOG_INFO("TxRxWorker[%zu]: synchronized\n", tid_);
    // }

    bool TxRxWorker::NotifyComplete(const Request &req)
    {
      auto enqueue_status =
          request_q_->enqueue(req);
      if (enqueue_status == false)
      {
        LOG(log_level::info) << "TxRxWorker[" << tid_ << "]: socket message enqueue failed";
        throw std::runtime_error("TxRxWorker: socket message enqueue failed");
      }
      return enqueue_status;
    }

    std::vector<Request> TxRxWorker::GetPendingTxRequests(size_t max_dequeue)
    {
      std::vector<Request> tx_requests(max_dequeue);
      // Single producer ordering in q is preserved
      //LOG(log_level::info) << "Try to dequeue a tx event from the request_q";
      auto dequeue_items = request_q_->try_dequeue_bulk(tx_requests.data(), max_dequeue);
      //if(dequeue_items != 0) {
      //  LOG(log_level::info) << "Finish dequeue, status is: " << dequeue_items;
      //}
      // FIXME this may be empty
      tx_requests.resize(dequeue_items);
      return tx_requests;
    }

  }
}