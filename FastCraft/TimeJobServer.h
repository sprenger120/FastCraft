/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011 - 2012 Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_TIMEJOBSERVER
#define _FASTCRAFTHEADER_TIMEJOBSERVER
#include <vector>
#include <Poco/Stopwatch.h>

using std::vector;

template<typename ptrClass> 
class TimeJobServer {
	struct ListEntry {
		void (ptrClass::* funcPtr) (void);
		unsigned long long lastCall;
		int callInterval;
	};

	vector<ListEntry> _vEntries;
	Poco::Stopwatch _Timer;
	ptrClass* _pClass;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : This pointer of class
	*/
	TimeJobServer(ptrClass*);


	/*
	* Destructor
	*/
	~TimeJobServer();


	/*
	* Adds a new job
	* Throws FCRuntimeException if calling interval is below zero
	* Throws FCRuntimeException if function pointer is NULL

	Parameter:
	@1 : Function pointer of the target
	@2 : Call interval in milli seconds
	*/
	void addJob(void (ptrClass::*) (void), int);


	/*
	* Checks if a function has to be called
	*/
	void doJobs();
};


template<typename ptrClass>
TimeJobServer<ptrClass>::TimeJobServer(ptrClass* ptr) {
	_Timer.start();
	_pClass = ptr;
}


template<typename ptrClass>
TimeJobServer<ptrClass>::~TimeJobServer() {
}

template<typename ptrClass>
void TimeJobServer<ptrClass>::addJob(void (ptrClass::* pFuncPtr)(void),int iInterval) {
	if (pFuncPtr == NULL) { throw FCRuntimeException("Invalid function pointer"); }
	if (iInterval < 0) {throw FCRuntimeException("Illegal interval");}

	ListEntry entry;
	entry.callInterval = iInterval;
	entry.funcPtr = pFuncPtr;
	entry.lastCall = 0;
	_vEntries.push_back(entry);
}


template<typename ptrClass>
void TimeJobServer<ptrClass>::doJobs() {
	if (_vEntries.empty()) {return;}
	for (int x=0;x<=_vEntries.size()-1;x++) {
		if (_vEntries[x].lastCall + _vEntries[x].callInterval <  _Timer.elapsed()/1000) {
			(_pClass->*(_vEntries[x].funcPtr))();
			_vEntries[x].lastCall = _Timer.elapsed()/1000;
		}
	}
}
#endif
