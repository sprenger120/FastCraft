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

#ifndef _FASTCRAFTHEADER_ACCEPTHREAD
#define _FASTCRAFTHEADER_ACCEPTHREAD
#include <Poco/Net/ServerSocket.h>
#include "ServerThreadBase.h"
#include <string>
#include <Poco/Thread.h>
#include <Poco/Stopwatch.h>
#include <vector>

//Forward definitions
class PlayerPool;
class MinecraftServer;

using std::vector;
using std::string;
using Poco::Timestamp;

class AcceptThread : public ServerThreadBase {
private:
	MinecraftServer* _pMinecraftServer;
	Poco::Net::ServerSocket _ServerSock;
	string _preparedServerFullMsg;
public:
	/*
	* Constructor

	Parameter:
	@1 : Pointer to minecraft instance that runs this class
	*/
	AcceptThread(MinecraftServer*);


	/*
	* Destructor
	* Will shutdown socket and thread
	*/
	~AcceptThread();


	/*
	* Thread main
	*/
	virtual void run(); 
private:
	void cutOffPort(string,string&);
	int search(vector<std::pair<string,std::pair<Timestamp::TimeDiff,Timestamp::TimeDiff>>>&,string);
};
#endif