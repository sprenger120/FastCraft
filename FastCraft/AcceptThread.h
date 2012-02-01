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

#ifndef _FASTCRAFTHEADER_ACCEPTHREAD
#define _FASTCRAFTHEADER_ACCEPTHREAD

#include <iostream>
#include <Poco/Runnable.h>
#include <Poco/Net/ServerSocket.h>
#include <string>

using std::string;
class PlayerPool;

class AcceptThread : public Poco::Runnable {
private:
	string _sIP;
	string _ServerFullMsg;
	Poco::Net::ServerSocket _ServerSock;
	PlayerPool& _rPlayerPool;
	bool _fRunning;
public:
	//De- /constructor
	AcceptThread(PlayerPool&);
	~AcceptThread();

	virtual void run(); //Thread Main
	void shutdown();
};

#endif