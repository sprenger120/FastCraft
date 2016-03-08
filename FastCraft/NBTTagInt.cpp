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
#include "NBTTagInt.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagInt::NBTTagInt(string sName) : 
NBTBase(sName,FC_NBT_TYPE_INT)
{
	Data = 0;
}

char NBTTagInt::getType() {
	return (char)FC_NBT_TYPE_INT;
}

void NBTTagInt::load(istream& rStrm, bool fReadName) {
	try {
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		
		rStrm.read((char*)&Data,4);
		if (rStrm.eof()) {throw FCException("Unexpected stream end");}

		Data = Poco::ByteOrder::flipBytes((Poco::Int32)Data);
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagInt::save(ostream& rStream,bool fWriteName) {
}
