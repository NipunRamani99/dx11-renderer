#pragma once
#include<vector>
#include<functional>
#include<map>
#include <chrono>
#include <thread>
#include <deque>
#include <iostream>
#include <mutex>

class TaskManager
{
public:
	struct TimedTask
	{
		std::time_t _startTime;
		std::function<void(void)> _task;
		bool done = false;
		TimedTask(std::function<void(void)>&& task, std::time_t startTime)
			:
			_startTime(startTime),
			_task(std::move(task))
		{

		}

		void Run()
		{
			if (std::time(nullptr) > _startTime && !done)
			{
				if (_task)
				_task();
				done = true;
			}
		}

		TimedTask()
		{
			_startTime = -1;
			_task = []() {};
		}
	};

	struct Task
	{
		std::function<void(void)> _task;
		bool done = false;
		Task(std::function<void(void)>&& task)
			:
			_task(std::move(task))
		{

		}

		void Run()
		{

			if (!done)
			{
				_task();
				done = true;
			}
		}

		Task()
		{
			done = true;
			_task = []() {};
		}
	};

private:
	std::vector<TimedTask> _timedTasks;
	std::deque<Task> _tasks;
	bool keepRunning = true;
	std::thread _th;
	std::mutex _mtx;
	TaskManager()
		:
		_th(&TaskManager::Run, this)
	{
	}

	void Run()
	{
		while (keepRunning)
		{
			std::lock_guard<std::mutex> guard(_mtx);
			for (auto& task : _timedTasks)
			{
				task.Run();
			}
			while (!_tasks.empty())
			{
				auto & task = _tasks.front();
				task.Run();
				_tasks.pop_front();
			}
			_timedTasks.erase(std::remove_if(_timedTasks.begin(), _timedTasks.end(), [](TimedTask& task) { return task.done; }));
			_timedTasks.shrink_to_fit();
		}
	}

	~TaskManager()
	{
		std::cout << "Destructor for task manager called.\n";
	}

public:
	static TaskManager& Get()
	{
		static TaskManager manager;
		return manager;
	}

	void AddTask(std::function<void(void)>&& task)
	{
		std::lock_guard<std::mutex> guard(_mtx);
		_tasks.push_back({ std::move(task) });
	}

	void AddTimedTask(std::function<void(void)>&& timedTask, std::chrono::milliseconds executeAfter)
	{
		std::lock_guard<std::mutex> guard(_mtx);
		std::time_t time_t_value = std::chrono::duration_cast<std::chrono::seconds>(executeAfter).count();
		_timedTasks.push_back({ std::move(timedTask), std::time(nullptr) + time_t_value });
	}

	void Stop()
	{
		keepRunning = false;
	}

	void Wait()
	{
		_th.join();
	}
};
