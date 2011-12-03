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

class PlayerThread;
class NetworkIO;
class ChunkRoot;
class PackingThread;

using std::vector;

class ChunkProvider {
private:
	vector<ChunkCoordinates> _vSpawnedChunks;
	ChunkCoordinates _PlayerCoordinates;
	ChunkCoordinates _oldPlayerCoordinates;

	NetworkIO* _pNetwork;
	ChunkRoot* _pChunkRoot;
	PackingThread* _pPackingThread;

	const int _ViewDistance; //chunk count to each direction
	bool _fConnected;
	bool _fNewConnection;
public:
	ChunkProvider(ChunkRoot*,NetworkIO*,PackingThread*);
	~ChunkProvider();

	void newConnection();
	void Disconnect();

	void HandleMovement(const EntityCoordinates&);
	bool isFullyCircleSpawned();
public: 
	void sendDespawn(int,int);
	void sendSpawn(int,int);
	
	bool isConnected();
	bool isSpawned(ChunkCoordinates);
	bool CheckChunkSet();
	void CheckSpawnedChunkList();

	bool playerChangedPosition();
	int CalculateChunkCount();

	void AddChunkToList(int,int);
};
#endif