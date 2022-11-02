/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */



#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "db.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "olc.h"

extern struct wis_app_type wis_app[];
extern struct con_app_type con_app[];
extern int siteok_everyone;

/* local functions */
int parse_class(char arg);
long find_class_bitvector(char arg);
cbyte saving_throws(int class_num, int type, int level);

void roll_real_abils(char_data * ch);
void do_start(char_data * ch);
void roll_taveren(char_data *ch);
void check_autowiz(char_data *ch);
int backstab_add(int level);
int invalid_class(char_data *ch, struct obj_data *obj);
int level_exp(int level);
extern int max_human_moves;
extern int max_trolloc_moves;
const char *title_male(int race);
extern struct clan_data clan_list[50];

void remove_player_clan(char_data *ch, int number)
{

	struct clan_player_data *p, *temp;

	for(p = clan_list[number].players;p;p = p->next)
	{
		if(!str_cmp(p->name, GET_NAME(ch)))
		{
			REMOVE_FROM_LIST(p, clan_list[number].players, next);
		}
	}		
}

char *race_name(char_data *ch)
{
	
	char line[200];

	if(IS_DREADLORD(ch))
		return "dreadlord";

	else if(IS_FADE(ch))
		return "myrddraal";

	else if(IS_BLADEMASTER(ch))
		return "blademaster";

	else if(IS_HUMAN(ch))
		return "human";

	else if(IS_TROLLOC(ch))
		return "trolloc";

	else if(IS_SEANCHAN(ch))
		return "seanchan";

	else if(IS_AIEL(ch))
		return "aiel";

	else
	{
		return "Unknown";
		sprintf(line, "Player %s returning race %d.", GET_NAME(ch), GET_RACE(ch));
		log(line);
	}
		
}

int race_alias(char_data *ch, char *alias)
{

	if(!ch || !alias)
		return 0;

	if(GET_LEVEL(ch) >= LVL_IMMORT || IS_NPC(ch))
		return 0;

	if(IS_FADE(ch) && !str_cmp(alias, "dark") || !str_cmp(alias, "fade"))
		return 1;

	if(IS_DREADLORD(ch) && !str_cmp(alias, "dark") || !str_cmp(alias, "dreadlord"))
		return 1;

	if(IS_GREYMAN(ch) && !str_cmp(alias, "dark") || !str_cmp(alias, "greyman"))
		return 1;

	if(IS_TROLLOC(ch) && !IS_FADE(ch) && !IS_DREADLORD(ch) && !IS_GREYMAN(ch) &&
		(!str_cmp(alias, "dark") || !str_cmp(alias, "trolloc")))
		return 1;

	if(IS_HUMAN(ch) && !PLR_FLAGGED(ch, PLR_DARKFRIEND) && !str_cmp(alias, "human") && GET_LEVEL(ch) > 15)
		return 1;

	if(GET_LEVEL(ch) <= 15 && !str_cmp(alias, "newbie"))
		return 1;

	return 0;
}

void perform_stat_limit(char_data *ch)
{

		if(ch->real_abils.str > 19)
			ch->real_abils.str = 19;
		
		if(ch->real_abils.str < 11)
			ch->real_abils.str = 11;
		
		if(ch->real_abils.dex > 19)
			ch->real_abils.dex = 19;
		
		if(ch->real_abils.dex < 11)
			ch->real_abils.dex = 11;
		
		if(ch->real_abils.intel > 19)
			ch->real_abils.intel = 19;
		
		if(ch->real_abils.intel < 11)
			ch->real_abils.intel = 11;
		
		if(ch->real_abils.wis > 19)
			ch->real_abils.wis = 19;
		
		if(ch->real_abils.wis < 11)
			ch->real_abils.wis = 11;

		if(ch->real_abils.con > 19)
			ch->real_abils.con = 19;
		
		if(ch->real_abils.con < 11)
			ch->real_abils.con = 11;

	switch(GET_CLASS(ch))
	{
	
	case CLASS_WARRIOR:
	case CLASS_RANGER:
	case CLASS_THIEF:
		
		if(ch->real_abils.intel > 15)
			ch->real_abils.intel = 15;
		
		if(ch->real_abils.wis > 15)
			ch->real_abils.wis = 15;
		
		break;

	case CLASS_CHANNELER:
		
		if(ch->real_abils.str > 18 && GET_SEX(ch) != SEX_MALE)
			ch->real_abils.str = 18;

		if(ch->real_abils.con > 17)
			ch->real_abils.con = 17;

		if(ch->real_abils.dex > 18)
			ch->real_abils.dex = 18;

		break;
	}


	if(ch->real_abils.str >= 19 && ch->real_abils.con >= 19 && ch->real_abils.dex >= 19)
	{
		switch(number(1, 3))
		{

		case 1:
			--ch->real_abils.str;
			break;

		case 2:
			--ch->real_abils.dex;
			break;

		case 3:
			--ch->real_abils.con;
			break;
		}
	}

	for(int i = 1;i < 3;i++)
	{
		if(number(1,3))
		{
			switch(number(1,3))
			{
			case 1:
				if(ch->real_abils.str > 17)
				{
					--ch->real_abils.str;
				}
				
				break;

			case 2:
				
				if(ch->real_abils.dex > 17)
				{
					--ch->real_abils.dex;
				}

				break;

			case 3:

				if(ch->real_abils.con > 17)
				{
					--ch->real_abils.con;
				}

				break;
			}
		}
	}

}

