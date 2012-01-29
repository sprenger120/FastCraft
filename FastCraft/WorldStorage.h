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

#ifndef _FASTCRAFTHEADER_WORLDSTORAGE
#define _FASTCRAFTHEADER_WORLDSTORAGE
#include <vector>
#include <string>
#include <Poco/Path.h>

using std::vector;
using std::string;
class PlayerPool; 
class World;

class WorldStorage {
private:
	static vector<World*> _vpWorlds;
	PlayerPool& _rPlayerPool;
	Poco::Path _WorldPath;
public:
	/*
	* Constructor

	Parameter:
	@1 : A valid instance of PlayerPool
	@2 : Path to FastCrafts world directory
	*/
	WorldStorage(PlayerPool&,Poco::Path);


	/*
	* Destructor
	*/
	~WorldStorage();


	/*
	* Loads a new world from local hard disc
	* Will generate a new world if given name not exists
	* Will throw Poco::RuntimeException if name is invalid (blank/contains no letters or numbers)
	* Will throw Poco::RuntimeException if given world is already loaded or a error occurs

	Parameter:
	@1 : Worldname
	@2 : Dimension
	*/
	void loadWorld(string,char = 1);


	/*
	* Loads all worlds from local hard disc
	* Will rethrow all errors from loadWorld
	*/
	void loadAllWorlds();


	/*
	* Returns a reference to a world object by given name
	* Search is case insensitive
	* Will return NULL if given world not exists

	Parameter:
	@1 : World name
	*/
	static World* getWorldByName(string);


	/*
	* Forces a chunk data flush to HDD
	* Will flush all worlds
	*/
	void forceSave();


	/*
	* Returns true if all worlds were loaded
	*/
	static bool isReady();
};
#endif