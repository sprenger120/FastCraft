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

#ifndef _FASTCRAFTHEADER_CONSTANTS
#define _FASTCRAFTHEADER_CONSTANTS

#define FC_PLAYERSPAWNRADIUS			100.0
#define FC_MAXLOGINTIME					10000 //10 Seconds to complete login procedure

//Properties
#define FC_SERVMODE_SURVIVAL			0
#define FC_SERVMODE_CREATIVE			1

#define FC_DIFFICULTY_PEACEFUL			0
#define FC_DIFFICULTY_EASY				1
#define FC_DIFFICULTY_NORMAL			2
#define FC_DIFFICULTY_HARD				3

//Interval in milli seconds
#define FC_INTERVAL_TIMESEND			5000
#define FC_INTERVAL_KEEPALIVE			15000
#define FC_INTERVAL_HANDLEMOVEMENT		500
#define FC_INTERVAL_MOVEMENT			50
#define FC_INTERVAL_CALCULATESPEED		1000
#define FC_INTERVAL_CHECKPOSITION		1000

//Chunk Constants
#define FC_CHUNK_BLOCKCOUNT				32768
#define FC_CHUNK_NIBBLECOUNT			16384
#define FC_CHUNK_DATACOUNT              81920

//Disconnect modes
#define FC_LEAVE_KICK					1
#define FC_LEAVE_QUIT					2
#define FC_LEAVE_OTHER					3

//Animation ID's 
#define FC_ANIM_NOANIM					0	//Send to remove all animations 
#define FC_ANIM_SWINGARM				1   
#define FC_ANIM_DAMAGE					2   //entity gets a red color
#define FC_ANIM_LEAVEBED				3   
#define FC_ANIM_EATFOOD					5   
#define FC_ANIM_CROUCH					104
#define FC_ANIM_UNCROUCH				105

//Actions
#define FC_ACTION_CROUCH				1
#define FC_ACTION_UNCROUCH				2
#define FC_ACTION_LEAVEBED				3
#define FC_ACTION_STARTSPRINTING		4
#define FC_ACTION_STOPSPRINTING			5

//Entity Type ID's
#define FC_ENTITY_PLAYER				1
#define FC_ENTITY_PICKUP				2
#define FC_ENTITY_MOB					3
#define FC_ENITIY_DRIVEABLE				4 //minecarts, boats..

//Queue Types
#define FC_QUEUE_LOW					1
#define FC_QUEUE_HIGH					2

//Entity Status Codes
#define FC_ENTITYSTATUS_HURT			2	
#define FC_ENTITYSTATUS_DEAD			3
#define FC_ENTITYSTATUS_WOLF_TAMING		6
#define FC_ENTITYSTATUS_WOLF_TAMED		7
#define FC_ENTITYSTATUS_WOLF_SHAKE		8
#define FC_ENTITYSTATUS_ACCEPTEATING	9

#endif