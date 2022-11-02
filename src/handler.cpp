/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
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
#include "interpreter.h"
#include "screen.h"
#include "dg_scripts.h"

/* external vars */
extern int seconds;
extern int global_minutes;
extern int top_of_world;
extern struct legend_data legend[8];
extern char_data *combat_list;
extern struct room_data *world;
extern struct obj_data *object_list;
extern char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct memory_data *mlist;
extern sh_int find_target_room(char_data * ch, char * rawroomstr);
extern const char *MENU;
char *race_name(char_data *ch);
extern struct track_data *track_list;
int boot_high = 0;

/* local functions */
int apply_ac(char_data * ch, int eq_pos);
void update_object(struct obj_data * obj, int use);
void update_char_objects(char_data * ch);
void stop_riding(char_data *ch);

/* external functions */
int invalid_class(char_data *ch, struct obj_data *obj);
int update_legend(char_data *ch);
void remove_follower(char_data * ch);
void clearMemory(char_data * ch);
void die_follower(char_data * ch);
void add_to_watch(char_data *ch);
void remove_from_watch(char_data *ch);
void update_track_age(room_rnum room);
void remove_object(struct obj_data *obj);
int dodge_find(char_data *ch);
int parry_find(char_data *ch);
int offense_find(char_data *ch);
int abs_find(char_data *ch);
int get_watch_time(char_data *ch);
int get_watch_seconds(int ammount);
int get_watch_minutes(int ammount);
int check_multiplaying(char_data *ch);
int weapon_skill(char_data *ch, int weapon_type);

struct watch_data *watch_head = nullptr;
ACMD(do_return);

/* Galnor: 5-28-2004 Will return 1 if item is fount in container.
Types:	Thorough	-	Will search unlimited ammount of containers.
		Basic		-	Will search just one container for item
*/
int search_for_content(struct obj_data *container, int vnum, int type)
{
	struct obj_data *obj;

	for(obj = container->contains;obj;obj = obj->next_content)
	{
		if(GET_OBJ_VNUM(obj) == vnum)
			return 1;

		if(obj->contains && type == THOROUGH)
		{
			if(search_for_content(obj, vnum, type))
				return 1;
		}
	}

	return 0;
}

/* Return the weight of held object and back object(worn), of a player. Galnor: 6-23-2004 */
int held_and_back(char_data *ch)
{
	int weight = 0;

	if(GET_EQ(ch, WEAR_HOLD))
		weight += object_weight(GET_EQ(ch, WEAR_HOLD));

	if(GET_EQ(ch, WEAR_BACK))
		weight += object_weight(GET_EQ(ch, WEAR_BACK));

	return weight;
}

int weapon_skill(char_data *ch, int weapon_type)
{
	int skill_type = 0;

	switch(weapon_type)
	{

	case WEAPON_LONG_BLADE:
		skill_type = SKILL_LONG_BLADE;
		break;

	case WEAPON_SHORT_BLADE:
		skill_type = SKILL_SHORT_BLADE;
		break;

	case WEAPON_STAFF:
		skill_type = SKILL_STAFF;
		break;

	case WEAPON_SPEAR:
		skill_type = SKILL_SPEAR;
		break;

	case WEAPON_AXE:
		skill_type = SKILL_AXE;
		break;

	case WEAPON_CLUB:
		skill_type = SKILL_CLUB;
		break;

	case WEAPON_CHAIN:
		skill_type = SKILL_CHAIN;
		break;

	case WEAPON_LANCE:
		skill_type = SKILL_LANCE;
		break;

	default:
		return -1;

	}

	return GET_SKILL(ch, skill_type);

}

int specific_inventory_count(char_data *ch, int vnum)
{
	struct obj_data *obj;
	int count = 0;

	for(obj = ch->carrying;obj;obj = obj->next_content)
	{
		if(real_object(vnum) == GET_OBJ_RNUM(obj))
			count++;
	}

	return count;
}
	


void check_boot_high(void)
{
	int i = 0;
	char_data *ch;

	for(ch = character_list;ch;ch = ch->next)
		if(!IS_NPC(ch))
			i++;

	if(i > boot_high)
		boot_high = i;
}

int check_multiplaying(char_data *ch)
{

	struct descriptor_data *d;
	int count = 0;
	
	if(IS_NPC(ch))
		return 0;

	if(!ch->desc)
		return 0;

	for(d = descriptor_list;d;d = d->next)
		if(!str_cmp(d->host, ch->desc->host))
			if(d->character)
				if(GET_NAME(d->character))
					count++;

	if(count > 1)
		for(d = descriptor_list;d;d = d->next)
			if(!str_cmp(d->host, ch->desc->host))
				if(d->character)
					if(GET_NAME(d->character)) 
						if(GET_LEVEL(d->character) < LVL_IMPL && !LOCAL_IP(d))
						{
							sprintf(buf, "%s is MULTIPLAYING!", GET_NAME(d->character));
							mudlog(buf, NRM, LVL_GRGOD, TRUE);
						}



	return 0;
}

int get_watch_seconds(int ammount)
{

	int i = ammount;

	i -= get_watch_minutes(ammount) * 60;

	return i;
}
	
int get_watch_minutes(int ammount)
{

	int min = 0;

		min = (ammount / 60);
	
	return min;
}

int get_watch_time(char_data *ch)
{
	struct watch_data *cur;

	if(IS_NPC(ch))
		return 0;

	if(!ch->desc)
		return 0;



	for(cur = watch_head;cur;cur = cur->next)
	{
		if(cur->name)
			if(str_cmp(cur->name, GET_NAME(ch)) && !str_cmp(ch->desc->host, cur->host))
			{
				return seconds - cur->time;
			}
	}
		

	return 0;
}

void add_to_watch(char_data *ch)
{
	struct watch_data *entry;

	CREATE(entry, watch_data, 1);

	if(IS_NPC(ch))
		return;

	if(!ch->desc)
		return;


	strcpy(entry->host, ch->desc->host);
	strcpy(entry->name, GET_NAME(ch));
	entry->time = seconds;

	entry->next = watch_head;
	watch_head = entry;

}

void remove_from_watch(char_data *ch)
{

	struct watch_data *removed;
	struct watch_data *temp;

	if(IS_NPC(ch))
		return;

	if(!ch->desc)
		return;

	for(removed = watch_head;removed;removed = removed->next)
		if(removed->name)
			if(!str_cmp(removed->name, GET_NAME(ch)))
				REMOVE_FROM_LIST(removed, watch_head, next);

	if(removed)
		delete [] removed;

}

/* Galnor October 26, 2003- Return the legend if the character's name matches the person on the list */
int char_data::getLegend()
{
	int pos = 1;

	for(pos = 1;pos <= 8;pos++)
		if(!str_cmp(legend[pos].person, GET_NAME(this)))
			return pos;
	
	return -1;

}

