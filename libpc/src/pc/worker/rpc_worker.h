//
// Created by yupeng on 1/18/23.
//

#ifndef PC_LIBPC_SRC_PC_WORKER_RPC_WORKER_H_
#define PC_LIBPC_SRC_PC_WORKER_RPC_WORKER_H_

#include "concurrentqueue.h"
#include "pc/config.h"
#include "pc/datastructure/hashtable.h"

namespace pc
{
  namespace worker
  {
    class RpcWorker
    {
    private:
      unsigned int worker_id_;
      bool stop_;
      std::thread thread_;
      Config *config_;

    public:
      moodycamel::ConcurrentQueue<Request> *request_q_;
      moodycamel::ConcurrentQueue<Response> *response_q_;
      RpcWorker(Config *config, unsigned int worker_id, moodycamel::ConcurrentQueue<Request> *request_q,
                moodycamel::ConcurrentQueue<Response> *response_q);
      ~RpcWorker() {}
      void run();
      void stop();
      virtual void run_function(Request *req) = 0;
    };
  }
}

#endif // PC_LIBPC_SRC_PC_WORKER_RPC_WORKER_H_
