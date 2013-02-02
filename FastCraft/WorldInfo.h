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
#ifndef _FASTCRAFTHEADER_WORLDINFO
#define _FASTCRAFTHEADER_WORLDINFO
#include "NBTTagCompound.h"
#include "Types.h"
#include <Poco\Path.h>

class WorldInfo {
private:
	NBTTagCompound* _pLevelDat;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Path to world directory
	*/
	WorldInfo(Poco::Path&);


	/*
	* Destructor
	*/
	~WorldInfo();


	/*
	* Properties
	*/
	bool* AllowCommands;
	bool* Hardcore;
	bool* MapFeatures;
	bool* isRaining;
	bool* isThundering;
	bool* isInitialized;
	int* GameType;
	int* GeneratorVersion;
	int* RainTime;
	int* ThunderTime;
	int* SpawnX;
	int* SpawnY;
	int* SpawnZ;
	int* Version;
	Tick* DayTime;
	Tick* lastPlayed;
	Tick* Time;
	long long* Seed;
	string* generatorName;
	string* generatorOptions;
};
#endif