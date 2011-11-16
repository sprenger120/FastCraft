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

#include "ChunkMath.h"
#include "Constants.h"

ChunkCoordinates ChunkMath::toChunkCoords(BlockCoordinates BlockCoords) {
	ChunkCoordinates ChunkCoordinates;


	ChunkCoordinates.X = BlockCoords.X >> 4;
	ChunkCoordinates.Z = BlockCoords.Z >> 4;

	return ChunkCoordinates;
}


BlockCoordinates ChunkMath::toBlockCoords(ChunkCoordinates ChunkCoords) {
	BlockCoordinates BCoords;

	BCoords.Y = 0;
	BCoords.X = ChunkCoords.X << 4;
	BCoords.Z = ChunkCoords.Z << 4;
	return BCoords;
}

int ChunkMath::toIndex(int x,short y,int z) {
	int count = y + (z * 128) + (x * 128 * 16);

	if (!isIndexInBound(count)) {
		return -1;
	}

	return count;
}

bool ChunkMath::isIndexInBound(int index) {
	if (index > FC_CHUNK_BLOCKCOUNT) {
		return false;
	}else{
		return true;
	}
}