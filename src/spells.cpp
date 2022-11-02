/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "interpreter.h"
#include "olc.h"
#include "screen.h"

extern sh_int r_human_start_room;
extern sh_int r_trolloc_start_room;
extern int top_of_world;
extern const char *spells[];
extern struct room_data *world;
extern struct obj_data *object_list;
extern char_data *character_list;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern struct gate_data *first_gate, *last_gate;

extern FILE *player_fl;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern struct apply_mod_defaults *apmd;

#define NOLOC_CHILL_MESSAGE		"You feel a chill pass through your body.\r\n"

void clearMemory(char_data * ch);
void add_follower(char_data * ch, char_data * leader);
int mag_savingthrow(char_data * ch, int type);
void name_to_drinkcon(struct obj_data * obj, int type);
void name_from_drinkcon(struct obj_data * obj);

#define IS_NORTH		1
#define IS_EAST			2
#define IS_SOUTH		3
#define IS_WEST			4

#define IS_NORTHEAST	5
#define IS_NORTHWEST	6
#define IS_SOUTHEAST	7
#define IS_SOUTHWEST	8

#define IS_HERE			9

#define HERE			0
#define CLOSE			1
#define FAR				2
#define VERY_FAR		3
#define EXTREMELY_FAR	4

int reverse_gate_code(char *code)
{
	return (NOWHERE);
}

char *gate_code(int room)
{
	static char code[256];

	sprintf(code, "");

	return (code);
}

/* Tulon: 5-28-2004. Will return 1 if the person has a special item that will not allow them to be located. */
int special_noloc(char_data *ch)
{
	struct obj_data *obj;
	int i = 0;

	if(!ch)
		return 0;

	for(i = 0;i < NUM_WEARS;i++)
	{
		if(!(obj = GET_EQ(ch, i)))
			continue;

		if(IS_SILVER_CUFF(obj))
			return 1;

		if(obj->contains)
		{
			if(search_for_content(obj, SILVER_CUFF, THOROUGH))
				return 1;
		}
	}

	for(obj = ch->carrying;obj;obj = obj->next_content)
	{

		if(IS_SILVER_CUFF(obj))
			return 1;

		if(obj->contains)
		{
			if(search_for_content(obj, SILVER_CUFF, THOROUGH))
				return 1;
		}
	}

	return 0;
}

int find_distance(int to, int from)
{
	int x = 0;
	int y = 0;

	x = zone_table[to].x - zone_table[from].x;
	y = zone_table[to].y - zone_table[from].y;
	
	x = abs(x);
	y = abs(y);

	return MAX(x, y);
}

int find_zone_slope(int zone_obj, int zone_char)
{
	int loc_x = 0;	//East to West
	int loc_y = 0;	//North to South
	int loc;		//Final location

	/* First start off by finding out the basic location	*/

	if(zone_table[zone_obj].x > zone_table[zone_char].x)
		loc_x = IS_EAST;

	if(zone_table[zone_obj].x < zone_table[zone_char].x)
		loc_x = IS_WEST;

	if(zone_table[zone_obj].y > zone_table[zone_char].y)
		loc_y = IS_NORTH;

	if(zone_table[zone_obj].y < zone_table[zone_char].y)
		loc_y = IS_SOUTH;

	if(zone_table[zone_obj].x == zone_table[zone_char].x &&
	   zone_table[zone_obj].y == zone_table[zone_char].y)
		return (IS_HERE);

	/* Round Two Begins! Now we find whether or not the zone is in more than one direction. */

	if(loc_y == IS_NORTH && loc_x == IS_WEST)
		loc = IS_NORTHWEST;

	else if(loc_y == IS_NORTH && loc_x == IS_EAST)
		loc = IS_NORTHEAST;

	else if(loc_y == IS_SOUTH && loc_x == IS_WEST)
		loc = IS_SOUTHWEST;

	else if(loc_y == IS_SOUTH && loc_x == IS_EAST)
		loc = IS_SOUTHEAST;

	else
	{
		if(loc_x)
			loc = loc_x;

		else if(loc_y)
			loc = loc_y;

		else
			loc = IS_HERE;  // better than nothing
	}

	return loc;
}

