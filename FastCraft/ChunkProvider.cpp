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
#include "ChunkTerraEditor.h"
#include "Structs.h"

using std::cout;
using std::endl;

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
		_vMapChunks[x]->X = -1;
		_vMapChunks[x]->Z = -1;
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

			Block.BlockID = 7;

			try {

				ChunkTerraEditor::setPlate(_vMapChunks[index],0,Block); //Bedrock 

				for (short y=1;y<=50;y++) { //Stone
					ChunkTerraEditor::setPlate(_vMapChunks[index],y,Block);
				}

			} catch (Poco::RuntimeException& err) {
				cout<<"***GENERATING ERROR:"<<err.message()<<endl;
				throw Poco::RuntimeException("Generation failed!");
				return;
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
		if (_vMapChunks[x]->X == -1 && _vMapChunks[x]->Z == -1) {
			return x;
		}
	}
	return -1;
}

void ChunkProvider::sendChunks(PlayerThread* pPlayerThread) {
	cout<<"plah"<<"\n";
}