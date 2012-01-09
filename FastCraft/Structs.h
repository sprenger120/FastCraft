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

#ifndef _FASTCRAFTHEADER_STRUCTS
#define _FASTCRAFTHEADER_STRUCTS
#include "Constants.h"

struct BlockCoordinates {
	int X;
	char Y;
	int Z;
};

struct ChunkCoordinates {
	int X;
	int Z;
};

struct MapChunk {
	char Blocks[FC_CHUNK_BLOCKCOUNT];
	char Metadata[FC_CHUNK_NIBBLECOUNT];
	char BlockLight[FC_CHUNK_NIBBLECOUNT];
	char SkyLight[FC_CHUNK_NIBBLECOUNT];
};

struct Block {
	char BlockID;
	char Metadata;
};

struct Enchantment {
	short EnchID;
	short Level;
};

union IntToFloat {
	int i;
	float d;
};

union Int64ToDouble {
	long long i;
	double d;
};
#endif