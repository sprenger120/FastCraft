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

#ifndef _FASTCRAFTHEADER_CHUNKPROVIDER
#define _FASTCRAFTHEADER_CHUNKPROVIDER

#include "Structs.h"
#include <vector>

class PlayerThread;

class ChunkProvider {
private:
	std::vector<MapChunk*> _vMapChunks;
	int _iAllocatedChunkCount;
public:
	ChunkProvider(int); //Chunk Count 
	~ChunkProvider();

	void generateMap(
		int, //From X
		int, //From Z
		int, //To X
		int  //To Z
		);


	//Generates pre chunks based on player position and add it to its send queue
	void generatePreChunks(PlayerThread*); 
private:
	void ClearChunk(int);
	int getChunkIndexByCoords(int,int); //Returns -1 if chunk wasnt found
	int getFreeChunkSlot(); //Returns -1 if no chunk slots are free
};
#endif