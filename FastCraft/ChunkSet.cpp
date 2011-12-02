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

#include "ChunkSet.h"
#include <Poco/Exception.h>

#include <iostream>
using std::cout;

ChunkSet::ChunkSet(int iViewDistance) :
_vChunkSet(0)
{
	if (iViewDistance <= 0) {
		throw Poco::RuntimeException("Illegal View distance");
	}

	_iViewDistance = iViewDistance;
	_vChunkSet.resize(CalculateChunkCount());
	_fClear = true;
}

ChunkSet::~ChunkSet() {
	_vChunkSet.clear();
}

void ChunkSet::regenerate(ChunkCoordinates playerCoordinates) {
	ChunkCoordinates SquareStart,SquareEnd;
	_fClear = false;

	SquareStart.X = playerCoordinates.X - (_iViewDistance/2) - 1;
	SquareStart.Z = playerCoordinates.Z - (_iViewDistance/2) - 1;

	SquareEnd.X = SquareStart.X + _iViewDistance;
	SquareEnd.Z = SquareStart.Z + _iViewDistance;

	int index=0;
	for ( int X = SquareStart.X;X<=SquareEnd.X;X++) {
		for ( int Z = SquareStart.Z;Z<=SquareEnd.Z;Z++) {
			_vChunkSet[index].X = X;
			_vChunkSet[index].Z = Z;
			index++;
		}
	}

	_PlayerCoordinates = playerCoordinates;
}

bool ChunkSet::isUp2Date(ChunkCoordinates playerCoordinates) {
	if (_PlayerCoordinates.X == playerCoordinates.X && _PlayerCoordinates.Z == playerCoordinates.Z) {
		return true;
	}else{
		return false;
	}
}

void ChunkSet::clear() {
	_fClear = true;
}

bool ChunkSet::isEmpty() {
	return _fClear;
}

ChunkCoordinates ChunkSet::at(int index) {
	if ( index > (_vChunkSet.size() -1) || index < 0 ) {
		throw Poco::RuntimeException("Invalid index");
	}

	return _vChunkSet[index];
}

int ChunkSet::CalculateChunkCount(int iVD) {
	iVD /= 2;
	return iVD * 4 + (iVD * iVD) * 4 + 1; 
}

int ChunkSet::CalculateChunkCount() {
	int VDdiv2 = _iViewDistance/2;

	return VDdiv2 * 4 + (VDdiv2 * VDdiv2) * 4 + 1; 
}