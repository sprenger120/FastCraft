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
#include "World.h"
#include "ChunkTerraEditor.h"
#include "Constants.h"
#include "SettingsHandler.h"
#include "ChunkMath.h"
#include "ItemInfoStorage.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <math.h>
#include <Poco/Exception.h>
#include <Poco/ScopedLock.h>
#include "PlayerEvents.h"
#include "PlayerPool.h"
using std::cout;


World::World(string Name,char iDimension,PlayerPool& rPP) :
_WorldName(Name),
	_iDimension(iDimension),
	_rPlayerPool(rPP),
	_vChunks(0)
{
	_iLoadedChunks = 0;
	generateChunks(-10,-10,10,10);
}


World::~World() {
	cout<<"destruct!"<<"\n";
	//Free chunks
	for (int x=0;x<=_vChunks.size()-1;x++) {
		for(int z=0;z<=_vChunks[x].size()-1;z++) {
			delete _vChunks[x][z].Chunk;
		}
	}
}

void World::generateChunks(int FromX,int FromZ,int ToX,int ToZ) {
	try {
		for (int x=FromX;x<=ToX;x++) {
			for(int z=FromZ;z<=ToZ;z++) {
				generateChunk(x,z);
			}
		}
	} catch(Poco::RuntimeException) {
		std::cout<<"World::generateChunks Chunk generation aborted due a error"<<"\n";
	}
}

MapChunk* World::generateChunk(int X,int Z) {
	bool fXRowExisting=false;
	int XRow,ZRow; //Index of chunk in list
	MapChunk* pAffectedChunk=NULL;

	Poco::Mutex::ScopedLock ScopedLock(_Mutex);

	/*
	Reserve a new place for chunk
	*/
	//Look for a existing x row 
	if (_vChunks.size() > 0) {
		for (int x=0;x<=_vChunks.size()-1;x++) {
			if (_vChunks[x][0].X == X) {
				//There is a existing row		
				//Add a new chunk
				ChunkInternal Chk;

				Chk.X=X;
				Chk.Z=Z;
				Chk.Chunk = pAffectedChunk = new MapChunk; 

				if (Chk.Chunk == NULL) {
					std::cout<<"World::generateChunk unable to allocate memory"<<"\n";
					throw Poco::RuntimeException("Generation failed!");
				}

				_vChunks[x].push_back(Chk);//Add to array

				XRow=x;
				ZRow=_vChunks[x].size()-1;
				fXRowExisting=true;
			}
		} 	
	}

	//Doesn't exist
	if (!fXRowExisting) {
		vector<ChunkInternal> vZRow(1); 

		vZRow[0].Chunk = pAffectedChunk = new MapChunk;
		vZRow[0].X=X;
		vZRow[0].Z=Z;

		if (vZRow[0].Chunk == NULL) {
			std::cout<<"World::generateChunk unable to allocate memory"<<"\n";
			throw Poco::RuntimeException("Generation failed!");
		}

		_vChunks.push_back(vZRow);
		XRow = _vChunks.size()-1;
		ZRow = 0;
	}


	/*
	* Generate Chunk
	*/
	Block b;
	b.BlockID = 7;

	std::memset(pAffectedChunk->Blocks,0,FC_CHUNK_BLOCKCOUNT); //Clear blocks
	try {
		ChunkTerraEditor::setPlate(pAffectedChunk,0,b); //Bedrock 

		b.BlockID = 35;

		for (short y=1;y<=70;y++) {
			ChunkTerraEditor::setPlate(pAffectedChunk,y,b);
		}
	} catch (Poco::RuntimeException& err) {
		std::cout<<"World::generateChunk unable to generate chunk ("<<err.message()<<")"<<"\n";

		//Erase chunk from list
		_vChunks[XRow].erase(_vChunks[XRow].begin() + ZRow);
		if (_vChunks[XRow].size() == 0) {
			_vChunks.erase(_vChunks.begin() + XRow);
		}

		throw Poco::RuntimeException("Generation failed!");
	}

	//Light & Metadata
	std::memset(pAffectedChunk->Metadata,0x00,FC_CHUNK_NIBBLECOUNT);
	std::memset(pAffectedChunk->BlockLight,0x00,FC_CHUNK_NIBBLECOUNT);
	std::memset(pAffectedChunk->SkyLight,0xFF,FC_CHUNK_NIBBLECOUNT);
	_iLoadedChunks++;

	return pAffectedChunk;
}


