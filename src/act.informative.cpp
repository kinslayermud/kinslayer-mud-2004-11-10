/**************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
***************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include <time.h>

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "constants.h"
#include "dg_scripts.h"
#include "olc.h"

/* extern variables */
extern int top_of_helpt;
extern class wizlist_data *wizlist;
extern struct help_index_element *help_table;
extern char *help;
extern struct time_info_data time_info;
extern struct index_data *obj_index;
extern char *weekdays[];
extern char *month_name[];
extern const char *pc_race_types[];
extern struct legend_data legend[8];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern char_data *character_list;
extern struct obj_data *object_list;
extern struct memory_data *mlist;
extern int top_of_world;
int rank_req[14] = {0, 15, 30, 75, 150, 350, 700, 1100, 1600, 2200, 3000, 4000, 5200, 8000 };
extern struct battle tg;
extern struct clan_data clan_list[50];
extern sh_int find_target_room(char_data * ch, char * rawroomstr);
extern int countdown;
extern int boot_high;
extern int idle_void;
extern int seconds;

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern const char *spells[];
extern const char *class_abbrevs[];

int offense_find(char_data *ch);
int dodge_find(char_data *ch);
int parry_find(char_data *ch);
int abs_find(char_data *ch);
int is_viewable_clan(char_data *ch, int number);
char *DARKNESS_CHECK(char_data *ch, char_data *vict);

void update_track_age(room_rnum room);

extern FILE *player_fl;

/* extern functions */
ACMD(do_action);
long find_class_bitvector(char arg);
int level_exp(int level);
extern struct help_index_element *help_table;

struct time_info_data *real_time_passed(time_t t2, time_t t1);

/* local functions */
ACMD(do_look);
ACMD(do_examine);
ACMD(do_battle);
ACMD(do_clans);
ACMD(do_gold);
ACMD(do_score);
ACMD(do_stat);
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_help);
ACMD(do_who);
ACMD(do_legends);
ACMD(do_users);
ACMD(do_view);
ACMD(do_gen_ps);
ACMD(do_incognito);
ACMD(do_whois);
ACMD(do_where);
ACMD(do_scan);
ACMD(do_levels);
ACMD(do_consider);
ACMD(do_diagnose);
ACMD(do_color);
ACMD(do_toggle);
ACMD(do_commands);
ACMD(do_exits);
void sort_commands(void);
void print_object_location(int num, struct obj_data * obj, char_data * ch, int recur);
void show_obj_to_char(struct obj_data * object, char_data * ch, int mode, int ammount);
void list_obj_to_char(struct obj_data * listy, char_data * ch, int mode, int show);
void perform_mortal_where(char_data * ch, char *arg);
void perform_immort_where(char_data * ch, char *arg);
void diag_char_to_char(char_data * i, char_data * ch);
void do_auto_exits( char_data * ch);
void look_at_char( char_data * i, char_data * ch);
void list_one_char( char_data * i, char_data * ch);
void list_char_to_char( char_data * listy, char_data * ch);
void look_in_direction( char_data * ch, int dir);
void look_in_obj( char_data * ch, char *arg);
void look_at_target( char_data * ch, char *arg);
char *find_exdesc(char *word, struct extra_descr_data * listy);
char *race_name(char_data *ch);
char *PERS(char_data *ch, char_data *vict);
char *age_string(int age);
char *find_suf(int number);
int search_timer(char_data *ch);

char *find_suf(int number)
{

	if (number == 1)
		return "st";
	
	else if (number == 2)
		return "nd";
	
	else if (number == 3)
		return "rd";
	
	else if (number < 20)
		return "th";
	
	else if ((number % 10) == 1)
		return "st";
	
	else if ((number % 10) == 2)
		return "nd";
	
	else if ((number % 10) == 3)
		return "rd";
	
	else
		return "th";
}

int level_timer(char_data *ch, int level)
{

	if(IS_NPC(ch) || GET_LEVEL(ch) >= LVL_GRGOD)
		return 0;
	
	if(level >= 75)
		return 5;

	if(level >= 50)
		return 9;

	if(level >= 25)
		return 15;

	if(level >= 0)
		return 22;

	return 0;
}
	
bool char_data::canViewClan(int clannum)
{
	//Non existing?!
	if(clannum < 0)
		return false;

	//GRGOD or higher can view secret clans
	if(GET_LEVEL(this) >= LVL_GRGOD)
		return true;

	//Is this person part of the clan?
	if(this->getClan(clannum))
		return true;

	//Not a secret clan
	if(clan_list[clannum].secret == 0)
		return true;

	return false;
}

char *age_string(int age)
{

	if(age >= 48)
		return "two days old";
	
	if(age >= 24)
		return "day old";

	if(age >= 20)
		return "almost day old";
	
	if(age >= 12)
		return "half day old";
	
	if(age >= 9)
		return "faint";
	
	if(age >= 5)
		return "fairly recent";
	
	if(age >= 3)
		return "very recent";
	
	if(age >= 1)
		return "almost fresh";
	
	if(age == 0)
		return "fresh";
	
	else
		return "BUGGY AGED";

}

void list_warrants(char_data *ch)
{
	int i, found = 0;

	for(i = 1;i <= NUM_CLANS;i++)
		if(IS_WARRANTED(ch, i))
			found = 1;

	if(!found)
		return;

	send_to_char("You are warranted by the following clans:  ", ch);
	
	for(i = 1;i <= NUM_CLANS;i++)
		if(IS_WARRANTED(ch, i))
			message(ch, "%s%s%s, ",COLOR_CYAN(ch, CL_COMPLETE), clan_list[i].name, COLOR_NORMAL(ch, CL_COMPLETE));

	send_to_char("\r\n", ch);
}
			

char *word_text[20] =
{
	"Zero",
	"One",
	"Two",
	"Three",
	"Four",
	"Five",
	"Six",
	"Seven",
	"Eight",
	"Nine",
	"Ten",
	"Eleven",
	"Twelve",
	"Thirteen",
	"Fourteen",
	"Fifteen",
	"Sixteen",
	"Seventeen",
	"Eighteen",
	"Nineteen"
};

char *mood_type(int mood) {
	
	if(mood == MOOD_WIMPY)
		return "Wimpy";

	else if(mood == MOOD_NORMAL)
		return "Normal";

	else if(mood == MOOD_BRAVE)
		return "Brave";

	else
		return "ERROR!";
}

void list_legends(char_data *ch)
{

	int i = 1, to;
	char name[MAX_INPUT_LENGTH];
	char_data *victim;

	if(GET_LEVEL(ch) >= LVL_GRGOD)
		to = 12;
	else
		to = 8;

	send_to_char("Below is a list of the current Legends of the Wheel.\r\n\n", ch);

	for(i = 1; i <= to;i++)
	{

		strcpy(name, legend[i].person);
		CREATE(victim, char_data, 1);
		clear_char(victim);

		if (victim->load(name))  
		{ 

			if(GET_LEVEL(ch) >= LVL_GRGOD)
			{
				message(ch, "~  %d. %s the %s. - 	%d\r\n", i, GET_NAME(victim),
				race_name(victim), GET_WP(victim));
			}

			else
			{
				message(ch, "~ %d. %s the %s.\r\n", i, GET_NAME(victim), race_name(victim));
			}

			if(*GET_SLEW_MESSAGE(victim))
			{
				message(ch, "	%s\r\n\n", GET_SLEW_MESSAGE(victim));
			}

			else
			{
				send_to_char("\r\n", ch);
			}
		}

		if(victim)
			delete(victim);
	}

}

void list_council(char_data *ch, int number)
{
	int loop = 0;
	char_data *vict;
	struct clan_player_data *p;

	for(p = clan_list[number].players;p;p = p->next)
	{

		vict = new char_data;
		clear_char(vict);

		if (vict->load(p->name))
		{

			if(!loop)
			{
				message(ch, "\r\n%s", vict->AES_SEDAI() ? "Sitters: " : "Council Members: ");
				loop++;
			}

			if(PRF_FLAGGED(vict, PRF_COUNCIL))
				message(ch, " %s,", GET_NAME(vict));
		}

		if(vict)
			delete(vict);

	}
	
	send_to_char("\r\n", ch);
}

ACMD(do_scan)
{
	int x, i = 0;
	char arg[MAX_INPUT_LENGTH];
	
	one_argument(argument, arg);

	if (!*arg)
	{
		send_to_char("Scan which direction?\r\n", ch);
		return;
	}
	else
	{
		switch(*arg)
		{
		case 'n':
			x = NORTH;
			break;
		case 'e':
			x = EAST;
			break;
		case 's':
			x = SOUTH;
			break;
		case 'w':
			x = WEST;
			break;
		case 'u':
			x = UP;
			break;
		case 'd':
			x = DOWN;
			break;
		default:
			send_to_char("What direction is that?\r\n", ch);
			return;
		}
	}

	if (world[IN_ROOM(ch)].dir_option[x] && (i = world[IN_ROOM(ch)].dir_option[x]->to_room) != -1)
	{
		list_char_to_char(world[i].people, ch);   
		send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);
	}
}

/* By Galnor on 10-27-2003. Used to view the legends list. */
ACMD(do_legends)
{
	list_legends(ch);
}

/* Imped by Galnor on 10-8-2003. Command is used to view different things such as clans */
ACMD(do_view)
{
	int i = 0, count = 0;
	char_data *victim = 0;
	clan_player_data *p;
	player_clan_data *cl;

	half_chop(argument, buf1, argument);
	half_chop(argument, buf2, argument);

	if(!*buf1)
	{
		send_to_char("View list: Clan\r\n", ch);
		return;
	}

	if(!*buf2)
	{
		send_to_char("But what clan?!\r\n", ch);
		return;
	}

	if(str_cmp(buf1, "clan"))
	{
		send_to_char("Invalid view command\r\n", ch);
		return;
	}

	CREATE(victim, char_data, 1);
	clear_char(victim);

	for(i = 1;i <= NUM_CLANS;i++)
	{
		if (atoi(buf2) != i && strn_cmp(clan_list[i].name, buf2, strlen(buf2)))
			continue;

		if(!ch->canViewClan(i))
		{
			send_to_char("You don't know anything about them.\r\n", ch);
			break;
		}

		sprintf(buf, "Players in Clan %s\r\n", clan_list[i].name);
		send_to_char(buf, ch);

		for(count = 1;count <= 13;count++)
		{
			sprintf(buf, "\r\nRank %s:\r\n", word_text[count]);
			send_to_char(buf, ch);

			for(p = clan_list[i].players;p;p = p->next)
			{			
				if (!victim->load(p->name))
					continue;

				if(!(cl = victim->getClan(i)) || cl->rank != count)
					continue;

				else
				{
					sprintf(buf, "%s, ", GET_NAME(victim));
					send_to_char(buf, ch);
				}
			}
			list_council(ch, i);

			//We don't want them viewing more than one clan, such as "view clan black" showing Black Tower and Black Ajah
			break;
		}
		
	}

	if(victim)
		delete(victim);

}

