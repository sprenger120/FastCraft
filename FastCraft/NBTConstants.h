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
#ifndef _FASTCRAFTHEADER_NBT_CONSTANTS
#define _FASTCRAFTHEADER_NBT_CONSTANTS

#define FC_NBT_TYPE_BASE					0
#define FC_NBT_TYPE_BYTE					1
#define FC_NBT_TYPE_SHORT					2
#define FC_NBT_TYPE_INT						3
#define FC_NBT_TYPE_INT64					4
#define FC_NBT_TYPE_FLOAT					5
#define FC_NBT_TYPE_DOUBLE					6
#define FC_NBT_TYPE_BYTEARRAY				7
#define FC_NBT_TYPE_STRING					8
#define FC_NBT_TYPE_LIST					9
#define FC_NBT_TYPE_COMPOUND				10

#define FC_NBT_OUTPUT_RAW					false
#define FC_NBT_OUTPUT_GZIP					true

#define FC_NBT_INPUT_RAW					false
#define FC_NBT_INPUT_GZIP					true

#define FC_NBT_FLAG_HEADERLESS				true
#endif