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
#include "PlayerPoolEvent.h"
#include "ThreadSafeQueue.h"
#include "EntityPlayer.h"

class PlayerThread;
class PackingThread;

using std::vector;
using std::string;

class PlayerPool : public Poco::Runnable {
private:
	Poco::ThreadPool _ThreadPool;
	vector<PlayerThread*> _vPlayerThreads;
	ThreadSafeQueue<PlayerPoolEvent> _qEventQueue;
	
	PackingThread& _PackingThread;
public:
	/*
	* De/constructor
	*/
	PlayerPool(PackingThread&); //Constructor
	~PlayerPool(); //Destructor


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
	void Event(PlayerPoolEvent&);


	/*
	* List all connected players
	* Returns a string vector

	Parameter:
	@1 : Maxiaml size of vector
	*/
	vector<string> ListPlayers(int);


	/*
	* Builds a EntityPlayer instance from a PlayerThread pointer
	* Will throw Poco::RuntimeException if Player isn't spawned

	Parameter:
	@1 : Pointer to a spawned PlayerThread
	*/
	static EntityPlayer buildEntityPlayerFromPlayerPtr(PlayerThread*);


	/*
	* Look for PlayerThread thats name fits into given and return it's pointer
	* Returns NULL if nothing found
	* This is a cas insensitive search

	Parameter:
	@1 : Players name
	@2 : this pointer of class that calls this function
	*/
	PlayerThread* getPlayerByName(string,PlayerThread*);


	/*
	* Returns true if your setBlock action will hurt others

	Parameter:
	@1 : Affected BlockCoordiantes 
	@2 : this pointer of class that calls this function
	*/
	bool willHurtOther(BlockCoordinates,PlayerThread*);
private:
	int getFreeSlot(); //Returns -1 if there is no free slot
	void sendMessageToAll(string);
};
#endif