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
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Runnable.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Thread.h>
#include "ThreadSafeQueue.h"
#include "Structs.h"
#include "NetworkIO.h"
#include "ChunkProvider.h"
#include "EntityFlags.h"
#include "NetworkWriter.h"

class EntityProvider;
class PlayerPool;
class ChunkRoot;
class PackingThread;

using std::string;
using std::stringstream;

struct TimeJobs {
	long long LastTimeSend;
	long long LastKeepAliveSend;
	long long LastHandleMovement;
};

class PlayerThread : public Poco::Runnable {
private:
	//Player specific data
	EntityFlags _Flags; //Burning,eating...
	EntityCoordinates _Coordinates; //Coordinates
	EntityCoordinates _lastCoordinates;
	ChunkCoordinates _lastChunkCoordinates;
	string _sName; //Minecraft.net Username
	string _sIP; //IP
	string _sLeaveMessage;
	int _iEntityID;
	short _iHealth;
	short _iFood;
	float _nSaturation;
    bool _fSpawned;
	int _Spawned_PlayerInfoList;

	//TCP stuff
	string _sTemp;
	Poco::Net::StreamSocket _Connection;
	ThreadSafeQueue<string> _lowLevelSendQueue;
	ThreadSafeQueue<string> _highLevelSendQueue;
	NetworkIO _lowNetwork;	
	NetworkIO _highNetwork;

	NetworkWriter _NetworkWriter;
	Poco::Thread _threadNetworkWriter;

	//Needed Classes
	EntityProvider& _rEntityProvider;
	PlayerPool* _pPoolMaster;
	PlayerPoolEvent _ppEvent;
	ChunkProvider _ChunkProvider;

	//Thread specific
	bool _fAssigned;
	long long _iThreadTicks;
	static int _PlayerCount;

	//Verification
	Poco::Net::HTTPClientSession _Web_Session;
	Poco::Net::HTTPResponse _Web_Response;
	string _sConnectionHash;

	//Time jobs
	TimeJobs _TimeJobs;
public:
	/*
	* De- / constructor
	*/
	PlayerThread(EntityProvider&,PlayerPool*,ChunkRoot&,PackingThread&);
	~PlayerThread();


	/*
	* Thread  Main
	*/
	virtual void run(); // Thread Main


	/*
	* Connect a TCP player connection for server <-> interchange
	*/
	void Connect(Poco::Net::StreamSocket&);


	/*
	* Returns the actual Thread state 
	* false:  Thread is free
	* true:  Thread is busy 
	*/
	bool isAssigned(); 


	/*
	* Kicks a player without reason
	* The thread will be free after call
	*/
	void Kick(); 


	/*
	* Kicks a player with reason
	* The thread will be free after call
	*/
	void Kick(string);


	/*
	* Update health and food from client
	* This function will send informations to client
	
	Prameter:
	@1 : Health (0-20)
	@2 : Food   (0-20)
	@3 : Saturation (0.0F - 5.0F)
	*/
	void UpdateHealth(short,short,float);


	/*
	* Interts text into player chat queue

	Parameter:
	@1 : Text to insert 
	*/
	void insertChat(string&);


	/*
	* Returns true if Player's login is done
	*/
	bool isSpawned(); 


	/*
	* Returns Playername
	* If Authstep FC_AUTHSTEP_HANDSHAKE isn't done, it will return a blank string
	*/
	string getUsername();


	/*
	* Returns IP and port of player
	*  IP:port
	*/
	string getIP();


	/*
	* Returns a reference to player's NetworkIO object
	* For right NetworkIO handling, have a look in NetworkIO.h
	*/
	NetworkIO& getConnection();


	/*
	* Returns Players coordinates and look
	*/
	EntityCoordinates getCoordinates();
	

	/*
	* Returns count of actual connected players
	*/
	static int getConnectedPlayers();


	/*
	* Adds/Removes player from PlayerInfo List (Press Tab in Client)

	Parameters:
	@1 : true for spawn player, false for despawn player
	@2 : player name
	*/
	void PlayerInfoList(bool,string&);

	/*
	* Closes connection and clears object

	Parameter:
	@1 : Disconnect Reason - look in Constants.h  (FC_LEAVE_)
	*/
	void Disconnect(char);

	/*
	* These are internal functions 
	* DONT USE THEM
	*/
	void sendLowClientPosition();
	void appendQueue(string&);
private:
	//Queue
	void ClearQueue();
	void ProcessQueue();
	 
	//Interval functions
	void Interval_KeepAlive();
	void Interval_Time();
	void Interval_HandleMovement();

	void sendTime();
	void pushChatEvent(string&);
	void sendClientPosition();

	//Ticks
	long long getTicks(); 

	//Other
	string generateConnectionHash(); //Generate a new connection hash, write it to _ConnectionHash	
	template <class T> T fixRange(T,T,T);

	//Packets - receive only
	void Packet0_KeepAlive();
	void Packet1_Login();
	void Packet2_Handshake();
	void Packet3_Chat();
	void Packet10_Player();
	void Packet11_Position();
	void Packet12_Look();
	void Packet13_PosAndLook();
	void Packet254_ServerListPing();
	void Packet255_Disconnect();
};

#endif