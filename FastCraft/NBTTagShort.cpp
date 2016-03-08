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
#include "NBTTagShort.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagShort::NBTTagShort(string sName) : 
NBTBase(sName,FC_NBT_TYPE_SHORT)
{
	Data = 0;
}

char NBTTagShort::getType() {
	return (char)FC_NBT_TYPE_SHORT;
}

void NBTTagShort::load(istream& rStrm, bool fReadName) {
	try {
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}

		rStrm.read((char*)&Data,2);
		if (rStrm.eof()) {throw FCException("Unexpected stream end");}

		Data = Poco::ByteOrder::flipBytes((Poco::Int16)Data);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagShort::save(ostream& rStream,bool fWriteName) {
}
