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

#ifndef _FASTCRAFTHEADER_MINECRAFTSERVER
#define _FASTCRAFTHEADER_MINECRAFTSERVER
#include <iostream>
#include <Poco/Path.h>
#include <Poco/ThreadPool.h>
#include <vector>
#include <Poco/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/Path.h>
#include "ServerThreadBase.h"
#include <Poco/Stopwatch.h>
#include <Poco/Mutex.h>


//Forward definitions
class AcceptThread;
class SettingsHandler;
class PlayerPool;
class PackingThread;
class ItemInformationProvider;
class PlayerPool;
class World;

using std::string;
using std::vector;

typedef unsigned long long Tick;

class MinecraftServer : public ServerThreadBase {
private:
	/*Settings*/
	unsigned short _iPort;
	unsigned short _iPlayerSlotCount;
	string _sServerDescription;
	string _sMOTD;
	string _sServerFullMessage;
	bool _iServerMode;
	char _iDifficulty;
	bool _fOnlineMode;
	bool _fWhitelist;
	bool _fAllowNeather;
	bool _fLoadEnd;
	vector<string> _vsAdditionalWorlds;
	string _sMainMapName;
	bool _fSpawnPeacefulMobs;
	bool _fSpawnHostileMobs;
	bool _fPVP;
	bool _fAllowFlyMod;
	double _dMaxFlyingSpeed;
	string _sServerName;

	/* ID generation */
	int _iID;
	Poco::Mutex _idMutex;

	/* Base classes */
	vector<World*> _vpWorlds;
	AcceptThread* _pAcceptThread;
	PlayerPool* _pPlayerPool;
	ItemInformationProvider* _pItemInformationProvider;

	/* Time stuff */
	Poco::Stopwatch _clockCreation;
	Tick   _iInGameTime;


	/* Other */
	bool _fMarkedForShutdown;
public:
	/*
	* Constructor
	* Throws FCRuntimeException if a module is unable to start

	Parameter:
	@1 : Name of server instance
	@2 : Path to FastCraft root directory
	@3 : A reference to a vector of the already used ports
	*/
	MinecraftServer(string,Poco::Path,vector<unsigned short>&);


	/*
	* Destructor
	* Will shutdown server if is still running
	*/
	~MinecraftServer();

	
	/*
	* Thread main
	*/
	void run();


	/*
	* Shuts down server
	*/
	void shutdown();


	/*
	* Returns true if server is marked for shutdown
	*/
	bool isMarkedForShutdown();

	/*
	* Returns a pointer to the internal PlayerPool instance
	*/
	PlayerPool* getPlayerPool();


	/*
	* Returns a pointer to the internal ItemInfornationProvider instance
	*/
	ItemInformationProvider* getItemInfoProvider();


	/*
	* Returns the port which is the server is running on
	*/
	unsigned short getPort(); 
	
	
	/*
	* Returns how many players can join the server at the same time
	*/
	unsigned short getPlayerSlotCount();


	/*
	* Returns the server description, shown in the server list of the client
	*/
	string getServerDescription();


	/*
	* Returns the message of the day 
	* This will be posted into chat after a player join
	*/
	string getServerMOTD();


	/*
	* Returns the server mode
	* false for survival, true for creative
	*/
	bool getServerMode();


	/*
	* Returns the server difficulty 
	* Constants.h -> FC_DIFFICULTY_
	*/
	char getDifficulty();


	/*
	* Returns true if name verification is required 
	*/
	bool isOnlineModeActivated();


	/*
	* Returns true if whitelist verification is required
	*/
	bool isWhitelistActivated();


	/*
	* Returns the spawn map name
	*/
	string getMainWorldName();


	/*
	* Returns true if neather is allowed
	*/
	bool isNeatherAllowed(); 


	/*
	* Returns true if peacful/hostile mobs are allowed to spawn
	*/
	bool isPeacefulAnimalSpawningActivated();
	bool isHostileAnimalSpawningActivated();


	/*
	* Returns true if player vs. player actions will affect their health
	*/
	bool isPVPActivated();


	/*
	* Returns true if zombe's flymod is allowed
	*/
	//bool isFlyModAllowed();


	/*
	* Returns maximal flyspeed
	*/
	//double getMaxFlySpeed();


	/*
	* Returns the ServerFull message
	*/
	string getServerFullMessage();


	/* 
	* Returns pointer to world by given name
	* Returns NULL if world wasn't found
	* Search is case sensitive

	Parameter:
	@1 : World name to search
	*/
	World* getWorldByName(string);


	/*
	* Returns actual in-game time
	*/
	Tick getInGameTime();


	/*
	* Returns time in milliseconds since class was created
	*/
	Tick getTimestamp();


	/*
	* Generates a new unique ID
	*/
	int generateID();
private:
	bool Int2Bool(int&);
	bool parseNodeInt(Poco::AutoPtr<Poco::XML::Document>,string,int&,int,int);
	bool parseNodeDouble(Poco::AutoPtr<Poco::XML::Document>,string,double&,double,double);
	bool parseNodeString(Poco::AutoPtr<Poco::XML::Document>,string,string&);
	char readConfiguration(Poco::Path&);
};
#endif