/*
 * Special spells appear below.
 */

ASPELL(spell_bond)
{
	if(!victim)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	if(IS_NPC(victim))
	{
		message(ch, "%s doesn't look like %s wants to be bonded.\r\n", GET_NAME(victim), HSSH(victim));
		return;
	}

	if(victim == ch)
	{
		message(ch, "You can't bond yourself!");
		return;
	}

	if(IS_NPC(ch))
	{
		message(ch, "What?! A MOB trying to bond?! That could be fatal!\r\n");
		return;
	}

	if(IS_BONDED(ch))
	{
		message(ch, "You are already bonded to %s.\r\n", GET_BOND(ch));
		return;
	}

	if(IS_BONDED(victim))
	{
		message(ch, "%s is already bonded.\r\n", GET_NAME(victim));
		return;
	}

	message(ch, "You place your hands in front of %s's head as you form a bond from %s to you.\r\n",
		GET_NAME(victim), HMHR(victim));
	message(victim, "%s places %s hands near your head, almost touching, as you feel a bond form.\r\n",
		GET_NAME(ch), HSHR(ch));

	act("$n places $s hands in front of $N's head for a long moment, obviousely channeling something complex.",
		FALSE, ch, 0, victim, TO_NOTVICT);

	// Bond em up!
	strcpy(GET_BOND(ch), GET_NAME(victim));
	strcpy(GET_BOND(victim), GET_NAME(ch));

	// Save
	ch->save();
	victim->save();

	// Side Affects
	GET_MOVE(victim) = 0;
	GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch) / 2, GET_MOVE(ch));
	GET_MANA(ch) = 5;
	GET_MAX_MANA(ch) += 45;
}

ASPELL(spell_gate)
{
	int room = 0, distance = 0, zone = 0;
	char code[MAX_INPUT_LENGTH];
	struct gate_data *gate;

	one_argument(argument, code);

	if ( (room = reverse_gate_code(code)) == NOWHERE)
		room = number(0, top_of_world);

	zone = world[room].zone;
	distance = find_distance(world[IN_ROOM(ch)].zone, zone);

	if(ROOM_FLAGGED(room, ROOM_PRIVATE) || ROOM_FLAGGED(room, ROOM_DEATH) ||
		ROOM_FLAGGED(room, ROOM_NOPORT) || ROOM_FLAGGED(room, ROOM_NOMAGIC) ||
		(zone_table[zone].x == 0 && zone_table[zone].y == 0))
	{
		send_to_char("Your Gate collapses, unable to find your destination.\r\n", ch);
		return;
	}

	CREATE(gate, struct gate_data, 1);

	gate->creator = ch;
	gate->in_room = IN_ROOM(ch);
	gate->to_room = room;
	gate->alive = true;
	gate->time_of_creation = time(0);

	ADD_FIRST_NODE(world[room].first_live_gate, world[room].last_live_gate, gate, prev, next);
	ADD_FIRST_NODE(first_gate, last_gate, gate, prev_in_world, next_in_world);

	sprintf(buf, "You weave flows of %s into a Gate.\r\n", GET_SEX(ch) == SEX_MALE ? "saidin" : "saidar");
	send_to_char(buf, ch);

	sprintf(buf, "A line of light appears, and rotates open into a Gate.\r\n");
	send_to_room(buf, IN_ROOM(ch));
	send_to_room(buf, room);
}

ASPELL(spell_restore)
{
	int i = 0;

	if(!victim)
	{
		send_to_char("They are not here.\r\n", ch);
		return;
	}

	act("$n puts $s hands on $N's forehead, removing all illnesses from $M.", TRUE, ch, nullptr, victim, TO_NOTVICT);
	act("$n places $s hands on your forehead...\r\nYou feel much better.", TRUE, ch, nullptr, victim, TO_VICT);

	for(i = 0;i < NUM_AFF_FLAGS;i++)
	{
		if(i == AFF_INVISIBLE || i == AFF_SANCTUARY || i == AFF_GROUP || i == AFF_INCOGNITO || i == AFF_SNEAK ||
		i == AFF_HIDE || i == AFF_NOTICE || i == AFF_NOQUIT || i == AFF_NIGHT_VISION || i == AFF_HASTE)
			continue;

		if(AFF_FLAGGED(victim, i))
			affect_from_char(victim, 0, i);
	}
}

