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

/*
* This class handles player thread assignment and slot management.
*/
#ifndef _FASTCRAFTHEADER_PLAYERPOOL
#define _FASTCRAFTHEADER_PLAYERPOOL
#include <iostream>
#include <vector>
#include <Poco/Net/StreamSocket.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include "EntityProvider.h"
#include "ServerTime.h"

class SettingsHandler;
class PlayerThread;
class ChunkRoot;

using std::vector;

class PlayerPool : public Poco::Runnable {
private:
	EntityProvider _EntityProvider;
	Poco::ThreadPool _ThreadPool;
	ServerTime _ServerTime;
	vector<PlayerThread*> _vPlayerThreads;
	ChunkRoot* _pChunkRoot;
public:
	PlayerPool(SettingsHandler*); //Constructor
	~PlayerPool(); //Destructor

	virtual void run(); // Thread main
	
	bool isAnySlotFree(); //Returns true if there is any free slot
	void Assign(Poco::Net::StreamSocket&); //Assigns a connection to a free thread
private:
	int getFreeSlot(); //Returns -1 if there is no free slot
};
#endif