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

#include "ChunkMath.h"
#include "Constants.h"
#include <cmath>
#include <stdlib.h>
#include <iostream>

ChunkCoordinates ChunkMath::toChunkCoords(BlockCoordinates BlockCoords) {
	ChunkCoordinates ChunkCoordinates;


	ChunkCoordinates.X = BlockCoords.X >> 4;
	ChunkCoordinates.Z = BlockCoords.Z >> 4;

	return ChunkCoordinates;
}

ChunkCoordinates ChunkMath::toChunkCoords(EntityCoordinates EntityCoords) {
	ChunkCoordinates ChunkCoordinates;

	ChunkCoordinates.X = int(EntityCoords.X) >> 4;
	ChunkCoordinates.Z = int(EntityCoords.Z) >> 4;

	return ChunkCoordinates;
}


BlockCoordinates ChunkMath::toBlockCoords(ChunkCoordinates ChunkCoords) {
	BlockCoordinates BCoords;

	BCoords.Y = 0;
	BCoords.X = ChunkCoords.X << 4;
	BCoords.Z = ChunkCoords.Z << 4;
	return BCoords;
}


int ChunkMath::Distance(ChunkCoordinates c1,ChunkCoordinates c2) {
	double distance;

	distance = sqrt(  double ((c1.X-c2.X) * (c1.X-c2.X) + 
					  (c1.Z-c2.Z) * (c1.Z-c2.Z)
				   ));


	return int(distance);
}

int ChunkMath::toIndex(int x,int y,int z) {
	return x + (z<<4) + (y<<8);

	//x + (z * Width) + (y * Height * Width); 
}

bool ChunkMath::isIndexInBound(int index) {
	if (index > (FC_CHUNK_BLOCKCOUNT-1) || index < 0) {
		return false;
	}else{
		return true;
	}
}

int ChunkMath::toChunkInternal(int n) {
	return  n - ((n>>4)<<4);
}

BlockCoordinates ChunkMath::toBlockCoords(EntityCoordinates eCoords) {
	BlockCoordinates blockCoords;

	blockCoords.X = (int)floor(eCoords.X);
	blockCoords.Y = (int)floor(eCoords.Y);
	blockCoords.Z = (int)floor(eCoords.Z);
	return blockCoords;
}