MapChunk* World::getChunkByChunkCoordinates(int X,int Z) {
	if (_vChunks.empty()) {
		cout<<"***No chunks generated!"<<"\n";
		return NULL;
	}
	for (int x=0;x<=_vChunks.size()-1;x++) { //Search in X rows
		if (_vChunks[x].empty()) {continue;}
		if (_vChunks[x][0].X == X) { //Row found
			for (int z=0;z<=_vChunks[x].size()-1;z++) { //Search z entry
				if (_vChunks[x][z].Z ==Z) { //found
					if (_vChunks[x][z].Chunk == NULL) {
						cout<<"World::getChunkByChunkCoordinates invalid ptr!"<<"\n";
					}
					return _vChunks[x][z].Chunk;
				}
			}
			break; 
		}
	}


	MapChunk* p;

	//Try to generate chunk
	try  {
		p = generateChunk(X,Z);
	} catch(Poco::RuntimeException& ex) {
		std::cout<<"World::getChunkByChunkCoordinates Chunk generation aborted due a error"<<"\n";
		ex.rethrow();
	}
	return p;
}

pair<ChunkCoordinates,int> World::WorldCoordinateConverter(int X,short Y,int Z) {
	ChunkCoordinates Coords;
	int Index;

	if (Y < 0 || Y > SettingsHandler::getWorldHeight()-1) {
		std::cout<<"World::WorldCoordinateConverter  Y is invalid"<<"\n";
		throw Poco::RuntimeException("Y is invalid");
	}

	Coords.X = X>>4;
	Coords.Z = Z>>4;

	X = ChunkMath::toChunkInternal(X);
	Z = ChunkMath::toChunkInternal(Z);	

	try {
		Index = ChunkMath::toIndex(X,Y,Z);
	}catch(Poco::RuntimeException) {
		std::cout<<"World::WorldCoordinateConverter index error"<<"\n";
		throw Poco::RuntimeException("Index error");
	}


	pair<ChunkCoordinates,int> Pair(Coords,Index);
	return Pair;
}

char World::getFreeSpace(int X,int Z) {
	int ChunkX = X>>4;
	int ChunkZ = Z>>4;
	MapChunk* pChunk;

	try {
		pChunk = getChunkByChunkCoordinates(ChunkX,ChunkZ);
	}catch (Poco::RuntimeException& ex) {
		std::cout<<"World::getFreeSpace chunk not found"<<"\n";
		ex.rethrow();
	}

	X = ChunkMath::toChunkInternal(X);
	Z = ChunkMath::toChunkInternal(Z);	


	int iOffset = ChunkMath::toIndex(X,0,Z);
	unsigned char y;
	if (iOffset==-1) {
		std::cout<<"World::getFreeSpace could not calculate index"<<"\n";
		throw Poco::RuntimeException("toIndex error");
	}

	//Get height
	for (y=SettingsHandler::getWorldHeight()-1;y>0;y--) { //For from 128 -> 1
		if (ItemInfoStorage::isSolid(pChunk->Blocks[iOffset+y])) {
			return y+1;
		}
	}

	return SettingsHandler::getWorldHeight();
}

bool World::isSuffocating(EntityCoordinates Coords) {
	if (Coords.Y <= 0.9) {
		return true;
	}

	MapChunk* pChunk;

	try {
		pChunk = getChunkByChunkCoordinates(int(Coords.X)>>4,int(Coords.Z)>>4);
	}catch (Poco::RuntimeException& ex) {
		std::cout<<"World::isSuffocating chunk not found"<<"\n";
		ex.rethrow();
	}

	if (Coords.Y > (double)SettingsHandler::getWorldHeight()) {
		return false;
	}

	int iOffset = ChunkMath::toIndex( 
		ChunkMath::toChunkInternal((int)floor(Coords.X)),
		(int)floor(Coords.Y),
		ChunkMath::toChunkInternal((int)floor(Coords.Z))
		);

	return ItemInfoStorage::isSolid((short)pChunk->Blocks[iOffset]);
}

