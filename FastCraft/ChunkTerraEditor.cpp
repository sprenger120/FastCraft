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

#include "ChunkTerraEditor.h"
#include "ChunkMath.h"
#include <Poco/Exception.h>

void ChunkTerraEditor::setPlate(MapChunk* pChunk,short y,Block& Block) {
	int index;
	char * pBlockArray = pChunk->Blocks;

	for (int x=0;x<=15;x++) {
		for (int z=0;z<=15;z++) {
			index = ChunkMath::toIndex(x,y,z);
			if (index==-1) {
				throw Poco::RuntimeException("Chunk index out of bound");
			}
			pBlockArray[index] = Block.BlockID;
		}
	}
}
