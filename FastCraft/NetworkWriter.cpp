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
#include "NetworkWriter.h"
#include "PlayerThread.h"
#include "Constants.h"
#include <Poco/Thread.h>
#include <Poco/Net/NetException.h>
#include "FCRuntimeException.h"

using Poco::Thread;

NetworkWriter::NetworkWriter(ThreadSafeQueue<string*>& lowQ,ThreadSafeQueue<string*>& highQ,Poco::Net::StreamSocket& s,PlayerThread* p) :
_rLowQueue(lowQ),
	_rHighQueue(highQ),
	_rStrm(s),
	_pPlayer(p),
	ServerThreadBase("NetworkWriter")
{
	_fClear = false;
	startThread(this);
}

NetworkWriter::~NetworkWriter() {
	killThread();
	if (!_rLowQueue.empty()) {_rLowQueue.clear();}
	if (!_rHighQueue.empty()) {_rHighQueue.clear();}
}

void NetworkWriter::run() {
	_iThreadStatus = FC_THREADSTATUS_RUNNING;
	while (_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		if (_fClear) {
			_fClear = false;

			string* pStr = NULL;

			while(!_rLowQueue.empty()) {
				pStr = _rLowQueue.front();
				delete pStr;
				_rLowQueue.pop();
			}

			while(!_rHighQueue.empty()) {
				pStr = _rHighQueue.front();
				delete pStr;
				_rHighQueue.pop();
			}
		}

		if (!_pPlayer->isSpawned()) {
			Thread::sleep(50);
			continue;
		}

		try {
			string* pStr = NULL;

			/* High priority packets */
			while (!_rHighQueue.empty()) {
				pStr = _rHighQueue.front();
				
				_rStrm.sendBytes(pStr->c_str(),pStr->length()); 
				
				_rHighQueue.pop();
				delete pStr;
			}


			/* Low priority packets */
			if (_rLowQueue.empty()) {
				Thread::sleep(10);
				continue;
			}
			pStr = _rLowQueue.front();
			_rStrm.sendBytes(pStr->c_str(),pStr->length()); 
			
			_rLowQueue.pop();
			delete pStr;
		}catch(...) {
			waitTillDisconnected();
			continue;
		}		
	}
	_iThreadStatus = FC_THREADSTATUS_DEAD;
}

void NetworkWriter::waitTillDisconnected() {
	while(_pPlayer->isSpawned()) {
		Thread::sleep(50);
	}
}

void NetworkWriter::clearQueues() {
	_fClear = true;
}