const char *race_abbrevs[] = {
	"Hum",
	"Tro",
	"Aie",
	"\n"
};

const char *pc_race_types[] = {
	"human",
	"trolloc",
	"aiel",
	"animal",
	"other",
	"\n"
};

/* The menu for choosing a race in interpreter.c: */
const char *race_menu =
"\r\n"
"Select a race:\r\n"
"  [H]uman\r\n"
"  [T]rolloc\r\n"
// "  [A]iel\r\n"
;


void set_height_by_race(char_data *ch)
{
	if (GET_SEX(ch) == SEX_MALE)
	{
		if (IS_TROLLOC(ch)) 
			GET_HEIGHT(ch) = 80 + dice(6, 18);
		else /* if (IS_HUMAN(ch)) */
			GET_HEIGHT(ch) = 60 + dice(2, 10);
	} else /* if (IS_FEMALE(ch)) */ {
		if (IS_TROLLOC(ch)) 
			GET_HEIGHT(ch) = 70 + dice(4, 15);
		else /* if (IS_HUMAN(ch)) */
			GET_HEIGHT(ch) = 59 + dice(2, 10);
	}

	return;
}


void set_weight_by_race(char_data *ch)
{
	if (GET_SEX(ch) == SEX_MALE)
	{
		if (IS_TROLLOC(ch)) 
			GET_WEIGHT(ch) = 160 + dice(20, 60);
		else /* if (IS_HUMAN(ch)) */
			GET_WEIGHT(ch) = 140 + dice(1, 60);
}
	else /* if (IS_FEMALE(ch)) */ {
		if (IS_TROLLOC(ch)) 
			GET_WEIGHT(ch) = 185 + dice(20, 75);
		else /* if (IS_HUMAN(ch)) */
			GET_WEIGHT(ch) = 100 + dice(6, 10);
	}

	return;
}

int parse_race(char arg)
{
	arg = LOWER(arg);

	switch (arg) {
		case 'h': return RACE_HUMAN;
		case 't': return RACE_TROLLOC;
		case 's': return RACE_SEANCHAN;
		case 'a': return RACE_AIEL;
		case 'w': return RACE_ANIMAL;
		case 'o': return RACE_OTHER;
		default:
			return RACE_UNDEFINED;
	}
}



/////////////Classes///////////////

const char *class_abbrevs[] = {
  "Wa",
  "Th",
  "Ra",
  "Ch",
  "My",
  "Dr",
  "Bm",
  "Gr",
  "Dk",
  "\n"
};

const char *pc_class_types[] =
{
	"warrior",
	"thief",
	"ranger",
	"channeler",
	"myrddraal",
	"dreadlord",
	"blademaster",
	"greyman",
	"draghkar",
	"\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *human_class_menu =
"\r\n"
"Select a class:\r\n"
"  [W]arrior\r\n"
"  [T]hief\r\n"
"  [R]anger\r\n"
"  [C]hanneler\r\n";

const char *other_class_menu =
"\r\n"
"Select a class:\r\n"
"  [W]arrior\r\n"
"  [T]hief\r\n"
"  [R]anger\r\n";


/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg)
{
	arg = LOWER(arg);

	switch (arg) {
	case 'w':
		return CLASS_WARRIOR;
	
	case 't':
		return CLASS_THIEF;
	
	case 'r':
		return CLASS_RANGER;
	
	case 'c':
		return CLASS_CHANNELER;
	
	case 'f':
		return CLASS_FADE;
	
	case 'd':
		return CLASS_DREADLORD;

	case 'b':
		return CLASS_BLADEMASTER;

	case 'g':
		return CLASS_GREYMAN;


	default:
		return CLASS_UNDEFINED;
	}
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
	arg = LOWER(arg); 

	switch (arg)
	{
		case 'w':
			return (1 << CLASS_WARRIOR);

		case 't':
			return (1 << CLASS_THIEF);

		case 'r':
			return (1 << CLASS_RANGER);

		case 'c':
			return (1 << CLASS_CHANNELER);

		default: 
			return 0;
	}
}


/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL	0
#define SKILL	1

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell' or 'skill'?	*/

int prac_params[4][NUM_CLASSES] =
{
	/* MAG	CLE	THE	WAR */
	{99,		99,	99,	99, 99, 99},		/* learned level */
	{14,		14,	14,	14, 15, 15},		/* max per prac */
	{11,		11,	9,	9, 9, 9},		/* min per pac */
	{SPELL,	SPELL,	SKILL,	SKILL, SKILL, SPELL}		/* prac name */
};

int int_apply(int intel)
{

	if(intel < 9)
		return 0;

	if(intel < 13)
		return 1;

	if(intel < 17)
		return 2;

	return 3;
}


