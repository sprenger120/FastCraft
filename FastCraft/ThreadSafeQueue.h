/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_THREADSAVEQUEUE
#define _FASTCRAFTHEADER_THREADSAVEQUEUE

#include <Poco/Mutex.h>
#include <queue>
#include <Poco/Exception.h>
#include <iostream>

using Poco::RuntimeException;
using std::queue;


template <typename T>
class ThreadSafeQueue {
public:
	void push(T&);
	void pop();
	void clear();
	T& front();

	int size();
	bool empty();

	ThreadSafeQueue(void);
	~ThreadSafeQueue();
private:
	Poco::Mutex _Mutex;
	queue<T> _q;

	T* _pfront;
};

template <typename T>
inline ThreadSafeQueue<T>::ThreadSafeQueue(void) : 
_pfront(NULL)
{
}

template <typename T>
inline ThreadSafeQueue<T>::~ThreadSafeQueue() {
	clear();
}


template <typename T> 
inline void ThreadSafeQueue<T>::push(T& t) {
	_Mutex.lock();
	_q.push(t);
	if (_q.size()==1) {
		_pfront=&(_q.front());
	}
	_Mutex.unlock();
}


template <typename T> 
inline void ThreadSafeQueue<T>::pop() {
	_Mutex.lock();
	if (_q.empty()) { 
		std::cout<<"POP exception, queue is empty"<<"\n";
		throw Poco::RuntimeException("Queue is empty");
	}
	_q.pop();
	if (_q.size() > 0) {
		_pfront=&(_q.front());
	}else{
		_pfront=NULL;
	}
	_Mutex.unlock();
}

template <typename T> 
inline void ThreadSafeQueue<T>::clear() {
	_Mutex.lock();
	while (!_q.empty()) {
		_q.pop();
	}
	_Mutex.unlock();
}

template <typename T> 
inline T& ThreadSafeQueue<T>::front() {
	if (_q.empty()) { 
		std::cout<<"FRONT exception, queue is empty"<<"\n";
		throw Poco::RuntimeException("Queue is empty");
	}
	return (T&)(*_pfront);
}


template <typename T> 
inline int ThreadSafeQueue<T>::size() {
	return _q.size();
}

template <typename T> 
inline bool ThreadSafeQueue<T>::empty() {
	return _q.empty();
}
#endif