void perform_search(char_data *ch, int direction)
{


	if(EXIT(ch, direction))
		if(GET_SKILL(ch, SKILL_SEARCH) >= EXIT(ch, direction)->hidden && EXIT(ch, direction)->keyword)
		{
			sprintf(buf, "You discovered the %s!\r\n", fname(EXIT(ch, direction)->keyword));
			send_to_char(buf, ch);
			return;
		}

		
		send_to_char("You found nothing in that direction.\r\n", ch);
}

ACMD(do_search)
 {

	int direction;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if(IS_NPC(ch))
	{
		send_to_char("Mobs can't do this.\r\n", ch);
		return;
	}

	if((GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) < LVL_GRGOD))
	{
		send_to_char("You shouldn't be doing this...\r\n", ch);
		return;
	}

	if ((direction = search_block(arg, (const char **) dirs, FALSE)) < 0)
	{
		send_to_char("That isn't a direction!\r\n", ch);
		return;
	}

	if(EXIT(ch, direction))
		if(!EXIT_FLAGGED(EXIT(ch, direction), EX_CLOSED))
		{
			send_to_char("The way is already open!\r\n", ch);
			return;
		}

	if(!IS_NPC(ch))
		if(!ch->desc->command_ready)
		{
			ch->desc->timer = level_timer(ch, GET_SKILL(ch, SKILL_SEARCH));
			ch->desc->command_ready = 1;
			return;
		}

	if(ch->desc)
		if(ch->desc->timer <= 0)
			perform_search(ch, direction);
}



/* Added by Galnor on 10-8-2003. Command shows a list of clans */
ACMD(do_clans)
{
	int i;

	for(i = 1;i < 50; i++)
	{
		if(str_cmp(clan_list[i].name, ""))
		{
			sprintf(buf, "#%d %s\r\n", i, clan_list[i].name);
			send_to_char(buf, ch);
		}
	}
}

/*	Added by Galnor on 9-28-2003. Command shows the current struct of the "battle" system. 
	Which side has how many Weave Points... */
ACMD(do_battle)
{

	send_to_char("The battle between the Light and Dark continues...\r\n", ch);
	message(ch, "The Light has collected %d Weave Points from the Dark.\r\n"
				 "The Dark has collected %d Weave Points from the Light.\r\n", tg.humanwp, tg.trollwp);
}


/* Added by Rhei late 2002. Command shows information about selected player. */
ACMD(do_whois) 
{ 
	char buf[1024];
	player_clan_data *cl;
	char leg_string[MAX_INPUT_LENGTH], clan_string[MAX_INPUT_LENGTH];
	string class_name;
   
	const char *immlevels[LVL_IMPL - (LVL_IMMORT - 1)] =
	{
		"is an Immortal of the Wheel.",   // highest mortal level +1 //
		"is a Builder of the Wheel.",    // BLDER level //
		"is an Apprentice of the Wheel", //APPR Level //
		"is a Teller of the Wheel.",     // highest mortal level +2 //
		"is a Lord of the Wheel.",       // highest mortal level +3 //
		"is a Creator.",                 // highest mortal level +4 //
	}; 
 
	char_data *victim = 0;
	skip_spaces(&argument); 
 
	if (argument == nullptr)
		send_to_char("Who?\r\n", ch); 

	else 
	{ 
		CREATE(victim, char_data, 1);
		clear_char(victim); 

		if (victim->load(argument))
		{ 
			*buf = '\0'; 
 
			if (GET_LEVEL(victim) >= LVL_IMMORT)
			{
				sprintf(buf + strlen(buf), "%s %s %s\r\n",
					GET_NAME(victim), GET_TITLE(victim), immlevels[GET_LEVEL(victim)-LVL_IMMORT]); 
			}

			else
			{ 
		
				if(IS_WARRIOR(victim))
					class_name = "warrior";
				else if(IS_THIEF(victim))
					class_name = "thief";
				else if(IS_RANGER(victim))
					class_name = "ranger";
				else if(IS_CHANNELER(victim))
					class_name = "channeler";
				else if(IS_GREYMAN(victim))
					class_name = "greyman";

				sprintf(leg_string, " is Legend %d.", victim->getLegend());

				clan_string[0] = ' ';
				clan_string[1] = 0;
				for(cl = ch->clans;cl;cl = cl->next)
				{
					if(ch->canViewClan(cl->clan))
					{
						sprintf(clan_string + strlen(clan_string), " %s Rank %d ", clan_list[cl->clan].name, cl->rank);
						sprintf(clan_string + strlen(clan_string), "%s",
							PRF_FLAGGED(victim, PRF_COUNCIL) ? victim->AES_SEDAI() ? "Sitter " : "Council " : "");
					}
				}

				if(PRF_FLAGGED(victim, PRF_INCOG) && GET_LEVEL(ch) < LVL_GOD)
					sprintf(buf + strlen(buf), "%s %s%s is a %s.\r\n", GET_NAME(victim),
					GET_TITLE(victim), victim->getLegend() > 0 ? leg_string : " ", race_name(victim));

				else if(*GET_WHOIS_EXTRA(victim))
					sprintf(buf + strlen(buf), "%s %s is %s.\r\n", GET_NAME(victim),
					GET_TITLE(victim), GET_WHOIS_EXTRA(victim));

				else
					sprintf(buf + strlen(buf), "%s %s%s is a level %d%s%s %s.\r\n", 
					GET_NAME(victim), GET_TITLE(victim), victim->getLegend() > 0 ? leg_string : "",
					GET_LEVEL(victim), clan_string, race_name(victim), class_name.c_str());
 
			}

			send_to_char(buf, ch); 
			send_to_char(victim->player.description, ch); 
		}
		
		else  
			send_to_char("There is no such player.\r\n", ch);  
 
		if(victim)
			free_char(victim);
	
	}
}

/* Added by Galnor early 2003. Command hides information of a player(UNFINISHED!) */
ACMD(do_incognito) 
{
	if (!PRF_FLAGGED(ch, PRF_INCOG))
	{
		send_to_char("Your identity is now safe.\r\n", ch);
		SET_BIT_AR(PRF_FLAGS(ch), PRF_INCOG);
	}

	else
	{
		send_to_char("Your identity is no longer a secret.\r\n", ch);
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_INCOG);
	}
}


/*
 * This function screams bitvector... -gg 6/45/98
 */
void show_obj_to_char(struct obj_data * object, char_data * ch,
			int mode, int ammount)
{
	bool found;
	char text[MAX_INPUT_LENGTH];

	*buf = '\0';
	
	if ((mode == 0) && object->description)
		strcpy(buf, object->description);
	
	else if (object->short_description && ((mode == 1) ||
	(mode == 2) || (mode == 3) || (mode == 4)))
		strcpy(buf, object->short_description);

	if(ammount > 1)
	{
		sprintf(text, "		[%d]", ammount);
		strcat(buf, text);
	}
  
	else if (mode == 5)
	{
		if (GET_OBJ_TYPE(object) == ITEM_NOTE)
		{
			if (object->action_description)
			{
				strcpy(buf, "There is something written upon it:\r\n\r\n");
				strcat(buf, object->action_description);
				page_string(ch->desc, buf, 1);
			} 
			
			else
				act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      
			return;
		} 
		
		else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON)
			strcpy(buf, "You see nothing special..");

		else			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
			strcpy(buf, "It looks like a drink container.");
	}

	if (mode != 3)
	{
		found = FALSE;
    
		if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
		{
			strcat(buf, " (invisible)");
			found = TRUE;
		}
	}
  
	strcat(buf, "\r\n");
	page_string(ch->desc, buf, TRUE);
}

int count_object(struct obj_data *obj)
{
	int count = 0;
	struct obj_data *i;

	for(i = obj;i;i = i->next_content)
	{

		if(GET_OBJ_VNUM(obj) == GET_OBJ_VNUM(i) && GET_OBJ_VNUM(obj) > -1)
			count++;

		else 
			if (!str_cmp(obj->short_description, i->short_description))
				count++;
	}

	return count;
}

void list_obj_to_char(struct obj_data * listy, char_data * ch,
int mode, int show)

{
	struct obj_data *i, *temporary;
	int count = 0, cur = 0, number = 0, cur2 = 0;
	int shown[2000];
	char *showns[250];
	bool has_seen = FALSE;
	bool found = FALSE;

	// Serai - 06/15 - fixed some if()'s based on uninitialised values
	// Consider turning the two arrays into linked lists...
	if(PRF_FLAGGED(ch, PRF_SPAM))
	{
		for(count = 0;count < 2000;count++)
		{
			shown[count] = -2;

			if(count < 250)
				CREATE(showns[count], char, 300);
		}

		has_seen = FALSE;
		found = FALSE;

		for(i = listy;i;i = i->next_content)
		{
			temporary = i;
			cur++;

			for(count = 0;count < 2000;count++)
			{
				if(GET_OBJ_VNUM(temporary) > -1 && GET_OBJ_VNUM(temporary) == shown[count])
				{
					has_seen = TRUE;
					break;
				}

				else
					if(count < 250 && showns[count] && temporary->short_description)
						if(!str_cmp(temporary->short_description, showns[count]))
						{
							has_seen = TRUE;
							break;
						}
			}

			if(!has_seen) 
			{
				number = count_object(temporary);
				show_obj_to_char(temporary, ch, mode, number);

				if(GET_OBJ_VNUM(temporary) > -1)
					shown[cur] = GET_OBJ_VNUM(temporary);

				else
				{
					strcpy(showns[cur2], temporary->short_description);
					cur2++;
				}
			}

			has_seen = FALSE;
			found = TRUE;
			i = temporary;
		}

		for(count = 0;count < 250;count++)
		{
			if(showns[count])
				delete[] (showns[count]);
		}
	}
	else
	{
		for (i = listy; i; i = i->next_content)
		{
			show_obj_to_char(i, ch, mode, 1);
			found = TRUE;
		}
	}

	if (!found && show)
		send_to_char(" Nothing.\r\n", ch);
}


