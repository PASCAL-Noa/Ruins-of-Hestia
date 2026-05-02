#pragma once
#include <condition_variable>
#include <mutex>
#include <thread>

namespace GPC {

    class WorkerThread {
        std::jthread m_Thread;
        std::condition_variable m_ConditionVariableStart;
        std::condition_variable m_ConditionVariableWait;
        std::mutex m_Mutex;
        void* mp_DataUnsafe;
        uint64_t m_DataSizeUnsafe;
        bool m_HasJob;
        bool m_JobDone;

        void WorkerProc(std::stop_token st);

    public:

        WorkerThread();
        virtual ~WorkerThread();

        void SubmitData(void* pData, uint64_t data_size);
        void Start();
        void Wait();
        void Stop();

        virtual void Callback(void* pData, uint64_t data_size) = 0;
    };

    class WorkerThreadTest : public WorkerThread {
    public:
        WorkerThreadTest() = default;
        ~WorkerThreadTest() override = default;
        void Callback(void* pData, uint64_t data_size) override;
    };
}
