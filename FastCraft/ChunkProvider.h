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
#include "ChunkSet.h"
#include <vector>
#include <sstream>
#include <Poco/DeflatingStream.h>

class PlayerThread;
class NetworkIO;
class ChunkRoot;

using std::vector;

class ChunkProvider {
private:
	vector<ChunkCoordinates> _vSpawnedChunks;
	ChunkCoordinates _PlayerCoordinates;

	NetworkIO* _pNetwork;
	ChunkRoot* _pChunkRoot;
	ChunkSet _ChunkSet;

	//Packing
	std::stringstream _stringStrm;
	Poco::DeflatingOutputStream _deflatingStrm;

	const int _ViewDistance; //chunk count to each direction
	bool _fConnected;
	bool _fNewConnection;
public:
	ChunkProvider(ChunkRoot*,NetworkIO*);
	~ChunkProvider();

	void newConnection();
	void Disconnect();

	void HandleMovement(const EntityCoordinates&);
	bool isFullyCircleSpawned();
public: 
	void sendChunk(MapChunk*); //Sends packed chunk (no prechunk will be send)
	void sendPreChunk_Spawn(int,int);
	void sendPreChunk_Despawn(int,int);
	
	bool isConnected();
	bool isSpawned(ChunkCoordinates);
	bool CheckChunkSet();
	void CheckSpawnedChunkList();
};
#endif