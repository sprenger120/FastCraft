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

#include "ChunkProvider.h"
#include <iostream>
#include <Poco/Stopwatch.h>
#include <Poco/Exception.h>
#include <Poco/DeflatingStream.h>
#include <sstream>
#include <ostream>
#include "ChunkTerraEditor.h"
#include "Structs.h"
#include "NetworkIO.h"
#include "PlayerThread.h"

using std::cout;
using std::endl;
using std::memcpy;
using std::stringstream;
using Poco::DeflatingOutputStream;

ChunkProvider::ChunkProvider(int iMaxChunks) {
	if (iMaxChunks <= 0) {
		throw Poco::RuntimeException("Illegal chunk count");
		return;
	}

	_iAllocatedChunkCount = iMaxChunks;
	_vMapChunks.resize(iMaxChunks);

	//Allocate memory
	for (int x=0;x<=iMaxChunks-1;x++) {
		_vMapChunks[x] = new MapChunk;	
		_vMapChunks[x]->Empty = true;
	}
}

ChunkProvider::~ChunkProvider() {
	//release memory
	for (int x=0;x<=_iAllocatedChunkCount-1;x++) {
		delete (_vMapChunks[x]);
	}
}

void ChunkProvider::generateMap(int FromX,int FromZ,int ToX,int ToZ) {
	Poco::Stopwatch Timer;
	Timer.start();
	int iCount=0;
	int index=0;
	Block Block;

	if (FromX > ToX || FromZ > ToZ) {
		throw Poco::RuntimeException("Illegal Arguments, From > To");
		return;
	}

	for(int chunkX = FromX;chunkX<=ToX;chunkX++) {
		for (int chunkZ = FromZ;chunkZ<=ToZ;chunkZ++) {
			index = getFreeChunkSlot();
			if (index == -1) {
				throw Poco::RuntimeException("Chunk slots are full!");
				return;
			}

			cout<<"Generating chunk X:"<<chunkX<<" Z:"<<chunkZ<<"\n";

			_vMapChunks[index]->X = chunkX;
			_vMapChunks[index]->Z = chunkZ;
			_vMapChunks[index]->Empty = false;

			ClearChunk(index);

			Block.BlockID = 7;

			try {

				ChunkTerraEditor::setPlate(_vMapChunks[index],0,Block); //Bedrock 

				Block.BlockID = 2;

				for (short y=1;y<=50;y++) { //Stone
					ChunkTerraEditor::setPlate(_vMapChunks[index],y,Block);
				}

			} catch (Poco::RuntimeException& err) {
				cout<<"***GENERATING ERROR:"<<err.message()<<endl;
				throw Poco::RuntimeException("Generation failed!");
				return;
			}

			//Generating Light & Metadata
			for (int x=0;x<=FC_CHUNK_NIBBLECOUNT-1;x++) {
				_vMapChunks[index]->Metadata[x] = 0;
				_vMapChunks[index]->SkyLight[x] = 0xff;
				_vMapChunks[index]->BlockLight[x] = 0xff;
			}

			iCount++;
		}
	}

	Timer.stop();
	cout<<"Generated "<<iCount<<" chunks in "<<Timer.elapsed() / 1000<<" ms."<<endl;
}


void ChunkProvider::ClearChunk(int Index) {
	if (Index < 0 || Index > _iAllocatedChunkCount) {return;}

	MapChunk * pAffectedChunk = _vMapChunks[Index];

	for(int x=0;x<=FC_CHUNK_BLOCKCOUNT-1;x++) {
		pAffectedChunk->Blocks[x] = 0;
	}
}

int ChunkProvider::getChunkIndexByCoords(int X,int Z) {
	for (int x=0;x<=_iAllocatedChunkCount-1;x++) {
		if (_vMapChunks[x]->X == X && _vMapChunks[x]->Z == Z) {
			return x;
		}
	}
	return -1;
}

int ChunkProvider::getFreeChunkSlot() {
	for (int x=0;x<=_iAllocatedChunkCount-1;x++) {
		if (_vMapChunks[x]->Empty == true) {
			return x;
		}
	}
	return -1;
}

void ChunkProvider::sendChunks(PlayerThread* pPlayerThread) {
	int index = 0;
	NetworkIO& rNetwork = pPlayerThread->getConnection();

	Poco::Stopwatch Timer;
	Timer.start();

	std::stringstream ss;
	DeflatingOutputStream defStream(ss,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1);

	int packtime = 0;

	for(int X = 0;X<=20;X++) {
		for (int Z = 0;Z<=20;Z++) {

			index = getChunkIndexByCoords(X,Z);
			if (index == -1) {
				//Generate chunk....
				throw Poco::RuntimeException("Chunk not found");
			}

			rNetwork.addByte(0x32);
			rNetwork.addInt(X);
			rNetwork.addInt(Z);
			rNetwork.addBool(1);
			rNetwork.Flush();


			rNetwork.addByte(0x33);
			rNetwork.addInt(X<<4);
			rNetwork.addShort(0);
			rNetwork.addInt(Z<<4);
			rNetwork.addByte(15);
			rNetwork.addByte(127);
			rNetwork.addByte(15);

			//deflate
			Poco::Stopwatch sw;
			sw.start();
			defStream.write(_vMapChunks[index]->Blocks,FC_CHUNK_BLOCKCOUNT);
			defStream.write(_vMapChunks[index]->Metadata,FC_CHUNK_NIBBLECOUNT);
			defStream.write(_vMapChunks[index]->BlockLight,FC_CHUNK_NIBBLECOUNT);
			defStream.write(_vMapChunks[index]->SkyLight,FC_CHUNK_NIBBLECOUNT);

			defStream.flush();

			rNetwork.addInt(ss.str().length());
			rNetwork.Str().append(ss.str());

			
			ss.clear();
			defStream.clear();

			sw.stop();
			packtime += sw.elapsed() /1000;
			//cout<<"packtime:"<<sw.elapsed()/1000<<"\n";

		}

	}

	defStream.close();
	Timer.stop();

	cout<<"packtime:"<<packtime<<"\n";
	cout<<"done! "<<Timer.elapsed()/1000<<" ms"<<endl;
}