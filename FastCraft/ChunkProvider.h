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

#ifndef _FASTCRAFTHEADER_CHUNKPROVIDER
#define _FASTCRAFTHEADER_CHUNKPROVIDER
#include "Structs.h"
#include "EntityCoordinates.h"
#include "PackingThread.h"
#include <vector>
#include "ServerThreadBase.h"

class NetworkOutRoot;
class World;
class PlayerThread;
class MinecraftServer;

using std::vector;

class ChunkProvider  : public ServerThreadBase {
private:
	vector<ChunkCoordinates> _vSpawnedChunks;
	vector<ChunkCoordinates> _vToBeSendChunks;
	ChunkCoordinates _PlayerCoordinates;
	ChunkCoordinates _oldPlayerCoordinates;

	World* _pWorld;
	NetworkOutRoot& _rNetwork;
	PlayerThread* _pPlayer;
	MinecraftServer* _pMCServer;

	static char _sDespawnCode[];
	bool _fHasToClear;
public:
	/* 
	* Constructor

	Parameter:
	@1 : Clients NetworkOutRoot instance
	@2 : Servers PackingThread instance
	@3 : Clients this pointer
	*/ 
	ChunkProvider(NetworkOutRoot&,PlayerThread*,MinecraftServer*);


	/*
	* Destructor
	*/
	~ChunkProvider();

	/*
	* handle functions
	*/
	void HandleNewPlayer();
	void HandleDisconnect();


	/*
	* Thread main
	*/
	void run();
private:
	bool isSpawned(ChunkCoordinates&);
	void CheckChunkList(vector<ChunkCoordinates>&,bool=true);
	void despawnChunk(ChunkCoordinates&);

	void NextChunks();
	void CheckChunkCircle();
	void addSpawnJob(int,int);
};
#endif