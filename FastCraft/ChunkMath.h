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

#ifndef _FASTCRAFTHEADER_CHUNKMATH
#define _FASTCRAFTHEADER_CHUNKMATH
#include "Structs.h"
#include "EntityCoordinates.h"

class ChunkMath {
private:
	ChunkMath();
	~ChunkMath();
public:
	/*
	* Converts BlockCoordinates to ChunkCoordinates

	Parameter:
	@1 : BlockCoordinates to convert
	*/
	static ChunkCoordinates toChunkCoords(BlockCoordinates);
	
	
	/*
	* Converts EntityCoordinaets to ChunkCoordinates

	Parameter:
	@1 : EntityCoordinates to convert
	*/
	static ChunkCoordinates toChunkCoords(EntityCoordinates);
	
	
	/*
	* Converts ChunkCoordinates to BlockCoordinates

	Parameter:
	@1 : ChunkCoordinates to convert
	*/
	static BlockCoordinates toBlockCoords(ChunkCoordinates);


	/*
	*  Converts EntityCoordinates to BlockCoordinates

	Parameter:
	@1 : EntityCoordinates to convert
	*/
	static BlockCoordinates toBlockCoords(EntityCoordinates);


	/*
	* Calculates distance between two chunks

	Parameter:
	@1 : Chunk 1
	@2 : Chunk 2
	*/
	static int Distance(ChunkCoordinates,ChunkCoordinates);
	

	/*
	* Calculates block index from chunk internal coordinates

	Parameter:
	@1 : X in chunk internal coordinates
	@2 : Y in chunk internal coordinates
	@3 : Z in chunk internal coordinates
	*/
	static int toIndex(int,short,int); //Only for full chunks


	/*
	* Checks if block is in chunk bound

	Parameter:
	@1 : Index to check
	*/
	static bool isIndexInBound(int);


	/*
	* Converts a WorldCoordiantes value (X or Z) to it's corresponding chunk internal coordiante valie ( X or Z )

	Parameter:
	@1 : a WorldCoordiantes value ( X or Z )
	*/
	static int toChunkInternal(int);
};

#endif