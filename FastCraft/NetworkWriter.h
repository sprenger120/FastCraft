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
#ifndef _FASTCRAFTHEADER_NETWORKWRITER
#define _FASTCRAFTHEADER_NETWORKWRITER
#include <Poco/Net/StreamSocket.h>
#include <string>
#include "ThreadSafeQueue.h"
#include "ServerThreadBase.h"


class PlayerThread;
using std::string;

class NetworkWriter : public ServerThreadBase { 
private:
	ThreadSafeQueue<string>& _rLowQueue;
	ThreadSafeQueue<string>& _rHighQueue;
	Poco::Net::StreamSocket& _rStrm;
	PlayerThread* _pPlayer;

	bool _fClear;
public:
	/*
	* Constructor

	Parameter:
	@1 : Reference to a queue that will contain chunks and multi block changes
	@2 : Reference to a queue that will contain all other packets
	@3 : Reference to the StreamSocket instance of the player
	@4 : this pointer of PlayerThread class this class
	*/
	NetworkWriter(	ThreadSafeQueue<string>&,
					ThreadSafeQueue<string>&,
					Poco::Net::StreamSocket&,
					PlayerThread*
				 );


	/*
	* Destructor
	*/
	~NetworkWriter();

	
	/*
	* Thread main
	*/
	void run();


	/*
	* Exception preventing method for clearing the write queues
	*/
	void clearQueues();
private:
	void waitTillDisconnected();
};
#endif