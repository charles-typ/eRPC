#ifndef PC_LIBPC_SRC_PC_WORKER_BTREE_SCAN_WORKER_H_
#define PC_LIBPC_SRC_PC_WORKER_BTREE_SCAN_WORKER_H_

#include "rpc_worker.h"

namespace pc
{
    namespace worker
    {
        class BtreescanWorker : public RpcWorker
        {
        public:
            BtreescanWorker(Config *config, unsigned int worker_id, moodycamel::ConcurrentQueue<Request> *request_q,
                            moodycamel::ConcurrentQueue<Response> *response_q) : RpcWorker(config, worker_id, request_q, response_q) {}
            void run_function(Request *req) final;
        };

    }
}

#endif // PC_LIBPC_SRC_PC_WORKER_BTREE_SCAN_WORKER_H_