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
#include "NBTBase.h"
#include <Poco\ByteOrder.h>
#include "NBTConstants.h"
#include "FCRuntimeException.h"

NBTBase::~NBTBase() {

}





void NBTBase::read(istream& rStream,bool fReadHeader) { 
	char iTag = -1;
	
	do {
		try {
			rStream.read(&iTag,1);
			switch(iTag) {
				case FC_NBT_TYPE_BYTE:

					break;
				case FC_NBT_TYPE_SHORT:
				case FC_NBT_TYPE_INT:
				case FC_NBT_TYPE_INT64:
				case FC_NBT_TYPE_FLOAT:
				case FC_NBT_TYPE_DOUBLE:
				case FC_NBT_TYPE_BYTEARRAY:
				case FC_NBT_TYPE_STRING:
				case FC_NBT_TYPE_LIST:
				case FC_NBT_TYPE_COMPOUND:
				case FC_NBT_TYPE_INTARRAY:
				case 0:
			default: 
				throw FCRuntimeException("Unknown Tag");
			}
		}catch(FCRuntimeException& ex) {
			ex.rethrow();
		}
	}while(iTag != 0);
}

void NBTBase::write(ostream& rStream,bool fWriteHeader) {
	char sData[4]= {0,0,0};
	rStream.write(sData,3);
	
	for (auto it = _tagMap.begin();it != _tagMap.end();it++) {
		it->second->write(rStream,fWriteHeader);
	}
}

void NBTBase::writeString(ostream& rStream,string& rSource) {
	if (rSource.empty()) {return;}
	short iSize = Poco::ByteOrder::flipBytes((Poco::Int16)rSource.size());

	rStream.write((char*)&iSize,2);
	rStream.write(rSource.c_str(),rSource.length());
}

bool NBTBase::readString(istream& rStream,string& rTarget) {
	short iSize;

	/* lenght */
	rStream.read((char*)&iSize,2);
	if (rStream.eof()) {return false;}

	iSize = Poco::ByteOrder::flipBytes((Poco::Int16)iSize);
	if (iSize < 0) {return false;}
	if (iSize == 0) {
		rTarget.assign("");
		return true;
	}


	/* content */
	char* pBuffer = new char[iSize];
	rStream.read(pBuffer,iSize);
	if (rStream.eof()){
		delete [] pBuffer;
		return false;
	}

	rTarget.assign(pBuffer,iSize);
	delete [] pBuffer;

	return true;
}

void NBTBase::writeInt(ostream& rStream,int iSource) {
	iSource = Poco::ByteOrder::flipBytes((Poco::Int32)iSource);
	rStream.write((char*)&iSource,4);
}

bool NBTBase::readInt(istream& rStream,int& rTarget) {
	int iData;
	
	rStream.read((char*)&iData,4);
	if (rStream.eof()) {return false;}

	rTarget = Poco::ByteOrder::flipBytes((Poco::Int32)iData);
	return true;
}