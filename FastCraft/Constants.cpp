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
#include "Constants.h"
#include "FCRuntimeException.h"
#include <iostream>

std::map<std::string,char> Constants::_Map;

void Constants::init() {
	_Map["/Entity/Alive/Animation/NoAnimation"]		= 0;
	_Map["/Entity/Alive/Animation/SwingArm"]		= 1;
	_Map["/Entity/Alive/Animation/Damage"]			= 2;
	_Map["/Entity/Alive/Animation/LeaveBed"]		= 3;
	_Map["/Entity/Alive/Animation/EatFood"]			= 5;
	_Map["/Entity/Alive/Animation/Crouch"]			= 104;
	_Map["/Entity/Alive/Animation/UnCrouch"]		= 105;

	_Map["/Entity/Alive/Status/Hurt"]				= 2;
	_Map["/Entity/Alive/Status/Dead"]				= 3;
	_Map["/Entity/Alive/Status/WolfTaming"]			= 6;
	_Map["/Entity/Alive/Status/WolfTamed"]			= 7;
	_Map["/Entity/Alive/Status/WolfShake"]			= 8;
	_Map["/Entity/Alive/Status/AcceptEating"]		= 9;

	_Map["/Entity/Alive/Action/Crouch"]				= 1;
	_Map["/Entity/Alive/Action/UnCrouch"]			= 2;
	_Map["/Entity/Alive/Action/LeaveBed"]			= 3;
	_Map["/Entity/Alive/Action/StartSprinting"]		= 4;
	_Map["/Entity/Alive/Action/StopSprinting"]		= 5;

	_Map["/Entity/Alive/TypeID/Creeper"]			= 50;
	_Map["/Entity/Alive/TypeID/Skeleton"]			= 51;
	_Map["/Entity/Alive/TypeID/Spider"]				= 52;
	_Map["/Entity/Alive/TypeID/GiantZombie"]		= 53;
	_Map["/Entity/Alive/TypeID/Zombie"]				= 54;
	_Map["/Entity/Alive/TypeID/Slime"]				= 55;
	_Map["/Entity/Alive/TypeID/Ghast"]				= 56;
	_Map["/Entity/Alive/TypeID/ZombiePigman"]		= 57;
	_Map["/Entity/Alive/TypeID/Enderman"]			= 58;
	_Map["/Entity/Alive/TypeID/CaveSpider"]			= 59;
	_Map["/Entity/Alive/TypeID/Silverfish"]			= 60;
	_Map["/Entity/Alive/TypeID/Blaze"]				= 61;
	_Map["/Entity/Alive/TypeID/MagmaCube"]			= 62;
	_Map["/Entity/Alive/TypeID/EnderDragon"]		= 63;
	_Map["/Entity/Alive/TypeID/Pig"]				= 90;
	_Map["/Entity/Alive/TypeID/Sheep"]				= 91;
	_Map["/Entity/Alive/TypeID/Cow"]				= 92;
	_Map["/Entity/Alive/TypeID/Chicken"]			= 93;
	_Map["/Entity/Alive/TypeID/Squid"]				= 94;
	_Map["/Entity/Alive/TypeID/Wolf"]				= 95;
	_Map["/Entity/Alive/TypeID/Mooshroom"]			= 96;
	_Map["/Entity/Alive/TypeID/Snowman"]			= 97;
	_Map["/Entity/Alive/TypeID/Ocelot"]				= 98;
	_Map["/Entity/Alive/TypeID/IronGolem"]			= 99;
	_Map["/Entity/Alive/TypeID/Villager"]			= 120;
	_Map["/Entity/Alive/TypeID/Player"]				= -1;

	_Map["/Entity/Object/TypeID/Boat"]				= 1;
	_Map["/Entity/Object/TypeID/Minecart"]			= 10;
	_Map["/Entity/Object/TypeID/StorageMinecart"]	= 11;
	_Map["/Entity/Object/TypeID/PoweredMinecart"]	= 12;
	_Map["/Entity/Object/TypeID/ActiveTNT"]			= 50;
	_Map["/Entity/Object/TypeID/EnderCrystal"]		= 51;
	_Map["/Entity/Object/TypeID/Arrow"]				= 60;
	_Map["/Entity/Object/TypeID/Snowball"]			= 61;
	_Map["/Entity/Object/TypeID/Egg"]				= 62;
	_Map["/Entity/Object/TypeID/FallingSand"]		= 70;
	_Map["/Entity/Object/TypeID/FallingGravel"]		= 71;
	_Map["/Entity/Object/TypeID/EyeOfEnder"]		= 72;
	_Map["/Entity/Object/TypeID/DragonEgg"]			= 74;
	_Map["/Entity/Object/TypeID/FishingFloat"]		= 90;

	_Map["/Color/White"]							= 0;
	_Map["/Color/Orange"]							= 1;
	_Map["/Color/Magenta"]							= 2;
	_Map["/Color/LightBlue"]						= 3;
	_Map["/Color/Yellow"]							= 4;
	_Map["/Color/Lime"]								= 5;
	_Map["/Color/Pink"]								= 6;
	_Map["/Color/Gray"]								= 7;
	_Map["/Color/Silver"]							= 8;
	_Map["/Color/Cyan"]								= 9;
	_Map["/Color/Purple"]							= 10;
	_Map["/Color/Blue"]								= 11;
	_Map["/Color/Brown"]							= 12;
	_Map["/Color/Green"]							= 13;
	_Map["/Color/Red"]								= 14;
	_Map["/Color/Black"]							= 15;
		
	_Map["/ServerMode/Survival"]					= 0;
	_Map["/ServerMode/Creative"]					= 1;

	_Map["/Difficulty/Peaceful"]					= 0;
	_Map["/Difficulty/Easy"]						= 1;
	_Map["/Difficulty/Normal"]						= 2;
	_Map["/Difficulty/Hard"]						= 3;
}

char Constants::get(std::string str) {
	auto it = _Map.find(str);

	if (it == _Map.end()) {
		std::cout<<"Constants::get "<<str<<" not found!\n";
		throw FCRuntimeException("Key not found!");
	}
	return it->second;
}

bool Constants::isDefined(char iVal,std::string sPath) {
	if (_Map.empty()) {return false;}
	if (sPath.empty()) {return false;}

	//Remove / from end
	if (sPath[sPath.length()-1] == '/') {
		auto it = sPath.rbegin();
		int x=0;
		
		while(*it == '/'){it++;x++;}
		if (x>0) {sPath.resize(sPath.size()-x);}
	}

	//search
	for(auto it = _Map.begin();it != _Map.end();it++) {
		if (cutOffEndElement(it->first).compare(sPath) == 0 && it->second == iVal) {return true;}
	}

	return false;
}


string Constants::cutOffEndElement(string str) {
	if (str.empty()) {return string("");}
	for (int x = str.length()-1;x>0;x--) {
		if (str[x] == '/'&& !(x==0) && !(str[x-1] == '/') ) {
			str.erase(str.begin()+x,str.end());
			return str;
		}
	}

	return str;
}
