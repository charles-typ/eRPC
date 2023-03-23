//
// Created by yupeng on 1/18/23.
//

#include "rpc_worker.h"
#include "pc/utils/core_util.h"

namespace pc
{
  namespace worker
  {

    RpcWorker::RpcWorker(Config* config, unsigned int worker_id, moodycamel::ConcurrentQueue<Request> *request_q,
                         moodycamel::ConcurrentQueue<Response> *response_q) : thread_(&RpcWorker::run, this)
    {
      stop_ = false;
      worker_id_ = worker_id;
      request_q_ = request_q;
      response_q_ = response_q;
      config_ = config;
    }

    void RpcWorker::run()
    {
      PinToCore(worker_id_);
      LOG(log_level::info) << "Rpc Worker pin to core " << worker_id_;
      Response r;
      while (!stop_)
      {
        while (response_q_->try_dequeue(r))
        {
          run_function(reinterpret_cast<Request*>(&r));
        }
      }
    }

    void RpcWorker::stop()
    {
      stop_ = true;
      thread_.join();
      // FIXME this only works for one RPC worker
      config_->running = false;
    }

  }
}
