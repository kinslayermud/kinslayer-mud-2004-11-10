/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
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
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"

/* external structs */
extern char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern int top_of_world;
extern int no_specials;

char *race_name(char_data *ch);
char_data *faceoff(char_data *attacker, char_data *prey);
void hunt_victim(char_data *ch);
void perform_assist(char_data *ch, char_data *helpee);
int sharedClan(char_data *p1, char_data *p2);

ACMD(do_bash);
ACMD(do_get);
ACMD(do_hit);
ACMD(do_stand);

/* local functions */
void mobile_activity();
void clearMemory(char_data * ch);

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

char_data *find_first_assist(char_data *mob)
{
	char_data *buddy = nullptr, *cur = nullptr;
	player_clan_data *cl;
	int last = 0, level = 0, clan_num = 0;

	if(!MOB_FLAGGED(mob, MOB_HELPER))
		return nullptr;

	for(cur = world[mob->in_room].people;cur;cur = cur->next_in_room)
	{
		if(!FIGHTING(cur) || cur == mob)
			continue;

		if(GET_RACE(cur) != GET_RACE(mob))
			continue;
		
		if((GET_RACE(cur) == GET_RACE(mob)) && (GET_RACE(FIGHTING(cur)) != GET_RACE(cur)))
			level = 2;

		if(cur == mob->master)
			level = 4;
		
		if( (clan_num = sharedClan(mob, cur)) && (cl = cur->getClan(clan_num)) )
			level = 6 + cl->rank;

		if(IS_NPC(cur))
			++level;

/* If the current assist priority "level" is higher than the "last", then we
 * will switch the current "buddy" to whichever mob managed to get the higher
 * level
 */
		if(level > last)
		{
			last = level;
			buddy = cur;
		}
		level = 0;
	}

/* And by here, we either have a buddy, or we just return null */
	if(buddy)
		return buddy;
	else
		return nullptr;

}


int can_aggro(char_data *mob, char_data *victim)
{
		
		if(!mob || !victim)
			return 0;

		if(!IS_NPC(mob) || mob == victim)
			return 0;

		if(IS_NPC(victim) && !GET_AGGRO(mob, AGGRO_MOB))
			return 0;

		if(victim->wantedByPlayer(mob))
			return 1;

		if(GET_AGGRO(mob, AGGRO_ALL))
			return 1;

		if(IS_HUMAN(victim) && GET_AGGRO(mob, AGGRO_HUMAN))
			return 1;

		if(IS_TROLLOC(victim) && GET_AGGRO(mob, AGGRO_TROLLOC))
			return 1;

		if(IS_SEANCHAN(victim) && GET_AGGRO(mob, AGGRO_SEANCHAN))
			return 1;

		if(IS_AIEL(victim) && GET_AGGRO(mob, AGGRO_AIEL))
			return 1;

		if(!sharedClan(mob, victim) && GET_AGGRO(mob, AGGRO_NON_CLAN))
			return 1;

	return 0;
}