void diag_char_to_char(char_data * i, char_data * ch)
{
	int percent;

	if (GET_MAX_HIT(i) > 0)
		percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  
	else
		percent = -1;		/* How could MAX_HIT be < 1?? */

	strcpy(buf, PERS(i, ch));
	CAP(buf);

	if (percent >= 100)
		strcat(buf, " is in excellent condition.\r\n");
	else if (percent >= 90)
		strcat(buf, " has a few scratches.\r\n");
	else if (percent >= 75)
		strcat(buf, " has some small wounds and bruises.\r\n");
	else if (percent >= 50)
		strcat(buf, " has quite a few wounds.\r\n");
	else if (percent >= 30)
		strcat(buf, " has some big nasty wounds and scratches.\r\n");
	else if (percent >= 15)
		strcat(buf, " looks pretty hurt.\r\n");
	else if (percent >= 0)
		strcat(buf, " is in awful condition.\r\n");
	else
		strcat(buf, " is bleeding awfully from big wounds.\r\n");

	send_to_char(buf, ch);
}


void look_at_char(char_data * i, char_data * ch)
{
	int j, found;
	struct obj_data *tmp_obj;
	char msg[MAX_INPUT_LENGTH];

	if (!ch->desc)
		return;

	if (i->player.description)
		send_to_char(i->player.description, ch);
  
	else
		act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

	diag_char_to_char(i, ch);

	found = FALSE;
  
	for (j = 0; !found && j < NUM_WEARS; j++)
		if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
			found = TRUE;

	if (found)
	{
		act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    
		for (j = 0; j < NUM_WEARS; j++)
			if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
			{
				if(j == WEAR_WIELD && IS_OBJ_STAT(GET_EQ(i, j), ITEM_TWO_HANDED))
					strcpy(msg, "<wielded with two hands>    ");

				else
					strcpy(msg, where[j]);

				send_to_char(msg, ch);
				show_obj_to_char(GET_EQ(i, j), ch, 1, 1);
		}
	}
  
	if (ch != i && (IS_THIEF(ch) || GET_LEVEL(ch) >= LVL_IMMORT))
	{
		found = FALSE;
		act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
		
		for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
		{
			if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch)))
			{
				show_obj_to_char(tmp_obj, ch, 1, 1);
				found = TRUE;
			}
		}

		if (!found)
		send_to_char("You can't see anything.\r\n", ch);
  
	}
}


void list_one_char(char_data * i, char_data * ch)
{
	const char *positions[] = {
		" is lying here, dead",
		" is lying here, mortally wounded",
		" is lying here, incapacitated",
		" is lying here, stunned",
		" is sleeping here",
		" is resting here",
		" is sitting here",
		" fighting",
		" is standing here"
	};

	if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i))
	{
		if (AFF_FLAGGED(i, AFF_INVISIBLE))
			strcpy(buf, "*");
    
		else
			*buf = '\0';

		if(RIDDEN_BY(i) && RIDDEN_BY(i) != ch)
			return;

		strcat(buf, i->player.long_descr);
		send_to_char(buf, ch);

		if (AFF_FLAGGED(i, AFF_SANCTUARY))
			act("...$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
	
		if (AFF_FLAGGED(i, AFF_BLIND))
			act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);

		return;
	
	}

	if (IS_NPC(i))
	{
		strcpy(buf, i->player.short_descr);
		CAP(buf);
	} 
	
	else
		if(GET_RACE(ch) != GET_RACE(i))
			sprintf(buf, "*%s%s%s*", COLOR_RED(ch, CL_NORMAL), GET_NAME(i), COLOR_YELLOW(ch, CL_NORMAL));
	
		else
			sprintf(buf, "%s %s", GET_NAME(i), GET_TITLE(i));

	if (AFF_FLAGGED(i, AFF_INVISIBLE))
		strcat(buf, " (invisible)");

	if (!IS_NPC(i) && !i->desc)
		strcat(buf, " (linkless)");
	
	if (PLR_FLAGGED(i, PLR_WRITING))
		strcat(buf, " (writing)");

	if (GET_POS(i) != POS_FIGHTING)
		strcat(buf, positions[(int) GET_POS(i)]);
  
	else {
	
		if (FIGHTING(i)) {
			strcat(buf, " is here, fighting ");
			
			if (FIGHTING(i) == ch)
				strcat(buf, "YOU!");
			
			else {	
				if (i->in_room == FIGHTING(i)->in_room)
					strcat(buf, PERS(FIGHTING(i), ch));
				
				else
					strcat(buf, "someone who has already left");
	
				strcat(buf, "!");
			}
		} 
		
		else			/* NIL fighting pointer */
			strcat(buf, " is here struggling with thin air");
	}

	if(MOUNT(i))  {
		strcat(buf, ", riding ");
		strcat(buf, GET_NAME(MOUNT(i)));
	}



	//Added by Galnor. Copy over buf and dont show the name if hidden but can see. 9-13-2003 //
	if(AFF_FLAGGED(i, AFF_HIDE))
		sprintf(buf, "You feel the presence of someone nearby...");

	strcat(buf, ".\r\n");
	send_to_char(buf, ch);


	if (AFF_FLAGGED(i, AFF_SANCTUARY))
		act("...$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
}



void list_char_to_char( char_data * listy,  char_data * ch)
{
	int maxTaint = 1;
	char_data *i;

	for (i = listy; i; i = i->next_in_room)
		if (ch != i)
		{
			if (--maxTaint >= 0 && TAINT_CALC(ch) > number(0, 100))
				send_to_char("You feel the presence of someone nearby....\r\n", ch);
			
			if (CAN_SEE(ch, i))
				list_one_char(i, ch);
		}
}



void do_auto_exits(char_data * ch)
{
	int door, slen = 0;

	*buf = '\0';

	for (door = 0; door < NUM_OF_DIRS; door++)
		if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE)
			 if(!EXIT(ch, door)->hidden || !EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
				slen += sprintf(buf + slen, "%c ", LOWER(*dirs[door]));

	sprintf(buf2, "%s[ Exits: %s]%s\r\n", COLOR_CYAN(ch, CL_COMPLETE),
	*buf ? buf : "None! ", COLOR_NORMAL(ch, CL_COMPLETE));

	send_to_char(buf2, ch);
}


ACMD(do_exits)
{
	int door;

	*buf = '\0';

	if (AFF_FLAGGED(ch, AFF_BLIND))
	{
		send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
		return;
	}

	for (door = 0; door < NUM_OF_DIRS; door++)
		if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE)
			if (!EXIT(ch, door)->hidden || !EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
			{
      
			if (GET_LEVEL(ch) >= LVL_IMMORT)
				sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],			
				GET_ROOM_VNUM(EXIT(ch, door)->to_room),
				world[EXIT(ch, door)->to_room].name);

			else {
				sprintf(buf2, "%-5s - ", dirs[door]);
				
				if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
					strcat(buf2, "Too dark to tell\r\n");
	
				else {
					strcat(buf2, world[EXIT(ch, door)->to_room].name);
					strcat(buf2, "\r\n");
				}
			}

			strcat(buf, CAP(buf2));
		}
  
	send_to_char("Obvious exits:\r\n", ch);

	if (*buf)
		send_to_char(buf, ch);
	
	else
		send_to_char(" None.\r\n", ch);
}

void print_tracks(char_data *ch, char *arg, int auto_track)
{

	struct track_data *trax;
	int count = 0, num = 0;

	//update_track_age(IN_ROOM(ch));

	if(!OUTSIDE(ch))
		return;

	num = GET_SKILL(ch, SKILL_TRACK) / 20;

	for(count = 0, trax = world[IN_ROOM(ch)].tracks;count < num && trax;trax = trax->next_in_room)
	{

		if(*arg)
		{

			if(!isname(arg, trax->name))
			{
				continue;
			}

			if(!str_cmp(arg, "trolloc") && trax->race != RACE_TROLLOC)
				continue;

			if(!str_cmp(arg, "human") && trax->race != RACE_HUMAN)
				continue;

			if(!str_cmp(arg, "fade") && trax->race != RACE_TROLLOC)
				continue;

			if(!str_cmp(arg, "dreadlord") && trax->race != CLASS_DREADLORD)
				continue;

			if(!str_cmp(arg, "fade") && trax->race != CLASS_FADE)
				continue;

		}

		message(ch, "You see %s tracks of %s leaving %s.\r\n", age_string(trax->age),
		trax->name ? trax->name : "Unknown", dirs[trax->direction]);
		count++;
	}
	
	if(!count && !auto_track)
		send_to_char("You see no tracks here.\r\n", ch);
}


void look_at_room(char_data * ch, int ignore_brief)
{
	if (!ch->desc)
		return;

	if(ch->in_room < 0)
	{
		sprintf(buf, "Player %s's room is less than 0!", GET_NAME(ch));
		log(buf);
		char_from_room(ch);
		char_to_room(ch, find_target_room(ch, "1"));
	}

	if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch))
	{
		send_to_char("It is pitch black...\r\n", ch);
		return;
	} 
	
	else if (AFF_FLAGGED(ch, AFF_BLIND))
	{
		send_to_char("You see nothing but infinite darkness...\r\n", ch);
		return;
	}
  
	send_to_char(COLOR_CYAN(ch, CL_NORMAL), ch);
	
	if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS))
	{
		sprintbitarray(ROOM_FLAGS(ch->in_room), room_bits, RF_ARRAY_MAX, buf);
		sprintf(buf2, "[%5d] %s [ %s]", GET_ROOM_VNUM(IN_ROOM(ch)),
	    world[ch->in_room].name, buf);
		send_to_char(buf2, ch);
	} 
	
	else
		send_to_char(world[ch->in_room].name, ch);

	send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);
	send_to_char("\r\n", ch);

	if ((!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_BRIEF)) || ignore_brief ||
	ROOM_FLAGGED(ch->in_room, ROOM_DEATH))		
		send_to_char(world[ch->in_room].description, ch);

	/* autoexits */
	if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOEXIT))
		do_auto_exits(ch);

	if(ch->getClan(CLAN_CHOSEN) || ch->getClan(CLAN_SHIENARAN) ||
	IS_WARDER(ch) || ch->getClan(CLAN_KHOMON))
		print_tracks(ch, "", TRUE);

	/* now list characters & objects */
	send_to_char(COLOR_GREEN(ch, CL_NORMAL), ch);
	list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
	
	send_to_char(COLOR_YELLOW(ch, CL_NORMAL), ch);
	list_char_to_char(world[ch->in_room].people, ch);
	
	send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);
}



