/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include <stdio.h>
#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"

extern struct room_data *world;
extern char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct memory_data *mlist;
extern int top_of_world;
extern int seconds;
extern int global_minutes;
extern sh_int human_start_room;
extern sh_int trolloc_start_room;

void add_follower(char_data *ch, char_data *leader);
void death_cry(char_data *ch);
void raw_kill(char_data *ch, char_data *killer);
void do_doorcmd(char_data *ch, struct obj_data *obj, int door, int scmd);
int find_eq_pos(char_data * ch, struct obj_data * obj, char *arg);
int has_boat(char_data *ch);
int ok_pick(char_data *ch, int keynum, int pickproof, int scmd);
int find_door(char_data *ch, const char *type, char *dir, const char *cmdname);
int has_key(char_data *ch, int key);
int special(char_data *ch, int cmd, char *arg);
int can_move(char_data *ch, int dir, int need_specials_check);
int level_timer(char_data *ch, int level);
int can_edit_zone(char_data *ch, int number);
int buildwalk(char_data *ch, int dir);
void perform_group_noise(char_data *ch, int room_number, int coming_from, int size, char *message, int type);
void noise_addup(int room_number, int size, int type);
struct track_data *track_list = nullptr;
char *gate_code(int room);
char *door_state(int state);

ACMD(do_enter);
ACMD(do_fade);
ACMD(do_follow);
ACMD(do_gen_door);
ACMD(do_lead);
ACMD(do_leave);
ACMD(do_rest);
ACMD(do_sense);
ACMD(do_sit);
ACMD(do_sleep);
ACMD(do_stand);
ACMD(do_wake);

#define DOOR_FOUND(ch, door) ((!EXIT(ch, door)->hidden) || (EXIT(ch, door)->hidden && \
GET_SKILL(ch, SKILL_SEARCH) > EXIT(ch, door)->hidden))

// Serai - 06/15 - Noooo! "char code[300]; (do stuff); return code;"
// Returning references to local variables is Bad...
char *fade_code(int room)
{
	static char code[256];
	sprintf(code, "a%dFG%de%d", world[room].zone, GET_ROOM_VNUM(room) * 3 + 456, world[room].sector_type);

	return (code);
}

char *port_code(int room)
{
	static char code[256];
	sprintf(code, "r%dT%dPe%dO", world[room].sector_type, GET_ROOM_VNUM(room) * 2 + 123, world[room].zone);

	return (code);
}

char *door_state(int state)
{

	if(state == 0)
		return "open";
	
	else if(state == 1)
		return "closed";

	else if(state == 2)
		return "locked";

	else
		return "Unknown state";
}

/* simple function to determine if char can walk on water */
int has_boat(char_data *ch)
{
	struct obj_data *obj;
	int i;
	

	if (AFF_FLAGGED(ch, AFF_WATERWALK))
		return 1;

	/* non-wearable boats in inventory will do it */
	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, nullptr) < 0))
			return 1;

	/* and any boat you're wearing will do it too */
	for (i = 0; i < NUM_WEARS; i++)
		if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
			return 1;

	return 0;
}

void lay_tracks(char_data *ch, int direction)
{
	struct track_data *trax;

	if(ROOM_FLAGGED(ch->in_room, ROOM_INDOORS) || GET_LEVEL(ch) >= LVL_IMMORT || IS_NPC(ch))
		return;	

	trax = new track_data;
	
	strcpy(trax->name, GET_NAME(ch));
	trax->laytime = global_minutes;
	trax->age = 0;
	trax->direction = direction;
	trax->race = GET_RACE(ch);

	if(IS_DREADLORD(ch) || IS_FADE(ch))
		trax->race = GET_RACE(ch);
	
	trax->next = track_list;
	track_list = trax;

	trax->next_in_room = world[IN_ROOM(ch)].tracks;
	world[IN_ROOM(ch)].tracks = trax;

}

int find_room(char *name)
{

	int i = 0, found = 0;

	if(!name)
		return 0;
	
	for(i = 0;i < top_of_world;i++)
	{

		if(!str_cmp(name, fade_code(i)))
		{
			found =  1;
			break;
		}
	}

	if(!found || (ROOM_FLAGGED(i, ROOM_PRIVATE) || ROOM_FLAGGED(i, ROOM_DEATH)) || !&world[i])
		return -1;
	
	return i;
}

int fade_distance(char_data *ch)
{
	int distance = 0, skill = 0;

	skill = GET_SKILL(ch, SKILL_FADE);

	if(GET_RACE(ch) >= 6)
		distance = 1;

	if(skill > 75)
		distance += 4;
	
	if(skill > 50)
		distance += 3;

	if(skill > 25)
		distance += 2;

	distance += 1;
	return distance;
}

void perform_actual_fade(char_data *ch, sh_int room)
{

	
	send_to_char("You reach out for the shadows as they surround you completely.\r\n", ch);
	act("$n reaches is surrounded by shadows and vanishes.", TRUE, ch, 0, 0, TO_ROOM);
	
	char_from_room(ch);
	char_to_room(ch, room);
	
	act("$n steps out from the dark shadows nearby.", TRUE, ch, 0, 0, TO_ROOM);
	
	look_at_room(ch, 0);
}


