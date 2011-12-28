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
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "PlayerThread.h"
#include "Constants.h"
#include <iostream>
#include "Poco/Stopwatch.h"
using Poco::Thread;

PackingThread::PackingThread() : 
	_stringStrm(),
	_deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1),
	_vPackJobs()
{
	_iNeededTime = 1;
	_iProcessedJobs = 0;
}


PackingThread::~PackingThread() {
	_stringStrm.clear();
	_deflatingStrm.clear();
}

void PackingThread::run() {
	Poco::Stopwatch Timer;
	Timer.start();
	while (1) { 
		if (Timer.elapsed() > 1000 * 1000) {
			Timer.stop();
			Timer.reset();
			Timer.start();
			std::cout<<"Packer queue size:"<< _vPackJobs.size()<<"\t"<<" average i/s:"<<_iProcessedJobs/_iNeededTime<<" ms"<<"\t"<<" processed elements:"<<_iProcessedJobs<<"\n";
		}
		if (_vPackJobs.size() == 0) {
			Thread::sleep(10);
			continue;
		}
		while(!_vPackJobs.empty()) {
			_iProcessedJobs++;
			ProcessJob(_vPackJobs.front());
			_vPackJobs.pop();
		}
	}
}

void PackingThread::ProcessJob(PackJob& rJob) {	
	if (!rJob.pPlayer->isAssigned()) { //Player is offline but a job for him is in queue -> skip it
		return; 
	}
	Poco::Stopwatch Sw;
	Sw.start();
	NetworkOut Out = rJob.pNetwork->New();


	Out.addByte(0x33);
	Out.addInt((rJob.X)<<4);
	Out.addShort(0);
	Out.addInt((rJob.Z)<<4);
	Out.addByte(15);
	Out.addByte(127);
	Out.addByte(15);

	//std::cout<<"writing X:"<<rJob.X<<" Z:"<<rJob.Z<<"\n";

	//deflate
	if(rJob.pChunk==NULL) {
		std::cout<<"PackingThread::ProcessJob Nullpointer"<<"\n";
		return;
	}
	_deflatingStrm.write(rJob.pChunk->Blocks,FC_CHUNK_BLOCKCOUNT);
	_deflatingStrm.write(rJob.pChunk->Metadata,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->BlockLight,FC_CHUNK_NIBBLECOUNT);
	_deflatingStrm.write(rJob.pChunk->SkyLight,FC_CHUNK_NIBBLECOUNT);

	_deflatingStrm.flush();

	Out.addInt(_stringStrm.str().length());
	Out.getStr().append(_stringStrm.str());


	_stringStrm.clear();
	_deflatingStrm.clear();

	Out.Finalize(FC_QUEUE_LOW);
	Sw.stop();
	_iNeededTime += Sw.elapsed() / 1000;
}

void PackingThread::AddJobs(std::vector<PackJob> & rvJob) {
	_vPackJobs.multiPush(rvJob);
}