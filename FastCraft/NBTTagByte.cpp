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
#include "NBTTagByte.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"

NBTTagByte::NBTTagByte(string _sName) : 
NBTBase(_sName,FC_NBT_TYPE_BYTE)
{
	Data = 0;
}

char NBTTagByte::getType() {
	return (char)FC_NBT_TYPE_BYTE;
}

void NBTTagByte::load(istream& rStrm, bool fReadName) {
	try {
		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		rStrm.read(&Data,1);
		if (rStrm.eof()) {throw FCException("Unexpected stream end");}
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagByte::save(ostream& rStream,bool fWriteName) {
}