/* Is this person a master? -- Galnor */
bool char_data::isMaster()
{

	for(player_clan_data *cl = this->clans;cl;cl = cl->next)
	{
		if(cl->rank >= 8)
			return 1;
		
		if(IS_FADE(this) || IS_DREADLORD(this) || IS_GREYMAN(this) || IS_BLADEMASTER(this))
			return 1;
	}

	return 0;
}

/* Find Dodging Bonus -- Galnor */
int dodge_find(char_data *ch)
{
	int dodge = 0;
	int i;
	int add;
	float divide, db, result;

	add = (GET_DEX(ch) - 10) * 4;
	
	if(GET_LEVEL(ch) >= 30)
		dodge = DB_MAX_BASE;
		
	else
		dodge = GET_LEVEL(ch);

	dodge += GET_DB(ch);

	dodge += add;
	
	dodge -= ((held_and_back(ch) + carrying_weight(ch)) / 10);

	if(MOUNT(ch))
		dodge -= DB_RIDING_MALUS;

	if(GET_MOOD(ch) == MOOD_BRAVE)
		dodge -= DB_BRAVE_MALUS;

	if(GET_MOOD(ch) == MOOD_BERSERK)
		dodge -= DB_BERSERK_MALUS;

	if(IS_NPC(ch))
		dodge = GET_DB(ch);

	/* Get equipment bonuses */
	for(i = 0;i < NUM_WEARS;i++)
	{
		if(GET_EQ(ch, i))
			dodge += GET_OBJ_DB(GET_EQ(ch, i));
	}

	/* Chop off some for those who are sitting or below. */
	if(GET_POS(ch) < POS_FIGHTING)
		dodge = (int) ((double) dodge * DB_SITTING_MALUS);

	/* Now we chop it up according to the player's skill in dodge */
	divide = dodge;
	db = GET_SKILL(ch, SKILL_DODGE);
	result = db / 100;

	if(!IS_NPC(ch))
		dodge = (int) (divide * result);

	if(dodge < 0)
		dodge = 0;

	if(GET_LEVEL(ch) == 1 && IS_NPC(ch))
		dodge = 0;

	if(AFF_FLAGGED(ch, AFF_AGILITY))
		dodge += 12;

	return dodge;
}


/* Find Offense Bonus -- Galnor */
int offense_find(char_data *ch)
{
	
	int offense = 0;
	int i;
	float divide, ob, result;
	struct obj_data *wielded;

	if(IS_NPC(ch))
		offense = GET_OB(ch);
	
	else
	{

		if(!IS_NPC(ch))
		{
			if(GET_LEVEL(ch) >= 30)
				offense = OB_MAX_BASE;
		
			else
				offense = GET_LEVEL(ch);
		}
	}

	for(i = 0;i < NUM_WEARS;i++)
		if(GET_EQ(ch, i))
			offense += GET_OBJ_OB(GET_EQ(ch, i));

	if(!IS_NPC(ch))
		offense += GET_STR(ch);

	offense -= ((held_and_back(ch) + carrying_weight(ch)) / 10);

	/* From here we find the value by taking their skill in their weapon class and applying that */
	divide = offense;
	wielded = GET_EQ(ch, WEAR_WIELD);

	if(wielded)
		ob = weapon_skill(ch, GET_OBJ_VAL(wielded, 0));

	else
		ob = 0;


	result = ob / 100;

	/* And now we finish it off...*/

	if(!IS_NPC(ch))
		offense = (int) (divide * result);

	if(offense < 0)
		offense = 0;


	if(!IS_NPC(ch))
	{
		if(MOUNT(ch))
			offense += OB_RIDING_BONUS;

		if(GET_MOOD(ch) == MOOD_BRAVE)
			if(ch->isMaster())
				offense += OB_MASTER_BRAVE;
			
			else
				offense += OB_BRAVE;
	
		else if(GET_MOOD(ch) == MOOD_NORMAL)
			if(ch->isMaster())
				offense += OB_MASTER_NORMAL;
		
			else
				offense += OB_NORMAL;

		else if(GET_MOOD(ch) == MOOD_BERSERK)
			if(ch->isMaster())
				offense += OB_MASTER_BERSERK;
		
			else
				offense += OB_BERSERK;

	}

	return offense;
}

/* Find Parrying Bonus -- Galnor */
int parry_find(char_data *ch)
{
	int parry = 0;
	int i;
  
	if(IS_NPC(ch))
		parry = GET_PB(ch);

	for(i = 0;i < NUM_WEARS;i++)
	{
		if(GET_EQ(ch, i))
		{

			if(i == WEAR_WIELD)
				parry = (int) ((float) parry + ((float) GET_OBJ_PB(GET_EQ(ch, i)) * ((float) weapon_skill(ch, GET_OBJ_VAL(GET_EQ(ch, i), 0)) / 100)));

			else
				parry = (int) ((float) parry + (float) GET_OBJ_PB(GET_EQ(ch, i)) * ((float) GET_SKILL(ch, SKILL_PARRY) / 100));
		}
	}

	parry -= ((held_and_back(ch) + carrying_weight(ch)) / 10);

	if(!IS_NPC(ch))
	{
		if(GET_MOOD(ch) == MOOD_WIMPY)
		{

			if(ch->isMaster())
				parry += PB_MASTER_WIMPY;
			
			else
				parry += PB_WIMPY;
		}

		else if(GET_MOOD(ch) == MOOD_NORMAL)
		{
			
			if(ch->isMaster())
				parry += PB_MASTER_NORMAL;
		
			else
				parry += PB_NORMAL;
		}

		else if(GET_MOOD(ch) == MOOD_BERSERK)
		{
			
			if(ch->isMaster())
				parry += PB_MASTER_BERSERK;
		
			else
				parry += PB_BERSERK;
		}

	}

	if(parry < 0)
		parry = 0;

	return parry;
}

/* Find Absorb Bonus -- Galnor */
int abs_find(char_data *ch)
{
	int abs = 0;
	int i;

	for(i = 0;i < NUM_WEARS;i++)
		if(GET_EQ(ch, i))
			abs += GET_OBJ_ABS(GET_EQ(ch, i));


	return abs;
}

// Move a character from it's room to a random room -Galnor 2-3-2004
void move_char_random(char_data *ch, int bottom, int top, int inside_allowed)
{

	// I put in count in case there is no possible place for this person to go and to prevent MUD hanging.//
	int count = 0, i = 0;

	do
	{
		count ++;

		if(count > 5000)
			return;

		i = number(real_room(bottom), real_room(top));
		
		if(&world[i])
			if(SECT(i) == SECT_INSIDE && !inside_allowed)
				continue;

	}	while(!&world[i]);

	char_from_room(ch);
	char_to_room(ch, i);
}

// Move an object from it's room to a random room -Galnor 2-3-2004
void move_obj_random(struct obj_data *obj, int bottom, int top, int inside_allowed)
{
	int count = 0, i = 0;

	do
	{
		count ++;

		if(count > 5000)
			return;

		i = number(real_room(bottom), real_room(top));

		if(&world[i])
			if(SECT(i) == SECT_INSIDE && !inside_allowed)
				continue;

	}	while(!&world[i]);

	obj_from_room(obj);
	obj_to_room(obj, i);
}