int prac_add(char_data *ch, int percent, int skill)
{


	int add = 0;

	if(percent >= 99)
		return 0;

	// Is this a non spell? //
	if(skill > MAX_SPELLS)
	{
		
		if(percent < 45)
			add = 15;

		else if(percent < 64)
			add = 10;

		else if(percent < 74)
			add = 9;

		else if(percent < 80)
			add = 8;

		else if(percent < 92)
			add = 3;

		else if(percent < 99)
			add = 1;

		else
			add = 0;

		if(percent < 50)
			add += int_apply(GET_INT(ch));

		else if(percent < 75)
			add += int_apply(GET_INT(ch)) / 2;

	}

	else
	{
		add = 15;

		if(GET_WIS(ch) < 12)
			add += 0;

		else if(GET_WIS(ch) < 16)
			add += 1;

		else
			add += 2;
	}

	return add;
}


void roll_taveren(char_data *ch)
{
	int rolla;
	int rollb;

	rolla = number(1, 20);

	if(rolla > 18)
	{
		rollb = number(1, 10);

		if(rollb > 6)
			GET_TAVEREN(ch) = 2;
	
		else
			GET_TAVEREN(ch) = 3;
	}
	else
		GET_TAVEREN(ch) = 1;

	return;
}

cbyte saving_throws(int class_num, int type, int level)
{
	switch (class_num) {
	case CLASS_CHANNELER:

		switch (type) {
		case SAVING_PARA:	/* Paralyzation */
      switch (level) {
	case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 67;
      case  5: return 66;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 54;
      case 14: return 53;
      case 15: return 53;
      case 16: return 52;
      case 17: return 51;
      case 18: return 50;
      case 19: return 48;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 42;
      case 24: return 40;
      case 25: return 38;
      case 26: return 36;
      case 27: return 34;
      case 28: return 32;
      case 29: return 30;
      case 30: return 28;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage paralyzation saving throw.");
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 55;
      case  2: return 53;
      case  3: return 51;
      case  4: return 49;
      case  5: return 47;
      case  6: return 45;
      case  7: return 43;
      case  8: return 41;
      case  9: return 40;
      case 10: return 39;
      case 11: return 37;
      case 12: return 35;
      case 13: return 33;
      case 14: return 31;
      case 15: return 30;
      case 16: return 29;
      case 17: return 27;
      case 18: return 25;
      case 19: return 23;
      case 20: return 21;
      case 21: return 20;
      case 22: return 19;
      case 23: return 17;
      case 24: return 15;
      case 25: return 14;
      case 26: return 13;
      case 27: return 12;
      case 28: return 11;
      case 29: return 10;
      case 30: return  9;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage rod saving throw.");
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 63;
      case  3: return 61;
      case  4: return 59;
      case  5: return 57;
      case  6: return 55;
      case  7: return 53;
      case  8: return 51;
      case  9: return 50;
      case 10: return 49;
      case 11: return 47;
      case 12: return 45;
      case 13: return 43;
      case 14: return 41;
      case 15: return 40;
      case 16: return 39;
      case 17: return 37;
      case 18: return 35;
      case 19: return 33;
      case 20: return 31;
      case 21: return 30;
      case 22: return 29;
      case 23: return 27;
      case 24: return 25;
      case 25: return 23;
      case 26: return 21;
      case 27: return 19;
      case 28: return 17;
      case 29: return 15;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage petrification saving throw.");
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 60;
      case 10: return 59;
      case 11: return 57;
      case 12: return 55;
      case 13: return 53;
      case 14: return 51;
      case 15: return 50;
      case 16: return 49;
      case 17: return 47;
      case 18: return 45;
      case 19: return 43;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 37;
      case 24: return 35;
      case 25: return 33;
      case 26: return 31;
      case 27: return 29;
      case 28: return 27;
      case 29: return 25;
      case 30: return 23;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage breath saving throw.");
      }
    case SAVING_SPELL:	/* Generic spells */
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 58;
      case  3: return 56;
      case  4: return 54;
      case  5: return 52;
      case  6: return 50;
      case  7: return 48;
      case  8: return 46;
      case  9: return 45;
      case 10: return 44;
      case 11: return 42;
      case 12: return 40;
      case 13: return 38;
      case 14: return 36;
      case 15: return 35;
      case 16: return 34;
      case 17: return 32;
      case 18: return 30;
      case 19: return 28;
      case 20: return 26;
      case 21: return 25;
      case 22: return 24;
      case 23: return 22;
      case 24: return 20;
      case 25: return 18;
      case 26: return 16;
      case 27: return 14;
      case 28: return 12;
      case 29: return 10;
      case 30: return  8;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for mage spell saving throw.");
      }
    default:
      log("SYSERR: Invalid saving throw type.");
    }
    break;
  case CLASS_RANGER:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 48;
      case  4: return 46;
      case  5: return 45;
      case  6: return 43;
      case  7: return 40;
      case  8: return 37;
      case  9: return 35;
      case 10: return 34;
      case 11: return 33;
      case 12: return 31;
      case 13: return 30;
      case 14: return 29;
      case 15: return 27;
      case 16: return 26;
      case 17: return 25;
      case 18: return 24;
      case 19: return 23;
      case 20: return 22;
      case 21: return 21;
      case 22: return 20;
      case 23: return 18;
      case 24: return 15;
      case 25: return 14;
      case 26: return 12;
      case 27: return 10;
      case 28: return  9;
      case 29: return  8;
      case 30: return  7;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for ranger paralyzation saving throw.");
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 69;
      case  3: return 68;
      case  4: return 66;
      case  5: return 65;
      case  6: return 63;
      case  7: return 60;
      case  8: return 57;
      case  9: return 55;
      case 10: return 54;
      case 11: return 53;
      case 12: return 51;
      case 13: return 50;
      case 14: return 49;
      case 15: return 47;
      case 16: return 46;
      case 17: return 45;
      case 18: return 44;
      case 19: return 43;
      case 20: return 42;
      case 21: return 41;
      case 22: return 40;
      case 23: return 38;
      case 24: return 35;
      case 25: return 34;
      case 26: return 32;
      case 27: return 30;
      case 28: return 29;
      case 29: return 28;
      case 30: return 27;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for ranger rod saving throw.");
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 61;
      case  5: return 60;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 50;
      case 10: return 49;
      case 11: return 48;
      case 12: return 46;
      case 13: return 45;
      case 14: return 44;
      case 15: return 43;
      case 16: return 41;
      case 17: return 40;
      case 18: return 39;
      case 19: return 38;
      case 20: return 37;
      case 21: return 36;
      case 22: return 35;
      case 23: return 33;
      case 24: return 31;
      case 25: return 29;
      case 26: return 27;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for ranger petrification saving throw.");
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 76;
      case  5: return 75;
      case  6: return 73;
      case  7: return 70;
      case  8: return 67;
      case  9: return 65;
      case 10: return 64;
      case 11: return 63;
      case 12: return 61;
      case 13: return 60;
      case 14: return 59;
      case 15: return 57;
      case 16: return 56;
      case 17: return 55;
      case 18: return 54;
      case 19: return 53;
      case 20: return 52;
      case 21: return 51;
      case 22: return 50;
      case 23: return 48;
      case 24: return 45;
      case 25: return 44;
      case 26: return 42;
      case 27: return 40;
      case 28: return 39;
      case 29: return 38;
      case 30: return 37;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for ranger breath saving throw.");
      }
    case SAVING_SPELL:	/* Generic spells */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 74;
      case  3: return 73;
      case  4: return 71;
      case  5: return 70;
      case  6: return 68;
      case  7: return 65;
      case  8: return 63;
      case  9: return 60;
      case 10: return 59;
      case 11: return 58;
      case 12: return 56;
      case 13: return 55;
      case 14: return 54;
      case 15: return 53;
      case 16: return 51;
      case 17: return 50;
      case 18: return 49;
      case 19: return 48;
      case 20: return 47;
      case 21: return 46;
      case 22: return 45;
      case 23: return 43;
      case 24: return 41;
      case 25: return 39;
      case 26: return 37;
      case 27: return 35;
      case 28: return 34;
      case 29: return 33;
      case 30: return 32;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for ranger spell saving throw.");
      }
    default:
      log("SYSERR: Invalid saving throw type.");
    }
    break;
  case CLASS_THIEF:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 65;
      case  2: return 64;
      case  3: return 63;
      case  4: return 62;
      case  5: return 61;
      case  6: return 60;
      case  7: return 59;
      case  8: return 58;
      case  9: return 57;
      case 10: return 56;
      case 11: return 55;
      case 12: return 54;
      case 13: return 53;
      case 14: return 52;
      case 15: return 51;
      case 16: return 50;
      case 17: return 49;
      case 18: return 48;
      case 19: return 47;
      case 20: return 46;
      case 21: return 45;
      case 22: return 44;
      case 23: return 43;
      case 24: return 42;
      case 25: return 41;
      case 26: return 40;
      case 27: return 39;
      case 28: return 38;
      case 29: return 37;
      case 30: return 36;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief paralyzation saving throw.");
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 66;
      case  4: return 64;
      case  5: return 62;
      case  6: return 60;
      case  7: return 58;
      case  8: return 56;
      case  9: return 54;
      case 10: return 52;
      case 11: return 50;
      case 12: return 48;
      case 13: return 46;
      case 14: return 44;
      case 15: return 42;
      case 16: return 40;
      case 17: return 38;
      case 18: return 36;
      case 19: return 34;
      case 20: return 32;
      case 21: return 30;
      case 22: return 28;
      case 23: return 26;
      case 24: return 24;
      case 25: return 22;
      case 26: return 20;
      case 27: return 18;
      case 28: return 16;
      case 29: return 14;
      case 30: return 13;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief rod saving throw.");
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 60;
      case  2: return 59;
      case  3: return 58;
      case  4: return 58;
      case  5: return 56;
      case  6: return 55;
      case  7: return 54;
      case  8: return 53;
      case  9: return 52;
      case 10: return 51;
      case 11: return 50;
      case 12: return 49;
      case 13: return 48;
      case 14: return 47;
      case 15: return 46;
      case 16: return 45;
      case 17: return 44;
      case 18: return 43;
      case 19: return 42;
      case 20: return 41;
      case 21: return 40;
      case 22: return 39;
      case 23: return 38;
      case 24: return 37;
      case 25: return 36;
      case 26: return 35;
      case 27: return 34;
      case 28: return 33;
      case 29: return 32;
      case 30: return 31;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief petrification saving throw.");
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 79;
      case  3: return 78;
      case  4: return 77;
      case  5: return 76;
      case  6: return 75;
      case  7: return 74;
      case  8: return 73;
      case  9: return 72;
      case 10: return 71;
      case 11: return 70;
      case 12: return 69;
      case 13: return 68;
      case 14: return 67;
      case 15: return 66;
      case 16: return 65;
      case 17: return 64;
      case 18: return 63;
      case 19: return 62;
      case 20: return 61;
      case 21: return 60;
      case 22: return 59;
      case 23: return 58;
      case 24: return 57;
      case 25: return 56;
      case 26: return 55;
      case 27: return 54;
      case 28: return 53;
      case 29: return 52;
      case 30: return 51;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief breath saving throw.");
      }
    case SAVING_SPELL:	/* Generic spells */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 71;
      case  4: return 69;
      case  5: return 67;
      case  6: return 65;
      case  7: return 63;
      case  8: return 61;
      case  9: return 59;
      case 10: return 57;
      case 11: return 55;
      case 12: return 53;
      case 13: return 51;
      case 14: return 49;
      case 15: return 47;
      case 16: return 45;
      case 17: return 43;
      case 18: return 41;
      case 19: return 39;
      case 20: return 37;
      case 21: return 35;
      case 22: return 33;
      case 23: return 31;
      case 24: return 29;
      case 25: return 27;
      case 26: return 25;
      case 27: return 23;
      case 28: return 21;
      case 29: return 19;
      case 30: return 17;
      case 31: return  0;
      case 32: return  0;
      case 33: return  0;
      case 34: return  0;
      case 35: return  0;
      case 36: return  0;
      case 37: return  0;
      case 38: return  0;
      case 39: return  0;
      case 40: return  0;
      default:
	log("SYSERR: Missing level for thief spell saving throw.");
      }
    default:
      log("SYSERR: Invalid saving throw type.");
    }
    break;
  case CLASS_WARRIOR:
    switch (type) {
    case SAVING_PARA:	/* Paralyzation */
      switch (level) {
      case  0: return 90;
      case  1: return 70;
      case  2: return 68;
      case  3: return 67;
      case  4: return 65;
      case  5: return 62;
      case  6: return 58;
      case  7: return 55;
      case  8: return 53;
      case  9: return 52;
      case 10: return 50;
      case 11: return 47;
      case 12: return 43;
      case 13: return 40;
      case 14: return 38;
      case 15: return 37;
      case 16: return 35;
      case 17: return 32;
      case 18: return 28;
      case 19: return 25;
      case 20: return 24;
      case 21: return 23;
      case 22: return 22;
      case 23: return 20;
      case 24: return 19;
      case 25: return 17;
      case 26: return 16;
      case 27: return 15;
      case 28: return 14;
      case 29: return 13;
      case 30: return 12;
      case 31: return 11;
      case 32: return 10;
      case 33: return  9;
      case 34: return  8;
      case 35: return  7;
      case 36: return  6;
      case 37: return  5;
      case 38: return  4;
      case 39: return  3;
      case 40: return  2;
      default:
	log("SYSERR: Missing level for warrior paralyzation saving throw.");
      }
    case SAVING_ROD:	/* Rods */
      switch (level) {
      case  0: return 90;
      case  1: return 80;
      case  2: return 78;
      case  3: return 77;
      case  4: return 75;
      case  5: return 72;
      case  6: return 68;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 57;
      case 12: return 53;
      case 13: return 50;
      case 14: return 48;
      case 15: return 47;
      case 16: return 45;
      case 17: return 42;
      case 18: return 38;
      case 19: return 35;
      case 20: return 34;
      case 21: return 33;
      case 22: return 32;
      case 23: return 30;
      case 24: return 29;
      case 25: return 27;
      case 26: return 26;
      case 27: return 25;
      case 28: return 24;
      case 29: return 23;
      case 30: return 22;
      case 31: return 20;
      case 32: return 18;
      case 33: return 16;
      case 34: return 14;
      case 35: return 12;
      case 36: return 10;
      case 37: return  8;
      case 38: return  6;
      case 39: return  5;
      case 40: return  4;
      default:
	log("SYSERR: Missing level for warrior rod saving throw.");
      }
    case SAVING_PETRI:	/* Petrification */
      switch (level) {
      case  0: return 90;
      case  1: return 75;
      case  2: return 73;
      case  3: return 72;
      case  4: return 70;
      case  5: return 67;
      case  6: return 63;
      case  7: return 60;
      case  8: return 58;
      case  9: return 57;
      case 10: return 55;
      case 11: return 52;
      case 12: return 48;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      default:
	log("SYSERR: Missing level for warrior petrification saving throw.");
      }
    case SAVING_BREATH:	/* Breath weapons */
      switch (level) {
      case  0: return 90;
      case  1: return 85;
      case  2: return 83;
      case  3: return 82;
      case  4: return 80;
      case  5: return 75;
      case  6: return 70;
      case  7: return 65;
      case  8: return 63;
      case  9: return 62;
      case 10: return 60;
      case 11: return 55;
      case 12: return 50;
      case 13: return 45;
      case 14: return 43;
      case 15: return 42;
      case 16: return 40;
      case 17: return 37;
      case 18: return 33;
      case 19: return 30;
      case 20: return 29;
      case 21: return 28;
      case 22: return 26;
      case 23: return 25;
      case 24: return 24;
      case 25: return 23;
      case 26: return 21;
      case 27: return 20;
      case 28: return 19;
      case 29: return 18;
      case 30: return 17;
      case 31: return 16;
      case 32: return 15;
      case 33: return 14;
      case 34: return 13;
      case 35: return 12;
      case 36: return 11;
      case 37: return 10;
      case 38: return  9;
      case 39: return  8;
      case 40: return  7;
      default:
	log("SYSERR: Missing level for warrior breath saving throw.");
      }
    case SAVING_SPELL:	/* Generic spells */
      switch (level) {
      case  0: return 90;
      case  1: return 85;
      case  2: return 83;
      case  3: return 82;
      case  4: return 80;
      case  5: return 77;
      case  6: return 73;
      case  7: return 70;
      case  8: return 68;
      case  9: return 67;
      case 10: return 65;
      case 11: return 62;
      case 12: return 58;
      case 13: return 55;
      case 14: return 53;
      case 15: return 52;
      case 16: return 50;
      case 17: return 47;
      case 18: return 43;
      case 19: return 40;
      case 20: return 39;
      case 21: return 38;
      case 22: return 36;
      case 23: return 35;
      case 24: return 34;
      case 25: return 33;
      case 26: return 31;
      case 27: return 30;
      case 28: return 29;
      case 29: return 28;
      case 30: return 27;
      case 31: return 25;
      case 32: return 23;
      case 33: return 21;
      case 34: return 19;
      case 35: return 17;
      case 36: return 15;
      case 37: return 13;
      case 38: return 11;
      case 39: return  9;
      case 40: return  7;
      default:
	log("SYSERR: Missing level for warrior spell saving throw.");
      }
    default:
      log("SYSERR: Invalid saving throw type.");
    }
  default:
    log("SYSERR: Invalid class saving throw.");
  }

  /* Should not get here unless something is wrong. */
  return 100;
}