ASPELL(spell_tornado)
{

	int zone = 0;;
	char_data *vict, *next;

	zone = world[ch->in_room].zone;

	act("A huge tornado forms from thin air, lifting objects up into the sky!", FALSE, ch, nullptr, nullptr, TO_ROOM);
	for(vict = world[IN_ROOM(ch)].people;vict;vict = next)
	{

		next = vict->next_in_room;

		if(vict == ch)
			continue;

		move_char_random(ch, zone_table[zone].bot, zone_table[zone].top, FALSE);
		look_at_room(vict, FALSE);

		act("$n is lifted and thrown into the air by the tornado!", FALSE, vict, nullptr, nullptr, TO_ROOM);
		act("You are lifted into the air and thrown into a random direction by the huge tornado!", FALSE, vict, nullptr, nullptr, TO_CHAR);
		act("$n is thrown threw the air and smashes into the ground!", FALSE, vict, nullptr, nullptr, TO_ROOM);
	}
}

ASPELL(spell_locate_life)
{

	char_data *vict;
	char arg[MAX_INPUT_LENGTH];
	int distance = 0, location = 0, zone = 0, found = 0;
	
	one_argument(argument, arg);

	if(!*arg)
	{
		send_to_char("Who do you want to locate?\r\n", ch);
		return;
	}

	for(vict = character_list; vict; vict = vict->next)
	{
		if(!is_name(arg, GET_NAME(vict)) && !race_alias(vict, arg))
			continue;

		if(GET_LEVEL(vict) >= LVL_IMMORT)
			continue;

		if(special_noloc(vict))
		{
			send_to_char(NOLOC_CHILL_MESSAGE, vict);
			continue;
		}

		zone = world[vict->in_room].zone;
		distance = find_distance(zone, world[ch->in_room].zone);
		location = find_zone_slope(zone, world[ch->in_room].zone);

		if(ROOM_FLAGGED(vict->in_room, ROOM_NOPORT) || ROOM_FLAGGED(vict->in_room, ROOM_NOMAGIC)
			|| (zone_table[zone].x == 0 && zone_table[zone].y == 0))
			continue;

		if(distance <= GET_SKILL(ch, SPELL_LOCATE_LIFE) / 20)
		{
			found = 1;
			message(ch, "You sense %s%s %s.  --  [%s%s%s]\r\n",
				GET_NAME(vict), dist[distance], loc[location], COLOR_GREEN(ch, CL_COMPLETE),
				port_code(IN_ROOM(vict)), COLOR_NORMAL(ch, CL_COMPLETE));
		}
	}

	if(!found)
	{
		send_to_char("You do not sense that anywhere nearby.\r\n", ch);
		return;
	}	
}

ASPELL(spell_regen)
{

	if(victim == ch && type != CAST_POTION)
	{
		send_to_char("Why use restore energy that is about to be used?\r\n", ch);
		return;
	}

	if(GET_MOVE(victim) < (GET_MAX_MOVE(ch) * .75))
		GET_MOVE(victim) = (int) ((double) GET_MAX_MOVE(ch) * .75);

	act("Your legs feel much lighter.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n appears to be less tired.", FALSE, victim, 0, 0, TO_NOTVICT);

}

ASPELL(spell_create_water)
{
	int water;

	if (ch == nullptr || obj == nullptr)
		return;
  
	level = MAX(MIN(level, LVL_IMPL), 1);

	if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON)
	{
		if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0))
		{
			name_from_drinkcon(obj);
			GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);
		} 
		
		else
		{
			water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      
			if (water > 0)
			{
				if (GET_OBJ_VAL(obj, 1) >= 0)
					name_from_drinkcon(obj);
					GET_OBJ_VAL(obj, 2) = LIQ_WATER;
					GET_OBJ_VAL(obj, 1) += water;
					name_to_drinkcon(obj, LIQ_WATER);
					act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
			}
		}
	}
}