// Redone into function -Galnor November, 2003
char *PERS(char_data *target, char_data *looker)
{
	static char text[256];
	char name[256];

	if(GET_LEVEL(target) >= 15)
		strcpy(name, GET_NAME(target));
	else
		sprintf(name, "a %s", race_name(target));

	if(IS_NPC(target))
		return GET_NAME(target);

	if(GET_LEVEL(target) >= 100 || GET_LEVEL(looker) >= 100 || GET_RACE(target) == GET_RACE(looker))
		if(CAN_SEE(looker, target))
			return GET_NAME(target);

	if(CAN_SEE(looker, target))
	{
		sprintf(text, "*%s%s%s*", COLOR_RED(looker, CL_NORMAL), name, COLOR_NORMAL(looker, CL_NORMAL));
		return text;
	}

	else
		return "Someone";
}

// Can see, return name, else, return someone -Galnor November, 2003
char *DARKNESS_CHECK(char_data *ch, char_data *vict)
{
	
	if(CAN_SEE(ch, vict))
		return GET_NAME(vict);
	
	else
		return "Someone";
}

char *fname(char *namelist)
{
	
	static char holder[30];
	char *point;

	for (point = holder; isalpha(*namelist); namelist++, point++)
		*point = *namelist;

	*point = '\0';

	return (holder);
}

void stop_riding(char_data *ch)
{

	char_data *mount;

	mount = MOUNT(ch);

	if(mount)
	{
		MOUNT(ch) = nullptr;
		RIDDEN_BY(mount) = nullptr;
	}
	

}

int isname(const char *str, const char *namelist)
{
	const char *curname, *curstr;

	curname = namelist;
	
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if (!*curstr && !isalpha(*curname))
				return (1);

			if (!*curname)
				return (0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

		/* skip to next name */

		for (; isalpha(*curname); curname++);
			
		if (!*curname)
			return (0);
    
		curname++;			/* first char of new name */
	}
}


/* Stock isname().  Leave this here even if you put in a newer  *
 * isname().  Used in olc.c					*/
int is_name(const char *str, const char *namelist)
{
	const char *curname, *curstr;

	if (!*str || !*namelist || !str || !namelist)
		return (0);

	curname = namelist;
  
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if (!*curstr && !isalpha(*curname))
				return (1);

			if (!*curname)
				return (0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

	/* skip to next name */

	for (; isalpha(*curname); curname++);
		
	if (!*curname)
		return (0);
				
	curname++;			/* first char of new name */
  
	}
}



void aff_apply_modify(char_data * ch, cbyte loc, int mod, char* msg)
{
	int maxabil;

	maxabil = (IS_NPC(ch) ? 25 : 18);

	switch (loc)
	{
  
	case APPLY_NONE:
		break;

	case APPLY_STR:
		GET_STR(ch) += mod;
		break;
  
	case APPLY_DEX:
		GET_DEX(ch) += mod;
		break;
  
	case APPLY_INT:
		GET_INT(ch) += mod;
		break;
  
	case APPLY_WIS:
		GET_WIS(ch) += mod;
		break;
  
	case APPLY_CON:
		GET_CON(ch) += mod;
		break;
  
	case APPLY_CHA:
		GET_CHA(ch) += mod;
		break;

	case APPLY_CLASS:
		/* ??? GET_CLASS(ch) += mod; */
		break;

  /*
   * My personal thoughts on these two would be to set the person to the
   * value of the apply.  That way you won't have to worry about people
   * making +1 level things to be imp (you restrict anything that gives
   * immortal level of course).  It also makes more sense to set someone
   * to a class rather than adding to the class number. -gg
   */

	case APPLY_LEVEL:
		/* ??? GET_LEVEL(ch) += mod; */
		break;

	case APPLY_AGE:
		ch->player.time.birth -= (mod * SECS_PER_MUD_YEAR);
		break;

	case APPLY_CHAR_WEIGHT:
		GET_WEIGHT(ch) += mod;
		break;

	case APPLY_CHAR_HEIGHT:
		GET_HEIGHT(ch) += mod;
		break;

	case APPLY_MANA:
		GET_MAX_MANA(ch) += mod;
		break;

	case APPLY_HIT:
		GET_MAX_HIT(ch) += mod;
		break;

	case APPLY_MOVE:
		GET_MAX_MOVE(ch) += mod;
		break;

	case APPLY_GOLD:
		break;

	case APPLY_EXP:
		break;

	case APPLY_DB:
		GET_DB(ch) += mod;

	case APPLY_HITROLL:
		break;

	case APPLY_DAMROLL:
		GET_DAMROLL(ch) += mod;
		break;

	case APPLY_SAVING_PARA:
		GET_SAVE(ch, SAVING_PARA) += mod;
		break;

	case APPLY_SAVING_ROD:
		GET_SAVE(ch, SAVING_ROD) += mod;
		break;

	case APPLY_SAVING_PETRI:
		GET_SAVE(ch, SAVING_PETRI) += mod;
		break;

	case APPLY_SAVING_BREATH:
		GET_SAVE(ch, SAVING_BREATH) += mod;
		break;

	case APPLY_SAVING_SPELL:
		GET_SAVE(ch, SAVING_SPELL) += mod;
		break;

	case 28:
		break;

	default:
		log("SYSERR: Unkown apply adjust... Number %d, on char %s, and mod = %d.", loc, GET_NAME(ch), mod);
		break;

	} /* switch */
}

  

 
void affect_modify(char_data * ch, cbyte loc, sbyte mod, long bitv,
                        bool add)
{
	
	if (add)
	{
		SET_BIT_AR(AFF_FLAGS(ch), bitv);
	} 
	
	else
	{
		REMOVE_BIT_AR(AFF_FLAGS(ch), bitv);
		mod = -mod;
	}

	aff_apply_modify(ch, loc, mod, "affect_modify");
}

void affect_modify_ar(char_data * ch, cbyte loc, sbyte mod, int bitv[],
                        bool add)
{
	int i , j;

	if (add)
	{
		for(i = 0; i < AF_ARRAY_MAX; i++)
			for(j = 0; j < 32; j++)
				if(IS_SET_AR(bitv, (i*32)+j))
					SET_BIT_AR(AFF_FLAGS(ch), (i*32)+j);
	} 
		
	else
	{
		for(i = 0; i < AF_ARRAY_MAX; i++)
			for(j = 0; j < 32; j++)
				if(IS_SET_AR(bitv, (i*32)+j))
					REMOVE_BIT_AR(AFF_FLAGS(ch), (i*32)+j);
    
	mod = -mod;
  }

	aff_apply_modify(ch, loc, mod, "affect_modify_ar");
}


