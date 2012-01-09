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
#include "World.h"
#include "ChunkTerraEditor.h"
#include "Constants.h"
#include "SettingsHandler.h"
#include "ChunkMath.h"
#include "ItemInfoStorage.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <math.h>
#include <Poco/Exception.h>
using std::cout;


World::World(string Name) :
_WorldName(Name)
{
	_iLoadedChunks=0;
	generateChunks(-10,-10,10,10);
}


World::~World() {
	//Free chunks
	for (int x=0;x<=_vChunks.size()-1;x++) {
		for(int z=0;z<=_vChunks[x].size()-1;z++) {
			delete _vChunks[x][z].Chunk;
		}
	}
}

void World::generateChunks(int FromX,int FromZ,int ToX,int ToZ) {
	int x = FromX,z=FromZ;
	try {
		for (int x=FromX;x<=ToX;x++) {
			for(int z=FromZ;z<=ToZ;z++) {
				generateChunk(x,z);
			}
		}
	} catch(Poco::RuntimeException) {
		std::cout<<"World::generateChunks Chunk generation aborted due a error"<<"\n";
	}
}

MapChunk* World::generateChunk(int X,int Z) {
	bool fXRowExisting=false;
	int XRow,ZRow; //Index of chunk in list
	MapChunk* pAffectedChunk=NULL;

	_Mutex.lock();

	/*
	Reserve a new place for chunk
	*/
	//Look for a existing x row 
	if (_vChunks.size() > 0) {
		for (int x=0;x<=_vChunks.size()-1;x++) {
			if (_vChunks[x][0].X == X) {
				//There is a existing row		
				//Add a new one
				ChunkInternal Chk;

				Chk.X=X;
				Chk.Z=Z;
				Chk.Chunk = pAffectedChunk = new MapChunk; 

				if (Chk.Chunk == NULL) {
					std::cout<<"World::generateChunk unable to allocate memory"<<"\n";
					_Mutex.unlock();
					throw Poco::RuntimeException("Generation failed!");
				}

				_vChunks[x].push_back(Chk);//Add to array

				XRow=x;
				ZRow=_vChunks[x].size()-1;
				fXRowExisting=true;
			}
		} 	
	}

	//Doesn't exist
	if (!fXRowExisting) {
		vector<ChunkInternal> vZRow(1); 

		vZRow[0].Chunk = pAffectedChunk = new MapChunk;
		vZRow[0].X=X;
		vZRow[0].Z=Z;

		if (vZRow[0].Chunk == NULL) {
			std::cout<<"World::generateChunk unable to allocate memory"<<"\n";
			_Mutex.unlock();
			throw Poco::RuntimeException("Generation failed!");
		}

		_vChunks.push_back(vZRow);
		XRow = _vChunks.size()-1;
		ZRow = 0;
	}


	/*
	* Generate Chunk
	*/
	Block b;
	b.BlockID = 7;

	std::memset(pAffectedChunk->Blocks,0,FC_CHUNK_BLOCKCOUNT); //Clear blocks
	try {
		ChunkTerraEditor::setPlate(pAffectedChunk,0,b); //Bedrock 

		b.BlockID = 12;

		for (short y=1;y<=70;y++) {
			ChunkTerraEditor::setPlate(pAffectedChunk,y,b);
		}
	} catch (Poco::RuntimeException& err) {
		std::cout<<"World::generateChunk unable to generate chunk ("<<err.message()<<")"<<"\n";

		//Erase chunk from list
		_vChunks[XRow].erase(_vChunks[XRow].begin() + ZRow);
		if (_vChunks[XRow].size() == 0) {
			_vChunks.erase(_vChunks.begin() + XRow);
		}

		_Mutex.unlock();
		throw Poco::RuntimeException("Generation failed!");
	}

	//Light & Metadata
	std::memset(pAffectedChunk->Metadata,0,FC_CHUNK_NIBBLECOUNT);
	std::memset(pAffectedChunk->BlockLight,0xff,FC_CHUNK_NIBBLECOUNT);
	std::memset(pAffectedChunk->SkyLight,0xff,FC_CHUNK_NIBBLECOUNT);

	_Mutex.unlock();
	return pAffectedChunk;
}


