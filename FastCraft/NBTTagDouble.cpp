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
#include "NBTTagDouble.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagDouble::NBTTagDouble(string sName) : 
NBTBase(sName,FC_NBT_TYPE_DOUBLE)
{
	Data = 0.0;
}

char NBTTagDouble::getType() {
	return (char)FC_NBT_TYPE_DOUBLE;
}

void NBTTagDouble::load(istream& rStrm, bool fReadName) {
	try {
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		NBTTagCompound::uniLongDouble uniLongDouble;

		rStrm.read(uniLongDouble.s,8);
		if (rStrm.eof()) {throw FCException("Unexpected stream end");}

		uniLongDouble.i = Poco::ByteOrder::flipBytes((Poco::Int64)uniLongDouble.i);
		Data = uniLongDouble.f;
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagDouble::save(ostream& rStream,bool fWriteName) {
}
