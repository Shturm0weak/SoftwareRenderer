#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <map>

namespace sr {

	using Task = std::function<void()>;

	struct SyncParams
	{
	public:
		std::mutex s_Mtx;
		std::atomic<bool>* s_Ready = nullptr;
		std::condition_variable s_CondVar;

		SyncParams();
		~SyncParams();

		void ThreadFinished(size_t index);
		void WaitForAllThreads();
	};

	class ThreadPool {
	private:

		static ThreadPool* s_Instance;
		std::vector<std::thread> m_Threads;
		std::map <std::thread::id, bool> m_IsThreadBusy;
		std::mutex m_Mutex;
		std::thread::id m_MainId = std::this_thread::get_id();
		std::condition_variable m_CondVar;
		std::queue <Task> m_Tasks;
		int m_NumThreads = 0;
		bool m_IsStoped = false;
		static bool m_IsInitialized;

		void InfiniteLoopFunction();

		ThreadPool& operator=(const ThreadPool& rhs) { return *this; }
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(int n);
		~ThreadPool();
	public:

		inline uint32_t GetAmountOfThreads() { return m_Threads.size(); }
		void Shutdown();
		void Enqueue(Task task);
		static ThreadPool& GetInstance() { return *s_Instance; }
		static void Init();
	};

}