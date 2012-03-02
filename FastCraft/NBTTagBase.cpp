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
#include "NBTTagBase.h"
#include "NetworkOut.h"
#include <Poco/DeflatingStream.h>
#include <sstream>

NBTTagBase::NBTTagBase(string Name,char iType) :
_sName(Name),
_iElementType(iType)
{
}

NBTTagBase::~NBTTagBase(){
}

void NBTTagBase::compress(string& rSource) {
	std::stringstream ssTarget;
	Poco::DeflatingOutputStream deflator(ssTarget,Poco::DeflatingStreamBuf::STREAM_GZIP,-1);

	deflator.write(rSource.c_str(),rSource.length());  /* compress */
	deflator.flush();
	
	rSource.clear();				 /* write to source string */
	rSource.append(ssTarget.str());


	ssTarget.clear();    /* clear buffers */
	deflator.clear();
	deflator.close();
}

void NBTTagBase::addHeaderlessString(string& rTarget,string& rString) {
	NetworkOut::addShort(rTarget,rString.length());
	rTarget.append(rString);
}

string NBTTagBase::getName() {
	return _sName;
}

char NBTTagBase::getTagType() {
	return _iElementType;
}