/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(char_data * ch)
{
	struct affected_type *af;
	int i, j;

	for (i = 0; i < NUM_WEARS; i++)
	{
		if (GET_EQ(ch, i))
			for (j = 0; j < MAX_OBJ_AFFECT; j++)
				affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
				GET_EQ(ch, i)->affected[j].modifier,
				(int *) GET_EQ(ch, i)->obj_flags.bitvector, FALSE);
  }


	for (af = ch->affected; af; af = af->next)
		affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

	ch->aff_abils = ch->real_abils;

	for (i = 0; i < NUM_WEARS; i++)
	{
		if (GET_EQ(ch, i))
			for (j = 0; j < MAX_OBJ_AFFECT; j++)
				affect_modify_ar(ch, GET_EQ(ch, i)->affected[j].location,
				GET_EQ(ch, i)->affected[j].modifier,
				(int *) GET_EQ(ch, i)->obj_flags.bitvector, TRUE);
	}


	for (af = ch->affected; af; af = af->next)
		affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

	/* Make certain values are between 0..25, not < 0 and not > 25! */

	i = (IS_NPC(ch) ? 25 : 19);

	GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
	GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
	GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
	GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
	GET_STR(ch) = MAX(0, GET_STR(ch));

	if (IS_NPC(ch))
	{
		GET_STR(ch) = MIN(GET_STR(ch), i);
	} 
	
	else
	{
		if (GET_STR(ch) > 19)
		{
			i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
			GET_ADD(ch) = MIN(i, 100);
			GET_STR(ch) = 19;
		}
	}
}

/**********Weave/Affection List Matching**********/

/* Search for a weave attached to a caster by looking for the correct affection. */
struct weave_data *find_attached_weave(char_data *caster, struct affected_type *affection)
{
	struct weave_data *weave;

	for(weave = caster->weaves;weave;weave = weave->next)
	{
		if(weave->spell == affection)
		{
			return weave;
		}
	}

	return nullptr;
}

/* Search the affection list not saved to find the structure that matches sp. Galnor: 6-18-2004 */
struct affect_type_not_saved *find_affection(char_data *target, struct affected_type *sp)
{
	struct affect_type_not_saved *al;

	for(al = target->affection_list;al;al = al->next)
	{

		if(al->affect == sp)
		{
			return al;
		}
	}

	return nullptr;
}

/**********Non Saving Affection List**********/

/* This is the not_saved affected list that will make it easier to trace who casted a spell on someone */
void add_affection_to_list(char_data *ch, struct affect_type_not_saved *al)
{
	return;
	if(!ch || !al)
		return;

	al->next = ch->affection_list;
	ch->affection_list = al;
}

/* Remove the non-saving item from the non-saving affection list. */
void remove_affection_list(char_data *ch, struct affect_type_not_saved *al, int loop)
{

	return;
	struct affect_type_not_saved *temp;
	struct weave_data *weave;

	if(!ch || !al)
		return;

	if((weave = find_attached_weave(al->caster, al->affect)) && loop)
	{
		remove_weave(al->caster, weave, FALSE);
	}

	REMOVE_FROM_LIST(al, ch->affection_list, next);
	affect_remove(ch, al->affect);

	weave = nullptr;

	delete al;
}


/********WEAVES*********/

/* Attach a weave to the caster to give the ability to tie or release it. Galnor 6-16-2004 */
void attach_weave(char_data *ch, char_data *victim, struct affected_type *af)
{
	return;
	struct weave_data *weave;

	if(!weave || !ch || !victim)
		return;

	weave = new weave_data;

	weave->target = victim;
	weave->spell = af;
	weave->next = ch->weaves;
	ch->weaves = weave;
	weave->tied = false;
}

/* Remove a weave from someone. Galnor: 6-16-2004 */
void remove_weave(char_data *ch, struct weave_data *weave, int loop)
{
	return;
	struct weave_data *temp;
	struct affect_type_not_saved *al;

	al = find_affection(weave->target, weave->spell);
	REMOVE_FROM_LIST(weave, ch->weaves, next);

	if(al && loop)
	{
		remove_affection_list(weave->target, al, FALSE);
	}

	delete weave;
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(char_data * ch, struct affected_type * af)
{
	struct affected_type *affected_alloc;

	affected_alloc = new affected_type;

	*affected_alloc = *af;
	affected_alloc->next = ch->affected;
	ch->affected = affected_alloc;

	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
	affect_total(ch);
}

/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */
void affect_remove(char_data * ch, struct affected_type * af)
{
	struct affected_type *temp;
	struct affect_type_not_saved *al;

	if (!ch->affected)
	{
		core_dump();
		return;
	}

	if((al = find_affection(ch, af)))
	{
		remove_affection_list(ch, al, TRUE);
	}

	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
	REMOVE_FROM_LIST(af, ch->affected, next);
	delete(af);
	af = nullptr;
	affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(char_data * ch, int type, int bit)
{
	struct affected_type *hjp, *next;

	for (hjp = ch->affected; hjp; hjp = next)
	{
		next = hjp->next;
    
		if (hjp->type == type || hjp->bitvector == bit)
			affect_remove(ch, hjp);
	}
}



/*
 * Return if a char is affected by a spell (SPELL_XXX), nullptr indicates
 * not affected
 */
bool affected_by_spell(char_data * ch, int type)
{
	struct affected_type *hjp;

	for (hjp = ch->affected; hjp; hjp = hjp->next)
		if (hjp->type == type)
			return TRUE;

	return FALSE;
}



void affect_join(char_data * ch, struct affected_type * af,
		      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
	struct affected_type *hjp;
	bool found = FALSE;

	for (hjp = ch->affected; !found && hjp; hjp = hjp->next)
	{

		if ((hjp->type == af->type) && (hjp->location == af->location))
		{
			if (add_dur)
				af->duration += hjp->duration;
		
			if (avg_dur)
				af->duration /= 2;

			if (add_mod)
				af->modifier += hjp->modifier;
			
			if (avg_mod)
				af->modifier /= 2;

			affect_remove(ch, hjp);
			affect_to_char(ch, af);
			found = TRUE;
		}
	}
	if (!found)
		affect_to_char(ch, af);
}


/* move a player out of a room */
void char_from_room(char_data * ch)
{
	char_data *temp;

	if(ch->in_room == NOWHERE)
		return;

	if (ch == nullptr)
	{
		log("SYSERR: nullptr character in %s, char_from_room", __FILE__);
		exit(1);
	}

	if (FIGHTING(ch) != nullptr)
		stop_fighting(ch);

	if(!IS_NPC(ch) && ch->desc)
		if(ch->desc->timer > 0)
		{
			send_to_char("Cancelled.\r\n", ch);
			ch->desc->timer = 0;
			ch->desc->delayed_state = 0;
			ch->desc->forced = 0;
			ch->desc->command_ready = 0;
		}

	if (GET_EQ(ch, WEAR_LIGHT) != nullptr)
		if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
			if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2))	/* Light is ON */
				world[ch->in_room].light--;

	REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
	ch->in_room = NOWHERE;
	ch->next_in_room = nullptr;
}

