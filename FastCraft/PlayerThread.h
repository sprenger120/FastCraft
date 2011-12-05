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
	long long LastHandleMovement;
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
	queue<string> _ChatQueue;
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
	/*
	* De- / constructor
	*/
	PlayerThread(SettingsHandler*,EntityProvider*,ServerTime*,PlayerPool*,ChunkRoot*,PackingThread*);
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
	* Interts text into player chat

	Parameter:
	@1 : Text to insert 
	*/
	void insertChat(string&);


	/*
	* Returns true if PlayerPool can spawn Entits
	* This means that AuthStep FC_AUTHSTEP_PRECHUNKS is reached
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
	* Returns Players full coordinates and look
	*/
	EntityCoordinates getCoordinates();
	
	/*
	* Returns actual authentification step of player 
	* For authentification steps, have a look in the wiki or in Constants.h
	*/ 
	char getAuthStep();

	/*
	* Returns count of actual connected players
	*/
	static int getConnectedPlayers();

	/*
	* These are internal functions 
	* DONT USE THEM
	*/
	void sendClientPosition();
	void appendQueue(QueueJob&);
private:
	//Queue
	void ClearQueue(); //Clear send queue
	void ProcessQueue(); 

	//AuthStep
	void setAuthStep(char);
	bool isLoginDone(); //returns true if the full login procedure done
	void ProcessAuthStep();
	bool isNameSet(); //returns true if name is set

	//send... Functions
	void sendKeepAlive();
	void sendTime();//Sends time if required


	//Ticks
	long long getTicks(); 

	//Other
	void generateConnectionHash(); //Generate a new connection hash	
	template <class T> T fixRange(T,T,T);
	void Disconnect(bool = false); //Clear Player object

	//Packets
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