ACMD(do_sense)
{

	if(!IS_FADE(ch))
	{
		send_to_char("Sense?! What?! You want to sense something?\r\n", ch);
		return;
	}

	if(GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) <= LVL_GRGOD)
	{
		send_to_char("Should you be trying this?\r\n", ch);
		return;
	}

	if(!IS_NPC(ch) && ch->desc)
		if(!ch->desc->command_ready)
		{
			ch->desc->command_ready = 1;
			strcpy(LAST_COMMAND(ch), "sense");
			sprintf(LAST_COMMAND(ch) + strlen(LAST_COMMAND(ch)), " %s", argument); 
			return;
		}

	if(ch->desc)
		if(ch->desc->timer <= 0)
		{
		
			if(!IS_DARK(ch->in_room))
			{
				send_to_char("There is no darkness to sense here.\r\n", ch);
				return;
			}

			message(ch, "This room: [%s%s%s]\r\n", COLOR_GREEN(ch, CL_COMPLETE),
				fade_code(ch->in_room), COLOR_NORMAL(ch, CL_COMPLETE));
		}
}



ACMD(do_fade)
{

	int distance = 0, cost = 0, room = 0, oldroom = 0, old_shadow = 0;
	char arg[MAX_STRING_LENGTH];
	struct follow_type *f;
	char_data *trolloc;

	one_argument(argument, arg);

	if(!IS_FADE(ch))
	{
		send_to_char("Do you really think you can reach for the shadows?\r\n", ch);
		return;
	}

	if(!*arg)
	{
		send_to_char("Where is it you want the shadows to take you?\r\n", ch);
		return;
	}

	if(!IS_NPC(ch))
		if(!ch->desc->command_ready)
		{
			ch->desc->timer = level_timer(ch, GET_SKILL(ch, SKILL_FADE));
			ch->desc->timer *= 2.5;
			ch->desc->command_ready = 1;
			strcpy(LAST_COMMAND(ch), "fade");
			sprintf(LAST_COMMAND(ch) + strlen(LAST_COMMAND(ch)), " %s", argument); 
			return;
		}

	if(ch->desc)
		if(ch->desc->timer <= 0)
		{

			// This means the room was either not found, or found and unreachable //
			if((room = find_room(arg)) == -1)
			{
				send_to_char("You begin to reach for the shadows, but realize the other end cannot be reached!\r\n", ch);
				return;
			}

			distance = find_distance(world[room].zone, world[ch->in_room].zone);

			cost = 8;
			cost *= distance;

			old_shadow = GET_SHADOW(ch);
			GET_SHADOW(ch) = MAX(0, GET_SHADOW(ch) - cost);

			if(old_shadow - cost < 0)
			{
				send_to_char("You do not have enough strength to reach out for the shadows.\r\n", ch);
				return;
			}

			if(number(1, 120) > GET_SKILL(ch, SKILL_FADE))
			{
				send_to_char("You pull the shadows towards you and lose control!\r\n", ch);
				return;
			}

			if(!IS_DARK(ch->in_room))
			{
				send_to_char("There are no shadows to reach out to.\r\n", ch);
				return;
			}

			if(!room)
			{
				send_to_char("The shadows cannot take you there.\r\n", ch);
				return;
			}

			if(distance > fade_distance(ch))
			{
				send_to_char("You cannot control the shadows to pull you so far.\r\n", ch);
				return;
			}

			oldroom = IN_ROOM(ch);
			perform_actual_fade(ch, room);

			for (f = ch->followers; f; f = f->next)
			{

				trolloc = f->follower;
				
				if(IN_ROOM(trolloc) == oldroom && ((GET_SHADOW(ch) - 15) >= 0))
				{

					GET_SHADOW(ch) -= 15;
					perform_actual_fade(trolloc, room);
				}
			}
		}
}

int start_room(char_data *ch)
{

	if(IS_HUMAN(ch))
		return human_start_room;

	else if(IS_TROLLOC(ch))
		return trolloc_start_room;

	return (NOWHERE);
}

