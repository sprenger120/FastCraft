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
using namespace CryptoPP;

NetworkWriter::NetworkWriter(ThreadSafeQueue<string*>& lowQ,ThreadSafeQueue<string*>& highQ,Poco::Net::StreamSocket& s,PlayerThread* p) :
	_rLowQueue(lowQ),
	_rHighQueue(highQ),
	_rStrm(s),
	_pPlayer(p),
	ServerThreadBase("NetworkWriter")
{
	_fClear = false;
	startThread(this);

	_fCryptMode = false;
	_aesEncryptor = NULL;
}

NetworkWriter::~NetworkWriter() {
	killThread();
	if (!_rLowQueue.empty()) {_rLowQueue.clear();}
	if (!_rHighQueue.empty()) {_rHighQueue.clear();}
	if (_aesEncryptor != NULL) {delete _aesEncryptor;}
}

void NetworkWriter::run() {
	_iThreadStatus = FC_THREADSTATUS_RUNNING;
	while (_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		if (_fClear) {
			_fClear = false;

			clearQueue(_rLowQueue);
			clearQueue(_rHighQueue);
		}

		if (!_pPlayer->isSpawned()) {
			Thread::sleep(50);
			continue;
		}

		try {
			/* High priority packets */
			while (!_rHighQueue.empty()) {
				ProcessQueueElement(_rHighQueue);
			}

			/* Low priority packets */
			if (_rLowQueue.empty()) {
				Thread::sleep(10);
				continue;
			}
			ProcessQueueElement(_rLowQueue);
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


void NetworkWriter::setCryptMode(bool fMode) {
	_fCryptMode = fMode;
	if(fMode) {
		if (_aesEncryptor != NULL) {delete _aesEncryptor;}
		pair<char*,short>& rKey = _pPlayer->getSecretKey();
		memcpy(_IV,rKey.first,rKey.second);
		_aesEncryptor = new CryptoPP::CFB_Mode<AES>::Encryption((byte*)rKey.first,(unsigned int)rKey.second,_IV,1);
	}
}


void NetworkWriter::ProcessQueueElement(ThreadSafeQueue<string*>& rQueue) {
	try {
		if (rQueue.empty()) {return;}
		string* pStr = rQueue.front();

		if (_fCryptMode) {
			string Crypted("");

			StringSource(*pStr, true, 
				new StreamTransformationFilter( *_aesEncryptor,
				new StringSink(Crypted),BlockPaddingSchemeDef::NO_PADDING
				)   
				); 


			_rStrm.sendBytes(Crypted.c_str(),Crypted.length()); 
		}else{
			_rStrm.sendBytes(pStr->c_str(),pStr->length()); 
		}

		delete pStr;
		rQueue.pop();
	}catch(...) {
		throw FCRuntimeException("Queue processing error");
	}
}

void NetworkWriter::ProcessHighQueue() {
	try {
		ProcessQueueElement(_rHighQueue);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

void NetworkWriter::clearQueue(ThreadSafeQueue<string*>& rQueue) {
	string* pStr;
	while(!rQueue.empty()) {
		pStr = rQueue.front();
		delete pStr;
		rQueue.pop();
	}
}