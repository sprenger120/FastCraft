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

#include "PackingThread.h"
#include <Poco/Thread.h>
#include "NetworkIO.h"

#include <iostream>
using Poco::Thread;

PackingThread::PackingThread() : 
	_stringStrm(),
	_deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1),
	_vPackJobs()
{
}


PackingThread::~PackingThread() {
	_stringStrm.clear();
	_deflatingStrm.clear();
}

void PackingThread::run() {
	while (1) { 
		if (_vPackJobs.size() == 0) {
			Thread::sleep(50);
			continue;
		}
		
		while(_vPackJobs.size()) {
			ProcessJob(_vPackJobs.front());
			_vPackJobs.pop();
		}
	}
}

void PackingThread::ProcessJob(PackJob& rJob) {	
	//prechunk
	rJob.pNetwork->Lock();

	rJob.pNetwork->addByte(0x33);
	rJob.pNetwork->addInt((rJob.X)<<4);
	rJob.pNetwork->addShort(0);
	rJob.pNetwork->addInt((rJob.Z)<<4);
	rJob.pNetwork->addByte(15);
	rJob.pNetwork->addByte(127);
	rJob.pNetwork->addByte(15);


	//deflate
	_deflatingStrm.write(rJob.pChunk->Blocks,FC_CHUNK_BLOCKCOUNT);
	_deflatingStrm.write(rJob.pChunk->Metadata,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->BlockLight,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->SkyLight,FC_CHUNK_NIBBLECOUNT);

	_deflatingStrm.flush();

	rJob.pNetwork->addInt(_stringStrm.str().length());
	rJob.pNetwork->Str().append(_stringStrm.str());


	_stringStrm.clear();
	_deflatingStrm.clear();

	rJob.pNetwork->Flush();
	rJob.pNetwork->UnLock();
}

void PackingThread::AddJob(PackJob& rJob) {
	_vPackJobs.push(rJob);
}