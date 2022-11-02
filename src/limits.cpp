/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "db.h"
#include "handler.h"
#include "dg_scripts.h"
#include "screen.h"

extern char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern struct memory_data *mlist;
extern int max_exp_gain;
extern int max_exp_loss;
extern int idle_rent_time;
extern int idle_max_level;
extern int idle_void;
extern int use_autowiz;
extern int min_wizlist_lev;
extern int free_rent;

/* local functions */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
void check_autowiz(char_data * ch);
void delevel(char_data *ch);
void Crash_rentsave(char_data *ch, int cost);
int level_exp(int level);
char *title_male(int race);
void update_char_objects(char_data * ch);	/* handler.c */
extern class wizlist_data *wizlist;
extern struct index_data *obj_index;
extern sh_int find_target_room(char_data * ch, char * rawroomstr);

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

	if (age < 15)
		return (p0);		/* < 15   */
	
	else if (age <= 29)
		return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
	
	else if (age <= 44)
		return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
	
	else if (age <= 59)
		return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
	
	else if (age <= 79)
		return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
	
	else
		return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */

void roll_moves(char_data *ch)
{


	if(IS_GREYMAN(ch))
		GET_MAX_MOVE(ch) = number(220, 260);
	
	else if(IS_HUMAN(ch) || IS_FADE(ch) || IS_DREADLORD(ch))
		GET_MAX_MOVE(ch) = number(120, 145);

	else if(GET_RACE(ch) == RACE_AIEL)
		GET_MAX_MOVE(ch) = number(165, 230);

	else if(IS_TROLLOC(ch))
		GET_MAX_MOVE(ch) = number(210, 250);

	else
	{
		sprintf(buf, "Error with rolling moves. Character %s does not have a valid class.\r\n", GET_NAME(ch));
		mudlog(buf, NRM, LVL_IMMORT, TRUE);
		GET_MAX_MOVE(ch) = number(100, 150);
	}

	return;
}


int shadow_gain(char_data *ch)
{

	int gain = 0;

	if(!IS_FADE(ch) && !IS_DREADLORD(ch))
		return 0;

	gain = 12;

	switch (GET_POS(ch))
	{
    
	case POS_SLEEPING:
		gain *= 2;
		break;
    
	case POS_RESTING:
		gain += (gain / 2);	/* Divide by 2 */
		break;
    
	case POS_SITTING:
		gain += (gain / 4);	/* Divide by 4 */
		break;
	}

	if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
		gain /= 4;

	if (AFF_FLAGGED(ch, AFF_POISON))
		gain /= 4;

	return (gain);
}


int mana_gain(char_data *ch)
{
	int gain = 0;

	if (IS_NPC(ch))
	{
    
		/* Neat and fast */
		gain = GET_LEVEL(ch);
	} 
	
	else
	{
		gain = graf(age(ch)->year, 4, 8, 12, 16, 12, 10, 8);

		/* Class calculations */
		/* Skill/Spell calculations */
		/* Position calculations    */

		switch (GET_POS(ch))
		{
    
		case POS_SLEEPING:
			gain *= 2;
			break;
    
		case POS_RESTING:
			gain += (gain / 2);	/* Divide by 2 */
			break;
    
		case POS_SITTING:
			gain += (gain / 4);	/* Divide by 4 */
			break;
		}

		if (IS_CHANNELER(ch))
			gain *= 4;

		if(IS_DREADLORD(ch) || ch->AES_SEDAI())
			gain *= 5;

		if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
			gain /= 4;
	}

	if (AFF_FLAGGED(ch, AFF_POISON))
		gain /= 4;

	if(PRF_FLAGGED(ch, PRF_SOURCE))
		gain -= 2;

	return (gain);
}


/* Hitpoint gain pr. game hour */
int hit_gain(char_data * ch)
{
	int gain;

	if (IS_NPC(ch))
    gain = GET_LEVEL(ch);
 
	else
	{

		gain = GET_CON(ch) * 2;

		/* Class/Level calculations */
		/* Skill/Spell calculations */
		/* Position calculations    */

		switch (GET_POS(ch))
		{
    
		case POS_SLEEPING:
			gain += (gain / 2);	/* Divide by 2 */
			break;
    
		case POS_RESTING:
			gain += (gain / 4);	/* Divide by 4 */
			break;
    
		case POS_SITTING:
			gain += (gain / 8);	/* Divide by 8 */
			break;
		}


		if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
			gain /= 4;
	}

	if (AFF_FLAGGED(ch, AFF_POISON))
		gain /= 4;

	return (gain);
}



