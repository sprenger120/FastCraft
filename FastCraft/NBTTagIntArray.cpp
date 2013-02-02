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
#include "NBTTagIntArray.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagIntArray::NBTTagIntArray(string sName) : 
NBTBase(sName,FC_NBT_TYPE_INTARRAY),
Data(0)
{
}

char NBTTagIntArray::getType() {
	return (char)FC_NBT_TYPE_INTARRAY;
}

void NBTTagIntArray::load(istream& rStrm, bool fReadName) {
	try {
		if (!Data.empty()) {Data.resize(0);}
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		
		int iLen = NBTTagCompound::readInt(rStrm);
		if (iLen <= 0) {throw FCException("Illegal ByteArray length");}
		if (iLen >= 1048576) {throw FCException("Max size limit exceeded");}

		for(;iLen>0;iLen--) {
			Data.push_back(NBTTagCompound::readInt(rStrm));
		}
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagIntArray::save(ostream& rStream,bool fWriteName) {
}
