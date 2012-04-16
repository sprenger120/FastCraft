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
#include <Poco/Exception.h>

std::map<std::string,char> Constants::_Map;

void Constants::init() {
	_Map["/Entity/Animation/NoAnimation"]		= 0;
	_Map["/Entity/Animation/SwingArm"]			= 1;
	_Map["/Entity/Animation/Damage"]			= 2;
	_Map["/Entity/Animation/LeaveBed"]			= 3;
	_Map["/Entity/Animation/EatFood"]			= 5;
	_Map["/Entity/Animation/Crouch"]			= 104;
	_Map["/Entity/Animation/UnCrouch"]			= 105;

	_Map["/Entity/Status/Hurt"]					= 2;
	_Map["/Entity/Status/Dead"]					= 3;
	_Map["/Entity/Status/Wolf/Taming"]			= 6;
	_Map["/Entity/Status/Wolf/Tamed"]			= 7;
	_Map["/Entity/Status/Wolf/Shake"]			= 8;
	_Map["/Entity/Status/AcceptEating"]			= 9;

	_Map["/Entity/Action/Crouch"]				= 1;
	_Map["/Entity/Action/UnCrouch"]				= 2;
	_Map["/Entity/Action/LeaveBed"]				= 3;
	_Map["/Entity/Action/StartSprinting"]		= 4;
	_Map["/Entity/Action/StopSprinting"]		= 5;

	_Map["/Entity/Mob/TypeID/Creeper"]			= 50;
	_Map["/Entity/Mob/TypeID/Skeleton"]			= 51;
	_Map["/Entity/Mob/TypeID/Spider"]			= 52;
	_Map["/Entity/Mob/TypeID/GiantZombie"]		= 53;
	_Map["/Entity/Mob/TypeID/Zombie"]			= 54;
	_Map["/Entity/Mob/TypeID/Slime"]			= 55;
	_Map["/Entity/Mob/TypeID/Ghast"]			= 56;
	_Map["/Entity/Mob/TypeID/ZombiePigman"]		= 57;
	_Map["/Entity/Mob/TypeID/Enderman"]			= 58;
	_Map["/Entity/Mob/TypeID/CaveSpider"]		= 59;
	_Map["/Entity/Mob/TypeID/Silverfish"]		= 60;
	_Map["/Entity/Mob/TypeID/Blaze"]			= 61;
	_Map["/Entity/Mob/TypeID/MagmaCube"]		= 62;
	_Map["/Entity/Mob/TypeID/EnderDragon"]		= 63;
	_Map["/Entity/Mob/TypeID/Pig"]				= 90;
	_Map["/Entity/Mob/TypeID/Sheep"]			= 91;
	_Map["/Entity/Mob/TypeID/Cow"]				= 92;
	_Map["/Entity/Mob/TypeID/Chicken"]			= 93;
	_Map["/Entity/Mob/TypeID/Squid"]			= 94;
	_Map["/Entity/Mob/TypeID/Wolf"]				= 95;
	_Map["/Entity/Mob/TypeID/Mooshroom"]		= 96;
	_Map["/Entity/Mob/TypeID/Snowman"]			= 97;
	_Map["/Entity/Mob/TypeID/Ocelot"]			= 98;
	_Map["/Entity/Mob/TypeID/IronGolem"]		= 99;
	_Map["/Entity/Mob/TypeID/Villager"]			= 120;

	_Map["/Entity/Mob/Sheep/ShearnessFlag"]		= 1<<4;
	_Map["/Entity/Mob/Wolf/Flags/SittingDown"]	= 1;
	_Map["/Entity/Mob/Wolf/Flags/Aggressive"]	= 2;
	_Map["/Entity/Mob/Wolf/Flags/Tamed"]		= 4;

	_Map["/Entity/ObjectID/Boat"]				= 1;
	_Map["/Entity/ObjectID/Minecart"]			= 10;
	_Map["/Entity/ObjectID/StorageMinecart"]	= 11;
	_Map["/Entity/ObjectID/PoweredMinecart"]	= 12;
	_Map["/Entity/ObjectID/ActiveTNT"]			= 50;
	_Map["/Entity/ObjectID/EnderCrystal"]		= 51;
	_Map["/Entity/ObjectID/Arrow"]				= 60;
	_Map["/Entity/ObjectID/Snowball"]			= 61;
	_Map["/Entity/ObjectID/Egg"]				= 62;
	_Map["/Entity/ObjectID/FallingSand"]		= 70;
	_Map["/Entity/ObjectID/FallingGravel"]		= 71;
	_Map["/Entity/ObjectID/EyeOfEnder"]			= 72;
	_Map["/Entity/ObjectID/DragonEgg"]			= 74;
	_Map["/Entity/ObjectID/FishingFloat"]		= 90;

	_Map["/Entity/UnknownID"]					= -1;



	_Map["/Color/White"]						= 0;
	_Map["/Color/Orange"]						= 1;
	_Map["/Color/Magenta"]						= 2;
	_Map["/Color/LightBlue"]					= 3;
	_Map["/Color/Yellow"]						= 4;
	_Map["/Color/Lime"]							= 5;
	_Map["/Color/Pink"]							= 6;
	_Map["/Color/Gray"]							= 7;
	_Map["/Color/Silver"]						= 8;
	_Map["/Color/Cyan"]							= 9;
	_Map["/Color/Purple"]						= 10;
	_Map["/Color/Blue"]							= 11;
	_Map["/Color/Brown"]						= 12;
	_Map["/Color/Green"]						= 13;
	_Map["/Color/Red"]							= 14;
	_Map["/Color/Black"]						= 15;

	_Map["/ServerMode/Survival"]				= 0;
	_Map["/ServerMode/Creative"]				= 1;

	_Map["/Difficulty/Peaceful"]				= 0;
	_Map["/Difficulty/Easy"]					= 1;
	_Map["/Difficulty/Normal"]					= 2;
	_Map["/Difficulty/Hard"]					= 3;



	_Map["/Leave/Kick"]							= 1;
	_Map["/Leave/Quit"]							= 2;
	_Map["/Leave/Other"]						= 3;

	_Map["/ThreadStatus/Dead"]					= 1;
	_Map["/ThreadStatus/Running"]				= 2;
	_Map["/ThreadStatus/Terminating"]			= 3;

	_Map["/Queue/Low"]							= 1;
	_Map["/Queue/High"]							= 2;
}

char Constants::get(std::string str) {
	auto it = _Map.find(str);

	if (it == _Map.end()) {
		std::cout<<"Constants::get "<<str<<" not found!\n";
		throw Poco::RuntimeException("Key not found!");
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