/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 */

void roll_real_abils(char_data * ch)
{
	int i;
	
	int roll[6];

	for(i = 0;i < 6;i++)
	{
		roll[i] = number(11, 19);

	}
	ch->real_abils.intel	= roll[0] + number(0, 1);
	ch->real_abils.wis		= roll[1];
	ch->real_abils.dex		= roll[2];
	ch->real_abils.str		= roll[3] + number(0, 1);
	ch->real_abils.con		= roll[4] + number(0, 1);
	ch->real_abils.cha		= roll[5];


	switch(GET_CLASS(ch))
	{

	case CLASS_RANGER:
		ch->real_abils.con += 1;
		break;

	case CLASS_WARRIOR:
		ch->real_abils.str += 1;
		break;

	case CLASS_THIEF:
		ch->real_abils.dex += 1;
		break;

	case CLASS_CHANNELER:
		ch->real_abils.con -= 1;
		ch->real_abils.str -= 1;
		ch->real_abils.wis += 1;

		ch->real_abils.con -= number(0, 1);
		ch->real_abils.str -= number(0, 1);
		ch->real_abils.dex -= number(0, 1);

		break;
	}

	if(!IS_CHANNELER(ch))
		ch->real_abils.wis -= 3;

	perform_stat_limit(ch);
	ch->aff_abils = ch->real_abils;
}

