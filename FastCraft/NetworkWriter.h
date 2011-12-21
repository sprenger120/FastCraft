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
#ifndef _FASTCRAFTHEADER_NETWORKWRITER
#define _FASTCRAFTHEADER_NETWORKWRITER
#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <string>
#include "ThreadSafeQueue.h"


class PlayerThread;
using std::string;

class NetworkWriter : public Poco::Runnable { 
private:
	ThreadSafeQueue<string>& _rlowQ;
	ThreadSafeQueue<string>& _rhighQ;
	Poco::Net::StreamSocket& _rStrm;
	PlayerThread* _pPlayer;

	void waitTillDisconnected(); //Waits till fSpawned is false
public:
	NetworkWriter(ThreadSafeQueue<string>&,ThreadSafeQueue<string>&,Poco::Net::StreamSocket&,PlayerThread*);
	~NetworkWriter();

	void run();
};
#endif