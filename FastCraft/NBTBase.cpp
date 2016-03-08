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
#include "NBTAll.h"
#include "FCException.h"

NBTBase::NBTBase(ConstString& str,int id) : 
_sName(str)
{
	if (id < 1 || id > 11) {throw FCException("Unknown tag type");}
	_iTagID = (char)id;
}

NBTBase::~NBTBase() {
}


NBTBase* NBTBase::newTag(int tag,ConstString& sStr) {
	switch (tag) {
	case FC_NBT_TYPE_BYTE:
		return new NBTTagByte(sStr);
	case FC_NBT_TYPE_COMPOUND:
		return new NBTTagCompound(sStr);
	case FC_NBT_TYPE_DOUBLE:
		return new NBTTagDouble(sStr);
	case FC_NBT_TYPE_FLOAT:
		return new NBTTagFloat(sStr);
	case FC_NBT_TYPE_INT:
		return new NBTTagInt(sStr);
	case FC_NBT_TYPE_INT64:
		return new NBTTagInt64(sStr);
	case FC_NBT_TYPE_SHORT:
		return new NBTTagShort(sStr);
	case FC_NBT_TYPE_STRING:
		return new NBTTagString(sStr);
	case FC_NBT_TYPE_INTARRAY:
		return new NBTTagIntArray(sStr);
	case FC_NBT_TYPE_LIST:
		return new NBTTagList(sStr);
	case FC_NBT_TYPE_BYTEARRAY:
		return new NBTTagByteArray(sStr);
	default:
		return NULL;
	}

	return NULL;
}

ConstString& NBTBase::getName() {
	return _sName;
}

void NBTBase::load(istream&,bool) { 
}

void NBTBase::save(ostream&,bool) { 
}
