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
#include "ItemInfoStorage.h"
#include "Structs.h"
#include <string>

using std::string;


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
	~PlayerChatEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~ChatEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~PlayerJoinEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~PlayerDisconnectEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~PlayerAnimationEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~PlayerUpdateFlagsEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
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
	~PlayerMoveEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
};



/*
* Changing held items
*/
class PlayerChangeHeldEvent : public PlayerEventBase {
private:
	ItemID _Item;
	short _iSlot;
	bool _fIgnore;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	@2 : Item data of held
	@3 : Slot (0=In hand, 1-4 = Amor)
	*/
	PlayerChangeHeldEvent(PlayerThread*,ItemID,short);
	~PlayerChangeHeldEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
};




/*
* Set / Digg Block Events
*/
class PlayerSetBlockEvent : public PlayerEventBase {
private:
	ItemID _Item;
	BlockCoordinates _Coordinates;
	bool _fIgnore;
public:
	/*
	* Constructor
	
	Parameter:
	@1 : this pointer of PlayerThread class that pushes event
	@2 : Coordiantes of Block
	@3 : Item data of set item
	*/
	PlayerSetBlockEvent(PlayerThread*,BlockCoordinates,ItemID);
	~PlayerSetBlockEvent();

	virtual void Execute(vector<PlayerThread*>&,PlayerPool*);
};
#endif