void look_in_direction(char_data * ch, int dir)
{
	int hid;

	if (EXIT(ch, dir))
	{
		hid = EXIT(ch, dir)->hidden;
		
		if (EXIT(ch, dir)->general_description)
			send_to_char(EXIT(ch, dir)->general_description, ch);

		if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT(ch, dir)->keyword && !hid) {
			sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
			send_to_char(buf, ch);
		} 
		
		else if (EXIT_FLAGGED(EXIT(ch, dir), EX_ISDOOR) && EXIT(ch, dir)->keyword &&
			!EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED)) {
			sprintf(buf, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
			send_to_char(buf, ch);
		}
  
	} 
	
	else
		send_to_char("Nothing special there...\r\n", ch);
}



void look_in_obj(char_data * ch, char *arg)
{
	struct obj_data *obj = nullptr;
	char_data *dummy = nullptr;
	int amt, bits;

	if (!*arg)
		send_to_char("Look in what?\r\n", ch);


	
	else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
	FIND_OBJ_EQUIP, ch, &dummy, &obj))) {    
		sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
		send_to_char(buf, ch);
	} 
	
	else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) && (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
		send_to_char("There's nothing inside that!\r\n", ch);
  
	else {
		if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
			if (OBJVAL_FLAGGED(obj, CONT_CLOSED))
				send_to_char("It is closed.\r\n", ch);
      
			else {
				send_to_char(fname(obj->name), ch);
	
				switch (bits) {
					case FIND_OBJ_INV:
						send_to_char(" (carried): \r\n", ch);
						break;
					case FIND_OBJ_ROOM:
						send_to_char(" (here): \r\n", ch);
						break;
					case FIND_OBJ_EQUIP:
						send_to_char(" (used): \r\n", ch);
						break;
				}

				list_obj_to_char(obj->contains, ch, 2, TRUE);
			}
		} 

		else {		
			/* item must be a fountain or drink container */
			if (GET_OBJ_VAL(obj, 1) <= 0)
				send_to_char("It is empty.\r\n", ch);
      
			else {
				if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1) > GET_OBJ_VAL(obj, 0))
					sprintf(buf, "Its contents seem somewhat murky.\r\n"); /* BUG */
 
				else {
					amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
					sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt], color_liquid[GET_OBJ_VAL(obj, 2)]);
				}
				send_to_char(buf, ch);      
			}    
		}  
	}
}



char *find_exdesc(char *word, struct extra_descr_data * listy)
{
	struct extra_descr_data *i;

	for (i = listy; i; i = i->next)
		if (isname(word, i->keyword))
			return (i->description);

	return nullptr;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */

/*
 * BUG BUG: If fed an argument like '2.bread', the extra description
 *          search will fail when it works on 'bread'!
 * -gg 6/24/98 (I'd do a fix, but it's late and non-critical.)
 */
void look_at_target(char_data * ch, char *arg)
{
	int bits, found = FALSE, j;
	char_data *found_char = nullptr;
	struct obj_data *obj = nullptr, *found_obj = nullptr;
	char *desc;

	if (!ch->desc)
		return;

	if (!*arg) {
		send_to_char("Look at what?\r\n", ch);
		return;
	}
  
	bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

	/* Is the target a character? */
	if (found_char != nullptr) {
		look_at_char(found_char, ch);
    
		if (ch != found_char) {
			if (CAN_SEE(found_char, ch))
				act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
				act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
			}
    
		return;
	}
  
	/* Does the argument match an extra desc in the room? */
	if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != nullptr) {
		page_string(ch->desc, desc, FALSE);
		return;
	}
	
	/* Does the argument match an extra desc in the char's equipment? */
	for (j = 0; j < NUM_WEARS && !found; j++)
		if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
			if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != nullptr) {
				send_to_char(desc, ch);
				found = TRUE;
			}
  
			/* Does the argument match an extra desc in the char's inventory? */
			for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
				if (CAN_SEE_OBJ(ch, obj))
					if ((desc = find_exdesc(arg, obj->ex_description)) != nullptr) {
						send_to_char(desc, ch);
						found = TRUE;
					}
			}

  
			/* Does the argument match an extra desc of an object in the room? */
			for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
				if (CAN_SEE_OBJ(ch, obj))
					if ((desc = find_exdesc(arg, obj->ex_description)) != nullptr) {
						send_to_char(desc, ch);
						found = TRUE;
					}

  
					/* If an object was found back in generic_find */
					if (bits) {
    
						if (!found)
							show_obj_to_char(found_obj, ch, 5, 1);	/* Show no-description */
    
						else
							show_obj_to_char(found_obj, ch, 6, 1);	/* Find hum, glow etc */
					} 
					
					else if (!found)
						send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
	char arg2[MAX_INPUT_LENGTH];
	int look_type;

	if (!ch->desc)
		return;

	if (GET_POS(ch) < POS_SLEEPING)
		send_to_char("You can't see anything but stars!\r\n", ch);
  
	else if (AFF_FLAGGED(ch, AFF_BLIND))
		send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  
	else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
		send_to_char("It is pitch black...\r\n", ch);
		list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
	} 
	
	else {
		half_chop(argument, arg, arg2);

		if (subcmd == SCMD_READ) {
			if (!*arg)
				send_to_char("Read what?\r\n", ch);
      
			else
				look_at_target(ch, arg);
      
			return;
		}
    
		if (!*arg)			/* "look" alone, without an argument at all */
			look_at_room(ch, 1);
    
		else if (is_abbrev(arg, "in"))
			look_in_obj(ch, arg2);
    
		else if ((look_type = search_block(arg, (const char **) dirs, FALSE)) >= 0)
			look_in_direction(ch, look_type);
    
		else if (is_abbrev(arg, "at"))
			look_at_target(ch, arg2);
    
		else
			look_at_target(ch, arg);
	}
}



ACMD(do_examine)
{
	int bits;
	char_data *tmp_char;
	struct obj_data *tmp_object;

	one_argument(argument, arg);

	if (!*arg) {
		send_to_char("Examine what?\r\n", ch);
		return;
	}
  
	look_at_target(ch, arg);

	bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	if (tmp_object) {
		if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
		(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
		(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
			
			send_to_char("When you look inside, you see:\r\n", ch);
			look_in_obj(ch, arg);
		}
	}
}



ACMD(do_gold)
{
	if (GET_GOLD(ch) == 0)
		send_to_char("You're broke!\r\n", ch);
  
	else if (GET_GOLD(ch) == 1)
		send_to_char("You have one miserable little gold coin.\r\n", ch);
  
	else {
		sprintf(buf, "You have %d gold coins.\r\n", GET_GOLD(ch));
		send_to_char(buf, ch);
	}
}

ACMD(do_score)
{
	struct time_info_data playing_time;
	char mount_message[127], legend_message[127];
	int total_qp = 0;
	player_clan_data *cl;

	if(ch->getLegend() == -1)
		sprintf(legend_message, ".");
	
	else
		sprintf(legend_message, ", ranking you Legend %d.", ch->getLegend());

	for(cl = ch->clans;cl;cl = cl->next)
	{
		total_qp += cl->quest_points;
	}

	sprintf(buf,
	"You have %d(%d) hit", GET_HIT(ch), GET_MAX_HIT(ch));
  
	if(IS_CHANNELER(ch) || IS_DREADLORD(ch))
		sprintf(buf + strlen(buf), ", %d(%d) mana", GET_MANA(ch), GET_MAX_MANA(ch));

	if(IS_DREADLORD(ch) || IS_FADE(ch))
		sprintf(buf + strlen(buf), ", %d(%d) shadow points, ", GET_SHADOW(ch), GET_MAX_SHADOW(ch));

	sprintf(buf + strlen(buf), " and %d(%d) movement points.\r\n",
	GET_MOVE(ch), GET_MAX_MOVE(ch));

	sprintf(buf + strlen(buf), "You have scored %d exp, %d total quest points, and have %d gold coins.\r\n",
	GET_EXP(ch), total_qp, GET_GOLD(ch));

	if (!IS_NPC(ch))
	{
		if (GET_LEVEL(ch) < LVL_IMMORT)
			sprintf(buf + strlen(buf), "You need %d exp to reach your next level.\r\n",
			level_exp(GET_LEVEL(ch) + 1) - GET_EXP(ch));

		playing_time = *real_time_passed((time(0) - ch->player.time.logon) +
		ch->player.time.played, 0);
    
		sprintf(buf + strlen(buf), "You have been playing for %d days and %d hours.\r\n",
		playing_time.day, playing_time.hours);

		sprintf(buf + strlen(buf), "You have gathered %d Weave Points so far%s\r\n", GET_WP(ch), legend_message);

		if(GET_DP(ch) > 0)
		{
			sprintf(buf + strlen(buf), "You have gathered %d Dark Points.\r\n", GET_DP(ch));
		}

		sprintf(buf + strlen(buf), "This ranks you as %s %s (level %d)\r\n",
		GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch));
	}

	//List the player's clans and information about it
	if(cl = ch->clans)
	{
		sprintf(buf + strlen(buf), "You are in the following clans:\r\n");
		for(;cl;cl = cl->next)
		{
			sprintf(buf + strlen(buf), "%s : %d Quest Points, rank %d, %d Quest Points to rank.\r\n",
				clan_list[cl->clan].name, cl->quest_points, cl->rank, (rank_req[cl->rank + 1] - cl->quest_points));
		}
	}

	else
		sprintf(buf + strlen(buf), "You are not a member of any clans.\r\n");

	if(MOUNT(ch))
	{

		if(GET_MOVE(MOUNT(ch)) > -10)
			strcpy(mount_message, "Your mount looks haggard and beaten.");

		if(GET_MOVE(MOUNT(ch)) > (GET_MAX_MOVE(MOUNT(ch)) * .25))
			strcpy(mount_message, "Your mount looks weary.");

		if(GET_MOVE(MOUNT(ch)) > (GET_MAX_MOVE(MOUNT(ch)) * .50))
			strcpy(mount_message, "Your mount looks strong.");

		if(GET_MOVE(MOUNT(ch)) > (GET_MAX_MOVE(MOUNT(ch)) * .75))
			strcpy(mount_message, "Your mount is full of energy.");

		sprintf(buf + strlen(buf), "You are riding %s. %s\r\n", GET_NAME(MOUNT(ch)), mount_message);
	}

	switch (GET_POS(ch))
	{
  
	case POS_DEAD:
		strcat(buf, "You are DEAD!\r\n");
		break;
  
	case POS_MORTALLYW:
		strcat(buf, "You are mortally wounded!  You should seek help!\r\n");
		break;
  
	case POS_INCAP:
		strcat(buf, "You are incapacitated, slowly fading away...\r\n");
		break;
  
	case POS_STUNNED:
		strcat(buf, "You are stunned!  You can't move!\r\n");
		break;
  
	case POS_SLEEPING:
		strcat(buf, "You are sleeping.\r\n");
		break;
  
	case POS_RESTING:
		strcat(buf, "You are resting.\r\n");
		break;
  
	case POS_SITTING:
		strcat(buf, "You are sitting.\r\n");
		break;
  
	case POS_FIGHTING:
		if (FIGHTING(ch))
			sprintf(buf + strlen(buf), "You are fighting %s.\r\n",
			PERS(FIGHTING(ch), ch));
    
		else
			strcat(buf, "You are fighting thin air.\r\n");
    
		break;
  
	case POS_STANDING:
		strcat(buf, "You are standing.\r\n");
		break;
  
	default:
		strcat(buf, "You are floating.\r\n");
		break;
	}

	if (!IS_NPC(ch))
	{
		if (GET_COND(ch, DRUNK) > 10)
			strcat(buf, "You are intoxicated.\r\n");

		if (GET_COND(ch, FULL) == 0)
			strcat(buf, "You are hungry.\r\n");

		if (GET_COND(ch, THIRST) == 0)
			strcat(buf, "You are thirsty.\r\n");
	}

	send_to_char(buf, ch);
}



