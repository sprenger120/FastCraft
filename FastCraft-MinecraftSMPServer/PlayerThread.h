/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef _FASTCRAFTHEADER_PLAYERTHREAD
#define _FASTCRAFTHEADER_PLAYERTHREAD

#include <iostream>
#include <Poco/Runnable.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Thread.h>
#include "Structs.h"
#include <queue>

using std::string;
using std::queue;
using Poco::Thread;
using std::cout;
using std::endl;

class PlayerThread : public Poco::Runnable {
	private:
	PlayerFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	string _sName,_sNickName; //Minecraft.net Username and Ingame Nickname
	string _sIP; //IP

	queue<string> _SendQueue;

	Poco::Net::SocketStream _Connection;

	const int _iThreadID;

	bool _fLoggedIn;
	bool _fClear;

	void ClearQueue();
private:
	//De- / Constructor
	PlayerThread(int);
	~PlayerThread();

	virtual void run(); // Thread Main

	void Disconnect(); //Clear Player object
	void Connect(Poco::Net::SocketStream&,string); //Manage New Player connection | clear if necessary
	bool isConnected(); //Returns true if a player is assigned to this thread
	
	void Kick(); //Kicks player
	void Ban(int); //Bans player (int = Expiration, -1 for permanent ban)
	void Ban(string,int); //Bans player with reason (int = Expiration, -1 for permanent ban)


	//Accessator
	int getThreadID(); //Gets Thread ID - used for player instance identification
	string getUsername(); //Returns Player Name - not the edited nikname
	string getNickname(); //Gets actual Username thats shown in chat
	string getIP(); //Returns actual IP of player
	
	//Queue
	void appendQueue(std::string&); //Adds a job for the sending queue
};

#endif