void World::setBlock(int X,short Y,int Z,ItemID Block) {
	if (!ItemInfoStorage::isRegistered(Block)) {
		throw Poco::RuntimeException("Block not registered");
	}

	MapChunk* p;

	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		int iNibbleIndex = Coords.second/2;

		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		
		p->Blocks[Coords.second] = (char)Block.first; //Set Block
		p->Metadata[iNibbleIndex] = prepareNibble(  //Set Metadata
												char(Coords.second%2),
												p->BlockLight[iNibbleIndex],
												Block.second
												);

		//Push event
		BlockCoordinates BlockCoords;
		BlockCoords.X = X;
		BlockCoords.Y = (char)Y;
		BlockCoords.Z = Z;
		
		PlayerEventBase* p = new PlayerSetBlockEvent(NULL,BlockCoords,Block);
		_rPlayerPool.addEvent(p);
	} catch (Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}



ItemID World::getBlock(int X,short Y,int Z) {
	MapChunk* p;

	try {
		auto Coords = WorldCoordinateConverter(X,Y,Z);
		p = getChunkByChunkCoordinates(Coords.first.X,Coords.first.Z);
		
		ItemID Data;
		Data.first = p->Blocks[Coords.second];
		Data.second = p->Metadata[Coords.second/2];

		if (Coords.second%2) {
			 Data.second = (Data.second>>4) & 15;
		}else{
			 Data.second &= 15;
		}

		return Data;
	} catch (Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return std::make_pair(0,0);
}

ItemID World::getBlock(BlockCoordinates Coords) {
	return getBlock(Coords.X,(short)Coords.Y,Coords.Z);
} 

bool World::isSurroundedByAir(BlockCoordinates TargetBlock) {
	BlockCoordinates Temp = TargetBlock;

	try {

		//X--
		if (getBlock(Temp.X-1,Temp.Y,Temp.Z).first != 0) {
			return false;
		}

		//X++
		if (getBlock(Temp.X+1,Temp.Y,Temp.Z).first != 0) {
			return false;
		}

		//Z--
		if (getBlock(Temp.X,Temp.Y,Temp.Z-1).first != 0) {
			return false;
		}

		//Z++
		if (getBlock(Temp.X,Temp.Y,Temp.Z+1).first != 0) {
			return false;
		}

		//Y--
		if (TargetBlock.Y >= 1) {
			if (getBlock(Temp.X,Temp.Y-1,Temp.Z).first != 0) {
				return false;
			}
		}

		//Y++
		if (TargetBlock.Y <= SettingsHandler::getWorldHeight()-1) {
			if (getBlock(Temp.X,Temp.Y+1,Temp.Z).first != 0) {
				return false;
			}
		}

	}catch(Poco::RuntimeException& ex) {
		cout<<"Exception World::isSurroundedByAir: "<<ex.message()<<"\n";
	}
	return true;
}

void World::Load(Poco::Path worldPath) {
	cout<<"Loading: "<<worldPath[worldPath.depth()-1]<<"\n";
}

string World::getName() {
	return _WorldName;
}

char World::getDimension() {
	return _iDimension;
}

char World::prepareNibble(char iMod,char iOld,char iNew) {
	iNew &= 15; //Filter bottom 4 bit 

	if (iMod) { //Set top 4 bits
		iOld |= iNew<<4;
	}else{      //Set lower 4 bits
		iOld |= iNew;
	}	
	return iOld;
}


void World::setBlockLight(int X,short Y,int Z,char iLightLevel) {
	try {
		auto coord = WorldCoordinateConverter(X,Y,Z);
		if (coord.second==-1) {
			cout<<"World::setBlockLight invalid coordinates\n";
			throw Poco::RuntimeException("Invalid coordinates");
		}
		if (iLightLevel<0 || iLightLevel > 15) {
			cout<<"World::setBlockLight invalid light level\n";
			throw Poco::RuntimeException("Invalid light level");
		}

		MapChunk* p = getChunkByChunkCoordinates(coord.first.X,coord.first.Z);
		int iNibbleIndex = coord.second/2;


		p->BlockLight[iNibbleIndex] = prepareNibble( 
												char(coord.second%2),
												p->BlockLight[iNibbleIndex],
												iLightLevel
												);
	} catch (Poco::RuntimeException& ex) {
		ex.rethrow();
	}
}

void World::setBlockLight(BlockCoordinates Coords,char iLL) {
	setBlockLight(Coords.X,Coords.Y,Coords.Z,iLL);
}

void World::setBlock(BlockCoordinates Coords,ItemID Data) {
	setBlock(Coords.X,Coords.Y,Coords.Z,Data);
}