int can_move(char_data *ch, int dir, int need_specials_check)
{

	int need_movement;

	/*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */

	
	if (need_specials_check && special(ch, dir + 1, "")) /* XXX: Evaluate nullptr */
	{
		return 0;
	}

	if(GET_DEATH_WAIT(ch) > 0 && GET_ROOM_VNUM(IN_ROOM(ch)) == start_room(ch))
	{
		message(ch, "You must wait %d more minutes to regain strength.\r\n", GET_DEATH_WAIT(ch));
		return 0;
	}



  /* if this room or the one we're going to needs a boat, check for one */
	if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
	(SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM))
	{
		if (!has_boat(ch))
		{
			send_to_char("You need a boat to go there.\r\n", ch);
			return 0;
		}
	}

	if(MOUNT(ch) && SECT(world[ch->in_room].dir_option[dir]->to_room) == SECT_INSIDE)
	{
		send_to_char("You cannot ride in there!\r\n", ch);
		return 0;
	}

	if(GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) <= LVL_BLDER || PLR_FLAGGED(ch, PLR_ZONE_BAN))
		if(!can_edit_zone(ch, world[world[ch->in_room].dir_option[dir]->to_room].zone)
			&& world[world[ch->in_room].dir_option[dir]->to_room].zone != 0)
		{
			send_to_char("You must be higher level to leave your zone!\r\n", ch);
			return 0;
		}

  /* move points needed is avg. move loss for src and destination sect type */
	need_movement = (movement_loss[SECT(ch->in_room)] +
	movement_loss[SECT(EXIT(ch, dir)->to_room)]) / 2;

	if(IS_TROLLOC(ch))
	{
		need_movement = 2;
	}

	else if(ch->getClan(CLAN_WOLFBROTHER))
	{
		need_movement = 1;
	}

	else
	{
		if(need_movement > 5)
		{
			need_movement = 5;
		}
	}
		
	if(AFF_FLAGGED(ch, AFF_NOTICE) && !IS_TROLLOC(ch))
		need_movement += 1;


	if(MOUNT(ch) && GET_MOVE(MOUNT(ch)) < need_movement)
	{
		send_to_char("Your mount is too exhausted.\r\n", ch);
		act("$N is too exhausted to move $n any further.", FALSE, ch, 0, MOUNT(ch), TO_NOTVICT);
		return 0;
	}

	if (GET_MOVE(ch) < need_movement && (IS_HORSE(ch) || !IS_NPC(ch)))
	{
		if (need_specials_check && ch->master)
			send_to_char("You are too exhausted to follow.\r\n", ch);
		else
			send_to_char("You are too exhausted.\r\n", ch);


		return 0;
	}
  
	if (ROOM_FLAGGED(ch->in_room, ROOM_ATRIUM))
	{
		if (!House_can_enter(ch, GET_ROOM_VNUM(EXIT(ch, dir)->to_room)))
		{
			send_to_char("That's private property -- no trespassing!\r\n", ch);
			return 0;
		}
	}

	/* Mortals and low level gods cannot enter greater god rooms. */
	if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM) &&
		GET_LEVEL(ch) < LVL_IMMORT) {
		send_to_char("You aren't godly enough to use that room!\r\n", ch);
		return 0;
	}

	return 1;

  /* Now we know we're allow to go into the room. */
}


/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
*/

//These are different types of group noise messages. //
char *group_noise[MAX_TYPE] = {
	
	"You hear the $L sound of footsteps from $D.",
	"You can hear the $L sound of horse feet hitting the ground from $D.",
	"You can hear the $L sound of voices from $D.",
	"You can hear the $L sound of fighting going on from $D."
};


/*	Zeryn - December 27th, 2003. This is used to give off group noises to nearby rooms 
	coming_from is used for TYPE_MOVEMENT to make sure that the message gets passed from both rooms 
	when a group moves from a room. It also is there to make sure that that room does not receive the 
	movement message.

*/

// Struct for the lists of rooms already used 
struct Used_Room 
{ 
	struct Used_Room *next; 
	int room; 
};

// Add a room to a list, will create a list if the list is currently empty 
void add_to_sent(struct Used_Room *&base, int room) 
{ 
	struct Used_Room *newroom; 
   
	newroom = new struct Used_Room; 
	newroom->next = 0; 
	newroom->room = room; 
   
	if(base) 
	{ 
		struct Used_Room *temp; 
		
		for(temp = base; temp->next; temp = temp->next);

		temp->next = newroom;
	} 
	
	else
		base = newroom;
} 

// Free all memory associated with a list and set the base to 0. 
void kill_list(struct Used_Room* &base) 
{ 
	struct Used_Room *temp, *temp2; 

	temp = base; 
	while(temp) 
	{ 
		temp2 = temp->next; 
		delete[] (temp); 
		temp = temp2;
	} 
   
	base = 0; 
} 

// Test to see if a room number already exists in a list. 
bool room_used(struct Used_Room *base, int room) 
{ 
	for(struct Used_Room *temp = base; temp; temp = temp->next) 
	{
		if(temp->room == room) 
			return true; 
	} 
   
	return false; 
}

char *loudness(int distance)
{

	if(distance <= 0)
		return "strangely near";
	
	else if(distance == 1)
		return "very loud";

	else if(distance == 2)
		return "fairly loud";
	
	else if(distance == 3)
		return "soft";
	
	else if(distance == 4)
		return "faint";
	
	else if(distance == 5)
		return "very faint";

	else
		return "whispering";
}

int noise_travel[]	=
{
	5,	//Inside
	8,	//City
	7,	//Field
	6,	//Forest
	7,	//Hills
	4,	//Mountains
	6,	//Water Swimming
	6,	//Water No Swimming
	6,	//Underwater
	5	//Air
};

#define NOISE_DOOR_COST		5


void noise_send(const char *msg, int room, int distance, int direction)
{
	char real[MAX_STRING_LENGTH];
	char *temp = new char[MAX_STRING_LENGTH];
	char temp2[MAX_STRING_LENGTH];
	int i = 0;

	for(;;)
	{
		if(!*msg)
			break;

		if(*msg == '$')
		{


			msg++;

			switch(*msg)
			{

			case 'L':
				if(loudness(distance))
					temp = loudness(distance);
				
				else
					temp = "unknown loudness";

				break;

			case 'D':
				sprintf(temp2, "%s%s", ((direction == UP || direction == DOWN) ? "" : "the "),
				(direction == UP ? "below": direction == DOWN ? "above" : dirs[rev_dir[direction]]));
				temp = temp2;
				break;

			case '$':
				temp = "$";
				break;

			default:
				log("Illegal '$' argument inside of noise_send(). Argument = %c", *msg);
				break;
				
			}

			while ((real[i] = *(temp++)))
				i++;
	
			msg++;
		}
		
		else if (!(real[i++] = *(msg++)))
			break;

	}

	real[i] =		'\r';
	real[i + 1] =	'\n';
	real[i + 2] =	'\0';

	if(temp)
		delete[] (temp);
	
	send_to_room(real, room);

}

