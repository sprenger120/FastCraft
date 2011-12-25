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

#ifndef _FASTCRAFTHEADER_PACKINGTHREAD
#define _FASTCRAFTHEADER_PACKINGTHREAD
#include <Poco/Runnable.h>
#include <sstream>
#include <Poco/DeflatingStream.h>
#include "Structs.h"
#include "ThreadSafeQueue.h"

class NetworkOutRoot;

struct PackJob {
	int X;
	int Z;
	NetworkOutRoot* pNetwork;
	MapChunk* pChunk;
};

using std::queue;

class PackingThread : public Poco::Runnable{
private:
	ThreadSafeQueue<PackJob> _vPackJobs;
	std::stringstream _stringStrm;
	Poco::DeflatingOutputStream _deflatingStrm;
public:
	PackingThread();
	~PackingThread();

	virtual void run();

	void AddJob(PackJob&);
private:
	void ProcessJob(PackJob&);
};
#endif