void update_track_age(room_rnum room)
{
	track_data *trax, *next_track = nullptr, *temp;

	for(trax = world[room].tracks;trax;trax = next_track)
	{

		next_track = trax->next_in_room;

		if(trax)
			trax->age = global_minutes - trax->laytime;

		if(trax->age >= 60)
		{
			REMOVE_FROM_LIST(trax, world[room].tracks, next_in_room);
			REMOVE_FROM_LIST(trax, track_list, next);
			delete(trax);
		}

	}
	
}

/* place a character in a room */
void char_to_room(char_data * ch, room_rnum room)
{
	if (ch == nullptr || room < 0 || room > top_of_world)
		log("SYSERR: Illegal value(s) passed to char_to_room. (Room: %d/%d Ch: %p",
		room, top_of_world, ch);
  
	else
	{
		ch->next_in_room = world[room].people;
		world[room].people = ch;
		ch->in_room = room;
		update_track_age(room);

		if (GET_EQ(ch, WEAR_LIGHT))
			if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
				if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2))	/* Light ON */
					world[room].light++;

		/* Stop fighting now, if we left. */
		if (FIGHTING(ch) && IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)))
		{
			stop_fighting(FIGHTING(ch));
			stop_fighting(ch);
		}
	}
}


/* give an object to a char   */
void obj_to_char(struct obj_data * object, char_data * ch)
{
	if (object && ch)
	{
		object->next_content = ch->carrying;
		ch->carrying = object;
		object->carried_by = ch;
		object->in_room = NOWHERE;
		IS_CARRYING_N(ch)++;

		/* set flag for crash-save system, but not on mobs! */
		if (!IS_NPC(ch))
			SET_BIT_AR(PLR_FLAGS(ch), PLR_CRASH);
	}
	
	else
		log("SYSERR: nullptr obj (%p) or char (%p) passed to obj_to_char.", object, ch);
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
	struct obj_data *temp;

	if (object == nullptr)
	{
		log("SYSERR: nullptr object passed to obj_from_char.");
		return;
	}

	REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

	/* set flag for crash-save system, but not on mobs! */
	if (!IS_NPC(object->carried_by))
		SET_BIT_AR(PLR_FLAGS(object->carried_by), PLR_CRASH);

	IS_CARRYING_N(object->carried_by)--;
	object->carried_by = nullptr;
	object->next_content = nullptr;
}

int object_weight(struct obj_data *obj)
{
	int weight = 0;
	struct obj_data *j;

	weight += GET_OBJ_WEIGHT(obj);

	for(j = obj->contains;j;j = j->next_content)
	{
		weight += object_weight(j);
	}

	return weight;
}

int CAN_GET_OBJ(char_data *ch, struct obj_data *obj)
{

	if((CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&
	CAN_SEE_OBJ((ch),(obj))))
		return 1;

	else
		return 0;
}

int CAN_CARRY_OBJ(char_data *ch, struct obj_data *obj)
{
	if((carrying_weight(ch) + object_weight(obj)) > CAN_CARRY_W(ch))
		return 0;
	
	else
		return 0;
}

/* These next three functions return the amount of weight carried/worn by someone. */
int carrying_weight(char_data *ch)
{
	struct obj_data *obj;
	int weight = 0;

	for(obj = ch->carrying;obj;obj = obj->next_content)
	{
		weight += object_weight(obj);
	}

	return weight;
}

int wearing_weight(char_data *ch)
{
	int weight = 0, i = 0;

	for(i = 0;i < NUM_WEARS;i++)
	{
		if(GET_EQ(ch, i))
		{
			weight += object_weight(GET_EQ(ch, i));
		}
	}

	return weight;
}

int total_carrying_weight(char_data *ch)
{
	return (carrying_weight(ch) + wearing_weight(ch));
}

/********************************************************/

void equip_char(char_data * ch, struct obj_data * obj, int pos)
{
	int j;

	if (pos < 0 || pos >= NUM_WEARS)
	{
		core_dump();
		return;
	}

	if (GET_EQ(ch, pos))
	{
		log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch),
	    obj->short_description);
		return;
	}
  
	if (obj->carried_by)
	{
		log("SYSERR: EQUIP: Obj is carried_by when equip.");
		return;
	}
  
	if (obj->in_room != NOWHERE)
	{
		log("SYSERR: EQUIP: Obj is in_room when equip.");
		return;
	}


	GET_EQ(ch, pos) = obj;
	obj->worn_by = ch;
	obj->worn_on = pos;
 

	if (ch->in_room != NOWHERE) {
		if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
			if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
				world[ch->in_room].light++;
	}

	for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify_ar(ch, obj->affected[j].location,
		obj->affected[j].modifier,
		(int *) obj->obj_flags.bitvector, TRUE);

	affect_total(ch);
}



struct obj_data *unequip_char(char_data * ch, int pos)
{
	int j;
	struct obj_data *obj;

	if ((pos < 0 || pos >= NUM_WEARS) || GET_EQ(ch, pos) == nullptr) {
		core_dump();
		return nullptr;
	}

	obj = GET_EQ(ch, pos);
	obj->worn_by = nullptr;
	obj->worn_on = -1;


	if (ch->in_room != NOWHERE)
	{
		if (pos == WEAR_LIGHT && GET_OBJ_TYPE(obj) == ITEM_LIGHT)
			if (GET_OBJ_VAL(obj, 2))	/* if light is ON */
				world[ch->in_room].light--;
	}

	GET_EQ(ch, pos) = nullptr;

	for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify_ar(ch, obj->affected[j].location,
		obj->affected[j].modifier,
		(int *) obj->obj_flags.bitvector, FALSE);
  


	affect_total(ch);

	return (obj);
}


int get_number(char **name)
{
	int i;
	char *ppos;
	char number[MAX_INPUT_LENGTH];

	*number = '\0';

	if ((ppos = strchr(*name, '.')))
	{
		*ppos++ = '\0';
		strcpy(number, *name);
		memmove(*name, ppos, strlen(ppos) + 1);
//		strcpy(*name, ppos);  // Serai - *name, ppos overlap - use memmove()

		for (i = 0; *(number + i); i++)
			if (!isdigit(*(number + i)))
				return 0;

		return (atoi(number));
  	}
	
	return 1;
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *listy)
{
	struct obj_data *i;

	for (i = listy; i; i = i->next_content)
		if (GET_OBJ_RNUM(i) == num)
			return i;

	return nullptr;
}



/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(obj_rnum nr)
{
	struct obj_data *i;

	for (i = object_list; i; i = i->next)
		if (GET_OBJ_RNUM(i) == nr)
			return i;

	return nullptr;
}

int room_visibility(char_data *ch, char_data *vict) {


	if(!vict)
		return 0;

	if(IN_ROOM(ch) == IN_ROOM(vict) && CAN_SEE(ch, vict))
		return 1;

	return 0;
}

