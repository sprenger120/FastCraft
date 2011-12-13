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

#ifndef _FASTCRAFTHEADER_CHUNKPROVIDER
#define _FASTCRAFTHEADER_CHUNKPROVIDER

#include "Structs.h"
#include <vector>

class NetworkIO;
class ChunkRoot;
class PackingThread;
class PlayerThread;

using std::vector;

class ChunkProvider {
private:
	vector<ChunkCoordinates> _vSpawnedChunks;
	ChunkCoordinates _PlayerCoordinates;
	ChunkCoordinates _oldPlayerCoordinates;

	bool _fNewPlayer;
	ChunkRoot& _rChunkRoot;
	NetworkIO& _rNetwork;
	PackingThread& _rPackingThread;
	PlayerThread* _pPlayer;
public:
	ChunkProvider(ChunkRoot&,NetworkIO&,PackingThread&,PlayerThread*);
	~ChunkProvider();

	void HandleNewPlayer();
	void HandleDisconnect();

	void HandleMovement(const EntityCoordinates&);
	
public: 
	bool CheckChunkCircle();
	void CheckSpawnedChunkList();
	void sendDespawn(int,int);
	void sendSpawn(int,int);
	void AddChunkToList(int,int);
	
	bool isSpawned(ChunkCoordinates);
	bool isChunkListEmpty();
};
#endif