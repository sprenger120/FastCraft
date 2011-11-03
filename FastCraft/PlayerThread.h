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
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include "Structs.h"
#include "SettingsHandler.h"
#include <queue>

using std::string;
using std::queue;
using Poco::Thread;
using std::cout;
using std::endl;

class PlayerThread: public Poco::Runnable {
private:
	PlayerFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	string _sName,_sNickName; //Minecraft.net Username and Ingame Nickname
	string _sIP; //IP

	char _sBuffer[1024];
	string _sOutputBuffer;

	queue<QueueJob> _SendQueue;

	Poco::Net::StreamSocket _Connection;
	SettingsHandler* _pSettings;
	bool _fSettingsHandlerSet;

	bool _fLoggedIn; //Set to true if handshake is done (username known)
	bool _fAssigned;//true if a player is assigned to that thread
	bool _fReady; //true if thread is ready

	void ClearQueue();
	bool ProcessQueue(); //Returns true if connection is closed

	int _iThreadID;
	static int InstanceCounter;
	static int PlayerCount;
public:
	//De- / Constructor
	PlayerThread();
	~PlayerThread();

	//Management
	void setSettingsHandler(SettingsHandler*);
	bool Ready();

	virtual void run(); // Thread Main

	void Disconnect(); //Clear Player object
	void Connect(Poco::Net::StreamSocket&,string); //Manage New Player connection | clear if necessary
	bool isAssigned(); //Returns true if a player is assigned to this thread

	void Kick(); //Kicks player without reason
	void Kick(string); //Kick with reason
	void Ban(int); //Bans player (int = Expiration, -1 for permanent ban)
	void Ban(string,int); //Bans player with reason (int = Expiration, -1 for permanent ban)


	//Accessator
	int getThreadID(); //Gets Thread ID - used for player instance identification
	string getUsername(); //Returns Player Name - not the edited nikname
	string getNickname(); //Gets actual Username thats shown in chat
	string getIP(); //Returns actual IP of player

	//Queue
	void appendQueue(QueueJob&); //Adds a job for the sending queue
};

#endif