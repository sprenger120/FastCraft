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
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include "Structs.h"
#include "EntityProvider.h"
#include "SettingsHandler.h"
#include "ServerTime.h"
#include <queue>

using std::string;
using std::queue;
using Poco::Thread;
using std::cout;
using std::endl;

struct TimeJobs {
	long long LastTimeSend;
};

class PlayerThread: public Poco::Runnable {
private:
	//Player specific data
	PlayerFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	string _sName,_sNickName; //Minecraft.net Username and Ingame Nickname
	string _sIP; //IP
	int _iEntityID;
	char _iLoginProgress;

	//TCP stuff
	Poco::Net::StreamSocket _Connection;
	unsigned char _sBuffer[1024];
	string _sTemp;
	queue<QueueJob> _SendQueue;

	//Connection Hash
	string _sConnectionHash;
	long long _iLastConnHash;

	//Class pointer
	SettingsHandler* _pSettings;
	EntityProvider* _pEntityProvider;
	ServerTime* _pServerTime;

	//Thread specific
	bool _fAssigned;//true if a player is assigned to that thread
	long long _iThreadTicks;
	int _iThreadID;
	static int InstanceCounter;
	static int PlayerCount;

	//Verification
	Poco::Net::HTTPClientSession _Web_Session;
	Poco::Net::HTTPResponse _Web_Response;

	//Time jobs
	TimeJobs _TimeJobs;
public:
	//De- / Constructor
	PlayerThread(SettingsHandler*,EntityProvider*,ServerTime*);
	~PlayerThread();

	virtual void run(); // Thread Main

	void Disconnect(bool = false); //Clear Player object
	void Connect(Poco::Net::StreamSocket&); //Manage New Player connection | clear if necessary
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
private:
	bool isNameSet(); //returns true if name is set
	void ClearQueue(); //Clear send queue
	void ProcessQueue(); //Returns true if connection is closed
	void generateConnectionHash(); //Generate a new connection hash
	void setAuthStep(char);
	char getAuthStep();
	void sendTime();//Sends time if required
	long long getTicks();
	void IncrementTicks();
};

#endif