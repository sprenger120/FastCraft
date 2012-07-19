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
#include "Constants.h"
#include "MinecraftServer.h"
#include "ChunkMath.h"
#include "ItemInformationProvider.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <math.h>
#include "FCRuntimeException.h"
#include <Poco/ScopedLock.h>
#include "PlayerEvents.h"
#include "PlayerPool.h"
#include "Entity.h"
#include "PlayerThread.h"
#include <cmath>
using std::cout;


World::World(string Name,char iDimension,MinecraftServer* pServer) :
_WorldName(Name),
	_iDimension(iDimension)
{
	_pMinecraftServer = pServer;
	generateChunks(-10,-10,10,10);
}

World::~World() {
}

void World::generateChunks(int FromX,int FromZ,int ToX,int ToZ) {
	try {	
		for (int x=FromX;x<=ToX;x++) {
			for(int z=FromZ;z<=ToZ;z++) {
				generateChunk(x,z);
			}
		}
	} catch(FCRuntimeException) {
		std::cout<<"World::generateChunks Chunk generation aborted due a error"<<"\n";
	}
}

MapChunk* World::generateChunk(int X,int Z) {
	long long ChunkIndex = generateIndex(X,Z);
	MapChunk* pChunk = _heapChunks.get(ChunkIndex); 

	if (pChunk != NULL) {return pChunk;} /* Is this chunk already generated? */
	
	pChunk = new MapChunk;
	
	std::memset(pChunk->Blocks,0x00,FC_CHUNK_DATACOUNT);
	std::memset(pChunk->Metadata,0x00,FC_CHUNK_NIBBLECOUNT);
	std::memset(pChunk->BlockLight,0x00,FC_CHUNK_NIBBLECOUNT);
	std::memset(pChunk->SkyLight,0xFF,FC_CHUNK_NIBBLECOUNT);

	for (short y=0;y<=60;y++) {
		for (int x=0;x<=15;x++) {
			for (int z=0;z<=15;z++) {
				pChunk->Blocks[ChunkMath::toIndex(x,y,z)] = 12;
			}
		}
	}

	_heapChunks.add(ChunkIndex,pChunk);

	return pChunk;
}


MapChunk* World::getChunkByChunkCoordinates(int X,int Z) {
	long long Index = generateIndex(X,Z);
	MapChunk* pChunk = _heapChunks.get(Index);

	if (pChunk != NULL) {return pChunk;} 
	
	try  {
		pChunk = generateChunk(X,Z);
		return pChunk;
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return NULL;
}

pair<ChunkCoordinates,int> World::WorldCoordinateConverter(int X,short Y,int Z) {
	ChunkCoordinates Coords;
	int Index;

	if (Y < 0 || Y > FC_WORLDHEIGHT-1) {
		std::cout<<"World::WorldCoordinateConverter  Y is invalid"<<"\n";
		throw FCRuntimeException("Y is invalid");
	}

	Coords.X = X>>4;
	Coords.Z = Z>>4;

	X = ChunkMath::toChunkInternal(X);
	Z = ChunkMath::toChunkInternal(Z);	

	try {
		Index = ChunkMath::toIndex(X,Y,Z);
	}catch(FCRuntimeException) {
		std::cout<<"World::WorldCoordinateConverter index error"<<"\n";
		throw FCRuntimeException("Index error");
	}

	pair<ChunkCoordinates,int> Pair(Coords,Index);
	return Pair;
}

char World::getHeight(int X,int Z) {
	MapChunk* pChunk;

	try {
		pChunk = getChunkByChunkCoordinates(X>>4,Z>>4);
	}catch (FCRuntimeException& ex) {
		std::cout<<"World::getFreeSpace chunk not found"<<"\n";
		ex.rethrow();
	}

	int iOffset = ChunkMath::toIndex(ChunkMath::toChunkInternal(X),0,ChunkMath::toChunkInternal(Z));


	unsigned char y;
	if (iOffset==-1) {
		throw FCRuntimeException("toIndex error");
	}

	//Get height
	for (y=FC_WORLDHEIGHT-1;y>=0;y--) { //For from 127 -> 0
		if (pChunk->Blocks[iOffset+y]) {return y;}
	}

	return char(FC_WORLDHEIGHT);
}

bool World::isSuffocating(Entity* pEntity) {
	if (pEntity == NULL) {throw FCRuntimeException("Nulllpointer");}
	return isSuffocating(pEntity->Coordinates,pEntity->getHeight());
}

bool World::isSuffocating(PlayerThread* pPlayerThread) {
	if (pPlayerThread == NULL) {throw FCRuntimeException("Nulllpointer");}
	return isSuffocating(pPlayerThread->getCoordinates(),1.6F);
} 


bool World::isSuffocating(EntityCoordinates& Coords,float dHeight) {
	if (Coords.Y > ((double)FC_WORLDHEIGHT)-1.0) {return false;} /* Above the map */

	MapChunk* pChunk;
	int iOffset;

	try {
		pChunk = getChunkByChunkCoordinates(int(Coords.X)>>4,int(Coords.Z)>>4);
	}catch (FCRuntimeException& ex) {
		ex.rethrow();
	}

	/*cout<<"Y :"<<short(Coords.Y)<<" => "<<short(Coords.Y)+short(ceil(dHeight))<<"\n";*/

	for (short y=short(Coords.Y);y<=short(Coords.Y)+short(ceil(dHeight))-1;y++) {		
		iOffset = ChunkMath::toIndex(
		ChunkMath::toChunkInternal((int)floor(Coords.X)),
		y,
		ChunkMath::toChunkInternal((int)floor(Coords.Z))
		);

		/*cout<<"Y = "<<y<<" Blk:"<<int(pChunk->Blocks[iOffset])<<"\n";*/
		if (_pMinecraftServer->getItemInfoProvider()->getBlock((short)pChunk->Blocks[iOffset])->Solid) {
			return true;
		}
	}

	return false;
}

void World::setBlock(int X,short Y,int Z,ItemID& Block) {
	if (!_pMinecraftServer->getItemInfoProvider()->isRegistered(Block)) {
		throw FCRuntimeException("Block not registered");
	}
	MapChunk* p;

	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		int iNibbleIndex = Coords.second/2;

		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		
		p->Blocks[Coords.second] = (char)Block.first; //Set Block
		p->Metadata[iNibbleIndex] = prepareNibble(  //Set Metadata
												char(Coords.second%2),
												p->Metadata[iNibbleIndex],
												Block.second
												);

		//Push event
		BlockCoordinates BlockCoords;
		BlockCoords.X = X;
		BlockCoords.Y = (char)Y;
		BlockCoords.Z = Z;
		
		PlayerEventBase* p = new PlayerSetBlockEvent(BlockCoords,Block,this);
		_pMinecraftServer->getPlayerPool()->addEvent(p);
	} catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
}



