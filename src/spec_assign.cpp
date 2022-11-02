/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern int top_of_world;
extern int dts_are_dumps;
extern int mini_mud;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

SPECIAL(dump);
SPECIAL(pet_shops);
SPECIAL(postmaster);
SPECIAL(receptionist);
SPECIAL(cryogenicist);
SPECIAL(guild);
SPECIAL(mayor);
SPECIAL(bank);
SPECIAL(gen_board);
SPECIAL(taim);
SPECIAL(guardian);
SPECIAL(angry);
SPECIAL(mc);
SPECIAL(fade);
SPECIAL(trolloc);
SPECIAL(yurian);

/* local functions */
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void ASSIGNROOM(int room, SPECIAL(fname));
void ASSIGNMOB(int mob, SPECIAL(fname));
void ASSIGNOBJ(int obj, SPECIAL(fname));

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
	int rnum;

	if ((rnum = real_mobile(mob)) >= 0)
		mob_index[rnum].func = fname;
  
	else if (!mini_mud)
		log("SYSERR: Attempt to assign spec to non-existant mob #%d", mob);
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
	if (real_object(obj) >= 0)
		obj_index[real_object(obj)].func = fname;
  
	else if (!mini_mud)
		log("SYSERR: Attempt to assign spec to non-existant obj #%d", obj);
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
	if (real_room(room) >= 0)
		world[real_room(room)].func = fname;
  
	else if (!mini_mud)
		log("SYSERR: Attempt to assign spec to non-existant room #%d", room);
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
	ASSIGNMOB(500, guardian);
	ASSIGNMOB(2, taim);
	ASSIGNMOB(3, guild);
	ASSIGNMOB(3, taim);
	ASSIGNMOB(101, receptionist);
	ASSIGNMOB(403, receptionist);
	ASSIGNMOB(404, receptionist);
	ASSIGNMOB(8408,receptionist);
	ASSIGNMOB(8487,receptionist);
	ASSIGNMOB(18952,receptionist);
	ASSIGNMOB(1717,receptionist);
	ASSIGNMOB(19001,receptionist);
	ASSIGNMOB(1250,receptionist);
	ASSIGNMOB(8405,receptionist);
	ASSIGNMOB(117,receptionist);
	ASSIGNMOB(8023,receptionist);
	ASSIGNMOB(8079,receptionist);
	ASSIGNMOB(8022,receptionist);
	ASSIGNMOB(8018,receptionist);
	ASSIGNMOB(8077,receptionist);
	ASSIGNMOB(8016,receptionist);
	ASSIGNMOB(8172,receptionist);
	ASSIGNMOB(106,  bank);
	ASSIGNMOB(8020, bank);
	ASSIGNMOB(5412, bank);
	ASSIGNMOB(102, bank);
	ASSIGNMOB(4, mc);
	ASSIGNMOB(9905, yurian);
	ASSIGNMOB(104, angry);	
	ASSIGNMOB(1201, postmaster);
	ASSIGNMOB(2002, trolloc);
	ASSIGNMOB(2007, fade);
	ASSIGNMOB(400, guild);
	ASSIGNMOB(8016, guild);
	ASSIGNMOB(2007, guild);
	ASSIGNMOB(19502, guild);
	ASSIGNMOB(1731, guild);
	ASSIGNMOB(1732, guild);
	ASSIGNMOB(1733, guild);
	ASSIGNMOB(8172, guild);
	ASSIGNMOB(7, yurian);
	ASSIGNMOB(7, guild);

}



/* assign special procedures to objects */
void assign_objects(void)
{

	ASSIGNOBJ(402, gen_board);	// immortal board
	ASSIGNOBJ(400, gen_board);	// mortal board
}



// assign special procedures to rooms
void assign_rooms(void)
{
	//ASSIGNROOM(3030, dump);
	//ASSIGNROOM(3031, pet_shops);
}
