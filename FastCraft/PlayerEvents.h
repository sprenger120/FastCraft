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

#ifndef _FASTCRAFTHEADER_PLAYEREVENTS
#define _FASTCRAFTHEADER_PLAYEREVENTS
#include "PlayerEventBase.h"
#include "EntityCoordinates.h"
#include "EntityFlags.h"
#include "ItemInformationProvider.h"
#include "Structs.h"
#include <string>

using std::string;
class World;

/*
* Chat Events
*/
class PlayerChatEvent : public PlayerEventBase {
private:
	string _sMessage;
	EntityCoordinates _Coordinates;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : pointer to PlayerThread class that pushes event (Use Chat event for non player typed messages)
	@2 : Message
	@3 : PlayersPosition at the moment of submission
	*/
	PlayerChatEvent(PlayerThread*,string,EntityCoordinates);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};

class ChatEvent : public PlayerEventBase {
private:
	string _sMessage;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Message
	*/
	ChatEvent(string);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};




/*
* Join/Disconnect Events
*/
class PlayerJoinEvent : public PlayerEventBase {
public:
	/*
	* Constructor
	
	Parameter:
	@1 : pointer to PlayerThread class that pushes event
	*/
	PlayerJoinEvent(PlayerThread*);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};

class PlayerDisconnectEvent : public PlayerEventBase {
private:
	int _iEntityID;
	string _sName;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : pointer to PlayerThread class that pushes event
	@2 : EntityID of player
	@3 : Name of player
	*/
	PlayerDisconnectEvent(PlayerThread*,int,string);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};




/*
* Animation / Metadata Update / Move Event
*/
class PlayerAnimationEvent : public PlayerEventBase {
private:
	char _iAnimationID;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	@2 : Animation ID (Constants.h FC_ANIM_...)
	*/
	PlayerAnimationEvent(PlayerThread*,char);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};

class PlayerUpdateFlagsEvent : public PlayerEventBase {
private:
	EntityFlags _Flags;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	@2 : EntityFlags instance of player
	*/
	PlayerUpdateFlagsEvent(PlayerThread*,EntityFlags);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};

class PlayerMoveEvent : public PlayerEventBase {
private:
	EntityCoordinates _newCoordinates;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	@2 : new Position
	*/
	PlayerMoveEvent(PlayerThread*,EntityCoordinates);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};



/*
* Changing held items
*/
class PlayerChangeHeldEvent : public PlayerEventBase {
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	*/
	PlayerChangeHeldEvent(PlayerThread*);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};




/*
* Set / Digg Block Events
*/
class PlayerSetBlockEvent : public PlayerEventBase {
private:
	ItemID _Item;
	BlockCoordinates _Coordinates;
	World* _pWorld;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : Coordiantes of Block
	@2 : Item data of set item
	@3 : World who pushed this event
	*/
	PlayerSetBlockEvent(BlockCoordinates,ItemID,World*);

	void Execute(vector<PlayerThread*>&,PlayerPool*);
};
#endif