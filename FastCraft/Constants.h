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

#ifndef _FASTCRAFTHEADER_CONSTANTS
#define _FASTCRAFTHEADER_CONSTANTS

#define FC_JOB_NONE						0
#define FC_JOB_CLOSECONN				1

//Entitys
#define FC_ENTITY_PLAYER				1

//Properties
#define FC_SERVMODE_SURVIVAL			0
#define FC_SERVMODE_CREATIVE			1

#define FC_DIFFICULTY_PEACEFUL			0
#define FC_DIFFICULTY_EASY				1
#define FC_DIFFICULTY_NORMAL			2
#define FC_DIFFICULTY_HARD				3

//Interval in milli seconds
#define FC_INTERVAL_TIMESEND			5000
#define FC_INTERVAL_KEEPACTIVE			15000

//Chunk Constants
#define FC_CHUNK_BLOCKCOUNT				32768
#define FC_CHUNK_NIBBLECOUNT			16384
#define FC_CHUNK_DATACOUNT              81920

//Disconnect modes
#define FC_LEAVE_KICK					1
#define FC_LEAVE_QUIT					2
#define FC_LEAVE_OTHER					3

//PlayerPool Events
#define FC_PPEVENT_CHAT					1
#define FC_PPEVENT_MOVE					2
#define FC_PPEVENT_JOIN					3
#define FC_PPEVENT_DISCONNECT			4
#define FC_PPEVENT_ANIMATION			5
#endif