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
#include "NBTTagByteArray.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagByteArray::NBTTagByteArray(string sName) : 
NBTBase(sName,FC_NBT_TYPE_BYTEARRAY)
{
	Data = NULL;
}

NBTTagByteArray::~NBTTagByteArray() {
	if (Data != NULL) {delete [] Data;}
}

char NBTTagByteArray::getType() {
	return (char)FC_NBT_TYPE_BYTEARRAY;
}

void NBTTagByteArray::load(istream& rStrm, bool fReadName) {
	try {
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		if (Data != NULL) {delete [] Data;}
		
		int iLen = NBTTagCompound::readInt(rStrm);
		if (iLen <= 0) {throw FCException("Illegal ByteArray length");}
		if (iLen >= 4*1048576) {throw FCException("Max size limit exceeded");}

		Data = new char[iLen];
		rStrm.read(Data,iLen);

		if (rStrm.eof()) {throw FCException("Unexpected stream end");}
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagByteArray::save(ostream& rStream,bool fWriteName) {
}