/* move gain pr. game hour */
int move_gain(char_data * ch)
{
	int gain;

	if (IS_NPC(ch))
	{
		/* Neat and fast */
		gain = GET_LEVEL(ch);
	}	

	if(MOB_FLAGGED(ch, IS_HORSE(ch)))
		gain = GET_MAX_MOVE(ch) / 3;
  
	else
	{
		if(IS_TROLLOC(ch))
			gain = 42;   //Troll regen//
	  
		else if(ch->getClan(CLAN_ALGHOL) || IS_GREYMAN(ch))
			gain = 46;

		else
			gain = 28;   //Other regen//

		/* Class/Level calculations */
		/* Skill/Spell calculations */


		/* Position calculations    */
		if(IS_TROLLOC(ch))
		{
	
			switch(GET_POS(ch))
			{
	
			case POS_SLEEPING:
				gain += (gain / 4);	/* Divide by 4 */
				break;
    
			case POS_RESTING:
				gain += (gain / 8);	/* Divide by 8 */
				break;
		
			case POS_SITTING:
				gain += (gain / 12);	/* Divide by 12 */
				break;
			}
		}

		else
		{

			switch (GET_POS(ch))
			{
    
			case POS_SLEEPING:
				gain += (gain / 2);	/* Divide by 2 */
				break;
    
			case POS_RESTING:
				gain += (gain / 4);	/* Divide by 4 */
				break;
    
			case POS_SITTING:
				gain += (gain / 8);	/* Divide by 8 */
				break;
			}
		}

		if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
			gain /= 4;
	}

	if (AFF_FLAGGED(ch, AFF_POISON))
		gain /= 4;

	gain += GET_SKILL(ch, SKILL_VITALITY) / 20;

	return gain;
}

void set_title(char_data * ch, char *title)
{

	if(strlen(title) > MAX_TITLE_LENGTH - 1)
		return;

	if(GET_TITLE(ch))
		delete (GET_TITLE(ch));

	if(title == nullptr)
	{
		GET_TITLE(ch) = nullptr;
		return;
	}

	GET_TITLE(ch) = str_dup(title);
}

void check_autowiz(char_data * ch)
{
	class wizlist_data *temp, *cur;

	if(IS_NPC(ch))
		return;

	for(cur = wizlist;cur;cur = cur->next)
		if(cur->name == GET_NAME(ch))
			REMOVE_FROM_LIST(cur, wizlist, next);

	if(GET_LEVEL(ch) >= LVL_IMMORT)
	{
		cur = new wizlist_data;
		cur->name = GET_NAME(ch);
		cur->level = GET_LEVEL(ch);
		cur->next = wizlist;
		wizlist = cur;
	}
}

void gain_exp(char_data * ch, int gain)
{
	int is_altered = FALSE;
	int num_levels = 0;

	if(GET_LEVEL(ch) >= 50)
		return;

	if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
		return;

	if(FIGHTING(ch))
		if(GET_RACE(ch) == GET_RACE(FIGHTING(ch)))
			gain = 10;

	if (IS_NPC(ch))
	{
		GET_EXP(ch) += gain;
		return;
	}
	
	if (gain > 0)
	{
		gain = MIN(max_exp_gain, gain);	/* put a cap on the max gain per kill */
		GET_EXP(ch) += gain;
    
		while (GET_LEVEL(ch) < LVL_IMMORT && GET_EXP(ch) >= level_exp(GET_LEVEL(ch) + 1))
		{
			GET_LEVEL(ch) += 1;
			num_levels++;
			advance_level(ch, TRUE);
			is_altered = TRUE;
		}

		if (is_altered)
		{
			if (num_levels == 1)
				message(ch, "%s%sYou rise a level!\r\n%s",
					COLOR_BOLD(ch, CL_SPARSE), COLOR_GREEN(ch, CL_SPARSE), COLOR_NORMAL(ch, CL_SPARSE));
      
			else
				message(ch, "%s%sYou rise %d levels!%s\r\n",
					COLOR_BOLD(ch, CL_SPARSE), COLOR_GREEN(ch, CL_SPARSE), num_levels, COLOR_NORMAL(ch, CL_SPARSE));
      
		}

		is_altered = FALSE;
	
	}
	
	else if (gain < 0)
	{    
		gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
		GET_EXP(ch) += gain;
    
		if (GET_EXP(ch) < 0)
			GET_EXP(ch) = 0;
/*
		while (GET_LEVEL(ch) < LVL_IMMORT && GET_EXP(ch) <= level_exp(GET_LEVEL(ch)))
		{
			num_levels++;
			delevel(ch);
			is_altered = TRUE;
		}

		if (is_altered)
		{
			if (num_levels == 1)
				send_to_char("You lose a level!\r\n", ch);
      
			else
				message(ch, "You lose %d levels!\r\n", num_levels);

		}

	*/
	}
}