MapChunk* World::getChunkByChunkCoordinates(int X,int Z) {
	for (int x=0;x<=_vChunks.size()-1;x++) { //Search in X rows
		if (_vChunks[x][0].X == X) { //Row found
			for (int z=0;z<=_vChunks[x].size()-1;z++) { //Search z entry
				if (_vChunks[x][z].Z ==Z) { //found
					return _vChunks[x][z].Chunk;
				}
			}
			break; 
		}
	}


	MapChunk* p;

	//Try to generate chunk
	try  {
		p = generateChunk(X,Z);
	} catch(Poco::RuntimeException& ex) {
		std::cout<<"World::getChunkByChunkCoordinates Chunk generation aborted due a error"<<"\n";
		ex.rethrow();
	}
	return p;
}

pair<ChunkCoordinates,int> World::WorldCoordinateConverter(int X,short Y,int Z) {
	ChunkCoordinates Coords;
	int Index;

	if (Y < 0 || Y > SettingsHandler::getWorldHeight()-1) {
		std::cout<<"World::WorldCoordinateConverter  Y is invalid"<<"\n";
		throw Poco::RuntimeException("Y is invalid");
	}

	Coords.X = X>>4;
	Coords.Z = Z>>4;

	X = ChunkMath::toChunkInternal(X);
	Z = ChunkMath::toChunkInternal(Z);	

	try {
		Index = ChunkMath::toIndex(X,Y,Z);
	}catch(Poco::RuntimeException) {
		std::cout<<"World::WorldCoordinateConverter index error"<<"\n";
		throw Poco::RuntimeException("Index error");
	}


	pair<ChunkCoordinates,int> Pair(Coords,Index);
	return Pair;
}

char World::getFreeSpace(int X,int Z) {
	int ChunkX = X>>4;
	int ChunkZ = Z>>4;
	MapChunk* pChunk;

	try {
		pChunk = getChunkByChunkCoordinates(ChunkX,ChunkZ);
	}catch (Poco::RuntimeException& ex) {
		std::cout<<"World::getFreeSpace chunk not found"<<"\n";
		ex.rethrow();
	}

	X = ChunkMath::toChunkInternal(X);
	Z = ChunkMath::toChunkInternal(Z);	


	int iOffset = ChunkMath::toIndex(X,0,Z);
	unsigned char y;
	if (iOffset==-1) {
		std::cout<<"World::getFreeSpace could not calculate index"<<"\n";
		throw Poco::RuntimeException("toIndex error");
	}

	//Get height
	for (y=SettingsHandler::getWorldHeight()-1;y>0;y--) { //For from 128 -> 1
		if (pChunk->Blocks[iOffset+y] != 0) {
			return y+1;
		}
	}

	return SettingsHandler::getWorldHeight();
}

bool World::isSuffocating(EntityCoordinates Coords) {
	if (Coords.Y <= 0.9) {
		return true;
	}

	MapChunk* pChunk;

	try {
		pChunk = getChunkByChunkCoordinates(int(Coords.X)>>4,int(Coords.Z)>>4);
	}catch (Poco::RuntimeException& ex) {
		std::cout<<"World::isSuffocating chunk not found"<<"\n";
		ex.rethrow();
	}

	if (Coords.Y > (double)SettingsHandler::getWorldHeight()) {
		return false;
	}

	int iOffset = ChunkMath::toIndex( 
		ChunkMath::toChunkInternal((int)floor(Coords.X)),
		(int)floor(Coords.Y),
		ChunkMath::toChunkInternal((int)floor(Coords.Z))
		);

	if (pChunk->Blocks[iOffset] == 0) {
		return false;
	}else{
		return true;
	}
}

void World::setBlock(int X,short Y,int Z,char Block) {
	if (!ItemInfoStorage::isRegistered(Block)) {
		throw Poco::RuntimeException("Block not registered");
	}

	MapChunk* p;
	
	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		p->Blocks[Coords.second] = Block;
	} catch (Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}

char World::getBlock(int X,short Y,int Z) {
	MapChunk* p;
	
	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		return p->Blocks[Coords.second];
	} catch (Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return 0;
}

char World::getBlock(BlockCoordinates Coords) {
	return getBlock(Coords.X,(short)Coords.Y,Coords.Z);
} 