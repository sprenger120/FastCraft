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

#include "NetworkWriter.h"
#include "PlayerThread.h"
#include "Constants.h"
#include <Poco/Thread.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h>
#include <Poco/Stopwatch.h>

using Poco::Thread;

NetworkWriter::NetworkWriter(ThreadSafeQueue<string>& lowQ,ThreadSafeQueue<string>& highQ,Poco::Net::StreamSocket& s,PlayerThread* p) :
_rlowQ(lowQ),
	_rhighQ(highQ),
	_rStrm(s),
	_pPlayer(p)
{
}

NetworkWriter::~NetworkWriter() {
}

void NetworkWriter::run() {
	while (1) {
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
					waitTillDisconnected();
					continue;
				}catch(Poco::Net::InvalidSocketException) {
					waitTillDisconnected();
					continue;
				}catch(Poco::TimeoutException) {
					waitTillDisconnected();
					continue;
				}catch(Poco::Net::ConnectionResetException) {
					waitTillDisconnected();
					continue;
				}catch(Poco::IOException) {
					waitTillDisconnected();
					continue;
				}catch(Poco::RuntimeException& err) {
					std::cout<<err.message()<<"\n";
					waitTillDisconnected();
					continue;
				}

				_rhighQ.pop();
			}


			if (_rlowQ.empty()) {
				Thread::sleep(5);
				continue;
			}

			string & rStr = _rlowQ.front();

			try {
				_rStrm.sendBytes(rStr.c_str(),rStr.length()); //Send
			}catch(Poco::Net::ConnectionAbortedException) {
				waitTillDisconnected();
				continue;
			}catch(Poco::Net::InvalidSocketException) {
				waitTillDisconnected();
				continue;
			}catch(Poco::TimeoutException) {
				waitTillDisconnected();
				continue;
			}catch(Poco::Net::ConnectionResetException) {
				waitTillDisconnected();
				continue;
			}catch(Poco::IOException) {
				waitTillDisconnected();
				continue;
			}


			_rlowQ.pop();

		}catch(Poco::RuntimeException) {
			continue; //Queue exception
		}
	}
}

void NetworkWriter::waitTillDisconnected() {
	while(_pPlayer->isSpawned()) {
		Thread::sleep(50);
	}
}