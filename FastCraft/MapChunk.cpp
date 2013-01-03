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
#include "MapChunk.h"
#include "FCException.h"
#include <Poco\ScopedLock.h>
#include "ChunkMath.h"
#include "NetworkOut.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include "NBTAll.h"
#include <bitset>


using Poco::Mutex;
using std::stringstream;
using Poco::DeflatingOutputStream;

MapChunk::MapChunk(int X,int Z) {
	_iX = X;
	_iZ = Z;
	_pData = NULL;
	_pBiomeArray = NULL;
	_pLevelCompound = NULL;
	_iLastAccess = _timestamp.epochTime();
}

MapChunk::~MapChunk() {
	if (_pData) {delete _pData;}
	std::cout<<"chunk saving is not implemented\n";
}

int MapChunk::getX() {
	return _iX;
}

int MapChunk::getZ() {
	return _iZ;
}

bool MapChunk::isLoaded() {
	return _pData != NULL;
}

time_t MapChunk::getLastAccess() {
	return _iLastAccess;
}

void MapChunk::setBlock(ItemID& item,char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}

	Mutex::ScopedLock scopelock(_Mutex);
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	_vpBlocks[iPtrArrIndex].first[index] = (char)item.first;
	setNibble(index,item.second,_vpMetadata[iPtrArrIndex]);
}

ItemID MapChunk::getBlock(char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}
	
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	return ItemID(_vpBlocks[iPtrArrIndex].first[index],getNibble(index,_vpMetadata[iPtrArrIndex]));
}

void MapChunk::setBlockLight(char iLvl,char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}

	Mutex::ScopedLock scopelock(_Mutex);
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	setNibble(index,iLvl,_vpBlockLight[iPtrArrIndex]);
}

char MapChunk::getBlockLight(char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}
	
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	return getNibble(index,_vpBlockLight[iPtrArrIndex]);
}

void MapChunk::setSkyLight(char iLvl,char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}

	Mutex::ScopedLock scopelock(_Mutex);
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	setNibble(index,iLvl,_vpSkyLight[iPtrArrIndex]);
}

char MapChunk::getSkyLight(char X,short Y,char Z) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	if (X < 0 || Z < 0 || X > 15 || Z > 15 || Y < 0 || Y > 255) {throw FCException("Illegal coordinates");}
	
	_iLastAccess = _timestamp.epochTime();

	char iPtrArrIndex = getSegment(Y>>4);
	int index = ChunkMath::toIndex(X,Y%16,Z);
	
	return getNibble(index,_vpSkyLight[iPtrArrIndex]);
}

void MapChunk::setNibble(int iBlockOffset,char iNewData,char* apArray) {
	if (iBlockOffset < 0) {throw FCException("Illegal block offset");}
	int iNibbleOffset = iBlockOffset/2;

	if (iBlockOffset&1) { //Higher 4-bits   &1 is the same as %2
		apArray[iNibbleOffset] |= iNewData<<4;
	}else{//Lower 4-bits
		apArray[iNibbleOffset] |= iNewData & 0xF;
	}
}

char MapChunk::getNibble(int iBlockOffset,char* apArray) {
	if (iBlockOffset < 0) {throw FCException("Illegal block offset");}
	int iNibbleOffset = iBlockOffset/2;

	if (iBlockOffset&1) { //Higher 4-bits
		return apArray[iNibbleOffset]>>4;
	}else{//Lower 4-bits
		return apArray[iNibbleOffset]&0xF;
	}
}