/* Some initializations for characters, including initial skills */
void do_start(char_data * ch)
{

	GET_LEVEL(ch) = 1;
	GET_EXP(ch) = 1;
	GET_SICKNESS(ch) = 0;

	set_title(ch, "");
	ch->points.max_hit = 10;

	advance_level(ch, TRUE);

	GET_MAX_SHADOW(ch) = number(100, 160);
	GET_SHADOW(ch) = GET_MAX_SHADOW(ch);

	roll_moves(ch);

	GET_HIT(ch) = GET_MAX_HIT(ch);
	GET_MANA(ch) = GET_MAX_MANA(ch);
	GET_MOVE(ch) = GET_MAX_MOVE(ch);

	GET_COND(ch, THIRST) = 24;
	GET_COND(ch, FULL) = 24;
	GET_COND(ch, DRUNK) = 0;

	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	if (siteok_everyone)
		SET_BIT_AR(PLR_FLAGS(ch), PLR_SITEOK);
}

int char_data::rollHealth()
{
	int num = 0;

	if(IS_WARRIOR(this) && GET_LEVEL(this) <= 30)
		++num;

	if(GET_LEVEL(this) == 1)
	{
		num += number(20, 30);
		num += GET_CON(this) / 2;
	}
	else if(GET_LEVEL(this) <= 30)
		num += number(GET_CON(this) - 8, GET_CON(this) - 6);
	else
		num += number(1, 2);

	return num;
}

