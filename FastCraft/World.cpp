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
#include "MapChunk.h"
#include "WorldFileHandler.h"
#include "NBTAll.h"
#include "IOFile.h"
#include <exception>
using std::cout;


World::World(string Name,char iDimension,MinecraftServer* pServer) :
	_WorldName(Name),
	_iDimension(iDimension)
{
	_pMinecraftServer	= pServer;
	_pWorldFileHandler	= NULL;
	_pSettings			= NULL;
	_pLevelDataFile		= NULL;

	/* Init settings */
	_fHardcore		= false;
	_iMapFeatures	= 1;
	_fRaining		= false;
	_fThundering	= false;
	_iGameType		= 0;
	_iRainTime		= 0;
	_iSpawnX		= 0;
	_iSpawnY		= 0;
	_iSpawnZ		= 0;
	_iThunderTime	= 0;
	_iVersion		= 19133;
	_iLastPlayed	= 0L;
	_iSeed			= 0L;
	_iSizeOnDisk	= 0L;
	_iTime			= 0L;
	_sGeneratorName.assign("default");
	_sLevelName.assign("world");

	/* Prepare path */	
	Poco::Path path(pServer->getServerDirectory());
	path.pushDirectory(_WorldName);

	try {
		/* Check existance of needed directories */
		IOFile::createDir(path);
		path.pushDirectory("data");
		IOFile::createDir(path);
		path.popDirectory();
		path.pushDirectory("players");
		IOFile::createDir(path);
		path.popDirectory();
		path.pushDirectory("region");
		IOFile::createDir(path);
		path.popDirectory();
		path.setFileName("level.dat");


		/* try to load level.dat */
		Poco::File filepath(path);
		NBTTagCompound* pCompData;
		char* pData = NULL;

		try {
			try { //Seperate exception catchnig for bad errors
				_pLevelDataFile = new IOFile(filepath);
			}catch(FCRuntimeException& ex) {
				throw std::exception(ex.what());
			}

			if (_pLevelDataFile->getSize() < 2)  {throw FCRuntimeException("Unable to read level.dat",false);}

			pData = new char[_pLevelDataFile->getSize()];
			_pLevelDataFile->read(pData,_pLevelDataFile->getSize());

			NBTBinaryParser parser;
			_pSettings = parser.parse(pData,FC_NBT_IO_GZIP,_pLevelDataFile->getSize());
			pCompData = dynamic_cast<NBTTagCompound*>(_pSettings->getElementByName("Data"));
			if (pCompData == NULL) {throw FCRuntimeException("No Data compound");}
			delete [] pData;

		}catch(FCRuntimeException) {
			/* Something failed, generate own one */
			if (pData) {delete [] pData;}
			if (_pSettings) {delete _pSettings;}

			_pSettings = new NBTTagCompound("");
			_pSettings->addSubElement(pCompData = new NBTTagCompound("Data"));
		}catch(std::exception){
			throw FCRuntimeException("Unable to open/create level.dat");
		}


		NBTHelper<NBTTagByte,char>::read(pCompData,"hardcore",_fHardcore);
		NBTHelper<NBTTagByte,char>::read(pCompData,"MapFeatures",_iMapFeatures);
		NBTHelper<NBTTagByte,char>::read(pCompData,"raining",_fRaining);
		NBTHelper<NBTTagByte,char>::read(pCompData,"thundering",_fThundering);
		NBTHelper<NBTTagInt,int>::read(pCompData,"GameType",_iGameType);
		NBTHelper<NBTTagInt,int>::read(pCompData,"generatorVersion",_iGeneratorVersion);
		NBTHelper<NBTTagInt,int>::read(pCompData,"rainTime",_iRainTime);
		NBTHelper<NBTTagInt,int>::read(pCompData,"SpawnX",_iSpawnX);
		NBTHelper<NBTTagInt,int>::read(pCompData,"SpawnY",_iSpawnY);
		NBTHelper<NBTTagInt,int>::read(pCompData,"SpawnZ",_iSpawnZ);
		NBTHelper<NBTTagInt,int>::read(pCompData,"thunderTime",_iThunderTime);
		NBTHelper<NBTTagInt,int>::read(pCompData,"version",_iVersion);
		NBTHelper<NBTTagInt64,Tick>::read(pCompData,"LastPlayed",_iLastPlayed);
		NBTHelper<NBTTagInt64,long long>::read(pCompData,"RandomSeed",_iSeed);
		NBTHelper<NBTTagInt64,long long>::read(pCompData,"SizeOnDisk",_iSizeOnDisk);
		NBTHelper<NBTTagInt64,Tick>::read(pCompData,"Time",_iTime);
		NBTHelper<NBTTagString,string>::read(pCompData,"generatorName",_sGeneratorName);
		NBTHelper<NBTTagString,string>::read(pCompData,"LevelName",_sLevelName);


		/* Open region files */
		path.setFileName("");
		_pWorldFileHandler = new WorldFileHandler(path);



		/* Load chunks around the spawn point */
		int iLoadedChunks = 0;
		int iChunkSpawnX = _iSpawnX/16;
		int iChunkSpawnZ = _iSpawnZ/16;
		Poco::Stopwatch nextOutput;
		Poco::Stopwatch loadingTime;
	
		nextOutput.start();
		loadingTime.start();
		for (int x = iChunkSpawnX - 12;x<=iChunkSpawnX + 12;x++) {
			for (int z = iChunkSpawnZ - 12;z<=iChunkSpawnZ + 12;z++) {
				getChunk(x,z);
				iLoadedChunks++;

				if (nextOutput.elapsed() >= 1000000) {
					cout<<"\tProgress "<<((iLoadedChunks*100)/625)<<"%\n";
					nextOutput.restart();
				}
			}
		}
		loadingTime.stop();
		cout<<"\tLoading done in "<<(loadingTime.elapsed()/1000000)<<"s\n";
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

World::~World() {
	delete _pWorldFileHandler; 
	delete _pSettings;
	delete _pLevelDataFile;
}

void World::generateChunks(int FromX,int FromZ,int ToX,int ToZ) {
	try {	
		MapChunk* pChunk; 
		for (int x=FromX;x<=ToX;x++) {
			for(int z=FromZ;z<=ToZ;z++) {
				pChunk = new MapChunk(x,z);
				generateChunk(pChunk);
				_heapChunks.add(generateIndex(pChunk->getX(),pChunk->getZ()),pChunk);
			}
		}
	} catch(FCRuntimeException) {
		std::cout<<"World::generateChunks Chunk generation aborted due a error"<<"\n";
	}
}

void World::generateChunk(MapChunk* pChunk) {
	long long ChunkIndex = generateIndex(pChunk->getX(),pChunk->getZ());

	pChunk->alloc();
	pChunk->addSegment();
	std::memset(pChunk->_vpBlocks[0].first,12,4098);

	_heapChunks.add(ChunkIndex,pChunk);
}


MapChunk* World::getChunk(int X,int Z) {
	long long Index = generateIndex(X,Z);
	MapChunk* pChunk = _heapChunks.get(Index);

	if (pChunk != NULL) {return pChunk;} 

	try  {
		pChunk = new MapChunk(X,Z);

		if (_pWorldFileHandler->loadChunk(pChunk)) {
			_heapChunks.add(Index,pChunk);
		}else{
			generateChunk(pChunk);
		}

		return pChunk;
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return NULL;
}

std::pair<ChunkCoordinates,BlockCoordinates> World::toBlockAddress(int X,short Y,int Z) {
	if (Y < 0 || Y > FC_WORLDHEIGHT-1) {throw FCRuntimeException("Y is invalid");}

	ChunkCoordinates chunkCoordinates;
	BlockCoordinates chkInternalBlockCoordinates;

	chunkCoordinates.X = X/16;
	chunkCoordinates.Z = Z/16;
	chkInternalBlockCoordinates.X = X & 0xF; 
	chkInternalBlockCoordinates.Z = Z & 0xF;
	chkInternalBlockCoordinates.Y = (char)Y;

	return std::make_pair(chunkCoordinates,chkInternalBlockCoordinates);
}

char World::getHeight(int X,int Z) {
	//MapChunk* pChunk;

	//try {
	//	pChunk = getChunk(X>>4,Z>>4);
	//}catch (FCRuntimeException& ex) {
	//	ex.rethrow();
	//}

	//int iOffset = ChunkMath::toIndex(ChunkMath::toChunkInternal(X),0,ChunkMath::toChunkInternal(Z));
	//if (iOffset==-1) {throw FCRuntimeException("toIndex error");}

	////Get height
	//for (unsigned char y=FC_WORLDHEIGHT-1;y>=0;y--) { //For from 127 -> 0
	//	if (pChunk->_pBlocks[iOffset+y]) {return y;}
	//}

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
	//if (Coords.Y > ((double)FC_WORLDHEIGHT)-1.0) {return false;} /* Above the map */

	//MapChunk* pChunk;
	//int iOffset;

	//try {
	//	pChunk = getChunk(int(Coords.X)>>4,int(Coords.Z)>>4);
	//}catch (FCRuntimeException& ex) {
	//	ex.rethrow();
	//}

	//for (short y=short(Coords.Y);y<=short(Coords.Y)+short(ceil(dHeight))-1;y++) {		
	//	iOffset = ChunkMath::toIndex(
	//		ChunkMath::toChunkInternal((int)floor(Coords.X)),
	//		y,
	//		ChunkMath::toChunkInternal((int)floor(Coords.Z))
	//		);
	//	if (_pMinecraftServer->getItemInfoProvider()->getBlock((short)pChunk->_pBlocks[iOffset])->Solid) {
	//		return true;
	//	}
	//}

	return false;
}


void World::setBlock(int X,short Y,int Z,ItemID& Block) {
	if (!_pMinecraftServer->getItemInfoProvider()->isRegistered(Block)) {throw FCRuntimeException("Block not registered");}

	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		pChunk->setBlock(Block,
			(char)Coords.second.X,
			(char)Coords.second.Y,
			(char)Coords.second.Z
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

void World::setBlock(BlockCoordinates& Coords,ItemID& Data) {
	try{
		setBlock(Coords.X,Coords.Y,Coords.Z,Data);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}


ItemID World::getBlock(int X,short Y,int Z) {
	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		return pChunk->getBlock((char)Coords.second.X,
			(short)Coords.second.Y,
			(char)Coords.second.Z
			);
	} catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
	return ItemID(0,0); /* Compiler warning fix */
}

ItemID World::getBlock(BlockCoordinates& Coords) {
	try {
		return getBlock(Coords.X,(short)Coords.Y,Coords.Z);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return ItemID(0,0);
} 

string World::getName() {
	return _WorldName;
}

char World::getDimension() {
	return _iDimension;
}


void World::setBlockLight(BlockCoordinates& Coords,char iLL) {
	try {
		setBlockLight(Coords.X,Coords.Y,Coords.Z,iLL);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

void World::setBlockLight(int X,short Y,int Z,char iLightLevel) {
	if (Y < 0 || Y > FC_WORLDHEIGHT-1 || iLightLevel < 0 || iLightLevel > 15) {throw FCRuntimeException("Illegal parameter(s)");}
	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		pChunk->setBlockLight(iLightLevel,
			(char)Coords.second.X,
			(char)Coords.second.Y,
			(char)Coords.second.Z
			);
	}catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
}

char World::getBlockLight(BlockCoordinates& Coords) {
	try {
		return getBlockLight(Coords.X,Coords.Y,Coords.Z);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return 0;
}

char World::getBlockLight(int X,short Y,int Z) {
	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		return pChunk->getBlockLight((char)Coords.second.X,
			(char)Coords.second.Y,
			(char)Coords.second.Z
			);
	} catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
	return 0;
}



void World::setSkyLight(BlockCoordinates& Coords,char iLL) {
	try {
		setSkyLight(Coords.X,Coords.Y,Coords.Z,iLL);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

void World::setSkyLight(int X,short Y,int Z,char iLightLevel) {
	if (Y < 0 || Y > FC_WORLDHEIGHT-1 || iLightLevel < 0 || iLightLevel > 15) {throw FCRuntimeException("Illegal parameter(s)");}
	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		pChunk->setSkyLight(iLightLevel,
			(char)Coords.second.X,
			(char)Coords.second.Y,
			(char)Coords.second.Z
			);
	}catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
}

char World::getSkyLight(BlockCoordinates& Coords) {
	try {
		return getSkyLight(Coords.X,Coords.Y,Coords.Z);
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return 0;
}

char World::getSkyLight(int X,short Y,int Z) {
	try {
		auto Coords = toBlockAddress(X,Y,Z);
		MapChunk* pChunk = getChunk(Coords.first.X,Coords.first.Z);

		return pChunk->getSkyLight((char)Coords.second.X,
			(char)Coords.second.Y,
			(char)Coords.second.Z
			);
	} catch (FCRuntimeException& ex) {
		ex.rethrow();
	}
	return 0; 
}

long long World::generateIndex(int X,int Z) {
	long long x = X;
	long long z = Z;

	return ((x<<32) & 0xffffffff00000000) | (z & 0xffffffff);
}

int World::getSpawnX() {
	return _iSpawnX;
}

int World::getSpawnY() {
	return _iSpawnY;
}

int World::getSpawnZ() {
	return _iSpawnZ;
}