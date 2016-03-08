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
#include "NBTContainer.h"
#include "NBTTag.h"

NBTContainer::~NBTContainer() {
	for (auto it = _tagMap.begin();it != _tagMap.end();it++) {
		delete it->second;
	}
}

NBTTag* NBTContainer::operator[](string sName) {
	auto it = _tagMap.begin();
	if (it == _tagMap.end()) {return NULL;}

	return it->second;
}

bool NBTContainer::has(string sName) {
	auto it = _tagMap.find(sName);
	return it != _tagMap.end();
}