ACMD(do_stat)
{

	char_data *bonded;

	if(IS_HUMAN(ch))
		sprintf(buf2, "human");
	
	if(IS_AIEL(ch))
		sprintf(buf2, "aiel");
	
	if(IS_SEANCHAN(ch))
		sprintf(buf2, "seanchan");
	
	if(IS_TROLLOC(ch))
		sprintf(buf2, "trolloc");
	
	if(IS_FADE(ch))
		sprintf(buf2, "myrddraal");

	if(IS_DREADLORD(ch))
		sprintf(buf2, "dreadlord");

	if(IS_GREYMAN(ch))
		sprintf(buf2, "grey man");


	message(ch, "You are a %s%d%s year old %s%s%s.\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), GET_AGE(ch), COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), buf2, COLOR_NORMAL(ch, CL_COMPLETE));

	message(ch, "Your mood is currently: %s%s%s\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), mood_type(GET_MOOD(ch)), COLOR_NORMAL(ch, CL_COMPLETE));

	if(GET_WARNINGS(ch))
		message(ch, "You have %s%d%s warning%s.\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), GET_WARNINGS(ch), COLOR_NORMAL(ch, CL_COMPLETE), GET_WARNINGS(ch) == 1 ? "" : "s");

	if (age(ch)->month == 0 && age(ch)->day == 0)
	   message(ch, "  It's your birthday today.\r\n");


	message(ch, "Height %s%d%s cm, Weight %s%d%s pounds, Weight carried %s%d%s lbs, Worn: %s%d%s lbs.\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), GET_HEIGHT(ch), COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), GET_WEIGHT(ch), COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), carrying_weight(ch), COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), wearing_weight(ch), COLOR_NORMAL(ch, CL_COMPLETE));

	message(ch,
		"Offensive Bonus: %s%d%s Dodge Bonus: %s%d%s, Parry Bonus: %s%d%s\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), offense_find(ch), COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), dodge_find(ch)  , COLOR_NORMAL(ch, CL_COMPLETE),
		COLOR_CYAN(ch, CL_COMPLETE), parry_find(ch)  , COLOR_NORMAL(ch, CL_COMPLETE));

	list_warrants(ch);

	if(GET_LEVEL(ch) >= 5)  
	{
		message(ch, "Your abilities are: Str: %s%d%s, Int: %s%d%s, Wis: %s%d%s, Dex: %s%d%s, Con: %s%d%s\r\n",
			COLOR_CYAN(ch, CL_COMPLETE), GET_STR(ch), COLOR_NORMAL(ch, CL_COMPLETE),
			COLOR_CYAN(ch, CL_COMPLETE), GET_INT(ch), COLOR_NORMAL(ch, CL_COMPLETE),
			COLOR_CYAN(ch, CL_COMPLETE), GET_WIS(ch), COLOR_NORMAL(ch, CL_COMPLETE),
			COLOR_CYAN(ch, CL_COMPLETE), GET_DEX(ch), COLOR_NORMAL(ch, CL_COMPLETE),
			COLOR_CYAN(ch, CL_COMPLETE), GET_CON(ch), COLOR_NORMAL(ch, CL_COMPLETE));
	}

	message(ch, "Your armor absorbs %s%d%s%% damage on average.\r\n",
		COLOR_CYAN(ch, CL_COMPLETE), abs_find(ch), COLOR_NORMAL(ch, CL_COMPLETE));
  
	message(ch, "You are affected by:\r\n");

	if (AFF_FLAGGED(ch, AFF_BLIND))
		message(ch, "BLIND\r\n");

	if (AFF_FLAGGED(ch, AFF_SNEAK))
		message(ch, "SNEAK\r\n");

	if (AFF_FLAGGED(ch, AFF_SANCTUARY))
		message(ch, "SANCTUARY\r\n");

	if (AFF_FLAGGED(ch, AFF_POISON))
		message(ch, "POISON\r\n");

	if (AFF_FLAGGED(ch, AFF_PARANOIA))
		message(ch, "You are paranoid.\r\n");

	if (affected_by_spell(ch, SPELL_AGILITY))
		message(ch, "AGILITY\r\n");

	if (AFF_FLAGGED(ch, AFF_NOQUIT))
		message(ch, "NO QUIT\r\n");

	if (AFF_FLAGGED(ch, AFF_NOTICE))
		message(ch, "NOTICE\r\n");

	if (AFF_FLAGGED(ch, AFF_EFFUSION))
		message(ch, "EFFUSION\r\n");

	if (AFF_FLAGGED(ch, AFF_HASTE))
		message(ch, "HASTE\r\n");

	if(AFF_FLAGGED(ch, AFF_NIGHT_VISION))
		message(ch, "NIGHT VISION\r\n");

	if(AFF_FLAGGED(ch, AFF_STRENGTH))
		message(ch, "STRENGTH\r\n");

	if(IS_BONDED(ch))
		message(ch, "BOND TO: %s\r\n", GET_BOND(ch));

	if(PRF_FLAGGED(ch, PRF_BUILDWALK))
		message(ch, "BUILDWALK\r\n");


	/* The rest of this function is for bonded pairs showing eachother's condition to one another. */
	if(IS_BONDED(ch))
	{
		if(nullptr != (bonded = get_char_by_name(GET_BOND(ch), FALSE)))
		{

			/* Make sure we don't divide by 0 */
			int maxh = MAX(1, GET_MAX_HIT(bonded));
			int maxs = MAX(1, GET_MAX_MANA(bonded));
			int maxm = MAX(1, GET_MAX_MOVE(bonded));
		
			int hp = (100 * (GET_HIT(bonded) / maxh));
			int sp = (100 * (GET_MANA(bonded) / maxs));
			int mv = (100 * (GET_MOVE(bonded) / maxm));

			message(ch, "\r\n");
			message(ch, "%s's condition: HP: %s SP: %s MV: %s\r\n", 
				GET_NAME(bonded), health(hp), mana(sp), moves(mv));
		}
	}
}

ACMD(do_inventory)
{
	send_to_char("You are carrying:\r\n", ch);
	list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
	int i, found = 0;
	char msg[MAX_INPUT_LENGTH];

	send_to_char("You are using:\r\n", ch);
  
	for (i = 0; i < NUM_WEARS; i++)
	{
		if (GET_EQ(ch, i))
		{
				
			if(i == WEAR_WIELD && IS_OBJ_STAT(GET_EQ(ch, i), ITEM_TWO_HANDED))
				strcpy(msg, "<wielded with two hands>    ");

			else
				strcpy(msg, where[i]);

			send_to_char(msg, ch);
			show_obj_to_char(GET_EQ(ch, i), ch, 1, 1);
			found = TRUE;

		}
	}
  
	if (!found) 
		send_to_char(" Nothing.\r\n", ch);
  
}

ACMD(do_time)
{
	const char *suf;
	int weekday, day;
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );


	sprintf(buf, "It is %d o'clock %s, on ",
	((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	((time_info.hours >= 12) ? "pm" : "am"));

	/* 35 days in a month */
	weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

	strcat(buf, weekdays[weekday]);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	day = time_info.day + 1;	/* day in [1..35] */

	suf = find_suf(day);

	message(ch, "The %d%s Day of the %s, Year %d.\r\n",
	day, suf, month_name[(int) time_info.month], time_info.year);

	message(ch, "Server: %s\r\n", asctime(timeinfo));

	if(countdown >= 0)
		message(ch, "THE MUD WILL REBOOT IN %d MINUTES.\r\n", countdown);

	if(PLR_FLAGGED(ch, PLR_NOWIZLIST) || GET_LEVEL(ch) >= LVL_GRGOD)
	{
		message(ch, "THE NEXT MUD TICK WILL BE IN %s%s%d%s SECONDS.\r\n",
			COLOR_BOLD(ch, CL_COMPLETE), COLOR_GREEN(ch, CL_COMPLETE), (60 - (seconds % 60)), COLOR_NORMAL(ch, CL_COMPLETE));
	}

}


ACMD(do_weather)
{
	const char *sky_look[] =
	{
		"cloudless",
		"cloudy",
		"rainy",
		"lit by flashes of lightning"
	};

	if (OUTSIDE(ch))
	{
		sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
		(weather_info.change >= 0 ? "you feel a warm wind from south" :
		"your foot tells you bad weather is due"));
		send_to_char(buf, ch);
	} 
	
	else
		send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


struct help_index_element *find_help(char *keyword)
{
	int i;

	for (i = 0; i < top_of_helpt; i++)
		if (isname(keyword, help_table[i].keywords))
			return (help_table + i);

	return nullptr;
}


ACMD(do_help)
{
	struct help_index_element *this_help;
	char entry[MAX_STRING_LENGTH];

	if (!ch->desc)
		return;

	skip_spaces(&argument);

	if (!*argument)
	{
		page_string(ch->desc, help, 0);
		return;
	}
  
	if (!help_table)
	{
		send_to_char("No help available.\r\n", ch);
		return;
	}

	if (!(this_help = find_help(argument)))
	{
		send_to_char("There is no help on that word.\r\n", ch);
		sprintf(buf, "HELP: %s tried to get help on %s", GET_NAME(ch), argument);
		log(buf);
		return;
	}

	if (this_help->min_level > GET_LEVEL(ch))
	{
		send_to_char("There is no help on that word.\r\n", ch);
		return;
	}

	sprintf(entry, "%s\r\n%s", this_help->keywords, this_help->entry);
  
	page_string(ch->desc, entry, 0);

}

/*                                                          *
 * This hardly deserves it's own function, but I see future *
 * expansion capabilities...                                *
 *                                               TR 8-18-98 *
 *                                          Fixed:  8-25-98 */
int show_on_who_list(struct descriptor_data *d)
{
	if ((STATE(d) != CON_PLAYING) && (STATE(d) != CON_MEDIT) &&
	(STATE(d) != CON_OEDIT) && (STATE(d) != CON_REDIT) &&
	(STATE(d) != CON_SEDIT) && (STATE(d) != CON_ZEDIT) &&
	(STATE(d) != CON_HEDIT) && (STATE(d) != CON_AEDIT) &&
	(STATE(d) != CON_TEXTED) && (STATE(d) != CON_TRIGEDIT))
		return 0;
  
	else
		return 1;
}


#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-q] [-r] [-z]\r\n"

ACMD(do_who)
{
	char_data *tch;
	char name_search[MAX_INPUT_LENGTH];
	char mode;
	size_t i;
	int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
	int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
	int who_room = 0;

	skip_spaces(&argument);
	strcpy(buf, argument);
	name_search[0] = '\0';

	while (*buf)
	{
		half_chop(buf, arg, buf1);
    
		if (isdigit(*arg))
		{
			sscanf(arg, "%d-%d", &low, &high);
			strcpy(buf, buf1);
		} 
		
		else if (*arg == '-')
		{
			mode = *(arg + 1);       /* just in case; we destroy arg in the switch */
			switch (mode)
			{
			
			case 'o':
			case 'k':
				outlaws = 1;
				strcpy(buf, buf1);
				break;
      
			case 'z':
				localwho = 1;
				strcpy(buf, buf1);
				break;
      
			case 's':
				short_list = 1;
				strcpy(buf, buf1);
				break;
      
			case 'q':
				questwho = 1;
				strcpy(buf, buf1);
				break;
      
			case 'l':
				half_chop(buf1, arg, buf);
				sscanf(arg, "%d-%d", &low, &high);
				break;
      
			case 'n':
				half_chop(buf1, name_search, buf);
				break;
      
			case 'r':
				who_room = 1;
				strcpy(buf, buf1);
				break;
      
			case 'c':
				half_chop(buf1, arg, buf);
	
				for (i = 0; i < strlen(arg); i++)
					showclass |= find_class_bitvector(arg[i]);
	
				break;
      
			default:
				send_to_char(WHO_FORMAT, ch);
				return;
			}				/* end of switch */

		} 
		
		else
		{			/* endif */
			send_to_char(WHO_FORMAT, ch);
			return;
		}
	}				/* end while (parser) */

	send_to_char("Players\r\n-------\r\n", ch);

	for (tch = character_list;tch;tch = tch->next)
	{

		if ((tch->desc && !show_on_who_list(tch->desc)) || IS_NPC(tch))
			continue;

   
		if(GET_LEVEL(ch) < LVL_IMMORT)
		{
	 
			if(GET_LEVEL(tch) < LVL_IMMORT)
			{

				if(GET_RACE(tch) != GET_RACE(ch))
					continue;
			}
		}
    
		if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
		!strstr(GET_TITLE(tch), name_search))
			continue;
    
		if (!CAN_SEE(ch, tch) && !PLR_FLAGGED(tch, AFF_HIDE) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
			continue;
    
		if (outlaws && !PLR_FLAGGED(tch, PLR_DARKFRIEND) &&
		!PLR_FLAGGED(tch, PLR_MURDERER))
			continue;
    
		if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
			continue;
    
		if (who_room && (tch->in_room != ch->in_room))
			continue;
		
		if (showclass && !(showclass & (1 << GET_CLASS(tch))))
			continue;
    
		if (short_list)
		{
			sprintf(buf, "%s%-12.12s%s%s",
			(GET_LEVEL(tch) >= LVL_IMMORT ? COLOR_YELLOW(ch, CL_SPARSE) : ""),
			GET_NAME(tch), (GET_LEVEL(tch) >= LVL_IMMORT ? COLOR_NORMAL(ch, CL_SPARSE) : ""),
			((!(++num_can_see % 4)) ? "\r\n" : ""));
			
			send_to_char(buf, ch);
		} 
		
		else
		{
			num_can_see++;
			sprintf(buf, "%s%s %s",
			(GET_LEVEL(tch) >= LVL_IMMORT ? COLOR_YELLOW(ch, CL_SPARSE) : ""),
			GET_NAME(tch), GET_TITLE(tch));

			if (GET_INVIS_LEV(tch))
				sprintf(buf + strlen(buf), " (i%d)", GET_INVIS_LEV(tch));
      
			else if (AFF_FLAGGED(tch, AFF_INVISIBLE))
				strcat(buf, " (invis)");

			if (PLR_FLAGGED(tch, PLR_MAILING))
				strcat(buf, " (mailing)");
      
			else if (tch->desc && (STATE(tch->desc) >= CON_OEDIT) && (STATE(tch->desc) <= CON_TEXTED))
				strcat(buf, " (OLC)");
      
			else if (PLR_FLAGGED(tch, PLR_WRITING))
				strcat(buf, " (writing)");

			if (!IS_NPC(tch) && PRF_FLAGGED(tch, PRF_BUILDWALK))
				strcat(buf, " (buildwalk)");
      
			if (PRF_FLAGGED(tch, PRF_NOTELL))
				strcat(buf, " (notell)");
      
			if (PRF_FLAGGED(tch, PRF_QUEST))
				strcat(buf, " (quest)");
      
			if (PLR_FLAGGED(tch, PLR_MURDERER))
				strcat(buf, " (MURDERER)");
      
			if (PLR_FLAGGED(tch, PLR_DARKFRIEND))
				strcat(buf, " (DARKFRIEND)");

			if (tch->wantedByPlayer(ch))
				strcat(buf, " (WANTED)");

			if(tch->char_specials.timer > idle_void)
				strcat(buf, " (idle)");

			if(!tch->desc)
				strcat(buf, " (linkless) ");
      
			if (GET_LEVEL(tch) >= LVL_IMMORT)
				strcat(buf, COLOR_NORMAL(ch, CL_SPARSE));

			if(tch->getLegend() > 0)
				sprintf(buf + strlen(buf), "		Legend: %d", tch->getLegend());
      
			strcat(buf, "\r\n");
			send_to_char(buf, ch);
		}				/* endif shortlist */
	}				/* end of for */
  
	if (short_list && (num_can_see % 4))
		send_to_char("\r\n", ch);
  
	if (num_can_see == 0)
		sprintf(buf, "\r\nNo-one at all!\r\n");
  
	else if (num_can_see == 1)
		sprintf(buf, "\r\nOne lonely character displayed.\r\n");
  
	else
		sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
  
	send_to_char(buf, ch);
	message(ch, "Today there was a high of %d player%s.\r\n", boot_high, boot_high == 1 ? "" : "s");
}




#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
	const char *format = "%3d %-7s %-12s %-14s %-3s %-8s ";
	char line[200], line2[220], idletime[10], classname[20];
	char state[30], *timeptr, mode;
	char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
	char_data *tch;
	struct descriptor_data *d;
	size_t i;
	int low = 0, high = LVL_IMPL, num_can_see = 0;
	int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

	host_search[0] = name_search[0] = '\0';

	strcpy(buf, argument);
  
	while (*buf)
	{

		half_chop(buf, arg, buf1);
	
		if (*arg == '-')
		{
			mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
			switch (mode)
			{
      
			case 'o':
			case 'k':
				outlaws = 1;
				playing = 1;
				strcpy(buf, buf1);
				break;
	
			case 'p':
				playing = 1;
				strcpy(buf, buf1);
				break;

			case 'd':
				deadweight = 1;
				strcpy(buf, buf1);
				break;

			case 'l':
				playing = 1;
				half_chop(buf1, arg, buf);
				sscanf(arg, "%d-%d", &low, &high);
					break;
	
			case 'n':
				playing = 1;
				half_chop(buf1, name_search, buf);
				break;

			case 'h':
				playing = 1;
				half_chop(buf1, host_search, buf);
				break;

			case 'c':
				playing = 1;
				half_chop(buf1, arg, buf);

				for (i = 0; i < strlen(arg); i++)
					showclass |= find_class_bitvector(arg[i]);

				break;

			default:
				send_to_char(USERS_FORMAT, ch);
				return;
			}	/* end of switch */

		}

		else
		{			/* endif */
			send_to_char(USERS_FORMAT, ch);
			return;
		}
	}				/* end while (parser) */

	strcpy(line,
		"Num Class   Name         State          Idl Login@   Site\r\n");

	strcat(line,
		"--- ------- ------------ -------------- --- -------- ------------------------\r\n");

	send_to_char(line, ch);

	one_argument(argument, arg);

	for (d = descriptor_list; d; d = d->next)
	{

		if (STATE(d) != CON_PLAYING && playing)
			continue;

		if (STATE(d) == CON_PLAYING && deadweight)
			continue;

		if (STATE(d) == CON_PLAYING)
		{
			if (d->original)
				tch = d->original;
		
			else if (!(tch = d->character))
				continue;

			if (*host_search && !strstr(d->host, host_search))
				continue;

			if (*name_search && str_cmp(GET_NAME(tch), name_search))
				continue;

			if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
				continue;

			if (outlaws && !PLR_FLAGGED(tch, PLR_DARKFRIEND) &&
			!PLR_FLAGGED(tch, PLR_MURDERER))
				continue;

			if (showclass && !(showclass & (1 << GET_CLASS(tch))))
				continue;

			if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
				continue;

			if (d->original)
				sprintf(classname, "[%2d %s]", GET_LEVEL(d->original),
				CLASS_ABBR(d->original));

			else
				sprintf(classname, "[%2d %s]", GET_LEVEL(d->character),
				CLASS_ABBR(d->character));
		}
		
		else
			strcpy(classname, "   -   ");

		timeptr = asctime(localtime(&d->login_time));
		timeptr += 11;
		*(timeptr + 8) = '\0';
	
		if (STATE(d) == CON_PLAYING && d->original)
			strcpy(state, "Switched");

		else
			strcpy(state, connected_types[STATE(d)]);

		if (d->character && STATE(d) == CON_PLAYING && GET_LEVEL(d->character) < LVL_GOD)
		sprintf(idletime, "%3d", d->character->char_specials.timer *
			SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);

		else
			strcpy(idletime, "");

		if (d->character && d->character->player.name)
		{
			if (d->original)
				sprintf(line, format, d->desc_num, classname,
				d->original->player.name, state, idletime, timeptr);

			else
				sprintf(line, format, d->desc_num, classname,
				d->character->player.name, state, idletime, timeptr);
		}
		
		else
			sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
			state, idletime, timeptr);

		if (d->host && *d->host)
		{
			if(LOCAL_IP(d) && d->character && !IS_KING(d->character))
				sprintf(line + strlen(line), "[]\r\n");

			else
				sprintf(line + strlen(line), "[%s]\r\n", d->host);
		}

		else
			strcat(line, "[Hostname unknown]\r\n");

		if (STATE(d) != CON_PLAYING)
		{
			sprintf(line2, "%s%s%s", COLOR_GREEN(ch, CL_SPARSE), line, COLOR_NORMAL(ch, CL_SPARSE));
			strcpy(line, line2);
		}

		if (STATE(d) != CON_PLAYING ||
		(STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character)))
		{
			send_to_char(line, ch);
			num_can_see++;
		}
	}

	sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
	send_to_char(line, ch);
}

void print_wizards(char_data *ch, int level)
{
	int i = 0, total = 0, idle = 0;
	wizlist_data *cur;
	char_data *imm;

	for(cur = wizlist;cur;cur = cur->next)
	{
		if(cur->level == level)
		{
			total++;
		}
	}

	imm = new char_data;
	clear_char(imm);

	for(cur = wizlist;cur;cur = cur->next)
	{

		if(cur->level == level)
		{
			i++;
			if(!(i % 5))
				message(ch, "\r\n					");

			/* Print the name in color depending on their idle time. */
			if(imm->load( (char *)cur->name.c_str()) && GET_LEVEL(ch) >= LVL_GRGOD)
			{
				idle = ((time(0) - LAST_LOGON(imm)) / SECS_PER_REAL_DAY);

				if(idle >= 3)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_CYAN(ch, CL_NORMAL));

				if(idle >= 7)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_GREEN(ch, CL_NORMAL));

				if(idle >= 14)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_YELLOW(ch, CL_NORMAL));

				if(idle >=21)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_DARK_RED(ch, CL_NORMAL));

				if(idle >= 28)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_RED(ch, CL_NORMAL));

				if(idle >= 56)
					message(ch, "%s%s", COLOR_BOLD(ch, CL_NORMAL), COLOR_BLUE(ch, CL_NORMAL));

				if(idle >= 102)
					message(ch, "%s", COLOR_CYAN(ch, CL_NORMAL));

				if(idle < 3)
					message(ch, "%s", NORMAL);
			}



			message(ch, "%s%s", cur->name.c_str(), NORMAL);

			if(i < total)
				message(ch, ", ");
		}
	}

	if(imm)
		delete [] imm;

	message(ch, "\r\n\n");
}

