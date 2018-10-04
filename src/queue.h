#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

// ************************************************************************************
// Multithreading queue with registering mechanism:
//   * The queue can report whether it is in wainitng for new data state or there will be no new data
// *******************************************************************************************
template<typename T> class CRegisteringPriorityQueue
{
	typedef priority_queue<pair<uint64_t, T>, vector<pair<uint64_t, T>>, greater<pair<uint64_t, T>>> priority_queue_t;

	priority_queue_t q;
	bool is_completed;
	int n_producers;
	uint32_t n_elements;
	uint64_t c_priority;

	mutable mutex mtx;								// The mutex to synchronise on
	condition_variable cv_queue_empty;

	bool is_empty()
	{
		return n_elements == 0 || q.top().first != c_priority;
	}

public:
	CRegisteringPriorityQueue(int _n_producers)
	{
		Restart(_n_producers);
	};

	~CRegisteringPriorityQueue()
	{};

	void Restart(int _n_producers)
	{
		unique_lock<mutex> lck(mtx);

		is_completed = false;
		n_producers = _n_producers;
		n_elements = 0;
		c_priority = 0;
	}

	bool IsEmpty()
	{
		lock_guard<mutex> lck(mtx);
		return is_empty();
	}

	bool IsCompleted()
	{
		lock_guard<mutex> lck(mtx);

		return n_elements == 0 && n_producers == 0;
	}

	void MarkCompleted()
	{
		lock_guard<mutex> lck(mtx);
		n_producers--;

		if (!n_producers)
			cv_queue_empty.notify_all();
	}

	void Push(uint64_t priority, T data)
	{
		unique_lock<mutex> lck(mtx);
		bool was_empty = is_empty();
		q.push(make_pair(priority, data));
		++n_elements;

		if (was_empty)
			cv_queue_empty.notify_all();
	}
	bool Pop(uint64_t &priority, T &data)
	{
		unique_lock<mutex> lck(mtx);
		cv_queue_empty.wait(lck, [this] {return !is_empty() || !this->n_producers; });

		if (n_elements == 0)
			return false;

		priority = q.top().first;
		data = q.top().second;
		q.pop();
		--n_elements;
		++c_priority;
		if (n_elements == 0)
			cv_queue_empty.notify_all();

		return true;
	}

	uint32_t GetSize()
	{
		return n_elements;
	}
};

// EOF
