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

#ifndef _FASTCRAFTHEADER_PACKINGTHREAD
#define _FASTCRAFTHEADER_PACKINGTHREAD
#include <vector>
#include "Structs.h"
#include "ThreadSafeQueue.h"
#include "ServerThreadBase.h"

class NetworkOutRoot;
class PlayerThread;

struct PackJob {
	int X;
	int Z;
	NetworkOutRoot* pNetwork;
	MapChunk* pChunk;
	PlayerThread* pPlayer;
};

using std::queue;

class PackingThread : public ServerThreadBase{
private:
	ThreadSafeQueue<PackJob> _vPackJobs;
public:
	/*
	* Constructor
	*/
	PackingThread();


	/*
	* Destructor
	*/
	~PackingThread();


	/*
	* Thread main
	*/
	virtual void run();


	/*
	* Adds a vector of new job to the queue
	*/
	void AddJobs(std::vector<PackJob>&);


	/*
	* adds a new job to the queue
	*/
	void AddJob(PackJob&);
private:
	void ProcessJob(PackJob&);
};
#endif
