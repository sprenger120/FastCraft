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

#ifndef _FASTCRAFTHEADER_PLAYERPOOLEVENT
#define _FASTCRAFTHEADER_PLAYERPOOLEVENT
#include <string>
#include "EntityCoordinates.h"
#include "EntityFlags.h"
#include "ItemSlot.h"
using std::string;
class PlayerThread;

class PlayerPoolEvent{
private:
	//General
	PlayerThread* _pThread; 
	char _JobID;

	//Chat event
	EntityCoordinates _Coordinates; //Used for Move and chat
	string _Message; //Used for chat 	

	//Join/Disconnect event
	string _Name;
	bool _fKicked;
	bool _fMode;
	
	//Animation event
	char _iAnimID;

	//Metadata event
	EntityFlags _Flags;

	//Change held event
	short _iSlotID;
	ItemSlot _Item;
public:
	/*
	* Constructs as a chat event

	Parameter:
	@1 : Coordinates of event source (player who sends message)
	@2 : Message 
	@3 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(EntityCoordinates,string,PlayerThread*);


	/*
	* Construct as a connect/disconnect event

	Parameter:
	@1 : Mode  (true for connect, false for disconnect)
	@2 : Kicked?
	@3 : Username 
	@4 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(bool,bool,string,PlayerThread*);


	/*
	* Construct as a movement event

	Parameter:
	@1 : New position
	@2 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(EntityCoordinates,PlayerThread*);


	/*
	* Construct as an animation event

	Parameter: 
	@1 : Animation ID - For a id table have a look at Constants.h (FC_ANIM_)
	@2 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(char,PlayerThread*);


	/*
	* Construct as a metadata update event

	Parameter:
	@1 : EntityFlags instance of player
	@2 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(EntityFlags,PlayerThread*);


	/*
	* Construct as a held change event

	@1 : SlotID (0=held, 4=helmet,3=chestplate,2=pants,1=boots)
	@2 : Item data
	@3 : this pointer of class that pushes event to queue
	*/
	PlayerPoolEvent(short,ItemSlot,PlayerThread*);

	/*
	* Accessator
	* They will throw Poco::RuntimeException if requested data is unavailable
	*/

	//General
	PlayerThread* getPtr();
	char getJobID();

	//Chat event 
	EntityCoordinates getCoordinates(); // Used in move event too
	string getMessage();

	//Join/Disconnect event
	string getName();
	bool getMode();
	bool isKicked();

	//Animation Event
	char getAnimationID();

	//Metadata event
	EntityFlags getFlags();
	
	//Held change event
	ItemSlot getItem();
	short getSlot();


	/*
	* Destructor
	*/
	~PlayerPoolEvent();
};
#endif