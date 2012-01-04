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


#ifndef _FASTCRAFTHEADER_WORLD
#define _FASTCRAFTHEADER_WORLD
#include <vector>
#include <string>
#include <utility>
#include <Poco/Mutex.h>
#include "Structs.h"

using std::vector;
using std::string;
using std::pair;

struct ChunkInternal {
	int X;
	int Z;
	MapChunk* Chunk;
};

class World {
private:
	vector<vector<ChunkInternal>> _vChunks; //[x][z]
	int _iLoadedChunks;
	string _WorldName;
	Poco::Mutex _Mutex;
public:
	/*
	* Constructs a new World
	* Generates 20x20 chunks around 0,0

	Parameter:
	@1 : World name
	*/
	World(string);

	/*
	* Destruct world object
	* Will free map
	* Will save map to hdd *soon*
	*/
	~World();


	/*
	* Look for a chunk and returns a pointer to it
	* Generates chunk if not existing

	Paramter:
	@1 : X in chunkcoordinates
	@2 : Z in chunkcoordiantes
	*/
	MapChunk* getChunkByChunkCoordinates(int,int);


	/*
	* Converts WorldCoordinates to chunk coordinates and block index
	* Will not check chunk existance
	* Will not generate chunk
	* Will throw Poco::RuntimeException if Y is out of bound

	Parameter:
	@1 : X in world coordiantes
	@2 : Y in world coordiantes
	@3 : Z in world coordiantes
	*/
	static pair<ChunkCoordinates,int> WorldCoordinateConverter(int,short,int);
private:
	void generateChunks(int,int,int,int);
	MapChunk* generateChunk(int,int);
};

#endif