void mobile_activity()
{
	char_data *ch, *next_ch, *vict;
	struct obj_data *obj, *best_obj;
	int door, found, max, i = 0;
	memory_rec *names;

	for (ch = character_list; ch; ch = next_ch)
	{
		next_ch = ch->next;

		if (!IS_MOB(ch))
			continue;

		/* Examine call for special procedure */
		if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) 
		{
			if (mob_index[GET_MOB_RNUM(ch)].func == nullptr)
			{
				log("SYSERR: %s (#%d): Attempting to call non-existing mob function.",
				GET_NAME(ch), GET_MOB_VNUM(ch));
				REMOVE_BIT_AR(MOB_FLAGS(ch), MOB_SPEC);
			} 
			
			else 
			{
			/* XXX: Need to see if they can handle nullptr instead of "". */
	
				if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
					continue;		/* go to next char */
			}
		}

		/* If the mob has no specproc, do the default actions */
		if (FIGHTING(ch) || !AWAKE(ch))
			continue;

		/* Scavenger (picking up objects) */
		if (MOB_FLAGGED(ch, MOB_SCAVENGER) && !FIGHTING(ch) && AWAKE(ch))
			if (world[ch->in_room].contents && !number(0, 10))
			{
				max = 1;
				best_obj = nullptr;
	
				for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
					if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max)
					{
					
						best_obj = obj;
						max = GET_OBJ_COST(obj);
					}
	
					if (best_obj != nullptr)
					{
						obj_from_room(best_obj);
						obj_to_char(best_obj, ch);
						act("$n gets $p.", FALSE, ch, best_obj, nullptr, TO_ROOM);
					}
			}

		/* Mob Tracking */
		if(MOB_FLAGGED(ch, MOB_TRACK && HUNTING(ch) != nullptr))
		{
			if(GET_POS(ch) < POS_FIGHTING)
				do_stand(ch, nullptr, 0, 0);
				
			hunt_victim(ch);
		}	
	
	/********* Aggressive Mobs *********/
		found = FALSE;
		for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room)
		{

			if(FIGHTING(ch))
				continue;

			if (!CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
				continue;
	
			if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
				continue;

			if(can_aggro(ch, vict))
			{
				if(!AFF_FLAGGED(vict, AFF_HIDE) || (i = number(1, 3)) == 3)
				{
					
					vict = faceoff(ch, vict);
					set_fighting(ch, vict);
					found = TRUE;
				}
			}
		}


	
	if(MOB_FLAGGED(ch, MOB_BASH) && FIGHTING(ch) && number(1, 3) == 3)
	{
		do_bash(ch, GET_NAME(FIGHTING(ch)), 0, 0);
	}

		/* Mob Memory */
		if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch))
		{
			found = FALSE;
			for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room)
			{
	
				if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
					continue;
	
				for (names = MEMORY(ch); names && !found; names = names->next)
					if (names->id == GET_IDNUM(vict))
					{
						found = TRUE;
						do_hit(ch, vict->player.name, 0, 0);
					}
			}
		}

		/* Helper Mobs */
		if (MOB_FLAGGED(ch, MOB_HELPER))
		{

			if((vict = find_first_assist(ch)))
				perform_assist(ch, vict);
		}

		/* Mob Movement */
		if (!ch->master && !MOB_FLAGGED(ch, MOB_SENTINEL) && (GET_POS(ch) == POS_STANDING) &&
	    ((door = number(0, 18)) < NUM_OF_DIRS) && CAN_GO(ch, door) &&
	    !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB) &&
		!ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_DEATH) &&
		(!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
		(world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone)))
		{
			perform_move(ch, door, 1);
		}
    
		/* Add new mobile actions here */

	}				/* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void remember(char_data * ch, char_data * victim)
{
	memory_rec *tmp;
	bool present = FALSE;

	if (!IS_NPC(ch) || IS_NPC(victim) || PRF_FLAGGED(victim, PRF_NOHASSLE))
		return;

	for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
		if (tmp->id == GET_IDNUM(victim))
			present = TRUE;

	if (!present)
	{
		CREATE(tmp, memory_rec, 1);
		tmp->next = MEMORY(ch);
		tmp->id = GET_IDNUM(victim);
		MEMORY(ch) = tmp;
	}
}


/* make ch forget victim */
void forget(char_data * ch, char_data * victim)
{
	memory_rec *curr, *prev = nullptr;

	if (!(curr = MEMORY(ch)))
		return;

	while (curr && curr->id != GET_IDNUM(victim))
	{
		prev = curr;
		curr = curr->next;
	}

	if (!curr)
		return;			/* person wasn't there at all. */

	if (curr == MEMORY(ch))
		MEMORY(ch) = curr->next;
  
	else
		prev->next = curr->next;

	delete[] (curr);
}


/* erase ch's memory */
void clearMemory(char_data * ch)
{
	memory_rec *curr, *next;

	curr = MEMORY(ch);

	while (curr)
	{
		next = curr->next;
		delete[] (curr);
		curr = next;
	}

	MEMORY(ch) = nullptr;
}
