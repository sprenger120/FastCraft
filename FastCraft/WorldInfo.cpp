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
#include "WorldInfo.h"
#include "NBTAll.h"
#include <iostream>
using std::cout;

WorldInfo::WorldInfo(Poco::Path& LevelDataFile) {
	_pLevelDat = new NBTTagCompound(FC_NBT_COMPRESSION_GZIP,LevelDataFile);


	/* Check level data */
	if (!_pLevelDat->has("Data",FC_NBT_TYPE_COMPOUND)) {
		cout<<"[level.dat] File is missing Data compound\n";
	}else{
		NBTTagCompound* pData = _pLevelDat->getCompound("Data");
		NBTTagInt* pInt = NULL;
		NBTTagString* pStr = NULL;
		NBTTagByte* pByte = NULL;
		
		//version
		if (!pData->has("version",FC_NBT_TYPE_INT)) {
			pData->add((pInt = new NBTTagInt("version")));
			pInt->Data = 19133; 
		}
		Version = &(pData->getInt("version")->Data);


		//initialized
		if (!pData->has("initialized",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("initialized")));
			pByte->Data = 0;
		}
		isInitialized = (bool*)&(pData->getByte("initialized")->Data);


		//LevelName
		if (!pData->has("LevelName",FC_NBT_TYPE_STRING)) {
			pData->add((pStr = new NBTTagString("version")));
			pStr->Data.assign(LevelDataFile[LevelDataFile.depth()-1]);
		}
		if((pStr = pData->getString("LevelName"))->Data.compare(LevelDataFile[LevelDataFile.depth()-1]) != 0) {
			cout<<"[level.dat] LevelName is invalid. Correcting\n";
			pStr->Data.assign(LevelDataFile[LevelDataFile.depth()-1]);
		}


		//generatorName
		if (!pData->has("generatorName",FC_NBT_TYPE_STRING)) {
			pData->add((pStr = new NBTTagString("generatorName")));
			pStr->Data.assign("default");
		}
		if((pStr = pData->getString("generatorName"))->Data.compare("default") != 0 && 
			pStr->Data.compare("flat") != 0 && 
			pStr->Data.compare("largeBiomes") != 0)	
		{
			cout<<"[level.dat] Illegal world generator. Correcting\n";
			pStr->Data.assign("default");
		}
		generatorName = &(pStr->Data);


		//generatorOptions
		if (!pData->has("generatorOptions",FC_NBT_TYPE_STRING)) {
			pData->add((pStr = new NBTTagString("generatorOptions")));
		}
		generatorOptions = &(pData->getString("generatorOptions")->Data);
		

		//RandomSeed
		if (!pData->has("RandomSeed",FC_NBT_TYPE_INT64)) {
			pData->add(new NBTTagInt64("RandomSeed"));
		}
		Seed = &(pData->getInt64("RandomSeed")->Data);


		//MapFeatures
		if (!pData->has("MapFeatures",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("MapFeatures")));
			pByte->Data = true;
		}
		MapFeatures = (bool*)&(pData->getByte("MapFeatures")->Data);


		//LastPlayed
		if (!pData->has("LastPlayed",FC_NBT_TYPE_INT64)) {
			pData->add(new NBTTagInt64("LastPlayed"));
		}
		lastPlayed = (Tick*)&(pData->getInt64("LastPlayed")->Data);


		//allowCommands
		if (!pData->has("allowCommands",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("allowCommands")));
			pByte->Data = false;
		}
		AllowCommands = (bool*)&(pData->getByte("allowCommands")->Data);


		//hardcore
		if (!pData->has("hardcore",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("hardcore")));
			pByte->Data = false;
		}
		Hardcore = (bool*)&(pData->getByte("hardcore")->Data);


		//GameType
		if (!pData->has("GameType",FC_NBT_TYPE_INT)) {
			pData->add((pInt = new NBTTagInt("GameType")));
			pInt->Data = 0;
		}
		if ((pInt = pData->getInt("GameType"))->Data != 0 && 
			pInt->Data != 1 && 
			pInt->Data != 2) 
		{
			pInt->Data = 0;
		}
		GameType = &(pData->getInt("GameType")->Data);


		//Time
		if (!pData->has("Time",FC_NBT_TYPE_INT64)) {
			pData->add(new NBTTagInt64("Time"));
		}
		Time = (Tick*)&(pData->getInt64("Time")->Data);


		//DayTime
		if (!pData->has("DayTime",FC_NBT_TYPE_INT64)) {
			pData->add(new NBTTagInt64("DayTime"));
		}
		DayTime = (Tick*)&(pData->getInt64("DayTime")->Data);


		//SpawnX
		if (!pData->has("SpawnX",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("SpawnX"));
		}
		SpawnX = &(pData->getInt("SpawnX")->Data);


		//SpawnY
		if (!pData->has("SpawnY",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("SpawnY"));
		}
		SpawnY = &(pData->getInt("SpawnY")->Data);


		//SpawnZ
		if (!pData->has("SpawnZ",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("SpawnZ"));
		}
		SpawnZ = &(pData->getInt("SpawnZ")->Data);

		
		//raining
		if (!pData->has("raining",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("raining")));
			pByte->Data = false;
		}
		isRaining = (bool*)&(pData->getByte("raining")->Data);


		//rainTime
		if (!pData->has("rainTime",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("rainTime"));
		}
		RainTime = &(pData->getInt("rainTime")->Data);

		
		//thundering
		if (!pData->has("thundering",FC_NBT_TYPE_BYTE)) {
			pData->add((pByte = new NBTTagByte("thundering")));
			pByte->Data = false;
		}
		isThundering = (bool*)&(pData->getByte("thundering")->Data);


		//thunderTime
		if (!pData->has("thunderTime",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("thunderTime"));
		}
		ThunderTime = &(pData->getInt("thunderTime")->Data);

		
		
		//GeneratorVersion
		if (!pData->has("generatorVersion",FC_NBT_TYPE_INT)) {
			pData->add(new NBTTagInt("generatorVersion"));
		}
		GeneratorVersion = &(pData->getInt("generatorVersion")->Data);


		cout<<"";
		/* ToDo: GameRules parsen */
	}
}

WorldInfo::~WorldInfo() {
	/* ToDo Saving data */
	delete _pLevelDat;
}