// This function sends message to every room within radius rooms of the starting room. 
//   It will not go through closed doors. 
void area_send(int room_num, int radius, int distance, int direction_from, struct Used_Room *sent, struct Used_Room *base, char *message) 
{

	int p = 0;

	if(!room_used(sent, room_num))
	{
		noise_send(message, room_num, distance, direction_from);
		add_to_sent(sent, room_num);
	}
   
	//Prevent room from being called later on.
	add_to_sent(base, room_num);
	
	/* Go to each room exit */
	for(int i = 0;i < 6;i++)
	{

		p = radius;

		/* And make sure there IS an exit */
		if(world[room_num].dir_option[i] && world[room_num].dir_option[i]->to_room > -1)
		{
			/* If there is a door here, we knock off some of the potential distance of the sound */
			if(IS_SET(world[room_num].dir_option[i]->exit_info, EX_CLOSED))
			{
				p -= NOISE_DOOR_COST;
			}

			p -= noise_travel[SECT(world[room_num].dir_option[i]->to_room)];

			/* And lastly make sure we have not yet been there */
			if(!room_used(base, world[room_num].dir_option[i]->to_room) && radius > 0) 
			{
				area_send(world[room_num].dir_option[i]->to_room,
				p, distance + 1, i, sent, base, message); 
			}
		}
	}
}

void perform_group_noise(int room_number, int size, char *message, int type)
{ 

	return;

	room_data *room = &world[room_number]; 
	int depth = 0;

	if(!room) 
		return;

	if(size > 0)
		depth = size * 10;


	/*	Here we multiply the depth by three to give us a bigger ammount so we can change
		the ammount of distance that is subtracted later on.

	*/

	//Now give off the message for movement // 
	struct Used_Room *sent = 0, *temp = 0;

	if(type == -1)
	{
		add_to_sent(sent, room_number);
		area_send(room_number, size * 10, 0, 0, sent, temp, message);
	}

	//Send message outward from normal room but not to the room itself 
	if(type > 0)
	{
		add_to_sent(sent, room_number);
		area_send(room_number, depth, 0, 0, sent, temp, group_noise[type]);
		room->noise_level[type] = 0;
	}
   
	//Cleanup 
	kill_list(temp);
	kill_list(sent);
	
}

void noise_addup(int room_number, int size, int type)
{

	return;

	room_data *room = &world[room_number]; 

	if(!room) 
		return;

	room->noise_level[type] += size;
}

// Tulon 2-4-2004 Function takes characters from their current room and moves them into random rooms throughout the zone //
void perform_explosion(int room)
{

	struct obj_data *obj, *next_obj;
	char_data *ch, *next_char;
	int	zone = world[room].zone, boom = 0;

	for(obj = world[room].contents;obj;obj = next_obj)
	{
		next_obj = obj->next_content;

		if(GET_OBJ_VNUM(obj) == 209)
		{
			boom = 1;

			for(ch = character_list;ch;ch = ch->next)
			{
				if(ch->in_room != room && GET_ROOM_VNUM(ch->in_room) >= zone_table[zone].bot && GET_ROOM_VNUM(ch->in_room) <= zone_table[zone].top)
					send_to_char("The sky lights up with fire as you hear a huge explosion move through the air!\r\n", ch);
			}

			sprintf(buf, "%s lights up as if triggered to do so and explodes! BOOOM!!\r\n", obj->short_description);
			send_to_room(buf, room);
			obj_from_room(obj);

			if(SECT(room) == SECT_INSIDE)
				return;

			for(ch = world[room].people;ch;ch = next_char)
			{

				next_char = ch->next_in_room;

				act("$n is thrown through the air from the explosion!", FALSE, ch, nullptr, nullptr, TO_NOTVICT);
				act("You are thrown through the air from the explosion!\r\n\n", FALSE, ch, nullptr, nullptr, TO_CHAR);
				move_char_random(ch, zone_table[zone].bot, zone_table[zone].top, FALSE);
				act("$n falls from the air at a huge speed!", FALSE, ch, nullptr, nullptr, TO_NOTVICT);
				look_at_room(ch, FALSE);
			}
		}
	}

		/*	Since we don't want to have to deal with messed up objects while the loop above is searching for all of the explosion objects in
			the room, we do it after and do it in one swift blow. - Tulon 2-4-2004
			*/

	if(boom)
	{
		for(obj = world[room].contents;obj;obj = next_obj)
		{
			next_obj = obj->next_content;
			act("$p is thrown through the air from the explosion!", FALSE, nullptr, obj, nullptr, TO_ROOM);
			move_obj_random(obj, zone_table[zone].bot, zone_table[zone].top, FALSE);
			act("$p falls from the air at a huge speed!", FALSE, nullptr, obj, nullptr, TO_NOTVICT);
		}
	}
}

