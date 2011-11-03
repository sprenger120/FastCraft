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

#include "PlayerThread.h"
#include "TextHandler.h"
#include "Constants.h"
#include <Poco/Timespan.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Net/NetException.h>

PlayerThread::PlayerThread() : 
_sName(""),
	_sNickName(""),
	_sIP(""),
	_Connection(),
	_sOutputBuffer(""),
	_fReady(false),
	_fSettingsHandlerSet(false)
{
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;

	_fLoggedIn = false;
	_fAssigned = false;

	InstanceCounter++;
	_iThreadID = InstanceCounter;
}

int PlayerThread::InstanceCounter = 0;
int PlayerThread::PlayerCount = 0;

void PlayerThread::ClearQueue() {
	for (int x = 1;x<=_SendQueue.size();x++) {
		_SendQueue.pop();
	}
}

PlayerThread::~PlayerThread() {
	ClearQueue();
	InstanceCounter--;
}

bool PlayerThread::Ready() {
	return _fReady;
}

void PlayerThread::run() {
	_fReady=true;

	if (!_fSettingsHandlerSet) {
		while(!_fSettingsHandlerSet) { Thread::sleep(100); }
	}

	while (1) {
		if (!isAssigned()) {
			Thread::sleep(50);
			continue;
		} 

		if (ProcessQueue()) {continue;} //Queue Processor has closed connection

		try {
			_Connection.receiveBytes(_sBuffer,1);
		}catch(Poco::Net::InvalidSocketException) {
			Disconnect();
		}catch(Poco::TimeoutException) {
			if (_sName.compare("") == 0) { //Handshake isn't done
				cout<<"Player:"<<_sIP<<" timed out"<<"\n";
				continue;
			}
			cout<<"Player:"<<_sName<<" timed out"<<"\n";
			Disconnect();
		}


		switch ((unsigned char)_sBuffer[0]) {
			/*case 0x2: //Handshake
			PlayerCount++;
			break;
			*/
		case 0xFE: //Server List Ping
			_sOutputBuffer.assign("");

			_sOutputBuffer.assign(_pSettings->getServerDescription()); //Server Description
			_sOutputBuffer.append<int>(1,0xA7);

			Poco::NumberFormatter::append(_sOutputBuffer,PlayerCount); //Player count
			_sOutputBuffer.append<int>(1,0xA7);

			Poco::NumberFormatter::append(_sOutputBuffer,_pSettings->getMaxClients()); //player slots

	
			_sOutputBuffer.append("§");
			Kick(_sOutputBuffer);
			break;
		default: 
			cout<<"Unknown packet received! 0x"<<std::hex<<int(_sBuffer[0])<<endl;
			Disconnect();
			break;
		}

	}
}

void PlayerThread::Disconnect() {
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;

	if (_fLoggedIn) {
		PlayerCount--;
	}

	_fLoggedIn = false;
	_fAssigned = false;

	_sName.assign("");
	_sNickName.assign("");
	_sIP.assign("");
	ClearQueue();
	_Connection.close();

}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock,string IP) {
	_fAssigned=true;
	_sIP.assign(IP);

	_Connection = Sock; 
	_Connection.setReceiveTimeout( Poco::Timespan( 1000* 2000) );
}

bool PlayerThread::isAssigned() {
	return _fAssigned;
}

int PlayerThread::getThreadID() {
	return _iThreadID;
}

void PlayerThread::Kick(string sReason) {
	QueueJob qJob;

	qJob.Data.assign("");
	qJob.Data.append<int>(1,0xFF);
	TextHandler::packString16(qJob.Data,sReason);
	qJob.Special = FC_JOB_CLOSECONN;

	appendQueue(qJob);
}

void PlayerThread::Kick() {
	QueueJob qJob;

	qJob.Data.assign("");
	qJob.Data.append<int>(1,0xFF);
	qJob.Data.append<int>(2,0x0);
	qJob.Special = FC_JOB_CLOSECONN;

	appendQueue(qJob);
}

bool PlayerThread::ProcessQueue() {
	QueueJob qJob;
	if (_SendQueue.size() == 0) {return false;}

	qJob = _SendQueue.front();
	_SendQueue.pop();

	_Connection.sendBytes(qJob.Data.c_str(),qJob.Data.length());

	switch(qJob.Special) {
	case FC_JOB_CLOSECONN:
		Thread::sleep(200); 
		Disconnect();
		return true;
	default:
		cout<<"unknown job!"<<"\n";
		break;
	}

	return false;
}

void PlayerThread::appendQueue(QueueJob& Job) {
	_SendQueue.push(Job);
}

void PlayerThread::setSettingsHandler(SettingsHandler* settings) {
	_pSettings = settings;
	_fSettingsHandlerSet=true;
}