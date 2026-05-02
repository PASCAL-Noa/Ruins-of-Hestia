#include "WorkerThread.h"

#include "Debugger.h"

namespace GPC {

    void WorkerThread::WorkerProc(std::stop_token st) {

        std::unique_lock first_lock(m_Mutex);
        m_ConditionVariableStart.wait(first_lock, [&]{ return m_HasJob; });
        first_lock.unlock();
        while (!st.stop_requested()) {

            Callback(mp_DataUnsafe, m_DataSizeUnsafe);
            m_JobDone = true;
            m_HasJob = false;
            m_ConditionVariableWait.notify_one();

            std::unique_lock lock(m_Mutex);
            m_ConditionVariableStart.wait(lock, [&]{ return m_HasJob; });
        }
    }

    WorkerThread::WorkerThread()  :
        m_Thread([this](std::stop_token st){ this->WorkerProc(st); }),
        mp_DataUnsafe(nullptr),
        m_DataSizeUnsafe(0),
        m_HasJob(false),
        m_JobDone(true)
    { }

    WorkerThread::~WorkerThread() {
        Stop();
    }

    void WorkerThread::SubmitData(void *pData, uint64_t data_size) {
        mp_DataUnsafe = pData;
        m_DataSizeUnsafe = data_size;
    }

    void WorkerThread::Start() {
        m_HasJob = true;
        m_JobDone = false;
        m_ConditionVariableStart.notify_one();
    }

    void WorkerThread::Wait() {
        // if (m_HasJob == false) return;
        std::unique_lock lock(m_Mutex);
        m_ConditionVariableWait.wait(lock, [&]{ return m_JobDone; });
    }

    void WorkerThread::Stop() {
        m_Thread.request_stop();
        Start();
    }

    void WorkerThreadTest::Callback(void *pData, uint64_t data_size) {
        const char** txt = static_cast<const char**>(pData);
        for (uint64_t i = 0; i < data_size; ++i) {
            GPC_INFO << "Text Said : " << txt[i] << ENDL;
        }
    }
}