void print_wizlist(char_data *ch)
{

	message(ch, "			Below are a list of all of the Immortals of the Wheel.\r\n\n\n");

	
	message(ch, "						~Creators~\r\n					");
	print_wizards(ch, LVL_IMPL);
	
	message(ch, "						~Lords of the Wheel~\r\n					");
	print_wizards(ch, LVL_GRGOD);
	
	message(ch, "						~Tellers of the Wheel~\r\n					");
	print_wizards(ch, LVL_GOD);
	
	message(ch, "						~Apprentices of the Wheel~\r\n					");
	print_wizards(ch, LVL_APPR);

	message(ch, "						~Builders of the Wheel~\r\n					");
	print_wizards(ch, LVL_BLDER);
	
	message(ch, "						~Immortals of the Wheel~\r\n					");
	print_wizards(ch, LVL_IMMORT);
}

/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
	switch (subcmd) {
  
	case SCMD_CREDITS:
		page_string(ch->desc, credits, 0);
		break;
  
	case SCMD_NEWS:
		page_string(ch->desc, news, 0);
		break;
  
	case SCMD_INFO:
		page_string(ch->desc, info, 0);
		break;
  
	case SCMD_WIZLIST:
		print_wizlist(ch);
		break;
  
	case SCMD_IMMLIST:
		page_string(ch->desc, immlist, 0);
		break;
  
	case SCMD_HANDBOOK:
		page_string(ch->desc, handbook, 0);
		break;
  
	case SCMD_POLICIES:
		page_string(ch->desc, policies, 0);
		break;
  
	case SCMD_MOTD:
		page_string(ch->desc, motd, 0);
		break;
  
	case SCMD_IMOTD:
		page_string(ch->desc, imotd, 0);
		break;
	
	case SCMD_CLEAR:
		send_to_char("\033[H\033[J", ch);
		break;
  
	case SCMD_VERSION:
		break;
  
	default:
		return;
	
	}
}



