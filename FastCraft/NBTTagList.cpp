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
#include "NBTTagList.h"
#include "NBTConstants.h"
#include "FCException.h"
#include "NBTTagCompound.h"
#include <Poco\ByteOrder.h>

NBTTagList::NBTTagList(string sName) : 
NBTBase(sName,FC_NBT_TYPE_LIST),
Data(0)
{
}

NBTTagList::~NBTTagList() {
	if (Data.empty()) {return;}
	for(int i = 0;i<=Data.size()-1;i++) {
		delete Data[i];
	}
}

char NBTTagList::getType() {
	return (char)FC_NBT_TYPE_LIST;
}

void NBTTagList::load(istream& rStrm, bool fReadName) {
	try {
		if (!Data.empty()) {
			for(int i = 0;i<=Data.size()-1;i++) {
				delete Data[i];
			}
			Data.resize(0);
		}

		if (fReadName) {_sName = NBTTagCompound::readString(rStrm);}
		
		char iListTagType = 0;
		int iLen = 0;

		rStrm.read(&iListTagType,1);
		if (iListTagType < 1 || iListTagType > 11) {throw FCException("Unknown tag type");}

		iLen = NBTTagCompound::readInt(rStrm);
		if (iLen < 0 ) {return throw FCException("Illegal element count");}
		if (iLen == 0) {return;}
		if (iLen >= 1048576) {throw FCException("Max size limit exceeded");}

		for(;iLen>0;iLen--) {
			NBTBase* pElement = NBTBase::newTag(iListTagType,string(""));
			pElement->load(rStrm,false);
			Data.push_back(pElement);
		}
	}catch(FCException& ex) {
		ex.rethrow();
	}
}

void NBTTagList::save(ostream& rStream,bool fWriteName) {
}
