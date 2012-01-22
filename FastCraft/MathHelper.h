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

#ifndef _FASTCRAFTHEADER_MATHHELPER
#define _FASTCRAFTHEADER_MATHHELPER
#include "EntityCoordinates.h"
#include "Structs.h"

class MathHelper {
private:
	MathHelper();
	~MathHelper();
public:
	/*
	* Calculates distance between two points
	* 1 = source, 2=target

	Parameter:
	@1 : x1
	@2 : y1

	@3 : x2
	@4 : y2

	Parameter:
	@1 : EntityCoordinates1
	@2 : EntityCoordinates2
	*/
	static double distance2D(double,double,double,double);
	static double distance2D(EntityCoordinates,EntityCoordinates);
	
	/*
	* Calculates distance between two points
	* 1 = source, 2=target

	Parameter:
	@1 : x1
	@2 : y1
	@3 : z1

	@4 : x2
	@5 : y2
	@6 : z2

	Parameter:
	@1 : EntityCoordinates1
	@2 : EntityCoordinates2

	Parameter:
	@1 : BlockCoordinates1
	@2 : BlockCoordinates2
	*/
	static double distance3D(double,double,double, double,double,double);
	static double distance3D(EntityCoordinates,EntityCoordinates);
	static double distance3D(BlockCoordinates,BlockCoordinates);
};
#endif