void gain_exp_regardless(char_data * ch, int gain)
{
	int is_altered = FALSE;
	int num_levels = 0;

	GET_EXP(ch) += gain;
  
	if (GET_EXP(ch) < 0)
		GET_EXP(ch) = 0;

	if(gain > level_exp(GET_LEVEL(ch) + 1) / 10)
		gain = level_exp(GET_LEVEL(ch) + 1) / 10;

	if(gain < -(level_exp(GET_LEVEL(ch))))
		gain = -(level_exp(GET_LEVEL(ch)));


	if (!IS_NPC(ch))
	{
    
		while (GET_LEVEL(ch) < LVL_IMPL &&
		GET_EXP(ch) >= level_exp(GET_LEVEL(ch) + 1))
		{
      
			GET_LEVEL(ch) += 1;
			num_levels++;
			advance_level(ch, TRUE);
			is_altered = TRUE;
		}

		if (is_altered)
		{
			
			if (num_levels == 1)
				send_to_char("You rise a level!\r\n", ch);
      
			else
			{
				sprintf(buf, "You rise %d levels!\r\n", num_levels);
				send_to_char(buf, ch);
			}
      
			check_autowiz(ch);
		}
	}

	if (gain < 0 && !IS_NPC(ch))
	{    
		gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
		GET_EXP(ch) += gain;
    
		if (GET_EXP(ch) < 0)
			GET_EXP(ch) = 0;
/*
		while (GET_LEVEL(ch) < LVL_IMMORT && GET_EXP(ch) <= level_exp(GET_LEVEL(ch)))
		{
			GET_LEVEL(ch) -= 1;
			num_levels++;
			delevel(ch);
			is_altered = TRUE;
		}

		if (is_altered)
		{
			if (num_levels == 1)
				send_to_char("You lose a level!\r\n", ch);
      
			else
				message(ch, "You lose %d levels!\r\n", num_levels);
      
		}
		*/
	}

}


void gain_condition(char_data * ch, int condition, int value)
{
	bool intoxicated;

	if (IS_NPC(ch) || GET_COND(ch, condition) == -1)	/* No change */
		return;

	intoxicated = (GET_COND(ch, DRUNK) > 0);

	GET_COND(ch, condition) += value;

	GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
	GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

	if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
		return;

	switch (condition)
	{
  
	case FULL:
		send_to_char("You are hungry.\r\n", ch);
		return;
  
	case THIRST:
		send_to_char("You are thirsty.\r\n", ch);
		return;
  
	case DRUNK:
		
		if (intoxicated)
			send_to_char("You are now sober.\r\n", ch);
			return;
  
	default:
		break;
	}

}


void check_idling(char_data * ch)
{
	if (++(ch->char_specials.timer) > idle_void)
	{
		if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE)
		{
			GET_WAS_IN(ch) = ch->in_room;
			
			if (FIGHTING(ch))
			{
				stop_fighting(FIGHTING(ch));
				stop_fighting(ch);
			}
      
			act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
			send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
			ch->save();
			Crash_crashsave(ch);
			char_from_room(ch);
			char_to_room(ch, find_target_room(ch, "20"));
		} 
		
		else if (ch->char_specials.timer > idle_rent_time)
		{
			if (ch->in_room != NOWHERE)
				char_from_room(ch);
      
			char_to_room(ch, find_target_room(ch, "20"));
      
			if (ch->desc)
			{
				STATE(ch->desc) = CON_DISCONNECT;
	
				/*
				 * For the 'if (d->character)' test in close_socket().
				 * -gg 3/1/98 (Happy anniversary.)
				 */
	
				ch->desc->character = nullptr;
				ch->desc = nullptr;
			}
      
			if (free_rent)
				Crash_rentsave(ch, 0);
      
			else
				Crash_idlesave(ch);

			sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
			mudlog(buf, CMP, LVL_GOD, TRUE);
			extract_char(ch);
		}
	}
}

