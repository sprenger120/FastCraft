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

NetworkWriter::NetworkWriter(ThreadSafeQueue<string>& q,Poco::Net::StreamSocket& s,PlayerThread* p) :
_rQ(q),
_rStrm(s),
_pPlayer(p)
{
}

NetworkWriter::~NetworkWriter() {
}

void NetworkWriter::run() {
	//Poco::Stopwatch sw;
	//sw.start();
	while (1) {
		if (!_pPlayer->isSpawned()) {
			Thread::sleep(50);
			continue;
		}
		/*if (sw.elapsed()/1000 >= 500) {
			sw.stop();
			sw.reset();
			sw.start();
			std::cout<<"Queue Size of '"<<_pPlayer->getUsername()<<"' :"<<_rQ.size()<<"\n";
		}
		*/
		if (_rQ.empty()) {
			Thread::sleep(5);
			continue;
		}

		string & rJob = _rQ.front();

		try {
			_rStrm.sendBytes(rJob.c_str(),rJob.length()); //Send
		}catch(Poco::Net::ConnectionAbortedException) {
			_pPlayer->Disconnect(FC_LEAVE_OTHER);
		}catch(Poco::Net::InvalidSocketException) {
			_pPlayer->Disconnect(FC_LEAVE_OTHER);
		}catch(Poco::TimeoutException) {
			_pPlayer->Disconnect(FC_LEAVE_OTHER);
		}catch(Poco::Net::ConnectionResetException) {
			_pPlayer->Disconnect(FC_LEAVE_OTHER);
		}catch(Poco::IOException) {
			_pPlayer->Disconnect(FC_LEAVE_OTHER);
		}


		_rQ.pop();
	}
}