int do_simple_move(char_data *ch, int dir, int need_specials_check, int air_ok)
{
	int was_in, need_movement;
	char *mount_message;

	if(!can_move(ch, dir, need_specials_check))
		return 0;

	need_movement = (movement_loss[SECT(ch->in_room)] +
	movement_loss[SECT(EXIT(ch, dir)->to_room)]) / 2;

	/* Only give perks to players. */
	if(!IS_NPC(ch))
	{
		if(IS_TROLLOC(ch) || IS_WARDER(ch))
			need_movement = 2;

		else if(ch->getClan(CLAN_WOLFBROTHER))
			need_movement = 1;

		else
		{
			if(need_movement > 5)
			{
				need_movement = 5;
			}
			
			if(AFF_FLAGGED(ch, AFF_NOTICE) && !IS_TROLLOC(ch))
			{
				need_movement += 1;
			}
		}
	}

	/**** End of Clan/Race Perks ****/

  	if(MOUNT(ch) != nullptr)
	{
		sprintf(buf, ", riding %s.", GET_NAME(MOUNT(ch)));
	}
	else
	{
		*buf = '\0';
	}
	
	mount_message = buf;


	if(!AFF_FLAGGED(ch, AFF_EFFUSION) || GET_SKILL(ch, SKILL_EFFUSION) < number(1, 125) || MOUNT(ch))
	{
		sprintf(buf2, "$n leaves %s%s.", dirs[dir], mount_message);
		act(buf2, TRUE, ch, 0, 0, TO_ROOM);
	}

  /* see if an entry trigger disallows the move */
	if (!entry_mtrigger(ch))
		return 0;
	
	if (!enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
		return 0;

	if(!ROOM_FLAGGED(ch->in_room, ROOM_INDOORS) && !ROOM_FLAGGED(ch->in_room, ROOM_NOTRACK))
		lay_tracks(ch, dir);
	
	was_in = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, world[was_in].dir_option[dir]->to_room);
	noise_addup(ch->in_room, 1, TYPE_MOVEMENT);

	if(MOUNT(ch))
	{
		char_from_room(MOUNT(ch));
		char_to_room(MOUNT(ch), ch->in_room);
	}

	/* If they are riding, take the moves from the horse. */
	if(MOUNT(ch))
	{
		GET_MOVE(MOUNT(ch)) -= need_movement;
	}

	/* Otherwise, take their own moves. */
	else if (GET_LEVEL(ch) < LVL_IMMORT && (IS_HORSE(ch) || !IS_NPC(ch)))
	{
		GET_MOVE(ch) -= need_movement;
	}

	if ((!AFF_FLAGGED(ch, AFF_SNEAK) || number(1, 125) > GET_SKILL(ch, SKILL_SNEAK))
		&& !IS_GREYMAN(ch) && !ch->getClan(CLAN_WOLFBROTHER) || (MOUNT(ch))) 
	{

		if(MOUNT(ch))
			 sprintf(buf, ", riding %s.", GET_NAME(MOUNT(ch)));
		else
			sprintf(buf, ".");
	  
		mount_message = buf;

		sprintf(buf2, "$n has arrived from %s%s%s",
		((dir == UP || dir == DOWN) ? "" : "the "),
		(dir == UP ? "below": dir == DOWN ? "above" : dirs[rev_dir[dir]]), mount_message);

		act(buf2, TRUE, ch, 0, 0, TO_ROOM);
	}

	if (ch->desc != nullptr)
		look_at_room(ch, 0);

	if(IS_FADE(ch) && IS_DARK(ch->in_room))
		message(ch, "%s\r\nYou feel the presence of shadows all around you.%s\r\n", COLOR_RED(ch, CL_COMPLETE),
		COLOR_NORMAL(ch, CL_COMPLETE));

	if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT)
	{
		log_death_trap(ch);
		raw_kill(ch, 0);		

		if(IS_NPC(ch))
			extract_char(ch);
		else
			move_char_circle(ch);

		return 0;
	}

	entry_memory_mtrigger(ch);
	
	if (!greet_mtrigger(ch, dir))
	{
		char_from_room(ch);
		char_to_room(ch, was_in);
		look_at_room(ch, 0);
	}
	
	else greet_memory_mtrigger(ch);

	if(air_ok && !IS_NPC(ch))
		perform_explosion(ch->in_room);

	return 1;
}

int perform_move(char_data *ch, int dir, int need_specials_check)
{
	int was_in, ret_val = 0;
	struct follow_type *k, *next;

	if (ch == nullptr || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
		return 0;
  
	else if ((!EXIT(ch, dir) && !buildwalk(ch, dir)) || EXIT(ch, dir)->to_room == NOWHERE)
		send_to_char("Alas, you cannot go that way...\r\n", ch);

	else if(EXIT(ch, dir)->hidden > 0 && EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED))
		send_to_char("Alas, you cannot go that way...\r\n", ch);
  
	else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED))
	{
		if (EXIT(ch, dir)->keyword)
		{
			message(ch, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));

		} 
		
		else
			send_to_char("It seems to be closed.\r\n", ch);
	} 
	
	else
	{
		if (!ch->followers)
		{
			ret_val = do_simple_move(ch, dir, need_specials_check, FALSE);
			if(!IS_NPC(ch))
				perform_explosion(ch->in_room);
			
			return ret_val;
		}

		was_in = ch->in_room;
    
		if (!do_simple_move(ch, dir, need_specials_check, FALSE))
			return 0;

		for (k = ch->followers; k; k = next)
		{
			next = k->next;
		
			if ((k->follower->in_room == was_in) &&
			(GET_POS(k->follower) >= POS_STANDING))
			{
				act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
				perform_move(k->follower, dir, 1);
			}
		}

		if(!IS_NPC(ch))
			perform_explosion(ch->in_room);
		
		
		return 1;
	}
	
	return 0;
}	

