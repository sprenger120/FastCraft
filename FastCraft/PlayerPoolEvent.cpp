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
#include "PlayerPoolEvent.h"
#include "Constants.h"
#include <Poco/Exception.h>

//chat event
PlayerPoolEvent::PlayerPoolEvent(EntityCoordinates coord,string message,PlayerThread* pPlayer) :
_Message(message),
_Name("")
{
	_Coordinates = coord;
	_pThread = pPlayer;
	_JobID = FC_PPEVENT_CHAT;
}

//join/disconnect event
PlayerPoolEvent::PlayerPoolEvent(bool fMode,bool fKicked,string Name,PlayerThread* pPlayer) :
_Message(""),
_Name(Name)
{
	_fMode = fMode;
	_fKicked = fKicked;
	if (fKicked && fMode) {
		throw Poco::RuntimeException("Connection and kick flags can't be true at same time");
	}
	_pThread = pPlayer;
	if (fMode) {
		_JobID = FC_PPEVENT_JOIN;
	}else{
		_JobID = FC_PPEVENT_DISCONNECT;
	}
}

//movement event
PlayerPoolEvent::PlayerPoolEvent(EntityCoordinates coord,PlayerThread* pPlayer) :
_Message(""),
_Name("")
{
	_JobID = FC_PPEVENT_MOVE;
	_Coordinates = coord;
	_pThread = pPlayer;
}

//animation event
PlayerPoolEvent::PlayerPoolEvent(char anim,PlayerThread* p) :
_Message(""),
_Name("")
{
	_JobID = FC_PPEVENT_ANIMATION;
	_iAnimID = anim;
	_pThread = p;
}

PlayerPoolEvent::~PlayerPoolEvent() {
	_Message.clear();
	_Name.clear();
}

EntityCoordinates PlayerPoolEvent::getCoordinates() {
	if (_JobID != FC_PPEVENT_MOVE && _JobID != FC_PPEVENT_CHAT) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _Coordinates;
}

string PlayerPoolEvent::getMessage() {
	if (_JobID != FC_PPEVENT_CHAT) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _Message;
}

char PlayerPoolEvent::getJobID() {
	return _JobID;
}

bool PlayerPoolEvent::getMode() {
	if (_JobID != FC_PPEVENT_JOIN && _JobID != FC_PPEVENT_DISCONNECT) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _fMode;
}

char PlayerPoolEvent::getAnimationID() {
	if (_JobID != FC_PPEVENT_ANIMATION) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _iAnimID;
}

PlayerThread* PlayerPoolEvent::getPtr() {
	return _pThread;
}

bool PlayerPoolEvent::isKicked() {
	if (_JobID != FC_PPEVENT_DISCONNECT) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _fKicked;
}

string PlayerPoolEvent::getName() {
	if (_JobID != FC_PPEVENT_JOIN && _JobID != FC_PPEVENT_DISCONNECT) {
		throw Poco::RuntimeException("Data unavailable");
	}
	return _Name;
}