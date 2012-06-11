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

#ifndef _FASTCRAFTHEADER_PLAYERTHREAD
#define _FASTCRAFTHEADER_PLAYERTHREAD
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Thread.h>
#include <Poco/Random.h>
#include <vector>
#include "ThreadSafeQueue.h"
#include "Structs.h"
#include "NetworkIn.h"
#include "NetworkOutRoot.h"
#include "NetworkOut.h"
#include "ChunkProvider.h"
#include "EntityFlags.h"
#include "NetworkWriter.h"
#include "PlayerInventory.h"
#include "ItemSlot.h"
#include "EntityCoordinates.h"
#include "EntityPlayer.h"
#include <utility>
#include "TimeJobServer.h"
#include "ServerThreadBase.h"
#include "Heap.h"
#include "EntityLiving.h"

class World;
class PackingThread;
class MinecraftServer;
class EntityListEntry;

using std::string;
using std::stringstream;
using std::vector;
using std::pair;

class PlayerThread : public ServerThreadBase {
private:
	/* Client data */
	string _sName;
	string _sLeaveMessage;

	int _iEntityID;
	short _iHealth;
	short _iFood;
	float _nSaturation;

	EntityFlags _Flags; 
	EntityCoordinates _Coordinates;
	EntityCoordinates _lastCoordinates;
	ChunkCoordinates _lastChunkCoordinates;

	int _Spawned_PlayerInfoList;
	double _dRunnedMeters;


	/* System */
	bool _fSpawned;
	bool _fHandshakeSent;
	bool _fAssigned;


	/* Network */
	string _sIP;
	string _sTemp;
	Poco::Net::StreamSocket _Connection;
	NetworkOutRoot _NetworkOutRoot;
	NetworkIn _NetworkInRoot;
	short _iPlayerPing;


	/* Data delivering */
	PackingThread& _rPackingThread;
	ChunkProvider _ChunkProvider;
	NetworkWriter _NetworkWriter;


	/* Premium verification */
	string _sConnectionHash;
	Poco::Net::HTTPClientSession _Web_Session;
	Poco::Net::HTTPResponse _Web_Response;
	Poco::Random _Rand;


	/* Other classes */
	PlayerInventory _Inventory;
	MinecraftServer* _pMinecraftServer;
	World* _pActualWorld;
	Heap<EntityListEntry*,int> _heapSpawnedEntities;

    /* Time Management */
	TimeJobServer<PlayerThread> _timeJobServer;
    Poco::Stopwatch _timer_Ping;
	Poco::Stopwatch _timer_LastBlockPut;
	Poco::Stopwatch _timer_StartedEating;
	Poco::Stopwatch _timer_lastSpeedCalculation;
	Poco::Stopwatch _timer_lastArmSwing;
	Poco::Stopwatch _timer_lastPositionUpdateEvent;
public:
	/*
	* Constructor

	Parameter:
	@1 : PackingThread of this Minecraft Server instance
	@2 : MinecraftServer instance that runs this class
	*/
	PlayerThread(PackingThread&,MinecraftServer*);


	/* 
	* Destructor
	*/
	~PlayerThread();


	/*
	* Thread  Main
	*/
	void run();


	/*
	* Sets the actual connection
	* Disconnects the old player
	*/
	void Connect(Poco::Net::StreamSocket&);


	/*
	* Returns the actual Thread state 
	* false:  Thread is free
	* true:  Thread is busy 
	*/
	bool isAssigned(); 


	/*
	* Interts text into player chat queue

	Parameter:
	@1 : Text to insert 
	*/
	void insertChat(string);


	/*
	* Returns true if Player's login is done
	*/
	bool isSpawned(); 


	/*
	* Returns Playername
	*/
	string getUsername();


	/*
	* Returns IP and port of player
	* IP:port
	*/
	string getIP();


	/*
	* Returns Players coordinates and look
	*/
	EntityCoordinates getCoordinates();


	/*
	* Adds/Removes player from PlayerInfo List (Press Tab in Client)

	Parameters:
	@1 : true for spawn player, false for despawn player
	@2 : player name
	*/
	void PlayerInfoList(bool,string);


	/*
	* Closes connection and clears object

	Parameter:
	@1 : Kick message
	@2 : Disconnect reason code (FC_LEAVE_ in Constants.h)
	@3 : Set to true to show the kick message into chat 
	*/
	void Disconnect(string,bool = true);
	void Disconnect(char);

