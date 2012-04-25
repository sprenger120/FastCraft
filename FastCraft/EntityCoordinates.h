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

#ifndef _FASTCRAFTHEADER_ENTITYCOORDINATES
#define _FASTCRAFTHEADER_ENTITYCOORDINATES


class EntityCoordinates {
public:
	/*
	* Constructor
	* Will set all variables to 0
	*/
	EntityCoordinates();


	/*
	* Destructor
	*/
	~EntityCoordinates();


	/*
	* Variables
	*/
	double X;
	double Y;
	double Z;
	double Stance;
	float Yaw;
	float Pitch;
	float HeadYaw;
	bool OnGround;

	/*
	* Operators
	* Returns true if difference between variables is less than 0.1
	*/
	bool operator == (const EntityCoordinates&);


	/*
	* Compares only yaw/pitch
	* Returns true if difference between variables is less than 0.1
	*/
	bool LookEqual(const EntityCoordinates&);


	/*
	* Compares only X/Y/Z/Stance
	* Returns true if difference between variables is less than 0.1
	*/
	bool CoordinatesEqual(const EntityCoordinates&);
};
#endif