void sickness_change(char_data *ch)
{
	int gain = 0;

	int hitp = MAX(1, (GET_HIT(ch) * 100)) / MAX(1, GET_MAX_HIT(ch));
	int movep = MAX(1, (GET_MOVE(ch) * 100)) / MAX(1, GET_MAX_MOVE(ch));
//	int manap = MAX(1, (GET_MANA(ch) * 100)) / MAX(1, GET_MAX_MANA(ch));

	if(GET_COND(ch, FULL) <= 12 || GET_COND(ch, THIRST) <= 12 || GET_COND(ch, DRUNK) >= 12)
		gain += number(0, 2);

	if((GET_TAINT(ch) >= 15 && HAS_SOURCE(ch)) || GET_TAINT(ch) >= 50)
		gain += number(0, GET_TAINT(ch));

	if(GET_MOVE(ch) <= 50)
		gain += number(0, 5 - (MAX(1, GET_MOVE(ch)) / 10));

	if(GET_HIT(ch) <= 50)
		gain += number(0, 5 - (MAX(1, GET_HIT(ch)) / 10));


	if(GET_COND(ch, FULL) >= 18 || GET_COND(ch, THIRST) >= 18)
		gain -= number(0, 10);

	if((hitp >= 75 || movep >= 75))
		gain -= number(0, 10);

	GET_SICKNESS(ch) += gain;
	GET_SICKNESS(ch) = MAX(0, GET_SICKNESS(ch));
	GET_SICKNESS(ch) = MIN(1000, GET_SICKNESS(ch));
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
	char_data *i, *next_char;
	struct obj_data *j;
	bool corpse = true;

	/* characters */
	for (i = character_list; i; i = next_char)
	{
		next_char = i->next;
	
		if (GET_POS(i) >= POS_STUNNED)
		{

			GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
			GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
			GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
			GET_SHADOW(i) = MIN(GET_SHADOW(i) + shadow_gain(i), GET_MAX_SHADOW(i));

			sickness_change(i);

			if(GET_STRAIN(i) >= 250)
			{
				GET_TAINT(i)++;

				GET_TAINT(i) += (GET_STRAIN(i) - 250) / 50;
			}

			GET_STRAIN(i) -= GET_LEVEL(i) / 2 + (GET_WIS(i));
			GET_STRAIN(i) = MAX(0, GET_STRAIN(i));

			GET_MANA(i) = MAX(GET_MANA(i), 0);
			GET_SHADOW(i) = MAX(GET_SHADOW(i), 0);

			FLEE_LAG(i) -= 10;
			FLEE_LAG(i) = MAX(FLEE_LAG(i), 0);
      
			if (AFF_FLAGGED(i, AFF_POISON))
				if (damage(i, i, 2, SPELL_POISON) == -1)
					continue;	/* Oops, they died. -gg 6/24/98 */
      
				if (GET_POS(i) <= POS_STUNNED)
					update_pos(i);
		}
		
		else if (GET_POS(i) == POS_INCAP)
		{
			if (damage(i, i, 1, TYPE_SUFFERING) == -1)
				continue;
		} 
		
		else if (GET_POS(i) == POS_MORTALLYW)
		{
			if (damage(i, i, 2, TYPE_SUFFERING) == -1)
				continue;
		}
    
		if (!IS_NPC(i))
		{
			update_char_objects(i);
			
			if (GET_LEVEL(i) < idle_max_level)
				check_idling(i);
		}

		gain_condition(i, FULL, -1);
		gain_condition(i, DRUNK, -1);
		gain_condition(i, THIRST, -1);

	}

	for(j = object_list;j;j = j->next)
	{
		if(GET_OBJ_TIMER(j) > 0 && IS_CORPSE(j))
			GET_OBJ_TIMER(j)--;
	}

	while(corpse)
	{

		corpse = false;

		/* objects */
		for (j = object_list; j; j = j->next)
		{

			/* If this is a corpse */
			if (IS_CORPSE(j))
			{

				if (!GET_OBJ_TIMER(j))
				{
					corpse = true;

					if (j->carried_by)
					{
						act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
						obj_from_char(j);
					}
		
					else if ((j->in_room != NOWHERE) && (world[j->in_room].people))
					{
						act("$p rots, leaving nothing but bones.",
						TRUE, world[j->in_room].people, j, 0, TO_ROOM);
						
						act("$p rots, leaving nothing but bones.",
						TRUE, world[j->in_room].people, j, 0, TO_CHAR);
					}

					extract_obj(j);
					break;
				}
			}
		
			/* If the timer is set, count it down and at 0, try the trigger */
			/* note to .rej hand-patchers: make this last in your point-update() */
    
			else if (GET_OBJ_TIMER(j)>0)
			{
				GET_OBJ_TIMER(j)--; 
      
				if (!GET_OBJ_TIMER(j))
					timer_otrigger(j);
			}
		}
	}
}