/* search a room for a char, and return a pointer if found..  */
char_data *get_char_room(char *name, room_rnum room)
{
	char_data *i;
	int j = 0, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = tmpname;

	strcpy(tmp, name);
	if (!(number = get_number(&tmp)))
		return nullptr;

	for (i = world[room].people; i && (j <= number); i = i->next_in_room)
		if (isname(tmp, i->player.name))
			if (++j == number)
				return i;

	return nullptr;
}



/* search all over the world for a char num, and return a pointer if found */
char_data *get_char_num(mob_rnum nr)
{
	char_data *i;

	for (i = character_list; i; i = i->next)
		if (GET_MOB_RNUM(i) == nr)
			return i;

	return nullptr;
}



/* put an object in a room */
void obj_to_room(struct obj_data * object, room_rnum room)
{
	if (!object || room < 0 || room > top_of_world)
		log("SYSERR: Illegal value(s) passed to obj_to_room. (Room #%d/%d, obj %p)",
		room, top_of_world, object);
  
	else
	{
		object->next_content = world[room].contents;
		world[room].contents = object;
		object->in_room = room;
		object->carried_by = nullptr;
    
		if (ROOM_FLAGGED(room, ROOM_HOUSE))
		SET_BIT_AR(ROOM_FLAGS(room), ROOM_HOUSE_CRASH);
	}
}


/* Take an object from a room */
void obj_from_room(struct obj_data * object)
{
	struct obj_data *temp;

	if (!object || object->in_room == NOWHERE)
	{
		log("SYSERR: nullptr object (%p) or obj not in a room (%d) passed to obj_from_room",
		object, object->in_room);
		return;
	}

	REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

	if (ROOM_FLAGGED(object->in_room, ROOM_HOUSE))
		SET_BIT_AR(ROOM_FLAGS(object->in_room), ROOM_HOUSE_CRASH);
  
	object->in_room = NOWHERE;
	object->next_content = nullptr;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data * obj, struct obj_data * obj_to)
{
	if (!obj || !obj_to || obj == obj_to)
	{
		log("SYSERR: nullptr object (%p) or same source (%p) and target (%p) obj passed to obj_to_obj.",
		obj, obj, obj_to);
		return;
	}

	obj->next_content = obj_to->contains;
	obj_to->contains = obj;
	obj->in_obj = obj_to;
}


/* remove an object from an object */
void obj_from_obj(struct obj_data * obj)
{
	struct obj_data *temp, *obj_from;

	if (obj->in_obj == nullptr)
	{
		log("SYSERR: (%s): trying to illegally extract obj from obj.", __FILE__);
		return;
	}

	obj_from = obj->in_obj;
	REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

	obj->in_obj = nullptr;
	obj->next_content = nullptr;
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data * listy, char_data * ch)
{
	if (listy)
	{
		object_list_new_owner(listy->contains, ch);
		object_list_new_owner(listy->next_content, ch);
		listy->carried_by = ch;
	}
}

void remove_object(struct obj_data *obj)
{
	struct obj_data *temp;

	if (obj->worn_by != nullptr)
		if (unequip_char(obj->worn_by, obj->worn_on) != obj)
			log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
  
		if (obj->in_room != NOWHERE)
			obj_from_room(obj);
  
		else if (obj->carried_by)
			obj_from_char(obj);
 
		else if (obj->in_obj)
			obj_from_obj(obj);

	/* Get rid of the contents of the object, as well. */
	while (obj->contains)
		extract_obj(obj->contains);

	REMOVE_FROM_LIST(obj, object_list, next);

	if (GET_OBJ_RNUM(obj) >= 0)
		(obj_index[GET_OBJ_RNUM(obj)].number)--;

	if (SCRIPT(obj))
		extract_script(SCRIPT(obj));
}

/* Extract an object from the world */
void extract_obj(struct obj_data * obj)
{
	struct obj_data *temp;

	if (obj->worn_by != nullptr)
		if (unequip_char(obj->worn_by, obj->worn_on) != obj)
			log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
  
		if (obj->in_room != NOWHERE)
			obj_from_room(obj);
  
		else if (obj->carried_by)
			obj_from_char(obj);
 
		else if (obj->in_obj)
			obj_from_obj(obj);

	/* Get rid of the contents of the object, as well. */
	while (obj->contains)
		extract_obj(obj->contains);

	REMOVE_FROM_LIST(obj, object_list, next);

	if (GET_OBJ_RNUM(obj) >= 0)
		(obj_index[GET_OBJ_RNUM(obj)].number)--;

	if (SCRIPT(obj))
		extract_script(SCRIPT(obj));

	free_obj(obj);
}



void update_object(struct obj_data * obj, int use)
{
	/* dont update objects with a timer trigger */
	if (!SCRIPT_CHECK(obj, OTRIG_TIMER) && (GET_OBJ_TIMER(obj) > 0))
		GET_OBJ_TIMER(obj) -= use;
  
	if (obj->contains)
		update_object(obj->contains, use);
  
	if (obj->next_content)
		update_object(obj->next_content, use);
}


