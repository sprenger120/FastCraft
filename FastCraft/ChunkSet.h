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

#ifndef _FASTCRAFTHEADER_CHUNKSET
#define _FASTCRAFTHEADER_CHUNKSET

#include <vector>
#include "Structs.h"
using std::vector;

class ChunkSet {
private: 
	vector<ChunkCoordinates> _vChunkSet;
	ChunkCoordinates _PlayerCoordinates;

	int _iViewDistance;
	bool _fClear;
public:
	ChunkSet(int);
	~ChunkSet();

	void regenerate(ChunkCoordinates); //Regenerates ChunkSet
	bool isUp2Date(ChunkCoordinates);

	void clear();
	bool isEmpty();

	ChunkCoordinates at(int);
};
#endif