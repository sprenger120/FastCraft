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
#ifndef _FASTCRAFTHEADER_WORLD
#define _FASTCRAFTHEADER_WORLD
#include <string>
#include <utility>
#include <Poco/Mutex.h>
#include "Structs.h"
#include "EntityCoordinates.h"
#include "ItemInformationProvider.h"
#include "Heap.h"
#include "Types.h"

using std::string;
using std::pair;
class MinecraftServer;
class Entity;
class PlayerThread;
class WorldFileHandler;
class MapChunk;
class NBTTagCompound;
class IOFile;

class World {
private:
	string _WorldName;
	char _iDimension;
	Poco::Mutex _Mutex;

	Heap<MapChunk*,long long> _heapChunks;
	MinecraftServer* _pMinecraftServer;
	WorldFileHandler* _pWorldFileHandler;
	NBTTagCompound* _pSettings;
	IOFile* _pLevelDataFile;

	/* Settings */
	char _fHardcore;
	char _iMapFeatures;
	char _fRaining;
	char _fThundering;
	int _iGameType;
	int _iGeneratorVersion;
	int _iRainTime;
	int _iSpawnX;
	int _iSpawnY;
	int _iSpawnZ;
	int _iThunderTime;
	int _iVersion;
	Tick _iLastPlayed;
	long long _iSeed;
	long long _iSizeOnDisk;
	Tick _iTime;
	string _sGeneratorName;
	string _sLevelName;
public:
	/*
	* Constructs a new World
	* Generates 20x20 chunks around 0,0

	Parameter:
	@1 : World name
	@2 : Dimension ( 0=Overworld	1=End	-1=Neather )
	@3 : MinecraftServer instance that runs this class
	*/
	World(string,char,MinecraftServer*);


	/*
	* Destruct world object
	* Will free map
	* Will save map to hdd
	*/
	~World();


	/*
	* Look for a chunk and returns a pointer to it
	* Generates chunk if not existing
	* Will throw FCRuntimeException if generateChunks failed

	Paramter:
	@1 : X in chunkcoordinates
	@2 : Z in chunkcoordiantes
	*/
	MapChunk* getChunk(int,int);


	/*
	* Returns height at given X,Z coordiante
	* Will generate chunk if not exits
	* Will return FC_WORLDHEIGHT if blocks were built to max height 

	Parameter:
	@1 : X in world coordinates
	@2 : Z in world coordiantes
	*/
	char getHeight(int,int);


	/*
	* Returns true if given Entity will suffocate at his actual place
	* Will generate chunk if not exist
	* Throws FCRuntimeException if a nullpointer was given

	Parameter:
	@1 : Entity instance to check

	Parameter:
	@1 : PlayerThread instance to check

	Parameter:
	@1 : Coordinates to check
	@2 : Height of thing 
	*/
	bool isSuffocating(Entity*);
	bool isSuffocating(PlayerThread*);
	bool isSuffocating(EntityCoordinates&,float);


	/*
	* Sets block
	* Generates chunk if not existing
	* Will throw FCRuntimeException if generateChunks failed
	* Will throw FCRuntimeException if block not exists

	Parameter:
	@1 : X in world coordinates
	@2 : Y in world coordinates
	@3 : Z in world coordinates
	@4 : block ID
	*/
	void setBlock(int,short,int,ItemID&);
	void setBlock(BlockCoordinates&,ItemID&);

	/*
	* Gets block at given position
	* Generates chunk if not existing
	* Will throw FCRuntimeException if generateChunks failed

	Parameter:
	@1 : X in world coordinates
	@2 : Y in world coordinates
	@3 : Z in world coordinates
	*/
	ItemID getBlock(int,short,int);
	ItemID getBlock(BlockCoordinates&);


	/*
	* Returns worldname
	*/
	string getName();


	/*
	* Returns dimension
	*/
	char getDimension();


	/*
	* Sets block light of given block
	* Will throw FCRuntimeException if block light is invalid
	* Will rethrow all errors of getChunk

	Parameters:
	@1 : X in WorldCoordinates
	@2 : Y in WorldCoordinates
	@3 : Z in WorldCoordinates
	@4 : new light level
	*/
	void setBlockLight(BlockCoordinates&,char);
	void setBlockLight(int,short,int,char);

	char getBlockLight(BlockCoordinates&);
	char getBlockLight(int,short,int);

	void setSkyLight(BlockCoordinates&,char);
	void setSkyLight(int,short,int,char);

	char getSkyLight(BlockCoordinates&);
	char getSkyLight(int,short,int);


	/*
	* Returns spawn coordinates
	*/
	int getSpawnX();
	int getSpawnY();
	int getSpawnZ();
private:
	void generateChunks(int,int,int,int);
	void generateChunk(MapChunk*);

	std::pair<ChunkCoordinates,BlockCoordinates> toBlockAddress(int,short,int);
	long long generateIndex(int,int);
}; 

#endif
