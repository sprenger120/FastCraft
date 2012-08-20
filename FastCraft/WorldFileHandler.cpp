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
#include "WorldFileHandler.h"
#include "MapChunk.h"
#include <Poco\File.h>
#include "FCRuntimeException.h"
#include <Poco\String.h>
#include <cstring>
#include <Poco\ByteOrder.h>
#include <Poco\RegularExpression.h>
#include <Poco\NumberParser.h>
#include "ScopePtrArray.h"
#include "NBTAll.h"
#include <sstream>

using std::cout;
using Poco::File;

WorldFileHandler::WorldFileHandler(Path Path) : 
	_regionDirectory(Path)
{
	_regionDirectory.pushDirectory("region");
	Poco::File file(_regionDirectory);
	if (!file.exists()) {throw FCRuntimeException("Doesn't exist");}

	std::vector<Poco::File> vFileList(0);
	file.list(vFileList);

	if (vFileList.empty()) {return;}

	int iSize,i,x;
	string sPath;

	NBTBinaryParser parser;
	ScopePtrArray<char> Buffer(4096*255);

	for (x=0;x<=vFileList.size()-1;x++) {
		/*  Check filename */
		iSize = vFileList[x].path().size();
		sPath.assign(vFileList[x].path());


		if (iSize < 4) {continue;}
		if (Poco::icompare(sPath.substr(iSize-4,4),".mca") != 0) {continue;}



		/* Extract file coordinates from filename */
		try {
			listEntry* Entry = new listEntry;

			try {
				Poco::RegularExpression RegEx("r\\.(\\-?\\d+)\\.(\\-?\\d+)\\.mca$");
				vector<string> vsMatches;

				RegEx.split(sPath,vsMatches);
				if(vsMatches.size() != 3) {throw;}

				Entry->X = Poco::NumberParser::parse(vsMatches[1]);
				Entry->Z = Poco::NumberParser::parse(vsMatches[2]);
			}catch(...) {
				throw FCRuntimeException("Unable to extract coordinates from filename");
			}

			Entry->pMutex = new Mutex;
			Entry->pStream = new IOFile(vFileList[x]);

			/* Read Header */
			if (!Entry->pStream->read((char*)&Entry->OffsetTable,4096) || 
				!Entry->pStream->read((char*)&Entry->LastAccessTable,4096)) 
			{
				memset(Entry->LastAccessTable,0,1024*4);
				memset(Entry->OffsetTable,0,1024*4);
				cout<<"Error while parsing header("<<sPath<<")\n";
				continue;
			}

			/* Switch endianess of integers */ 
			for (i = 0; i<=1023;i++) {
				Entry->OffsetTable[i] = Poco::ByteOrder::flipBytes((Poco::UInt32)Entry->OffsetTable[i])>>8;
				Entry->LastAccessTable[i] = Poco::ByteOrder::flipBytes((Poco::UInt32)Entry->LastAccessTable[i]);
			}

			_vpFiles.push_back(Entry);
		}catch(FCRuntimeException& ex) {
			cout<<"Error while parsing "<<vFileList[x].path()<<" ("<<ex.getMessage()<<")\n";
			continue;
		}
	}
}


WorldFileHandler::~WorldFileHandler() {
	listEntry* pEntry;
	while(!_vpFiles.empty()) {
		pEntry = _vpFiles.back();
		delete pEntry->pStream;
		delete pEntry->pMutex;
		delete pEntry;
		_vpFiles.pop_back();
	}
}


bool WorldFileHandler::loadChunk(MapChunk* p) {
	if (p==NULL){throw FCRuntimeException("Nullpointer");}
	int iFileCoordinateX =(int) floor(double(p->getX()) / 32.0);
	int iFileCoordinateZ =(int) floor(double(p->getZ()) / 32.0);
	char *pData = NULL;

	try {
		/* Try to find the file that should contain the chunk */
		if (!_vpFiles.empty()) {
			for(int x=0;x<=_vpFiles.size()-1;x++) {
				if (_vpFiles[x]->X == iFileCoordinateX && _vpFiles[x]->Z == iFileCoordinateZ) {
					listEntry* pEntry = _vpFiles[x];
					Poco::Mutex::ScopedLock slock(*pEntry->pMutex);

					int iArrayIndex = abs((iFileCoordinateZ*32)-p->getZ())*32 + abs((iFileCoordinateX*32)-p->getX());
					if (iArrayIndex > 1023) {return false;}

					int iSize;
					char iCompressType;

					/* Read chunk header */
					if (pEntry->OffsetTable[iArrayIndex] == 0) {return false;}

					if (pEntry->OffsetTable[iArrayIndex]*4096 + 5 > pEntry->pStream->getSize()-1) {
						cout<<"Chunk at X:"<<p->getX()<<" Z:"<<p->getZ()<<" is corrupted\n";
						pEntry->OffsetTable[iArrayIndex] = 0;
						pEntry->LastAccessTable[iArrayIndex] = 0;
						return false;
					}

					pEntry->pStream->setPosition(pEntry->OffsetTable[iArrayIndex]*4096);
					pEntry->pStream->read((char*)&iSize,4);
					pEntry->pStream->read(&iCompressType,1);
					iSize = Poco::ByteOrder::flipBytes((Poco::UInt32)iSize);


					/* Check data */ 
					if (iSize <= 0 || (iCompressType != 1 && iCompressType != 2)) {
						cout<<"Chunk at X:"<<p->getX()<<" Z:"<<p->getZ()<<" is corrupted\n";
						pEntry->OffsetTable[iArrayIndex] = 0;
						pEntry->LastAccessTable[iArrayIndex] = 0;
						return false;
					}


					/* Parse it */
					pData = new char[iSize];
					pEntry->pStream->read(pData,iSize);

					NBTBinaryParser parser;
					NBTTagCompound* pComp = parser.parse(pData,iCompressType == 1 ? FC_NBT_IO_GZIP : FC_NBT_IO_ZLIB,iSize);

					delete [] pData;
					pData = NULL;


					/* Check NBT Structure */
					if (!isChunkNBTStructureValid(pComp)) {
						delete pComp;
						cout<<"Chunk at X:"<<p->getX()<<" Z:"<<p->getZ()<<" is corrupted\n";
						pEntry->OffsetTable[iArrayIndex] = 0;
						pEntry->LastAccessTable[iArrayIndex] = 0;
						return false;
					}

					p->_pData = pComp;
					p->_pLevelCompound = dynamic_cast<NBTTagCompound*>(pComp->getElementByName("Level"));
					p->updatePointerArrays();
					return true;
					//}
				}
			}
		}
		createNewRegionFile(iFileCoordinateX,iFileCoordinateZ);
	}catch(FCRuntimeException& ex) { 
		if (pData) {delete [] pData;}
		ex.rethrow();
	}
	return false;
}

