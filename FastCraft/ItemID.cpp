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
#include "ItemID.h"

ItemID::ItemID(short id,char metadata) {
	first = id;
	second = metadata;
}

ItemID::ItemID() {
	first = -1;
	second = -1;
}

bool ItemID::operator==(ItemID& other) {
	if (first == other.first && second == other.second) {return true;}
	return false;
}

bool ItemID::operator!=(ItemID& other) {
	if (first == other.first && second == other.second) {return false;}
	return true;
}

bool ItemID::isEmpty() {
	if (first == -1 && second == -1) {return true;}
	return false;
}

void ItemID::clear() {
	first = -1;
	second = -1;
}