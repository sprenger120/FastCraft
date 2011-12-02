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

#ifndef _FASTCRAFTHEADER_CHUNKROOT
#define _FASTCRAFTHEADER_CHUNKROOT
#include <vector>
#include <iostream>

using std::endl;
using std::vector;

struct MapChunk;

class ChunkRoot {
private:
	int _iChunkSlots; //Actual size of chunk vecotr
	int _iUsedChunkSlots;
	const int _iMaxChunkSlots; //chunk vector limit

	vector<MapChunk*> _vpChunks;
public:
	ChunkRoot();
	~ChunkRoot();

	void generateMap(int, //From X
		int, //From Z
		int, //To X
		int //To Z
		);

	void generateMap(int, //X
		int //Z
		);
	
	MapChunk* getChunk(int, //X
				int //Z
		);
private:
	int getChunkIndexByCoords(int, //X
		int //Z
		);
	int getFreeChunkSlot();
	int getFreeChunkSlotCount(); 
	int getUsedChunkSlotCount();
};




#endif