void perform_mortal_where(char_data * ch, char *arg)
{
	char_data *i;
	descriptor_data *d;
	bool horse = FALSE;
	int num_trolls = 0, counter;
	player_clan_data *cl;
	char *msg = "You catch a switft breaze of the decay of shadowspawn.\r\n";

	if (!*arg)
	{
		send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
		for (d = descriptor_list; d; d = d->next)
		{

			if (!show_on_who_list(d))
				continue;

			if ((i = (d->original ? d->original : d->character)) == nullptr)
				continue;

			if (i->in_room == NOWHERE || !CAN_SEE(ch, i))
				continue;

			if (world[ch->in_room].zone != world[i->in_room].zone)
				continue;

			if (MOUNT(i) && IS_TROLLOC(ch) && !IS_FADE(i) && !IS_DREADLORD(i) &&
				!IS_FADE(ch) && !IS_DREADLORD(ch) && GET_LEVEL(i) < LVL_IMMORT)
			{
				horse = TRUE;
			
				if(ch->getClan(CLAN_DHAVOL) && GET_LEVEL(ch) < LVL_IMMORT)
					send_to_char("a horse			- Nearby\r\n", ch);
			}

			if(!IS_TROLLOC(ch) && IS_TROLLOC(i) && GET_LEVEL(i) < LVL_IMMORT)
				num_trolls++;

			if(GET_RACE(ch) != GET_RACE(i) && GET_LEVEL(i) <= LVL_IMMORT)
				continue;

			sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
			send_to_char(buf, ch);
		}
	} 
  
	else
	{			/* print only FIRST char, not all. */
		for (i = character_list; i; i = i->next)
		{

			if (i->in_room == NOWHERE || i == ch)
				continue;

			if ((GET_INVIS_LEV(i) > GET_LEVEL(ch) || world[i->in_room].zone != world[ch->in_room].zone)
				&& !str_cmp(GET_NAME(i), GET_BOND(ch)))
				continue;

			if (!isname(arg, i->player.name))
				continue;

			if(GET_RACE(ch) != GET_RACE(i) && GET_LEVEL(i) <= LVL_IMMORT)
				continue;

			sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
			send_to_char(buf, ch);
			return;
		}
    
		send_to_char("No-one around by that name.\r\n", ch);
	}

  	if(horse == TRUE && ch->getClan(CLAN_DHAVOL) && !IS_FADE(ch) && !IS_DREADLORD(ch) && GET_LEVEL(ch) < LVL_IMMORT)
		send_to_char("You hear and smell horses in the surrounding area.\r\n", ch);
	
	if(num_trolls > number(1, 30))
		send_to_char(msg, ch);

	if(ch->getClan(CLAN_WOLFBROTHER) && GET_LEVEL(ch) < LVL_IMMORT)
	{
		for(counter = 0;counter < num_trolls;counter++)
		{
			message(ch, "You smell shadowspawn nearby.\r\n");
		}
	}

	if((IS_WARDER(ch) || ch->AES_SEDAI()) && num_trolls > 0)
		message(ch, "You sense shadowspawn nearby.\r\n");

	/* Greyman target mark. */
	if(IS_GREYMAN(ch))
	{
		if(GET_MARKED(ch))
		{
			int distance = find_distance(world[GET_MARKED(ch)->in_room].zone, world[ch->in_room].zone);
			int slope = find_zone_slope(world[GET_MARKED(ch)->in_room].zone, world[ch->in_room].zone);

			if(distance <= MAX(4, ( (cl = ch->getClan(CLAN_SOULLESS)) ? cl->rank / 2 : 1)))
			{
				message(ch, "%s%s%-25s - %s %s%s\r\n", COLOR_BOLD(ch, CL_COMPLETE), COLOR_RED(ch, CL_COMPLETE),
				GET_NAME(GET_MARKED(ch)), dist[distance], loc[slope], COLOR_NORMAL(ch, CL_COMPLETE));
			}

			else
			{
				message(ch, "%s%s%-25s - Somewhere %s%s\r\n", COLOR_BOLD(ch, CL_COMPLETE),
				COLOR_RED(ch, CL_COMPLETE), GET_NAME(GET_MARKED(ch)),
				loc[slope], COLOR_NORMAL(ch, CL_COMPLETE));
			}
		}

		else
		{
			message(ch, "%s%sYou have no target marked.%s\r\n", COLOR_BOLD(ch, CL_COMPLETE),
			COLOR_RED(ch, CL_COMPLETE), COLOR_NORMAL(ch, CL_COMPLETE));
		}
	}
}