int char_data::rollMana()
{
	if (GET_LEVEL(this) > 1 && GET_LEVEL(this) <= 30 && IS_CHANNELER(this))
		return GET_WIS(this) / 4;
	
	return 0;
}

/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(char_data * ch, int show)
{
	int i;

	GET_MAX_HIT(ch) += ch->rollHealth();
	GET_MAX_MANA(ch) += ch->rollMana();

	if(GET_LEVEL(ch) <= 30)
	{
		if(IS_TROLLOC(ch))
			GET_PRACTICES(ch) += 4;
		else
			GET_PRACTICES(ch) += 5;

		GET_SPRACTICES(ch) += 2;
	}
	else
	{
		GET_PRACTICES(ch) += 1;		
		GET_SPRACTICES(ch) += 1;
	}

	if (GET_LEVEL(ch) >= LVL_IMMORT)
	{
		for (i = 0; i < 3; i++)
			GET_COND(ch, i) = (char) -1;
    
		SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
	}

	if(GET_LEVEL(ch) == 5 && !PRF_FLAGGED(ch, PRF_STATTED))
	{
		SET_BIT_AR(PRF_FLAGS(ch), PRF_STATTED); 
		roll_real_abils(ch);

		//Re-roll them once statted
		GET_MAX_HIT(ch) = 0;
		for(GET_LEVEL(ch) = 1;GET_LEVEL(ch) < 5;++GET_LEVEL(ch))
		{
			GET_MAX_HIT(ch) += ch->rollHealth();
			GET_MAX_MANA(ch) += ch->rollMana();
		}
	}

	ch->save();
	check_autowiz(ch);

	if(show)
	{
		sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
		mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
	}
}

