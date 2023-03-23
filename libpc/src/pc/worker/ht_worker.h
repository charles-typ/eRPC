#ifndef PC_LIBPC_SRC_PC_WORKER_HT_WORKER_H_
#define PC_LIBPC_SRC_PC_WORKER_HT_WORKER_H_

#include "rpc_worker.h"

namespace pc
{
    namespace worker
    {
        class HashtableWorker : public RpcWorker
        {
        private:
            int counter;
        public:
            HashtableWorker(Config *config, unsigned int worker_id, moodycamel::ConcurrentQueue<Request> *request_q,
                            moodycamel::ConcurrentQueue<Response> *response_q) : RpcWorker(config, worker_id, request_q, response_q) {
                                counter = 0;
                            }
            void run_function(Request *req) final;
        };

    }

} // namespace pc

#endif // PC_LIBPC_SRC_PC_WORKER_HT_WORKER_H_