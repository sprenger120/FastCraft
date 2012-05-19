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
#include "Structs.h"
#include "EntityCoordinates.h"
#include "ThreadSafeQueue.h"
#include "EntityPlayer.h"
#include "ServerThreadBase.h"
#include "PackingThread.h"

class PlayerThread;
class PackingThread;
class PlayerEventBase;
class MinecraftServer;

using std::vector;
using std::string;

class PlayerPool : public ServerThreadBase {
private:
	MinecraftServer* _pMinecraftServer;
	vector<PlayerThread*> _vPlayerThreads;
	ThreadSafeQueue<PlayerEventBase*> _qEvents;
	
	PackingThread _PackingThread;
public:
	/*
	* Constructor

	Parameter:
	@1 : MinecraftServer instance that run this PlayerPool
	*/
	PlayerPool(MinecraftServer*); //Constructor


	/*
	* Destructor
	*/
	~PlayerPool();


	/*
	* Thread Main
	* This thread boosts packet delivering and handles Player sent events
	*/
	virtual void run(); // Thread main
	

	/*
	* Returns true if there is a free slot
	*/
	bool isAnySlotFree(); //Returns true if there is any free slot


	/*
	* Assign a new connection to a free PlayerThread

	Parameter:
	@1 : Reference to a connected StreamSocket
	*/
	void Assign(Poco::Net::StreamSocket&); //Assigns a connection to a free thread


	/*
	* Adds an event to playerpool's event queue
	* Coordinates,Job and pThread have to be filled with valid data !
	
	Parameter:
	@1 : Reference to a PlayerPoolEvent struct
	*/
	void addEvent(PlayerEventBase*);


	/*
	* Returns a reference to the internal playerThread vector
	*/
	vector<PlayerThread*>& ListPlayers();


	/*
	* Searchesr PlayerThread thats name fits into given and return it's pointer
	* Returns NULL if nothing found
	* This is a case insensitive search

	Parameter:
	@1 : Players name
	@2 : PlayerThread class that had called this function
	*/
	PlayerThread* getPlayerByName(string,PlayerThread*);


	/*
	* Sends a message to all connected and spawned players
	*/
	void sendMessageToAll(string);
	bool willHurtOther(BlockCoordinates,PlayerThread*);


	/*
	* Returns count of actual connected players
	*/
	short getConnectedPlayerCount();
private:
	int getFreeSlot(); //Returns -1 if there is no free slot
};
#endif