ACMD(do_lead)
{
	int percent;
	char arg[MAX_INPUT_LENGTH];
	char_data *target;

	one_argument(argument, arg);

	if(!(target = get_char_room_vis(ch, arg)))
	{
		send_to_char("You don't see them here.\r\n", ch);
		return;
	}

	if(RIDDEN_BY(target))
	{
		sprintf(buf, "%s is already riding $M.", GET_NAME(RIDDEN_BY(target)));
		act(buf, TRUE, ch, nullptr, target, TO_CHAR);
		return;
	}

	percent = number(0, 50);

	if((!MOB_FLAGGED(target, MOB_MOUNT)) && (!MOB_FLAGGED(target, MOB_SHADOW_MOUNT)) ||
	(MOB_FLAGGED(target, MOB_MOUNT) && IS_TROLLOC(ch)) || (MOB_FLAGGED(target, MOB_SHADOW_MOUNT) && !IS_TROLLOC(ch)))
	{
		send_to_char("Why would you think they would follow you?\r\n", ch);
		return;
	}

	if(percent < GET_SKILL(ch, SKILL_RIDE))
		do_follow(target, GET_NAME(ch), 0, 0);
	
	else
	{
		act("$N refuses to follow you!", FALSE, ch, 0, target, TO_CHAR);
		act("You refuse $n's attemt to lead you.", FALSE, ch, 0, target, TO_VICT); 
		act("$N refuses $n's attempt to lead $M.", FALSE, ch, 0, target, TO_ROOM);
		return;
	}
}





ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, subcmd - 1, 0);
}

int find_door(char_data *ch, const char *type, char *dir, const char *cmdname)
{
	int door;

	if (*dir)
	{			/* a direction was specified */
		if ((door = search_block(dir, (const char **) dirs, FALSE)) == -1)
		{	/* Partial Match */
			send_to_char("That's not a direction.\r\n", ch);
			return -1;
		}
    
		if (EXIT(ch, door))
		{	/* Braces added according to indent. -gg */
			if (EXIT(ch, door)->keyword)
			{
				if (isname(type, EXIT(ch, door)->keyword) && DOOR_FOUND(ch, door))
					return door;
	
				else
				{
					sprintf(buf2, "I see no %s there.\r\n", type);
					send_to_char(buf2, ch);
					return -1;
				}
			} 
			
			else
				if(DOOR_FOUND(ch, door))
					return door;
		} 
		
		else
		{
			sprintf(buf2, "I really don't see how you can %s anything there.\r\n", cmdname);
			send_to_char(buf2, ch);
			return -1;
    
		}
	} 
	
	else
	{			/* try to locate the keyword */
		
		if (!*type)
		{
			sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
			send_to_char(buf2, ch);
			return -1;
		}
    
		for (door = 0; door < NUM_OF_DIRS; door++)
			if (EXIT(ch, door))
				if (EXIT(ch, door)->keyword)
					if (isname(type, EXIT(ch, door)->keyword) && DOOR_FOUND(ch, door))
						return door;

    
		sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
		send_to_char(buf2, ch);
		return -1;
	}

	return -1;
}


int has_key(char_data *ch, int key)
{
	struct obj_data *o;

	for (o = ch->carrying; o; o = o->next_content)
		if (GET_OBJ_VNUM(o) == key)
			return 1;

	if (GET_EQ(ch, WEAR_HOLD))
		if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
			return 1;

	return 0;
}



#define NEED_OPEN	1
#define NEED_CLOSED	2
#define NEED_UNLOCKED	4
#define NEED_LOCKED	8

const char *cmd_door[] =
{
	"open",
	"close",
	"unlock",
	"lock",
	"pick"
};