void MapChunk::send(NetworkOut& Out) {
	if (!isLoaded()) {throw FCException("Not loaded");}

	try {
		char iSize = _vpBlocks.size();
		if (iSize == 0) {return;}


		std::stringstream _stringStrm;
		Poco::DeflatingOutputStream _deflatingStrm(_stringStrm,Poco::DeflatingStreamBuf::STREAM_ZLIB,-1);
		
		short iPrimaryBitmap = packData(_deflatingStrm);
		_deflatingStrm.flush();

		/* Pack */
		Out.addByte(0x33);
		Out.addInt(_iX);
		Out.addInt(_iZ);
		Out.addBool(true);

		Out.addShort(iPrimaryBitmap);
		Out.addShort(0);

		Out.addInt(_stringStrm.str().length());
		Out.getStr().append(_stringStrm.str());

		Out.Finalize(FC_QUEUE_LOW);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

short MapChunk::packData(Poco::DeflatingOutputStream& rStrm) {
	if (!isLoaded()) {throw FCException("Not loaded");}
	char iSize = _vpBlocks.size();
	if (iSize == 0) {return 0;}

	std::bitset<16> primaryBitset;
	vector<pair<char*,short>> vArrays;

	primaryBitset.reset();
	vArrays.resize(iSize*4);

	for(char x = 0;x<=iSize-1;x++) {
		primaryBitset.set(_vpBlocks[x].second,true);
		vArrays[x]			 = std::make_pair(_vpBlocks[x].first,4096);
		vArrays[x+iSize]	 = std::make_pair(_vpMetadata[x]	,2048);
		vArrays[x+(iSize*2)] = std::make_pair(_vpBlockLight[x]	,2048);
		vArrays[x+(iSize*3)] = std::make_pair(_vpSkyLight[x]	,2048);
	}
	for (short x = 0;x<=(iSize*4)-1;x++) {
		rStrm.write(vArrays[x].first,vArrays[x].second);
	}
	rStrm.write(_pBiomeArray,256);

	return (short)primaryBitset.to_ulong();
}

void MapChunk::unload(WorldFileHandler* p) {
	throw FCException("not implemented");
}

void MapChunk::save(WorldFileHandler* p) {
	throw FCException("not implemented");
}

void MapChunk::alloc() {
	if (isLoaded()) {return;}
	NBTTagByte* Byte;
	NBTTagInt* Int;
	NBTTagByteArray* ByteArray;

	_pData = _pLevelCompound = new NBTTagCompound("Level");

	_pData->addSubElement(Byte = new NBTTagByte("TerrainPopulated"));
	Byte->getDataRef() = 1;

	_pData->addSubElement(Int = new NBTTagInt("xPos"));
	Int->getDataRef() = _iX;

	_pData->addSubElement(Int = new NBTTagInt("zPos"));
	Int->getDataRef() = _iZ;

	_pData->addSubElement(ByteArray = new NBTTagByteArray("Biomes",_pBiomeArray = new char[256],256));
	memset(_pBiomeArray,0,256);

	_pData->addSubElement(new NBTTagList("Entities",FC_NBT_TYPE_COMPOUND));
	_pData->addSubElement(new NBTTagList("Sections",FC_NBT_TYPE_COMPOUND));
	_pData->addSubElement(new NBTTagList("TitleEntities",FC_NBT_TYPE_COMPOUND));
	_pData->addSubElement(new NBTTagIntArray("HeightMap",256));
}

void MapChunk::updatePointerArrays() {
	_vpBlocks.clear();
	_vpMetadata.clear();
	_vpBlockLight.clear();
	_vpSkyLight.clear();

	NBTTagList* pSections = dynamic_cast<NBTTagList*>(_pLevelCompound->getElementByName("Sections"));
	char iSize = 0;
	if ((iSize = pSections->size()) == 0) {return;}

	_vpBlocks.resize(iSize);
	_vpMetadata.resize(iSize);
	_vpBlockLight.resize(iSize);
	_vpSkyLight.resize(iSize);

	NBTTagCompound* pChunkPart;
	char iPos;

	for(char x = 0;x<=iSize-1;x++) {
		pChunkPart = dynamic_cast<NBTTagCompound*>((*pSections)[x]);
		iPos = dynamic_cast<NBTTagByte*>(pChunkPart->getElementByName("Y"))->getDataRef();

		_vpBlocks[x]		= std::make_pair(((NBTTagByteArray*)pChunkPart->getElementByName("Blocks"))->getDataRef(),iPos);
		_vpMetadata[x]		= ((NBTTagByteArray*)pChunkPart->getElementByName("Data"))->getDataRef();
		_vpBlockLight[x]	= ((NBTTagByteArray*)pChunkPart->getElementByName("BlockLight"))->getDataRef();
		_vpSkyLight[x]		= ((NBTTagByteArray*)pChunkPart->getElementByName("SkyLight"))->getDataRef();
	}
	_pBiomeArray = ((NBTTagByteArray*)_pLevelCompound->getElementByName("Biomes"))->getDataRef();
}

void MapChunk::addSegment() {
	if (_vpBlocks.size() == 16) {return;} //Maxmal amount of chunks

	char iY = -1;
	if (!_vpBlocks.empty()) {
		for (char x=0;x<=_vpBlocks.size()-1;x++) {
			if (_vpBlocks[x].second > iY) {iY = _vpBlocks[x].second;}
		}
		iY++;
	}else{
		iY = 0;
	}

	char* pBlocks	= new char[4096];
	char* pMetadata = new char[2048]; 
	char* pBlockLight = new char[2048]; 
	char* pSkyLight = new char[2048]; 

	memset(pBlocks,0,4096);
	memset(pMetadata,0,2048);
	memset(pBlockLight,0,2048);
	memset(pSkyLight,0xff,2048);

	
	//Add to pointer arrays
	_vpBlocks.push_back(std::make_pair(pBlocks,iY));
	_vpMetadata.push_back(pMetadata);
	_vpBlockLight.push_back(pBlockLight);
	_vpSkyLight.push_back(pSkyLight);


	//Create chunk part NBT structure
	NBTTagCompound* pChunkPart = new NBTTagCompound("");
	NBTTagByte* pByte;

	pChunkPart->addSubElement(pByte = new NBTTagByte("Y"));
	pByte->getDataRef() = iY;

	pChunkPart->addSubElement(new NBTTagByteArray("Blocks",pBlocks,4096));
	pChunkPart->addSubElement(new NBTTagByteArray("Data",pMetadata,2048));
	pChunkPart->addSubElement(new NBTTagByteArray("BlockLight",pBlockLight,2048));
	pChunkPart->addSubElement(new NBTTagByteArray("SkyLight",pSkyLight,2048));


	//Add to Sections
	NBTTagList* pSections = dynamic_cast<NBTTagList*>(_pLevelCompound->getElementByName("Sections"));
	pSections->addSubElement(pChunkPart);
}

char MapChunk::getSegment(char iSearchedSegment) {
	Poco::Mutex::ScopedLock lock(_segmentCreation);

	if (_vpBlocks.empty()) {
		addSegment();
		return 0;
	}else{
		for (char x=0;x<=_vpBlocks.size()-1;x++) {
			if (_vpBlocks[x].second == iSearchedSegment) {
				return x;
			}
		}
		addSegment();
		return char(_vpBlocks.size()-1);
	}
	return 0; //Compiler warning fix
}

bool MapChunk::isEmpty() { 
	return _vpBlocks.empty();
}
