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
#include <Poco/NumberFormatter.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Runnable.h>
#include <Poco/Net/StreamSocket.h>
#include <queue>
#include "Structs.h"
#include "NetworkIO.h"
#include "ChunkProvider.h"

class SettingsHandler;
class EntityProvider;
class ServerTime;
class PlayerPool;
class ChunkRoot;
class PackingThread;

using std::string;
using std::queue;

struct TimeJobs {
	long long LastTimeSend;
	long long LastKeepAliveSend;
};

class PlayerThread : public Poco::Runnable {
private:
	//Player specific data
	PlayerFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	string _sName; //Minecraft.net Username and Ingame Nickname
	string _sIP; //IP
	int _iEntityID;
	char _iLoginProgress;
	short _iHealth;
	short _iFood;
	float _nSaturation;

	//TCP stuff
	Poco::Net::StreamSocket _Connection;
	string _sTemp;
	queue<QueueJob> _SendQueue;
	NetworkIO _Network;

	//Connection Hash
	string _sConnectionHash;

	//Class pointer
	SettingsHandler* _pSettings;
	EntityProvider* _pEntityProvider;
	ServerTime* _pServerTime;
	PlayerPool* _pPoolMaster;

	ChunkProvider _ChunkProvider;

	//Thread specific
	bool _fAssigned;//true if a player is assigned to that thread
	long long _iThreadTicks;
	int _iThreadID;
	static int PlayerCount;

	//Verification
	Poco::Net::HTTPClientSession _Web_Session;
	Poco::Net::HTTPResponse _Web_Response;

	//Time jobs
	TimeJobs _TimeJobs;
public:
	//De- / Constructor
	PlayerThread(SettingsHandler*,EntityProvider*,ServerTime*,PlayerPool*,ChunkRoot*,PackingThread*);
	~PlayerThread();

	virtual void run(); // Thread Main

	void Disconnect(bool = false); //Clear Player object
	void Connect(Poco::Net::StreamSocket&); //Manage New Player connection | clear if necessary
	bool isAssigned(); //Returns true if a player is assigned to this thread

	void Kick(); //Kicks player without reason
	void Kick(string); //Kick with reason
	void Ban(int); //Bans player (int = Expiration, -1 for permanent ban)
	void Ban(string,int); //Bans player with reason (int = Expiration, -1 for permanent ban)
	void UpdateHealth(short,short,float);

	//For PlayerPool
	bool isSpawned(); //Returns true if the playerpool can spawn entitys

	//Accessator
	string getUsername(); //Returns Player Name - not the edited nikname
	string getIP(); //Returns actual IP of player
	NetworkIO& getConnection();
	EntityCoordinates getCoordinates();

	//Queue
	void appendQueue(QueueJob&); //Adds a job for the sending queue

	static int getConnectedPlayers();
private:
	//Queue
	void ClearQueue(); //Clear send queue
	void ProcessQueue(); 

	//AuthStep
	void setAuthStep(char);
	char getAuthStep();
	bool isLoginDone(); //returns true if the full login procedure done
	void ProcessAuthStep();
	bool isNameSet(); //returns true if name is set

	//send... Functions
	void sendClientPosition();
	void sendKeepAlive();
	void sendTime();//Sends time if required


	//Ticks
	long long getTicks();
	void IncrementTicks();

	//Other
	void generateConnectionHash(); //Generate a new connection hash	
	template <class T> T fixRange(T,T,T);
};

#endif