void delevel(char_data * ch)
{

	GET_MAX_HIT(ch) -= ch->rollHealth();
	GET_MAX_MANA(ch) -= ch->rollMana();

	if(GET_LEVEL(ch) < 30)
	{

		if(IS_TROLLOC(ch))
			GET_PRACTICES(ch) -= 3;

		else
			GET_PRACTICES(ch) -= 4;
		
		GET_SPRACTICES(ch) -= 2;
	}

	else 
	{
		GET_SPRACTICES(ch) -= 1;
		GET_PRACTICES(ch) -= 1;
	}

	GET_LEVEL(ch) -= 1;
	ch->save();
	check_autowiz(ch);

}
/*
 * This adds a bonus to stab damage, a random from 1 to
 * the stabber's level.
 */
int backstab_add(int level)
{
	if (level <= 0)
		return 1;	  /* level 0 */
	
	else
		return number(1, level);
}

/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(char_data *ch, struct obj_data *obj) {
	if ((IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && IS_CHANNELER(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_RANGER(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)))
		return 1;
  
	else
		return 0;
}




/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void init_spell_levels(void)
{

	spell_level(SPELL_CHILL_TOUCH, CLASS_CHANNELER, 3);
	spell_level(SPELL_AGILITY, CLASS_CHANNELER, 4);
	spell_level(SPELL_LOCATE_OBJECT, CLASS_CHANNELER, 6);
	spell_level(SPELL_STRENGTH, CLASS_CHANNELER, 6);
	spell_level(SPELL_SLEEP, CLASS_CHANNELER, 8);
	spell_level(SPELL_BLINDNESS, CLASS_CHANNELER, 9);
	spell_level(SPELL_POISON, CLASS_CHANNELER, 14);
	spell_level(SPELL_FIREBALL, CLASS_CHANNELER, 15);
	spell_level(SPELL_ENCHANT_WEAPON, CLASS_CHANNELER, 26);


	spell_level(SKILL_PARRY, CLASS_RANGER, 1);
	spell_level(SKILL_NOTICE, CLASS_RANGER, 1);

	/* THIEVES */
	spell_level(SKILL_SNEAK, CLASS_THIEF, 1);
	spell_level(SKILL_PICK_LOCK, CLASS_THIEF, 1);
	spell_level(SKILL_BACKSTAB, CLASS_THIEF, 1);
	spell_level(SKILL_STEAL, CLASS_THIEF, 1);
	spell_level(SKILL_HIDE, CLASS_THIEF, 1);
	spell_level(SKILL_TRACK, CLASS_THIEF, 1);
	spell_level(SKILL_ATTACK, CLASS_THIEF, 1);
	spell_level(SKILL_DODGE, CLASS_THIEF, 1);

	/* WARRIORS */
	spell_level(SKILL_KICK, CLASS_WARRIOR, 1);
	spell_level(SKILL_RESCUE, CLASS_WARRIOR, 1);
	spell_level(SKILL_TRACK, CLASS_WARRIOR, 1);
	spell_level(SKILL_BASH, CLASS_WARRIOR, 1);
}


/*
 * This is the exp given to implementors -- it must always be greater
 * than the exp required for immortality, plus at least 20,000 or so.
 */
#define EXP_MAX  10000000

/* Function to return the exp required for each class/level */
int level_exp(int level)
{
	if (level > LVL_IMPL || level < 0)
	{
		log("SYSERR: Requesting exp for invalid level!");
		return 0;
	}

  /*
   * Gods have exp close to EXP_MAX.  This statement should never have to
   * changed, regardless of how many mortal or immortal levels exist.
   */
	if (level > LVL_IMMORT)
		return EXP_MAX - ((LVL_IMPL - level) * 1000);

  /* Exp required for normal mortals is below */

	switch (level) {
		case  0: return 0;
		case  1: return 1;
		case  2: return 2500;
		case  3: return 5000;
		case  4: return 10000;
		case  5: return 20000;
		case  6: return 40000;
		case  7: return 60000;
		case  8: return 90000;
		case  9: return 135000;
		case 10: return 250000;
		case 11: return 375000;
		case 12: return 750000;
		case 13: return 1125000;
		case 14: return 1500000;
		case 15: return 1875000;
		case 16: return 2250000;
		case 17: return 2625000;
		case 18: return 3000000;
		case 19: return 3375000;
		case 20: return 3750000;
		case 21: return 4000000;
		case 22: return 4300000;
		case 23: return 4600000;
		case 24: return 4900000;
		case 25: return 5200000;
		case 26: return 5500000;
		case 27: return 5950000;
		case 28: return 6400000;
		case 29: return 6850000;
		case 30: return 8000000;
		case 31: return 10000000;
		case 32: return 12000000;
		case 33: return 14000000;
		case 34: return 16000000;
		case 35: return 18000000;
		case 36: return 21000000;
		case 37: return 24000000;
		case 38: return 27000000;
		case 39: return 30000000;
		case 40: return 34000000;
		case 41: return 37000000;
		case 42: return 41000000;
		case 43: return 45000000;
		case 44: return 50000000;
		case 45: return 55000000;
		case 46: return 60000000;
		case 47: return 65000000;
		case 48: return 70000000;
		case 49: return 75000000;
		case 50: return 85000000;
		case 51: return 90000000;
		case 52: return 91000000;
		case 53: return 92000000;
		case 54: return 93000000;
		case 55: return 94000000;
		case 56: return 95000000;
		case 57: return 96000000;
		case 58: return 97000000;
		case 59: return 98000000;
		case 60: return 99000000;
		case 99: return -1;
		case LVL_IMMORT: return 37500000;
	}

  /*
   * This statement should never be reached if the exp tables in this function
   * are set up properly.  If you see exp of 123456 then the tables above are
   * incomplete -- so, complete them!
   */
	log("SYSERR: XP tables not set up correctly in class.c!");
	return 123456;
}


/* 
 * Default titles of male characters.
 */
const char *title_male(int race)
{

	switch (race) {

		case RACE_HUMAN:
			return "the human";
			break;

		case RACE_TROLLOC:
			return "the trolloc";
			break;
	}

	return nullptr;
}

/*	Return a pointer to a player's clan data if they are in clan for "clan_num"
 *	otherwise, return null
 */
player_clan_data *char_data::getClan(int clan_num)
{
	player_clan_data *cl;

	for(cl = this->clans;cl;cl = cl->next)
	{
		if(cl->clan == clan_num)
			return cl;
	}
	return nullptr;
}

/* Search out a clan by a givin name, using strn_cmp() */
int clanByName(char *name)
{
	int i = 0;

	if(!name)
		return 0;

	for(i = 0;i < NUM_CLANS;++i)
	{
		if(!strn_cmp(name, clan_list[i].name, strlen(name)))
			return i;
	}

	return 0;
}

/* If this person is in an Aes Sedai clan, return true */
bool char_data::AES_SEDAI()
{
	if(this->getClan(17) || this->getClan(18) || this->getClan(19) || this->getClan(20) || this->getClan(21) ||
	this->getClan(22) || this->getClan(23))
		return true;

	return false;
}

/* Returns true if the person is in a tower clan other than Gaidin */
bool char_data::TOWER_MEMBER()
{
	if(this->TOWER_MEMBER() || this->getClan(17))
		return true;

	return false;
}

/* This checks to see if this is wanted by ch's clan */
bool char_data::wantedByPlayer(char_data *ch)
{
	player_clan_data *cl;

	for(cl = this->clans;cl;cl = cl->next)
	{
		if(IS_SET_AR(GET_WARRANTS(ch), cl->clan))
			return true;
	}

	return false;
}

/* Returns the total questpoints from every clan that a person has earned */
int char_data::totalQP()
{
	int total;
	player_clan_data *cl;

	for(total = 0, cl = this->clans;cl;cl = cl->next)
	{
		total += cl->quest_points;
	}

	return total;
}

/* This just returns the first clan that a player shares with another player */
int sharedClan(char_data *p1, char_data *p2)
{
	int i;

	for(i = 0;i < NUM_CLANS;++i)
	{
		if(p1->getClan(i) && p2->getClan(i))
			return i;
	}
	return 0;
}