ItemID World::getBlock(int X,short Y,int Z) {
	MapChunk* p;

	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		
		ItemID Data;
		Data.first = p->Blocks[Coords.second];
		Data.second = p->Metadata[Coords.second/2];

		if (Coords.second%2) {
			 Data.second = Data.second>>4;
		}else{
			 Data.second &= 15;
		}

		return Data;
	} catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
	return ItemID(0,0);
}

ItemID World::getBlock(BlockCoordinates& Coords) {
	return getBlock(Coords.X,(short)Coords.Y,Coords.Z);
} 

bool World::isSurroundedByAir(BlockCoordinates& TargetBlock) {
	BlockCoordinates Temp = TargetBlock;

	try {

		//X--
		if (getBlock(Temp.X-1,Temp.Y,Temp.Z).first != 0) {
			return false;
		}

		//X++
		if (getBlock(Temp.X+1,Temp.Y,Temp.Z).first != 0) {
			return false;
		}

		//Z--
		if (getBlock(Temp.X,Temp.Y,Temp.Z-1).first != 0) {
			return false;
		}

		//Z++
		if (getBlock(Temp.X,Temp.Y,Temp.Z+1).first != 0) {
			return false;
		}

		//Y--
		if (TargetBlock.Y >= 1) {
			if (getBlock(Temp.X,Temp.Y-1,Temp.Z).first != 0) {
				return false;
			}
		}

		//Y++
		if (TargetBlock.Y <= FC_WORLDHEIGHT-1) {
			if (getBlock(Temp.X,Temp.Y+1,Temp.Z).first != 0) {
				return false;
			}
		}

	}catch(FCRuntimeException& ex) {
		cout<<"Exception World::isSurroundedByAir: "<<ex.getMessage()<<"\n";
	}
	return true;
}

void World::Load(Poco::Path& worldPath) {
	cout<<"Loading: "<<worldPath[worldPath.depth()-1]<<"\n";
}

string World::getName() {
	return _WorldName;
}

char World::getDimension() {
	return _iDimension;
}

char World::prepareNibble(char iMod,char iOld,char iNew) {
	iNew &= 15; //Filter bottom 4 bit 

	if (iMod) { //Set top 4 bits
		iOld |= iNew<<4;
	}else{      //Set lower 4 bits
		iOld |= iNew;
	}	
	return iOld;
}


void World::setBlockLight(int X,short Y,int Z,char iLightLevel) {
	try {
		auto coord = WorldCoordinateConverter(X,Y,Z);
		if (coord.second==-1) {
			cout<<"World::setBlockLight invalid coordinates\n";
			throw FCRuntimeException("Invalid coordinates");
		}
		if (iLightLevel<0 || iLightLevel > 15) {
			cout<<"World::setBlockLight invalid light level\n";
			throw FCRuntimeException("Invalid light level");
		}

		MapChunk* p = getChunkByChunkCoordinates(coord.first.X,coord.first.Z);
		int iNibbleIndex = coord.second/2;


		p->BlockLight[iNibbleIndex] = prepareNibble( 
												char(coord.second%2),
												p->BlockLight[iNibbleIndex],
												iLightLevel
												);
		p->SkyLight[iNibbleIndex] = prepareNibble( 
												char(coord.second%2),
												p->BlockLight[iNibbleIndex],
												iLightLevel
												);

	}catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
}

void World::setBlockLight(BlockCoordinates& Coords,char iLL) {
	setBlockLight(Coords.X,Coords.Y,Coords.Z,iLL);
}

void World::setBlock(BlockCoordinates& Coords,ItemID& Data) {
	setBlock(Coords.X,Coords.Y,Coords.Z,Data);
}

long long World::generateIndex(int X,int Z) {
	long long x = X;
	long long z = Z;
	
	return ((x<<32) & 0xffffffff00000000) | (z & 0x00000000ffffffff);
}
