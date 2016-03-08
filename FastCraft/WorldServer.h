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
#ifndef _FASTCRAFTHEADER_WORLDSERVER
#define _FASTCRAFTHEADER_WORLDSERVER
#include <vector>
#include <Poco\Path.h>
#include <string>
#include <Poco\Runnable.h>
#include "ServerThreadBase.h"
using std::vector;
using std::string;

class World;

class WorldServer : public ServerThreadBase {
private:
	Poco::Path _serverDirectory;
	vector<World*> worlds;
public:
	/*
	* Constructor

	Parameter:
	@1 : Path to server directory
	@2 : Worlds to load
	     * first entry has to be the root world
		 * other worlds with end/neather will be prefixed with the world name 
	*/
	WorldServer(Poco::Path&,std::vector<string>&);


	/*
	* Destructor
	*/
	~WorldServer();


	/*
	* World accessators
	* Will return NULL if world can not be found

	Parameter:
	@1 : WorldName
	*/
	World* getOverworld(string); //Returns pointer to main overworld
	World* getNeather(string); // [...] main neather
	World* getEnd(string); // [...] main end
	World* getWorld(string); //used to get all the other worlds


	/*
	* Chunk unloader thread
	*/
	void run();
};

#endif