void print_object_location(int num, struct obj_data * obj, char_data * ch,
			        int recur)
{
	if (num > 0)
		sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
	 else
		sprintf(buf, "%33s", " - ");

	if (obj->in_room > NOWHERE)
	{
		sprintf(buf + strlen(buf), "[%5d] %s\r\n", GET_ROOM_VNUM(IN_ROOM(obj)), world[obj->in_room].name);
		send_to_char(buf, ch);
	} 
	
	else if (obj->carried_by)
	{
		sprintf(buf + strlen(buf), "carried by %s\r\n", PERS(obj->carried_by, ch));
		send_to_char(buf, ch);
	} 
	
	else if (obj->worn_by)
	{
		sprintf(buf + strlen(buf), "worn by %s\r\n", PERS(obj->worn_by, ch));
		send_to_char(buf, ch);
  } 
	
	else if (obj->in_obj)
	{
		sprintf(buf + strlen(buf), "inside %s%s\r\n", obj->in_obj->short_description, (recur ? ", which is" : " "));
		send_to_char(buf, ch);
    
		if (recur)
			print_object_location(0, obj->in_obj, ch, recur);
	} 
	
	else
	{
		sprintf(buf + strlen(buf), "in an unknown location\r\n");
		send_to_char(buf, ch);
	}
}



void perform_immort_where(char_data * ch, char *arg)
{
	char_data *i;
	struct obj_data *k;
	struct descriptor_data *d;
	int num = 0, found = 0;
	char color[300];

	if (!*arg)
	{
		send_to_char("Players\r\n-------\r\n", ch);
		
		for (d = descriptor_list; d; d = d->next)
			if (show_on_who_list(d))
			{
				i = (d->original ? d->original : d->character);
				
				if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE))
				{
					
					if (d->original)
						sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
						GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(d->character)),
						world[d->character->in_room].name, GET_NAME(d->character));
				else
				{
					if(IS_TROLLOC(i) && GET_LEVEL(i) < LVL_IMMORT)
						strcpy(color, YELLOW);
		  
					else if(IS_HUMAN(i) && GET_LEVEL(i) < LVL_IMMORT)
						strcpy(color, GREEN);
					else
						strcpy(color, NORMAL);

					sprintf(buf, "%s%-20s - [%5d] %s %s\r\n", color, GET_NAME(i),
					GET_ROOM_VNUM(IN_ROOM(i)), world[i->in_room].name, NORMAL);
					send_to_char(buf, ch);
				}
			}
		}
	} 
	
	else
	{
		for (i = character_list; i; i = i->next)
			if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name))
			{
				found = 1;
				sprintf(buf, "M%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
				GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
				send_to_char(buf, ch);
			}

		for (num = 0, k = object_list; k; k = k->next)
			if (k->name)
				if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name))
				{
					found = 1;
					print_object_location(++num, k, ch, TRUE);
				}

		if (!found)
			send_to_char("Couldn't find any such thing.\r\n", ch);
	}
}



ACMD(do_where)
{
  one_argument(argument, arg);

	if (GET_LEVEL(ch) >= LVL_GRGOD)
		perform_immort_where(ch, arg);
	
	else
		perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
	int i;

	if (IS_NPC(ch))
	{
		send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
		return;
	}
  
	*buf = '\0';

	for (i = 1; i < LVL_IMMORT; i++)
	{
		sprintf(buf + strlen(buf), "[%2d] %8d-%-8d", i,
		level_exp(i), level_exp(i+1) - 1);

		strcat(buf, "\r\n");
	}
  
	sprintf(buf + strlen(buf), "[%2d] %8d          : Immortality\r\n",
	LVL_IMMORT, level_exp(LVL_IMMORT));
	page_string(ch->desc, buf, 1);
}



ACMD(do_consider)
{
	char_data *victim;
	int diff;

	one_argument(argument, buf);

	if (!(victim = get_char_room_vis(ch, buf)))
	{
		send_to_char("Consider killing who?\r\n", ch);
		return;
	}
  
	if (victim == ch)
	{
		send_to_char("Easy!  Very easy indeed!\r\n", ch);
		return;
	}
  
	if (!IS_NPC(victim))
	{
		send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
		return;
	}
  
	diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

	if (diff <= -10)
		send_to_char("Now where did that chicken go?\r\n", ch);
  
	else if (diff <= -5)
		send_to_char("You could do it with a needle!\r\n", ch);
  
	else if (diff <= -2)
		send_to_char("Easy.\r\n", ch);
  
	else if (diff <= -1)
		send_to_char("Fairly easy.\r\n", ch);
  
	else if (diff == 0)
		send_to_char("The perfect match!\r\n", ch);
  
	else if (diff <= 1)
		send_to_char("You would need some luck!\r\n", ch);
  
	else if (diff <= 2)
		send_to_char("You would need a lot of luck!\r\n", ch);
  
	else if (diff <= 3)
		send_to_char("You would need a lot of luck and great equipment!\r\n", ch);
  
	else if (diff <= 5)
		send_to_char("Do you feel lucky, punk?\r\n", ch);
  
	else if (diff <= 10)
		send_to_char("Are you mad!?\r\n", ch);
 
	else if (diff <= 100)
		send_to_char("You ARE mad!\r\n", ch);

}



ACMD(do_diagnose)
{
	char_data *vict;

	one_argument(argument, buf);

	if (*buf) {
		if (!(vict = get_char_room_vis(ch, buf))) {
			send_to_char(NOPERSON, ch);
			return;
		} 
		
		else
			diag_char_to_char(vict, ch);
	} 
	
	else {
    
		if (FIGHTING(ch))
			diag_char_to_char(FIGHTING(ch), ch);
    
		else
			send_to_char("Diagnose who?\r\n", ch);
	}
}


char *ctypes[] = {
	"off",
	"sparse",
	"normal",
	"complete",
	"\n"
};

ACMD(do_color)
{
	int tp;

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (!*arg) {
		sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
		send_to_char(buf, ch);
		return;
	}
  
	if (((tp = search_block(arg, (const char **) ctypes, FALSE)) == -1)) {
		send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
		return;
	}
  
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
  
	if (tp & 1)
		SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
  
	if (tp & 2)
		SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
 
	sprintf(buf, "Your %scolor%s is now %s.\r\n", COLOR_RED(ch, CL_SPARSE),
	COLOR_NORMAL(ch, CL_OFF), ctypes[tp]);
	send_to_char(buf, ch);
}


ACMD(do_toggle)
{
	if (IS_NPC(ch))
		return;
  
	if (GET_WIMP_LEV(ch) == 0)
		strcpy(buf2, "OFF");
  
	else
		sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

	sprintf(buf,
		"Hit Pnt Display: %-3s    "
		"     Brief Mode: %-3s    "
		" Summon Protect: %-3s\r\n"

		"   Move Display: %-3s    "
		"   Compact Mode: %-3s    "
		"       On Quest: %-3s\r\n"

		"   Mana Display: %-3s    "
		"         NoTell: %-3s    "
		"   Repeat Comm.: %-3s\r\n"

		" Auto Show Exit: %-3s    "
		"     Wimp Level: %-3s\r\n"

		"Narrate Channel: %-3s    "
		"   Chat Channel: %-3s    "
		"    Color Level: %-3s\r\n"
		"      Buildwalk: %-3s\r\n",

		ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
		ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
		ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

		ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
		ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
		YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

		ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
		ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
		YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

		ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
		buf2,

		ONOFF(!PRF_FLAGGED(ch, PRF_NONARR)),
		ONOFF(!PRF_FLAGGED(ch, PRF_NOCHAT)),

		ctypes[COLOR_LEV(ch)],
		ONOFF(!PRF_FLAGGED(ch, PRF_BUILDWALK)));

	send_to_char(buf, ch);
}


struct sort_struct
{
	int sort_pos;
	cbyte is_social;
}	*cmd_sort_info = nullptr;

int num_of_cmds;


void sort_commands(void)
{
	int a, b, tmp;

	 num_of_cmds = 0;

	   /*
		* first, count commands (num_of_commands is actually one greater than the
		* number of commands; it inclues the '\n'.
		*/
  
	while (*complete_cmd_info[num_of_cmds].command != '\n')
		num_of_cmds++;

	/* check if there was an old sort info.. then free it -- aedit -- M. Scott*/
	if (cmd_sort_info)
		delete(cmd_sort_info);
  
	/* create data array */
	CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

	/* initialize it */
	
	for (a = 1; a < num_of_cmds; a++) {
		cmd_sort_info[a].sort_pos = a;
		cmd_sort_info[a].is_social = (complete_cmd_info[a].command_pointer == do_action);
	}

	/* the infernal special case */
	cmd_sort_info[find_command("insult")].is_social = TRUE;

	/* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
	for (a = 1; a < num_of_cmds - 1; a++)
		for (b = a + 1; b < num_of_cmds; b++)
			if (strcmp(complete_cmd_info[cmd_sort_info[a].sort_pos].command,
			complete_cmd_info[cmd_sort_info[b].sort_pos].command) > 0)
			{
	
				tmp = cmd_sort_info[a].sort_pos;
				cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
				cmd_sort_info[b].sort_pos = tmp;
			}
}

ACMD(do_commands)
{
	int no, i, cmd_num;
	int wizhelp = 0, socials = 0;
	char_data *vict;

	one_argument(argument, arg);

	if (*arg)
	{
		if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict))
		{
			send_to_char("Who is that?\r\n", ch);
			return;
		}
    
		if (GET_LEVEL(ch) < GET_LEVEL(vict))
		{
			send_to_char("You can't see the commands of people above your level.\r\n", ch);
			return;
		}
  
	} 
	
	else
		vict = ch;

	if (subcmd == SCMD_SOCIALS)
		socials = 1;
	
	else if (subcmd == SCMD_WIZHELP)
		wizhelp = 1;

	sprintf(buf, "The following %s%s are available to %s:\r\n",
		wizhelp ? "privileged " : "",
		socials ? "socials" : "commands",
		vict == ch ? "you" : GET_NAME(vict));

	/* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  
	for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++)
	{
		i = cmd_sort_info[cmd_num].sort_pos;
		
		if (complete_cmd_info[i].minimum_level >= 0 &&
		GET_LEVEL(vict) >= complete_cmd_info[i].minimum_level &&
		(complete_cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
		(wizhelp || socials == cmd_sort_info[i].is_social))
		{

			sprintf(buf + strlen(buf), "%-11s", complete_cmd_info[i].command);
			if (!(no % 7))
				strcat(buf, "\r\n");
			
			no++;
		}
	}

	strcat(buf, "\r\n");
	send_to_char(buf, ch);
}