ASPELL(spell_teleport)
{
	int i = 0, found = 0, distance = 0, zone = 0;
	char code[MAX_INPUT_LENGTH];

	one_argument(argument, code);

	send_to_char("You are surrounded by a beam of light and begin to Travel.\r\n", ch);

	for(i = 0;i < top_of_world;i++)
	{
		if(!str_cmp(code, port_code(i)))
		{
			found =  1;
			break;
		}
	}

	zone = world[i].zone;
	distance = find_distance(world[IN_ROOM(ch)].zone, zone);

	if(!found || (ROOM_FLAGGED(i, ROOM_PRIVATE) || ROOM_FLAGGED(i, ROOM_DEATH)) ||
		!&world[i] || (distance > 4) || (ROOM_FLAGGED(i, ROOM_NOPORT))
		|| (ROOM_FLAGGED(i, ROOM_NOMAGIC)) || (zone_table[zone].x == 0 && zone_table[zone].y == 0))
	{
		send_to_char("The Power is torn from your weave as you can't think up your destination clearly.\r\n", ch);
		return;
	}

	act("$n is surrounded by a beam of light and dissapears.", FALSE, ch, 0, 0, TO_ROOM);

	char_from_room(ch);
	char_to_room(ch, i);

	act("$n arrives, a beam of bright light surrounding $s body.", FALSE, ch, 0, 0, TO_ROOM);
	look_at_room(ch, 0);

	send_to_char("You arrive suddenly at your destination.\r\n", ch);
}


#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_locate_object)
{
	struct obj_data *i;
	char_data *bearer = 0;
	char name[MAX_INPUT_LENGTH];
	int zone = 0, location = 0, distance = 0, room = 0, any = 0;

	one_argument(argument, name);

	for (i = object_list;i; i = i->next)
	{
		if (!isname(name, i->name))
			continue;
		
		else
		{
			if(i->carried_by && GET_LEVEL(i->carried_by) < LVL_IMMORT)
			{
				zone = world[i->carried_by->in_room].zone;
				room = i->carried_by->in_room;
				bearer = i->carried_by;
			}

			else if (i->worn_by && GET_LEVEL(i->worn_by) < LVL_IMMORT)
			{
				zone = world[i->worn_by->in_room].zone;
				room = i->worn_by->in_room;
				bearer = i->worn_by;
			}

			else
			{
				if(i->in_room > -1)
				{
					zone = world[i->in_room].zone;
					room = i->in_room;
				}

				else
					continue;
			}

			if(bearer && special_noloc(bearer))
			{
				send_to_char(NOLOC_CHILL_MESSAGE, bearer);
				continue;
			}

			location = find_zone_slope(zone, world[ch->in_room].zone);
			distance = find_distance(zone, world[ch->in_room].zone);

			if(ROOM_FLAGGED(room, ROOM_NOPORT) || ROOM_FLAGGED(room, ROOM_NOMAGIC) ||
				(zone_table[zone].x == 0 && zone_table[zone].y == 0))
				continue;

			if(distance <= 4)
			{
				sprintf(buf, "You sense %s%s %s", i->short_description, dist[distance], loc[location]);

				if(bearer)
					sprintf(buf + strlen(buf), " carried by %s.", GET_NAME(bearer));

				sprintf(buf + strlen(buf), "  --  [%s%s%s]\r\n", COLOR_GREEN(ch, CL_COMPLETE),
				port_code(room), COLOR_NORMAL(ch, CL_COMPLETE));

				send_to_char(buf, ch);
				bearer = 0;
				any = 1;
			}
		}
	}

	if(!any)
		message(ch, "You don't feel any objects of that type nearby.\r\n");
}

ASPELL(spell_enchant_weapon)
{
	int i;

	if (ch == nullptr || obj == nullptr)
		return;

	if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
		!OBJ_FLAGGED(obj, ITEM_MAGIC))
	{

		for (i = 0; i < MAX_OBJ_AFFECT; i++)
			if (obj->affected[i].location != APPLY_NONE)
				return;

		SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

		obj->affected[0].location = APPLY_HITROLL;
		obj->affected[0].modifier = 1 + (level >= 18);

		obj->affected[1].location = APPLY_DAMROLL;
		obj->affected[1].modifier = 1 + (level >= 20);

		act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
	}
}
