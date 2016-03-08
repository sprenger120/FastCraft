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

#ifndef _FASTCRAFTHEADER_WORLDFILEHANDLER
#define _FASTCRAFTHEADER_WORLDFILEHANDLER
#include <Poco/Mutex.h>
#include <fstream>
#include <Poco/Path.h>
#include <vector>
#include "Heap.h"
#include "IOFile.h"
#include <Poco\Mutex.h>
using std::vector;
using std::fstream;
using Poco::Mutex;
using Poco::Path;
class MapChunk;
class NBTTagCompound;

class WorldFileHandler {
private:
	struct listEntry {
		IOFile* pStream;
		int X;
		int Z;
		int OffsetTable[32*32];
		int LastAccessTable[32*32];
		Poco::Mutex* pMutex;
	};
	vector<listEntry*> _vpFiles;
	Poco::Path _regionDirectory;
public:
	/*
	* Constructor
	* Opens all .mca files and parses them
	* Throws FCRuntimeException if directory doesn't exist

	Parameter:
	@1 : Path to world directory
	@2 : Reference to Worlds internal chunk heap
	*/
	WorldFileHandler(Poco::Path);


	/*
	* Destructor
	*/
	~WorldFileHandler();


	/*
	* Loads a chunk from file
	* Returns false if chunk wasn't saved so far
	* Throws FCRuntimeException if 
	   - MapChunk is loaded already
	   - Given pointer is null
	   - creation of new region file fails

	Parameter:
	@1 : MapChunk 
	*/
	bool loadChunk(MapChunk*);
private:
	void createNewRegionFile(int,int);
	bool isChunkNBTStructureValid(NBTTagCompound*);
};
#endif