const int flags_door[] =
{
	NEED_CLOSED | NEED_UNLOCKED,
	NEED_OPEN,
	NEED_CLOSED | NEED_LOCKED,
	NEED_CLOSED | NEED_UNLOCKED,
	NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

int change_door_state(int room, int dir, int state)
{

	int other_room = 0;

	if(!world[room].dir_option[dir])
		return 0;

	other_room = world[room].dir_option[dir]->to_room;

	if(!&world[other_room] || !&world[other_room].dir_option[rev_dir[dir]] || world[other_room].dir_option[rev_dir[dir]]->to_room != room)
		other_room = -1;

	if(other_room > -1 && world[other_room].dir_option[rev_dir[dir]]->exit_info != state)
		change_door_state(other_room, rev_dir[dir], state);

	switch(state)
	{
	
	case SCMD_OPEN:
		EXITN(room, dir)->exit_info = EX_CLOSED;
		break;

	case SCMD_CLOSE:
		world[room].dir_option[dir]->exit_info = EX_CLOSED;
		break;

	case SCMD_UNLOCK:
		world[room].dir_option[dir]->exit_info = EX_LOCKED;
		break;

	case SCMD_LOCK:
		world[room].dir_option[dir]->exit_info = EX_LOCKED;
		break;
	}

	return (1);
}

void do_doorcmd(char_data *ch, struct obj_data *obj, int door, int scmd)
{
	int other_room = 0;
	struct room_direction_data *back = 0;

	sprintf(buf, "$n %ss ", cmd_door[scmd]);
	
	if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
		if ((back = world[other_room].dir_option[rev_dir[door]]))
			if (back->to_room != ch->in_room)
				back = 0;

	switch (scmd)
	{
	
	case SCMD_OPEN:
	case SCMD_CLOSE:
		OPEN_DOOR(ch->in_room, obj, door);
    
		if (back && (EXITN(ch->in_room, door)->exit_info != EXITN(other_room, rev_dir[door])->exit_info))
			OPEN_DOOR(other_room, obj, rev_dir[door]);
    
		send_to_char(OK, ch);
		break;
  
	case SCMD_UNLOCK:
	case SCMD_LOCK:
		LOCK_DOOR(ch->in_room, obj, door);
    
		if (back)
			LOCK_DOOR(other_room, obj, rev_dir[door]);
    
		send_to_char("*Click*\r\n", ch);
		break;
  
	case SCMD_PICK:
		LOCK_DOOR(ch->in_room, obj, door);
    
		if (back)
			LOCK_DOOR(other_room, obj, rev_dir[door]);
    
		send_to_char("The lock quickly yields to your skills.\r\n", ch);
		strcpy(buf, "$n skillfully picks the lock on ");
		break;
	}

	/* Notify the room */
	sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	(EXIT(ch, door)->keyword ? "$F" : "door"));
  
	if (!(obj) || (obj->in_room != NOWHERE))
		act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

	/* Notify the other room */
	if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back)
	{
		sprintf(buf, "The %s is %s%s from the other side.\r\n",
		(back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
		(scmd == SCMD_CLOSE) ? "d" : "ed");
    
		if (world[EXIT(ch, door)->to_room].people)
		{
			act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
			act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
		}
	}
}


int ok_pick(char_data *ch, int keynum, int pickproof, int scmd)
{
	int percent;

	percent = number(1, 101);

	if (scmd == SCMD_PICK)
	{
    
		if (keynum < 0)
			send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    
		else if (pickproof)
			send_to_char("It resists your attempts to pick it.\r\n", ch);
    
		else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
			send_to_char("You failed to pick the lock.\r\n", ch);
    
		else
			return (1);
    
		return (0);
	}
	return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			OBJVAL_FLAGGED(obj, CONT_CLOSEABLE)) :\
			(EXIT_FLAGGED(EXIT(ch, door), EX_ISDOOR)))

#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!OBJVAL_FLAGGED(obj, CONT_CLOSED)) :\
			(!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)))

#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!OBJVAL_FLAGGED(obj, CONT_LOCKED)) :\
			(!EXIT_FLAGGED(EXIT(ch, door), EX_LOCKED)))

#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(OBJVAL_FLAGGED(obj, CONT_PICKPROOF)) : \
			(EXIT_FLAGGED(EXIT(ch, door), EX_PICKPROOF)))


#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
	int door = -1, keynum;
	char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
	struct obj_data *obj = nullptr;
	char_data *victim = nullptr;

	skip_spaces(&argument);
  
	if (!*argument)
	{
		sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
		send_to_char(CAP(buf), ch);
		return;
	}
  
	two_arguments(argument, type, dir);
	
	if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
		door = find_door(ch, type, dir, cmd_door[subcmd]);
 
	if ((obj) || (door >= 0))
	{
		keynum = DOOR_KEY(ch, obj, door);
    
		if (!(DOOR_IS_OPENABLE(ch, obj, door)))
			act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    
		else if (!DOOR_IS_OPEN(ch, obj, door) &&
			IS_SET(flags_door[subcmd], NEED_OPEN))
			send_to_char("But it's already closed!\r\n", ch);
    
		else if (!DOOR_IS_CLOSED(ch, obj, door) &&
			IS_SET(flags_door[subcmd], NEED_CLOSED))
			send_to_char("But it's currently open!\r\n", ch);
    
		else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
			IS_SET(flags_door[subcmd], NEED_LOCKED))
			send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    
		else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
			IS_SET(flags_door[subcmd], NEED_UNLOCKED))
			send_to_char("It seems to be locked.\r\n", ch);
    
		else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
			((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
			send_to_char("You don't seem to have the proper key.\r\n", ch);
    
		else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
			do_doorcmd(ch, obj, door, subcmd);
		}
	
	return;
}



ACMD(do_enter)
{
	int door;

	one_argument(argument, buf);

	if (*buf)
	{			/* an argument was supplied, search for door
				 * keyword */
		for (door = 0; door < NUM_OF_DIRS; door++)
			
			if (EXIT(ch, door))
				
				if (EXIT(ch, door)->keyword)
					
					if (!str_cmp(EXIT(ch, door)->keyword, buf))
					{
						perform_move(ch, door, 1);
						return;
					}
    
		sprintf(buf2, "There is no %s here.\r\n", buf);
		send_to_char(buf2, ch);
	} 
	
	else if (ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
		send_to_char("You are already indoors.\r\n", ch);
  
	else {
		/* try to locate an entrance */
		for (door = 0; door < NUM_OF_DIRS; door++)
			
			if (EXIT(ch, door))
				
				if (EXIT(ch, door)->to_room != NOWHERE)
					
					if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
					ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS))
					{
						perform_move(ch, door, 1);
						return;
					}
    
	send_to_char("You can't seem to find anything to enter.\r\n", ch);
	
	}
}