	/*
	* Spawns a player
	* Will throw FCRuntimeException if ID is already spawned

	Parameter:
	@1 : Valid pointer to a Entity-derived class
	*/
	void spawnEntity(Entity*);


	/*
	* Returns true if given entity id is spawned

	Parameter:
	@1 : EntityID
	*/
	bool isEntitySpawned(int);

	
	/*
	* Updates entitys position
	* Will throw FCRuntimeException if entity isn't spawned so far
	* Throws FCRuntimeException if pointer is NULL

	Parameter:
	@1 : Valid pointer to an entity-derived class
	*/
	void updateEntityPosition(Entity*);


	/*
	* Despawns entity
	* Will throw FCRuntimeException if entity isn't spawned so far

	Parameter:
	@1 : EntityID
	*/
	void despawnEntity(int);


	/*
	* Returns player's entity ID
	* Will throw Poco::RuntimeExcpetion if player isn't spawned so far
	*/
	int getEntityID();


	/*
	* Makes the given entity do a animation 
	* Will throw FCRuntimeException if Entity isn't spawned 
	* Throws FCRuntimeException if animation ID doesn't exists
	* Use only for swing arm!

	Parameter:
	@1 : EntityID
	@2 : Animation ID
	*/
	void playAnimationOnEntity(int,char);


	/*
	* Makes the given entity do an action
	* Will throw FCRuntimeException if Entity isn't spawned 

	Parameter:
	@1 : EntityID
	@2 : new EntityFlags
	*/
	void updateEntityMetadata(EntityLiving*);


	/*
	* Returns a reference to player's inventory
	*/
	PlayerInventory& getInventory();


	/*
	* Returns players flags
	*/
	EntityFlags getFlags();


	/*
	* Updates entitys equipment

	Parameter:
	@1 : EntityID
	*/
	void updateEntityEquipment(EntityLiving*);


	/*
	* Returns chunk queue size
	*/
	int getChunksInQueue();


	/*
	* Sets a block into players view circle
	* Will ignore call if given chunk isn't spawned
	* Will throw FCRuntimeException if block not exists

	Parameter:
	@1 : Coordiantes of block
	@2 : Block ID
	*/
	void spawnBlock(BlockCoordinates,ItemID);


	/*
	* Returns pointer to world who player is actually in
	* Throws FCRuntimeException if player isn't logged in
	*/
	World* getWorld();


	/*
	* Sets the statuscode of an entity

	Parameter:
	@1 : Entity ID
	@2 : Status code (Constants.h FC_ENTITYSTATUS_)
	*/
	void setEntityStatus(int,char);
	void setEntityStatus(char);     //Sets status on player's id


    /* 
    * Returns actual player ping 
    */
    short getPing();


	/*
	* Returns the pointer to the minecraft server instance
	*/
	MinecraftServer* getMinecraftServer();
private:
	//Interval functions
	void Interval_KeepAlive();
	/*void Interval_CheckSpeed();*/
	void Interval_CheckEating();

	//Packets
	void Packet0_KeepAlive();
	void Packet1_Login();
	void Packet2_Handshake();
	void Packet3_Chat();
	void Packet10_Player();
	void Packet11_Position();
	void Packet12_Look();
	void Packet13_PosAndLook();
	void Packet14_Digging();
	void Packet15_PlayerBlockPlacement();
	void Packet16_HoldingChange();
	void Packet18_Animation();
	void Packet19_EntityAction();
	void Packet101_CloseWindow();
	void Packet102_WindowClick();
	void Packet254_ServerListPing();
	void Packet255_Disconnect();

	//Network
	void ProcessQueue();
	void sendTime();
	void sendClientPosition();
	void sendKeepAlive();
	void ChatToAll(string&);
    void updatePing();

	//Handler
	void handleEating(); 


	//'fix' functions
	template <class T> T fixRange(T,T,T);

	//sync functions
	void syncHealth();
	void syncFlagsWithPP();
	void syncMovement(); 


	//Other
	string generateConnectionHash(); //Generate a new connection hash, write it to _ConnectionHash	
	void CheckPosition(); //checks players position and correct it. Will synchronize with player if bool is true
};

#endif