void WorldFileHandler::createNewRegionFile(int x,int z) {
	try {
		listEntry* p;
		_vpFiles.push_back(p = new listEntry);

		p->X = x;
		p->Z = z;
		p->pMutex = new Poco::Mutex;
		memset(p->LastAccessTable,0,4096);
		memset(p->OffsetTable,0,4096);

		std::stringstream sFilename;
		sFilename<<"r."<<x<<"."<<z<<".mca";
		_regionDirectory.setFileName(sFilename.str());  
		p->pStream = new IOFile(Poco::File(_regionDirectory));
	}catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
}

bool WorldFileHandler::isChunkNBTStructureValid(NBTTagCompound* pComp) { 
	NBTTagCompound* pLevelComp = dynamic_cast<NBTTagCompound*>(pComp->getElementByName("Level"));
	if (pLevelComp==NULL) {return false;}

	if (!pLevelComp->has("TerrainPopulated",FC_NBT_TYPE_BYTE) ||
		!pLevelComp->has("xPos",FC_NBT_TYPE_INT) ||
		!pLevelComp->has("zPos",FC_NBT_TYPE_INT) ||
		!pLevelComp->has("LastUpdate",FC_NBT_TYPE_INT64) ||
		!pLevelComp->has("Biomes",FC_NBT_TYPE_BYTEARRAY) ||
		!pLevelComp->has("Entities",FC_NBT_TYPE_LIST) ||
		!pLevelComp->has("Sections",FC_NBT_TYPE_LIST) ||
		!pLevelComp->has("TileEntities",FC_NBT_TYPE_LIST) ||
		!pLevelComp->has("HeightMap",FC_NBT_TYPE_INTARRAY))
	{
		return false;
	}


	/* Check Chunk compounds */
	NBTTagList* pSections = dynamic_cast<NBTTagList*>(pLevelComp->getElementByName("Sections"));
	if (!pSections->isEmpty()) {
		NBTTagCompound* pChunk;
		NBTTagByteArray* pArray;
		NBTTagByte* pByte;

		for(int x=0;x<=pSections->size()-1;x++) {
			pChunk = dynamic_cast<NBTTagCompound*>((*pSections)[x]);
			if (pChunk == NULL) {throw FCRuntimeException("NBT Structure is invalid");}

			//Existance
			if (!pChunk->has("Y",FC_NBT_TYPE_BYTE) ||
				!pChunk->has("BlockLight",FC_NBT_TYPE_BYTEARRAY) ||
				!pChunk->has("Blocks",FC_NBT_TYPE_BYTEARRAY) ||
				!pChunk->has("Data",FC_NBT_TYPE_BYTEARRAY) ||
				!pChunk->has("SkyLight",FC_NBT_TYPE_BYTEARRAY)) 
			{
				return false;
			}

			//Content & length
			pArray = dynamic_cast<NBTTagByteArray*>(pChunk->getElementByName("BlockLight"));
			if (pArray->size() != 2048) {return false;}

			pArray = dynamic_cast<NBTTagByteArray*>(pChunk->getElementByName("Blocks"));
			if (pArray->size() != 4096) {return false;}

			pArray = dynamic_cast<NBTTagByteArray*>(pChunk->getElementByName("Data"));
			if (pArray->size() != 2048) {return false;}

			pArray = dynamic_cast<NBTTagByteArray*>(pChunk->getElementByName("SkyLight"));
			if (pArray->size() != 2048) {return false;}

			pByte = dynamic_cast<NBTTagByte*>(pChunk->getElementByName("Y"));
			if (pByte->getDataRef() < 0 || pByte->getDataRef() > 16) {return false;}
		}
	}

	return true;
}