ACMD(do_leave)
{
	int door;

	if (!ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
		send_to_char("You are outside.. where do you want to go?\r\n", ch);
  
	else
	{
		for (door = 0; door < NUM_OF_DIRS; door++)
			
			if (EXIT(ch, door))
	
				if (EXIT(ch, door)->to_room != NOWHERE)
	  
					if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
					!ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS))
					{
						perform_move(ch, door, 1);
						return;
					}
		send_to_char("I see no obvious exits to the outside.\r\n", ch);
	}
}


ACMD(do_stand)
{
	switch (GET_POS(ch))
	{
	case POS_STANDING:
    
		act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	case POS_SITTING:
		act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_STANDING);
		break;
  
	case POS_RESTING:
		act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_STANDING);
		break;
  
	case POS_SLEEPING:
		act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	case POS_FIGHTING:
		act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	default:
		act("You stop floating around, and put your feet on the ground.",
		FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops floating around, and puts $s feet on the ground.",
		TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_STANDING);
		break;
	}
}


ACMD(do_sit)
{
  
	switch (GET_POS(ch))
	{
  
	case POS_STANDING:
		act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SITTING);
		break;
  
	case POS_SITTING:
		send_to_char("You're sitting already.\r\n", ch);
		break;
  
	case POS_RESTING:
		act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SITTING);
		break;
  
	case POS_SLEEPING:
		act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	case POS_FIGHTING:
		act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	default:
		act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SITTING);
		break;
	}
}


ACMD(do_rest)
{
  
	switch (GET_POS(ch))
	{
	
	case POS_STANDING:
		act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_RESTING);
		break;
  
	case POS_SITTING:
		act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
		act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_RESTING);
		break;
 
	case POS_RESTING:
		act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	case POS_SLEEPING:
		act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	case POS_FIGHTING:
		act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
		break;
  
	default:
		act("You stop floating around, and stop to rest your tired bones.",
		FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SITTING);
		break;
	}
}


ACMD(do_sleep)
{
	switch (GET_POS(ch))
	{
  
	case POS_STANDING:
	case POS_SITTING:
	case POS_RESTING:
    
		send_to_char("You go to sleep.\r\n", ch);
		act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SLEEPING);
		break;

	case POS_SLEEPING:
		send_to_char("You are already sound asleep.\r\n", ch);
		break;
  
	case POS_FIGHTING:
		send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
		break;
  
	default:
	
		act("You stop floating around, and lie down to sleep.",
		FALSE, ch, 0, 0, TO_CHAR);
		act("$n stops floating around, and lie down to sleep.",
		TRUE, ch, 0, 0, TO_ROOM);
		change_pos(ch, POS_SLEEPING);
		break;
	}
}


ACMD(do_wake)
{
	char_data *vict;
	int self = 0;

	one_argument(argument, arg);

	if (*arg)
	{
		if (GET_POS(ch) == POS_SLEEPING)
			send_to_char("Maybe you should wake yourself up first.\r\n", ch);
 
		else if ((vict = get_char_room_vis(ch, arg)) == nullptr)
			send_to_char(NOPERSON, ch);

		else if (vict == ch)
			self = 1;

		else if (GET_POS(vict) > POS_SLEEPING)
			act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    
		else if (AFF_FLAGGED(vict, AFF_SLEEP))
			act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    
		else if (GET_POS(vict) < POS_SLEEPING)
			act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    
		else
		{
			act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
			act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
			change_pos(ch, POS_SITTING);
		}
    
		if (!self)
			return;
	}
  
	if (AFF_FLAGGED(ch, AFF_SLEEP))
		send_to_char("You can't wake up!\r\n", ch);
  
	else if (GET_POS(ch) > POS_SLEEPING)
		send_to_char("You are already awake...\r\n", ch);
  
	else
	{
		send_to_char("You awaken, and sit up.\r\n", ch);
		act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
			change_pos(ch, POS_SITTING);
	}
}


ACMD(do_follow)
{
	char_data *leader;

	one_argument(argument, buf);

	if (*buf)
	{
		if (!(leader = get_char_room_vis(ch, buf)))
		{
			send_to_char(NOPERSON, ch);
			return;
		}
	}
	
	else
	{
		send_to_char("Whom do you wish to follow?\r\n", ch);
		return;
	}

	if(GET_RACE(leader) != GET_RACE(ch) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(leader) < LVL_IMMORT && !IS_NPC(ch))
	{
		send_to_char("What?! Don't you think they'd betray you?\r\n", ch);
		return;
	}

	if (ch->master == leader)
	{
		act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
		return;
	}

	else
	{			/* Not Charmed follow person */
		if (leader == ch)
		{
			if (!ch->master)
			{
				send_to_char("You are already following yourself.\r\n", ch);
				return;
			}
      
			stop_follower(ch);
		} 
		
		else
		{
			if (circle_follow(ch, leader))
			{
				act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
				return;
			}
      
			if (ch->master)
				stop_follower(ch);
      
			REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
			add_follower(ch, leader);
		}
	}
}
