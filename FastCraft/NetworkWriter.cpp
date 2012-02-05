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
#include <Poco/Exception.h>

using Poco::Thread;

NetworkWriter::NetworkWriter(ThreadSafeQueue<string>& lowQ,ThreadSafeQueue<string>& highQ,Poco::Net::StreamSocket& s,PlayerThread* p) :
_rlowQ(lowQ),
	_rhighQ(highQ),
	_rStrm(s),
	_pPlayer(p),
	_fClear(false)
{
	_fRunning=false;
}

NetworkWriter::~NetworkWriter() {
	if (_fRunning) {shutdown();}
	if (!_rhighQ.empty()) {
		_rhighQ.clear();
	}
	if(!_rlowQ.empty()) {
		_rlowQ.clear();
	}
}

void NetworkWriter::run() {
	_fRunning=true;
	while (_fRunning) {
		if (_fClear) {
			_fClear=false;
			if (!_rhighQ.empty()) {
				_rhighQ.clear();
			}
			if(!_rlowQ.empty()) {
				_rlowQ.clear();
			}
		}

		if (!_pPlayer->isSpawned()) {
			Thread::sleep(50);
			continue;
		}

		try{
			//Process high level queue
			while (!_rhighQ.empty()) {
				string & rStr = _rhighQ.front();
				try {
					_rStrm.sendBytes(rStr.c_str(),rStr.length()); //Send
				}catch(Poco::Net::ConnectionAbortedException) {
					_rhighQ.pop();
					waitTillDisconnected();
					continue;
				}catch(Poco::Net::InvalidSocketException) {
					_rhighQ.pop();
					waitTillDisconnected();
					continue;
				}catch(Poco::TimeoutException) {
					_rhighQ.pop();
					waitTillDisconnected();
					continue;
				}catch(Poco::Net::ConnectionResetException) {
					_rhighQ.pop();
					waitTillDisconnected();
					continue;
				}catch(Poco::IOException) {
					_rhighQ.pop();
					waitTillDisconnected();
					continue;
				}

				_rhighQ.pop();
			}

			if (_rlowQ.empty()) {
				Thread::sleep(10);
				continue;
			}
			string & rStr = _rlowQ.front();
			try {
				_rStrm.sendBytes(rStr.c_str(),rStr.length()); //Send
			}catch(Poco::Net::ConnectionAbortedException) {
				_rlowQ.pop();
				waitTillDisconnected();
				continue;
			}catch(Poco::Net::InvalidSocketException) {
				_rlowQ.pop();
				waitTillDisconnected();
				continue;
			}catch(Poco::TimeoutException) {
				_rlowQ.pop();
				waitTillDisconnected();
				continue;
			}catch(Poco::Net::ConnectionResetException) {
				_rlowQ.pop();
				waitTillDisconnected();
				continue;
			}catch(Poco::IOException) {
				_rlowQ.pop();
				waitTillDisconnected();
				continue;
			}


			_rlowQ.pop();
		}catch(Poco::RuntimeException& ex) {
			std::cout<<"NetworkWriter::run exception:"<<ex.message()<<"\n";
			waitTillDisconnected();
			continue; //Queue exception
		}
	}
	_fRunning=true;
}

void NetworkWriter::waitTillDisconnected() {
	while(_pPlayer->isSpawned()) {
		Thread::sleep(50);
	}
}

void NetworkWriter::clearQueues() {
	_fClear=true;
}

void NetworkWriter::shutdown() {
	if (!_fRunning) {return;}
	_fRunning=false;
	while(!_fRunning){ //Wait till _fRunning turns true
	}
	_fRunning=false;
}