void update_char_objects(char_data * ch)
{
	int i;

	if (GET_EQ(ch, WEAR_LIGHT) != nullptr)
		if (GET_OBJ_TYPE(GET_EQ(ch, WEAR_LIGHT)) == ITEM_LIGHT)
			if (GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2) > 0) {
				i = --GET_OBJ_VAL(GET_EQ(ch, WEAR_LIGHT), 2);
	
				if (i == 1) {
					act("Your light begins to flicker and fade.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n's light begins to flicker and fade.", FALSE, ch, 0, 0, TO_ROOM);
				} 
				
				else if (i == 0) {
					act("Your light sputters out and dies.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n's light sputters out and dies.", FALSE, ch, 0, 0, TO_ROOM);
					world[ch->in_room].light--;
				}
			}
 
	for (i = 0; i < NUM_WEARS; i++)
		if (GET_EQ(ch, i))
			update_object(GET_EQ(ch, i), 2);

	if (ch->carrying)
		update_object(ch->carrying, 1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(char_data * ch)
{
	char_data *k, *temp;
	struct descriptor_data *t_desc;
	struct obj_data *obj;
	int i = 0, freed = 0, room = 0;

	while(ch->weaves)
	{
		remove_weave(ch, ch->weaves, TRUE);
	}

	while(ch->affection_list)
	{
		remove_affection_list(ch, ch->affection_list, TRUE);
	}

	if (!IS_NPC(ch) && !ch->desc)
	{
		for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
			if (t_desc->original == ch)
				do_return(t_desc->character, nullptr, 0, 0);
	}
  
	if (ch->in_room == NOWHERE)
	{
		log("SYSERR: NOWHERE extracting char %s. (%s, extract_char)",
		GET_NAME(ch), __FILE__);
		exit(1);
	}

	stop_riding(ch);

	if (ch->followers || ch->master)
		die_follower(ch);

	/* Forget snooping, if applicable */
	if (ch->desc)
	{
		if (ch->desc->snooping) 
			if(ch->desc->snooping->snoop_by)
			{
				ch->desc->snooping->snoop_by = nullptr;
				ch->desc->snooping = nullptr;
			}
    
			if (ch->desc->snoop_by)
			{
				SEND_TO_Q("Your victim is no longer among us.\r\n",
				ch->desc->snoop_by);
				ch->desc->snoop_by->snooping = nullptr;
				ch->desc->snoop_by = nullptr;
			}
	}
	  
	/* transfer objects to room, if any */
	while (ch->carrying) 
	{
		obj = ch->carrying;
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
	}

	/* transfer equipment to room, if any */
	for (i = 0; i < NUM_WEARS; i++)
		if (GET_EQ(ch, i))
			obj_to_room(unequip_char(ch, i), ch->in_room);

	if (FIGHTING(ch))
		stop_fighting(ch);

	for (k = combat_list; k; k = temp) 
	{
		temp = k->next_fighting;
    
		if (FIGHTING(k) == ch)
			stop_fighting(k);
	}

	/* Set greyman marked target to nullptr if we are extracting their target. */
	for(k = character_list;k;k = k->next)
	{
		if(GET_MARKED(k) == ch)
		{
			GET_MARKED(k) = nullptr;
		}
	}

	ch->save();

	room = ch->in_room;
	char_from_room(ch);

	/* pull the char from the list */
	REMOVE_FROM_LIST(ch, character_list, next);

	if (ch->desc && ch->desc->original)
		do_return(ch, nullptr, 0, 0);

	if (!IS_NPC(ch)) 
	{
		Crash_delete_crashfile(ch);

		sprintf(buf, "%s extracted at room %d.", GET_NAME(ch), GET_ROOM_VNUM(GET_LOADROOM(ch)));
		mudlog(buf, CMP, MAX(GET_INVIS_LEV(ch), LVL_IMMORT), TRUE);

	}
	
	else 
	{
		if (ch->mob_specials.load)
			ch->mob_specials.load->mob = nullptr;

		ch->mob_specials.load = nullptr;

		if (GET_MOB_RNUM(ch) > -1)		/* if mobile */
			mob_index[GET_MOB_RNUM(ch)].number--;
    
		clearMemory(ch);		/* Only NPC's can have memory */
    
		if (SCRIPT(ch))
			extract_script(SCRIPT(ch));
    
		if (SCRIPT_MEM(ch))
			extract_script_mem(SCRIPT_MEM(ch));
    
		free_char(ch);
		freed = 1;
	}

	if (!freed && ch->desc) 
		STATE(ch->desc) = CON_CLOSE;
	
	else 
	{  /* if a player gets purged from within the game */
		if (!freed)
			free_char(ch);
	}
}

void move_char_circle(char_data * ch)
{
	sh_int location;

	if(IS_HUMAN(ch))
		location = find_target_room(ch, "500");
	
	else
		location = find_target_room(ch, "502");
	
	char_from_room(ch);
	char_to_room(ch, location);
	GET_HIT(ch) = 1;
    GET_MOVE(ch) = 1;
    GET_MANA(ch) = 1;
	look_at_room(ch, 0);
	change_pos(ch, POS_RESTING);

	if(GET_LEVEL(ch) <= 5)
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_KIT);
    
	act("$n suddenly arrives from the midst of the air.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("\nYou feel completely drained of all of your strength...", ch);
}

/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


char_data *get_player_vis(char_data * ch, char *name, int inroom)
{
	char_data *i;

	for (i = character_list; i; i = i->next)
		if (!IS_NPC(i) && (!inroom || i->in_room == ch->in_room) &&
		!str_cmp(i->player.name, name) && CAN_SEE(ch, i))
			return i;

	return nullptr;
}

char_data *get_char_room_vis(char_data *ch, char *name)
{
	char_data *i;
	int j = 0, number, tar_race = -1;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = tmpname;
	int newb_targ = 0;

  
	if (!str_cmp(name, "self") || !str_cmp(name, "me"))
		return (ch);

	/* 0.<name> means PC with name */
	strcpy(tmp, name);
  
	if (!(number = get_number(&tmp)))
		return get_player_vis(ch, tmp, 1);

	if (!str_cmp(name, "human"))
		tar_race = RACE_HUMAN;
  
	else if (!str_cmp(name, "dark"))
		tar_race = RACE_TROLLOC;
  
	else if (!str_cmp(name, "trolloc"))
		tar_race = RACE_TROLLOC;
  
	else if (!str_cmp(name, "seanchan"))
		tar_race = RACE_SEANCHAN;
  
	else if (!str_cmp(name, "aiel"))
		tar_race = RACE_AIEL;
  
	else if (!str_cmp(name, "ogier"))
		tar_race = RACE_OGIER;
 
	else if (!str_cmp(name, "newbie"))
		newb_targ = 1;


	for (i = world[IN_ROOM(ch)].people; i && j <= number; i = i->next_in_room)
		if (isname(tmp, i->player.name) ||
			(newb_targ == 1 && GET_LEVEL(i) <= 15 && GET_RACE(i) != GET_RACE(ch) && !IS_NPC(i)) ||
			((tar_race == RACE_HUMAN && i->player.race == RACE_HUMAN && !isname(name, i->player.name)) ||
			(tar_race == RACE_TROLLOC && i->player.race == RACE_TROLLOC && !isname(name, i->player.name)) ||
			(tar_race == RACE_SEANCHAN && i->player.race == RACE_SEANCHAN && !isname(name, i->player.name)) ||
			(tar_race == RACE_AIEL && i->player.race == RACE_AIEL && !isname(name, i->player.name)) ||
			(tar_race == RACE_OGIER && i->player.race == RACE_OGIER && !isname(name, i->player.name))) && GET_LEVEL(ch) < LVL_IMMORT &&
			GET_LEVEL(i) <= LVL_IMMORT && GET_RACE(ch) != tar_race && !IS_NPC(i) && !PLR_FLAGGED(i, PLR_DARKFRIEND) &&
			GET_LEVEL(i) > 15)
				if (CAN_SEE(ch, i))
					if (++j == number)
						return i;
  
	return nullptr;
}

/* Search for matching name regardless of anything else. */

char_data *get_char_by_name(char *name, int npc)
{
	char_data *i;

	if(!name)
		return nullptr;

	for (i = character_list; i; i = i->next)
	{
		if (isname(name, i->player.name) && (npc || !IS_NPC(i)))
		{
			return i;
		}
	}

	return nullptr;
}

char_data *get_char_vis(char_data * ch, char *name)
{
	char_data *i;
	int j = 0, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = tmpname;

	/* check the room first */
	if ((i = get_char_room_vis(ch, name)) != nullptr)
		return i;

	strcpy(tmp, name);
	if (!(number = get_number(&tmp)))
		return get_player_vis(ch, tmp, 0);

	for (i = character_list; i && (j <= number); i = i->next)
		if (isname(tmp, i->player.name) && CAN_SEE(ch, i))
			if (++j == number)
				return i;

	return nullptr;
}



struct obj_data *get_obj_in_list_vis(char_data * ch, char *name,
				              struct obj_data * listy)
{
	struct obj_data *i;
	int j = 0, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = tmpname;

	strcpy(tmp, name);
  
	if (!(number = get_number(&tmp)))
		return nullptr;

  
	for (i = listy; i && (j <= number); i = i->next_content)
	{

		if(!i->name)
			continue;

		if (isname(tmp, i->name))
			if (++j == number)
				return i;
	}

	return nullptr;
}




/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(char_data * ch, char *name)
{
	struct obj_data *i;
	int j = 0, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp = tmpname;

	/* scan items carried */
	if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
		return i;

	/* scan room */
	if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
		return i;

	strcpy(tmp, name);
	if (!(number = get_number(&tmp)))
		return nullptr;

	/* ok.. no luck yet. scan the entire obj list   */
	for (i = object_list; i && (j <= number); i = i->next)
		if (i->name && isname(tmp, i->name))
			if (CAN_SEE_OBJ(ch, i))
				if (++j == number)
					return i;
  
	return nullptr;
}



struct obj_data *get_object_in_equip_vis(char_data * ch,
		           char *arg, struct obj_data * equipment[], int *j)
{
  for ((*j) = 0; (*j) < NUM_WEARS; (*j)++)
    if (equipment[(*j)])
	if (isname(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return nullptr;
}


char *money_desc(int amount)
{
  static char buf[128];

  if (amount <= 0) {
    log("SYSERR: Try to create negative or 0 money (%d).", amount);
    return nullptr;
  }
	if (amount == 1)
		strcpy(buf, "a gold coin");
	else if (amount <= 10)
		strcpy(buf, "a tiny pile of gold coins");
	else if (amount <= 20)
		strcpy(buf, "a handful of gold coins");
	else if (amount <= 75)
		strcpy(buf, "a little pile of gold coins");
	else if (amount <= 200)
		strcpy(buf, "a small pile of gold coins");
	else if (amount <= 1000)
		strcpy(buf, "a pile of gold coins");
	else if (amount <= 5000)
		strcpy(buf, "a big pile of gold coins");
	else if (amount <= 10000)
		strcpy(buf, "a large heap of gold coins");
	else if (amount <= 20000)
		strcpy(buf, "a huge mound of gold coins");
	else if (amount <= 75000)
		strcpy(buf, "an enormous mound of gold coins");
	else if (amount <= 150000)
		strcpy(buf, "a small mountain of gold coins");
	else if (amount <= 250000)
		strcpy(buf, "a mountain of gold coins");
	else if (amount <= 500000)
		strcpy(buf, "a huge mountain of gold coins");
	else if (amount <= 1000000)
		strcpy(buf, "an enormous mountain of gold coins");
	else
		strcpy(buf, "an absolutely colossal mountain of gold coins");

	return buf;
}


struct obj_data *create_money(int amount)
{
	struct obj_data *obj;
	struct extra_descr_data *new_descr;
	char buf[200];

	int y;

	if (amount <= 0)
	{
		log("SYSERR: Try to create negative or 0 money. (%d)", amount);
		return nullptr;
	}
  
	obj = create_obj();
	new_descr = new extra_descr_data[1];

	if (amount == 1)
	{
		obj->name = str_dup("coin gold");
		obj->short_description = str_dup("a gold coin");
		obj->description = str_dup("One miserable gold coin is lying here.");
		new_descr->keyword = str_dup("coin gold");
		new_descr->description = str_dup("It's just one miserable little gold coin.");
	} 
	
	else
	{
		obj->name = str_dup("coins gold");
		obj->short_description = str_dup(money_desc(amount));
		sprintf(buf, "%s is lying here.", money_desc(amount));
		obj->description = str_dup(CAP(buf));

		new_descr->keyword = str_dup("coins gold");
    
		if (amount < 10)
		{
			sprintf(buf, "There are %d coins.", amount);
			new_descr->description = str_dup(buf);
		} 
		
		else if (amount < 100)
		{
			sprintf(buf, "There are about %d coins.", 10 * (amount / 10));
			new_descr->description = str_dup(buf);
		} 
		
		else if (amount < 1000)
		{
			sprintf(buf, "It looks to be about %d coins.", 100 * (amount / 100));
			new_descr->description = str_dup(buf);
		} 
		
		else if (amount < 100000)
		{
			sprintf(buf, "You guess there are, maybe, %d coins.",
			1000 * ((amount / 1000) + number(0, (amount / 1000))));
			new_descr->description = str_dup(buf);
		} 
		
		else
			new_descr->description = str_dup("There are a LOT of coins.");
	}

	new_descr->next = nullptr;
	obj->ex_description = new_descr;

	GET_OBJ_TYPE(obj) = ITEM_MONEY;
  
	for(y = 0; y < TW_ARRAY_MAX; y++)
		obj->obj_flags.wear_flags[y] = 0;
  
	SET_BIT_AR(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE);
	GET_OBJ_VAL(obj, 0) = amount;
	GET_OBJ_COST(obj) = amount;
	obj->item_number = NOTHING;

	return obj;
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be nullptr if no character was found, otherwise points     */
/* **tar_obj Will be nullptr if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, char_data * ch,
		     char_data ** tar_ch, struct obj_data ** tar_obj)
{
	int i, found;
	char name[256];

	one_argument(arg, name);

	if (!*name)
		return (0);

	*tar_ch = nullptr;
	*tar_obj = nullptr;

	if (IS_SET(bitvector, FIND_CHAR_ROOM))
	{	/* Find person in room */
		if ((*tar_ch = get_char_room_vis(ch, name)))
		{
			 return (FIND_CHAR_ROOM);
		}
	}
  
	if (IS_SET(bitvector, FIND_CHAR_WORLD))
	{
		if ((*tar_ch = get_char_vis(ch, name)))
		{
			return (FIND_CHAR_WORLD);
		}
	}
  
	if (IS_SET(bitvector, FIND_OBJ_EQUIP))
	{
		for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
			if (GET_EQ(ch, i) && isname(name, GET_EQ(ch, i)->name))
			{
				*tar_obj = GET_EQ(ch, i);
				found = TRUE;
			}
    
			if (found)
			{
				return (FIND_OBJ_EQUIP);
			}
	}
  
	if (IS_SET(bitvector, FIND_OBJ_INV))
	{
		if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
		{
			return (FIND_OBJ_INV);
		}
	}
  
	if (IS_SET(bitvector, FIND_OBJ_ROOM))
	{
		if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
		{
			return (FIND_OBJ_ROOM);
		}
	}
  
	if (IS_SET(bitvector, FIND_OBJ_WORLD))
	{
		if ((*tar_obj = get_obj_vis(ch, name)))
		{
			return (FIND_OBJ_WORLD);
		}
	}
  
	return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
	
	if (!strcmp(arg, "all"))
		return FIND_ALL;
  
	else if (!strncmp(arg, "all.", 4))
	{
		strcpy(arg, arg + 4);
		return FIND_ALLDOT;
	} 
	
	else
		return FIND_INDIV;
}
