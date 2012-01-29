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

#include "PackingThread.h"
#include <Poco/Thread.h>
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "PlayerThread.h"
#include "Constants.h"
#include <iostream>
using Poco::Thread;

PackingThread::PackingThread() : 
	_vPackJobs()
{
}


PackingThread::~PackingThread() {
}

void PackingThread::run() {
	while (1) { 
		if (_vPackJobs.size() == 0) {
			Thread::sleep(10);
			continue;
		}
		while(!_vPackJobs.empty()) {
			ProcessJob(_vPackJobs.front());
			_vPackJobs.pop();
		}
	}
}

void PackingThread::ProcessJob(PackJob& rJob) {	
	if (rJob.pPlayer==NULL || rJob.pNetwork==NULL) {
		std::cout<<"PackingThread::ProcessJob Nullpointer"<<"\n";
		return;
	}
	if (!rJob.pPlayer->isAssigned()) { //Player is offline but a job for him is in queue -> skip it
		return; 
	}

	std::stringstream _stringStrm;
	Poco::DeflatingOutputStream _deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1);

	NetworkOut Out(rJob.pNetwork);

	Out.addByte(0x33);
	Out.addInt((rJob.X)<<4);
	Out.addShort(0);
	Out.addInt((rJob.Z)<<4);
	Out.addByte(15);
	Out.addByte(127);
	Out.addByte(15);

	//deflate
	_deflatingStrm.write(rJob.pChunk->Blocks,FC_CHUNK_BLOCKCOUNT);
	_deflatingStrm.write(rJob.pChunk->Metadata,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->BlockLight,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->SkyLight,FC_CHUNK_NIBBLECOUNT);

	_deflatingStrm.flush();

	Out.addInt(_stringStrm.str().length());
	Out.getStr().append(_stringStrm.str());

	_stringStrm.clear();
	_deflatingStrm.clear();
	_deflatingStrm.close();

	Out.Finalize(FC_QUEUE_LOW);
}

void PackingThread::AddJobs(std::vector<PackJob> & rvJob) {
	_vPackJobs.multiPush(rvJob);
}

void PackingThread::AddJob(PackJob& rJob) {
	_vPackJobs.push(rJob);
}