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

#ifndef _FASTCRAFTHEADER_MAPCHUNK
#define _FASTCRAFTHEADER_MAPCHUNK
#include "ItemID.h"
#include <Poco\Mutex.h>
#include <Poco\Timestamp.h>
#include "Types.h"
#include <vector>
#include <utility>
#include <Poco\DeflatingStream.h>

using Poco::Mutex;
using std::vector;
using std::pair;
class NetworkOut;
class NBTTagCompound;
class WorldFileHandler;

class MapChunk {
	friend class World;
	friend class WorldFileHandler;
private:
	int _iX;
	int _iZ;

	time_t _iLastAccess;
	Mutex _Mutex;
	Mutex _segmentCreation;

	Poco::Timestamp _timestamp;

	NBTTagCompound* _pData;
	NBTTagCompound* _pLevelCompound;

	vector<pair<char*,char>> _vpBlocks;
	vector<char*> _vpMetadata;
	vector<char*> _vpBlockLight;
	vector<char*> _vpSkyLight;
	char* _pBiomeArray;
public:
	/*
	* Constructor
	
	Parmeter:
	@1 : Chunks X coordinate
	@2 : Chunks Z coordinate
	*/ 
	MapChunk(int,int);


	/*
	* Destructor
	* Saves/deallocates chunk data
	*/
	~MapChunk();


	/* 
	* Coordinate accessators
	*/
	int getX();
	int getZ();


	/*
	* Accessators for data arrays
	* All functions require chunk coordinates (x/z: 0-15, y 0-127)
	* All functions throw FCException if given coordiantes 
	  are invalid
	* All functions will throw FCException if chunk is not loaded
	* No functions will check block existance
	
	get... Parameters:
	@1 : X coordinate
	@2 : Y coordiante
	@3 : Z coordiante

	set... Parameters:
	@1 : Data to set
	@2 : X coordinate
	@3 : Y coordiante
	@4 : Z coordiante
	*/
	void setBlock(ItemID&,char,short,char);
	ItemID getBlock(char,short,char);
		
	void setBlockLight(char,char,short,char);
	char getBlockLight(char,short,char);

	void setSkyLight(char,char,short,char);
	char getSkyLight(char,short,char);


	/*
	* Appends a MapChunk packet to given NetworkOut
	* Throws FCException if chunk isn't loaded
	*/
	void send(NetworkOut&);


	/*
	* Returns true if chunk was loaded into memory
	*/ 
	bool isLoaded();


	/*
	* Unloads chunk from memory
	* Does nothing if is already unloaded

	Parameter:
	@1 : WorldFileHandler instance to handle saving
	*/ 
	void unload(WorldFileHandler*);


	/*
	* Saves chunk to given WorldFileHandler instance
	*/
	void save(WorldFileHandler*);


	/*
	* Returns tick timestamp of last access
	*/ 
	time_t getLastAccess();

	
	/*
	* Returns true if no chunk sections were allocated
	*/
	bool isEmpty();

	
	/*
	* Adds chunk data to given DeflatingOutputStream 
	* Returns primary bitmap
	*/
	short packData(Poco::DeflatingOutputStream&);
private:
	/* 
	* Throws FCException if block offset is invalid

	Parameter:
	@1 : Block offset
	@2 : new data
	@3 : array where the data has to be set
	*/
	void setNibble(int,char,char*);
	char getNibble(int,char*);


	/*
	* Allocates Chunks NBT structure
	*/
	void alloc();


	/*
	* Updates pointer storages
	*/
	void updatePointerArrays();


	/*
	* Adds a new 16x16x16 segment of chunk space
	*/ 
	void addSegment(); //Adds a new segment ontop of all others

	/*
	* Returns segment array index
	* If not, it generates a new one
	*/
	char getSegment(char);
};
#endif
