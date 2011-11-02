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

PlayerThread::PlayerThread() : 
_sName(""),
	_sNickName(""),
	_sIP(""),
	_Connection(),
	_fReady(false)
{
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;

	_fLoggedIn = false;
	_fClear = true;

	InstanceCounter++;
	_iThreadID = InstanceCounter;
}

int PlayerThread::InstanceCounter = 0;

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

	while (1) {
		Thread::sleep(100);
	}
}

void PlayerThread::Disconnect() {
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;

	_fLoggedIn = false;
	_fClear = true;

	_sName.assign("");
	_sNickName.assign("");
	_sIP.assign("");
	ClearQueue();
}


void PlayerThread::Connect(Poco::Net::SocketStream& Sock,string IP) {
	cout<<"New Connection :) "<<IP<<endl;

}