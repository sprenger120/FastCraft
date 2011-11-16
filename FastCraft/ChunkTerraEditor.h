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

#ifndef _FASTCRAFTHEADER_CHUNKTERRAEDITING
#define _FASTCRAFTHEADER_CHUNKTERRAEDITING

#include "Structs.h"

class ChunkTerraEditor {
private:
	ChunkTerraEditor();
	~ChunkTerraEditor();
public:
	static void setPlate(MapChunk*,short,Block&); //Handle to a chunk, y coordinate
};
#endif