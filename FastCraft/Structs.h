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
//Various Data Container 
#include <iostream>
#include "Constants.h"

class PlayerThread;

struct EntityCoordinates {
	double X;
	double Y;
	double Z;
	double Stance;
	float Yaw;
	float Pitch;
	bool OnGround;
};

struct BlockCoordinates {
	int X;
	int Y;
	int Z;
};

struct ChunkCoordinates {
	int X;
	int Z;
};


struct EntityType {
	int EntityID;
	char Type;
};

struct MapChunk {
	int X;
	int Z;
	char Blocks[FC_CHUNK_BLOCKCOUNT];
	char Metadata[FC_CHUNK_NIBBLECOUNT];
	char BlockLight[FC_CHUNK_NIBBLECOUNT];
	char SkyLight[FC_CHUNK_NIBBLECOUNT];
	bool Empty;
};

struct Block {
	char BlockID;
	char Metadata;
};

struct PlayerPoolEvent {
	std::string Message; //Used for chat 
	EntityCoordinates Coordinates; //Used for Move and chat
	char Job;
	int ID; //Used for Animation
	PlayerThread* pThread; //Pointer to affected class
};

struct Enchantment {
	short EnchID;
	short Level;
};
#endif