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

//#include <Poco/Mutex.h>
#include <queue>
#include <Poco/Exception.h>
#include <iostream>
#include <vector>

using Poco::RuntimeException;
using std::queue;
using std::vector;

template <typename T>
class ThreadSafeQueue {
public:
	void push(T&);
	void multiPush(vector<T>);
	void pop();
	void clear();
	T& front();

	int size();
	bool empty();

	ThreadSafeQueue(void);
	~ThreadSafeQueue();
private:
	bool _fLocked;
	//Poco::Mutex _Mutex;
	queue<T> _q;
};

template <typename T>
inline ThreadSafeQueue<T>::ThreadSafeQueue(void) :
_q()
{
	_fLocked=false;
}

template <typename T>
inline ThreadSafeQueue<T>::~ThreadSafeQueue() {
	std::cout<<"destroying queue container"<<"\n";
	clear();
}


template <typename T>
inline void ThreadSafeQueue<T>::multiPush(vector<T> v) {
	//_Mutex.lock();
	while(_fLocked) {
	}
	_fLocked = true;
	if (v.size()>0) {
		for (int x=0;x<=v.size()-1;x++) {
			_q.push(v[x]);
		}
	}
	_fLocked=false;
	//_Mutex.unlock();
}

template <typename T> 
inline void ThreadSafeQueue<T>::push(T& t) {
	//_Mutex.lock();
	while(_fLocked) {
	}
	_fLocked = true;
	_q.push(t);
	_fLocked=false;
	//_Mutex.unlock();
}


template <typename T> 
inline void ThreadSafeQueue<T>::pop() {
	//_Mutex.lock();
	if (_q.empty()) { 
		std::cout<<"ThreadSafeQueue<T>::pop queue is empty"<<"\n";
		throw Poco::RuntimeException("Queue is empty");
	}
	while(_fLocked) {
	}
	_fLocked = true;
	_q.pop();
	_fLocked=false;
	//_Mutex.unlock();
}

template <typename T> 
inline void ThreadSafeQueue<T>::clear() {
	//_Mutex.lock();
		while(_fLocked) {
	}
	_fLocked = true;
	while (!_q.empty()) {
		_q.pop();
	}
	_fLocked=false;
	//_Mutex.unlock();
}

template <typename T> 
inline T& ThreadSafeQueue<T>::front() {
	if (_q.empty()) { 
		std::cout<<"ThreadSafeQueue<T>::front queue is empty"<<"\n";
		throw Poco::RuntimeException("Queue is empty");
	}
	return _q.front();
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