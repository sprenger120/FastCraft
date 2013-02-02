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
#include "NBTAll.h"
#include "FCException.h"
#include <Poco\ByteOrder.h>
#include <Poco\InflatingStream.h>
#include <fstream>
#include <Poco\File.h>
#include "Util.h"
#include <Poco\InflatingStream.h>

using std::ifstream;

NBTTagCompound::NBTTagCompound(string name) : 
NBTBase(name,FC_NBT_TYPE_COMPOUND)
{
}


NBTTagCompound::NBTTagCompound(char iType,Poco::Path& file)  : 
NBTBase(string(""),FC_NBT_TYPE_COMPOUND)
{
	if (!Util::isReadWriteAble(file)) {throw FileIOException();}
	if (iType < 1 || iType > 3) {throw NBTIllegalCompressionTypeException();}

	std::ifstream filestream(file.toString(),ifstream::ios_base::binary | ifstream::ios_base::in);
	try {
		if (iType < 2) {
			Poco::InflatingStreamBuf::StreamType strmType;

			switch(iType) {
			case FC_NBT_COMPRESSION_GZIP:
				strmType = Poco::InflatingStreamBuf::STREAM_GZIP;
				break;
			case FC_NBT_COMPRESSION_ZLIB:
				strmType = Poco::InflatingStreamBuf::STREAM_ZLIB;
				break;
			}

			Poco::InflatingInputStream zipstrm(filestream,strmType);
			init(zipstrm);
			return;
		}

		init(filestream);
	} catch (FCException& ex) {
		ex.rethrow();
	}
}


NBTTagCompound::~NBTTagCompound() {
	for(auto it = _tagMap.begin(); it != _tagMap.end(); it++) {
		delete it->second;
	}
}


char NBTTagCompound::getType() {
	return (char)FC_NBT_TYPE_COMPOUND;
}

void NBTTagCompound::add(NBTBase* pNew) {
	if (pNew == NULL) {throw NullpointerException();}

	//Check existance
	auto it = _tagMap.find(pNew->getName());
	if (it != _tagMap.end()) {delete it->second;}

	_tagMap[pNew->getName()] = pNew;
}

template<typename Tag> Tag* NBTTagCompound::getElement(string sStr) {
	auto it = _tagMap.find(sStr);
	if (it == _tagMap.end()) {throw FCException(sStr + " was not found");}

	Tag* ptr = dynamic_cast<Tag*>(it->second);
	if (ptr == NULL) {throw FCException(sStr + " has an illegal type");}
	return ptr;
}

NBTTagCompound* NBTTagCompound::getCompound(string sStr) {
	try {
		return getElement<NBTTagCompound>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagByte* NBTTagCompound::getByte(string sStr) {
	try {
		return getElement<NBTTagByte>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


NBTTagDouble* NBTTagCompound::getDouble(string sStr) {
	try {
		return getElement<NBTTagDouble>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagFloat* NBTTagCompound::getFloat(string sStr) {
	try {
		return getElement<NBTTagFloat>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


NBTTagInt* NBTTagCompound::getInt(string sStr) {
	try {
		return getElement<NBTTagInt>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagInt64* NBTTagCompound::getInt64(string sStr) {
	try {
		return getElement<NBTTagInt64>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagShort* NBTTagCompound::getShort(string sStr) {
	try {
		return getElement<NBTTagShort>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagString* NBTTagCompound::getString(string sStr) {
	try {
		return getElement<NBTTagString>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagByteArray* NBTTagCompound::getByteArray(string sStr) {
	try {
		return getElement<NBTTagByteArray>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}


NBTTagIntArray* NBTTagCompound::getIntArray(string sStr) {
	try {
		return getElement<NBTTagIntArray>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

NBTTagList* NBTTagCompound::getList(string sStr) {
	try {
		return getElement<NBTTagList>(sStr);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

short NBTTagCompound::readShort(istream& strm) {
	short Data = 0; 

	strm.read((char*)&Data,2);
	if (strm.eof()) {throw EndOfFileException();}

	return Poco::ByteOrder::flipBytes((Poco::Int16)Data);
}

int NBTTagCompound::readInt(istream& strm) {
	int Data = 0; 

	strm.read((char*)&Data,4);
	if (strm.eof()) {throw EndOfFileException();}

	return Poco::ByteOrder::flipBytes((Poco::Int32)Data);
}

string NBTTagCompound::readString(istream& strm) {
	try {
		short iLen = readShort(strm); 
		if (iLen < 0) {throw NBTIllegalFormatException();}
		if (iLen == 0) {return string("");}

		char * pData = new char[iLen];
		string sReturn("");

		strm.read(pData,iLen);
		sReturn.append(pData,iLen);

		delete [] pData;

		return sReturn;
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagCompound::load(istream& rStrm,bool fHeaderParsing) {
	try {
		if (fHeaderParsing) {_sName = readString(rStrm);}
		char iTag;
		while (!rStrm.eof()) {
			rStrm.read(&iTag,1);
			if (iTag < 0 || iTag > 11) {throw NBTIllegalFormatException();}
			if (iTag == 0) {return;}

			NBTBase* pElement = NBTBase::newTag(iTag,readString(rStrm));
			pElement->load(rStrm,false);
			add(pElement);
		}
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagCompound::init(istream& rStrm) {
	try {
		char iTag = 0;
		rStrm.read(&iTag,1);

		if (rStrm.eof()) {throw EndOfFileException();}
		if (iTag != 0xa) {throw NBTIllegalFormatException();}

		_sName = readString(rStrm);
		load(rStrm,false);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

bool NBTTagCompound::has(string Name,char iType) {
	auto it = _tagMap.find(Name);
	if (it != _tagMap.end()) {
		if (it->second->getType() == iType) {
			return true;
		}else{	
#ifndef _DEBUG
			delete it->second;
			_tagMap.erase(it);
#else
			std::cout<<"A version of "<<Name<<" exists but with an incorrect type\n";
#endif
		}
	}
	return false;
}
