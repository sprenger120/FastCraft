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

#define FC_JOB_NO						0
#define FC_JOB_CLOSECONN				1

//Authentification steps
#define FC_AUTHSTEP_NOTCONNECTED		0
#define FC_AUTHSTEP_CONNECTEDONLY		1
#define FC_AUTHSTEP_HANDSHAKE			2
#define FC_AUTHSTEP_TIME				3
#define FC_AUTHSTEP_PRECHUNKS			4
#define FC_AUTHSTEP_SPAWNPOS			5
#define FC_AUTHSTEP_INVENTORY			6
#define FC_AUTHSTEP_USERPOS				7
#define FC_AUTHSTEP_DONE				8

//Entitys
#define FC_ENTITY_PLAYER				1

//Properties
#define FC_SERVMODE_SURVIVAL			0
#define FC_SERVMODE_CREATIVE			1

#define FC_DIFFICULTY_PEACEFUL			0
#define FC_DIFFICULTY_EASY				1
#define FC_DIFFICULTY_NORMAL			2
#define FC_DIFFICULTY_HARD				3

//Interval
#define FC_INTERVAL_TIMESEND			5000
#define FC_INTERVAL_KEEPACTIVE			15000

//Map Constants
#define FC_CHUNK_BLOCKCOUNT				32768
#define FC_CHUNK_NIBBLECOUNT			16384
#define FC_CHUNK_DATACOUNT              81920

#endif