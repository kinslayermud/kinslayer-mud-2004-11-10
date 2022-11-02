/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "olc.h"
#include "dg_scripts.h"

/*   external vars  */
extern memory_data *mlist;
extern clan_data clan_list[50];
extern time_info_data time_info;
extern room_data *world;
extern char_data *character_list;
extern obj_data *object_list;
extern descriptor_data *descriptor_list;
extern index_data *mob_index;
extern index_data *obj_index;
extern zone_data *zone_table;
extern player_index_element *player_table;
extern attack_hit_type attack_hit_text[];
extern trig_data *trigger_list;
extern const char *class_abbrevs[];
extern class note_data *note_head;
extern char *race_abbrevs[];
extern obj_data *Obj_from_store(obj_file_elem object);
extern time_t boot_time;
extern int top_of_zone_table;
extern int circle_shutdown, circle_reboot;
extern int circle_restrict;
extern int load_into_inventory;
extern int top_of_world;
extern int buf_switches, buf_largecount, buf_overflows;
extern int top_of_mobt;
extern int top_of_objt;
extern int countdown;
extern int rank_req[14];
extern struct track_data *track_list;
extern struct watch_data *watch_head;
extern struct obj_data *obj_proto;
extern char_data *mob_proto;
extern void check_autowiz(char_data *ch);
extern char mudname[256];
extern int mother_desc;
extern long top_id;

/* for chars */
extern char *credits;
extern char *news;
extern char *motd;
extern char *imotd;
extern char *help;
extern char *info;
extern char *background;
extern char *policies;
extern char *startup;
extern char *handbook;
extern const char *spells[];
extern const char *pc_class_types[];
extern const char *pc_race_types[];

/* extern functions */
extern void show_shops(char_data * ch, char *value);
extern void hcontrol_list_houses(char_data *ch);
extern void do_start(char_data *ch);
extern void appear(char_data *ch);
extern void reset_zone(int zone);
extern void roll_real_abils(char_data *ch);
extern void update_legend(char_data *ch);
extern void save_legends(void);
extern void obj_from_char(struct obj_data *obj);
extern void remove_player_clan(char_data *ch, int number);
extern void save_player_clan(char_data *ch, int number);
extern void remove_object(struct obj_data *obj);
extern void save_notes(void);
extern void save_ideas(void);
extern int level_exp(int level);
extern int parse_class(char arg);
extern int parse_race(char arg);
extern int Crash_load(char_data *ch, int show);
extern int save_all(void);
extern int abs_find(char_data *ch);
extern int dodge_find(char_data *ch);
extern int parry_find(char_data *ch);
extern int offense_find(char_data *ch);
extern int can_edit_zone(char_data *ch, int number);
extern char_data *find_char(long n);
extern char *PERS(char_data *ch, char_data *vict);
extern int prep_execl();

extern list <NoteData> notes;
extern list <NoteData> mob_notes;
extern class Ideas *idea_list;
extern FILE *player_fl;

int zone_rnum();
//ACMD(do_saveolc);
//ACMD(do_saveall);

/* local functions */
ACMD(do_advance);
ACMD(do_at);
ACMD(do_award);
ACMD(do_awardd);
ACMD(do_awardh);
ACMD(do_clan);
ACMD(do_council);
ACMD(do_countdown);
ACMD(do_copyover);
ACMD(do_cqpurge);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_demote);
ACMD(do_dig);
ACMD(do_disable);
ACMD(do_echo);
ACMD(do_enable);
ACMD(do_find);
ACMD(do_force);
ACMD(do_gecho);
ACMD(do_goto);
ACMD(do_invis);
ACMD(do_ipfind);
ACMD(do_lag);
ACMD(do_last);
ACMD(do_legupdate);
ACMD(do_load);
ACMD(do_memory);
ACMD(do_note);
ACMD(do_pardon);
ACMD(do_pfind);
ACMD(do_poofset);
ACMD(do_purge);
ACMD(do_qval);
ACMD(do_rank);
ACMD(do_reset);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_send);
ACMD(do_set);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_snoop);
ACMD(do_statfind);
ACMD(do_swap);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_teleport);
ACMD(do_trans);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_unbond);
ACMD(do_warrant);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wiznoise);
ACMD(do_wizutil);
ACMD(do_zap);
ACMD(do_zcmd_find);
ACMD(do_zreset);

room_rnum find_target_room(char_data * ch, char *rawroomstr);
int perform_set(char_data *ch, char_data *vict, int mode, char *val_arg, int file);
void perform_immort_invis(char_data *ch, int level);
void do_stat_room(char_data * ch);
void do_stat_object(char_data * ch, struct obj_data * j);
void do_stat_character(char_data * ch, char_data * k);
void stop_snooping(char_data * ch);
void print_zone_to_buf(char *bufptr, int zone);
void perform_immort_vis(char_data *ch);
void perform_warrant(char_data *ch, char_data *victim, int clan);
void reset_skills(char_data *ch);
void delevel(char_data *ch);
void redit_save_internally(descriptor_data *d);
int can_communicate(char_data *speaker, char_data *receiver);
int clanByName(char *name);

// Tulon: 7-21-2004 - Finding zone commands */
ACMD(do_zcmd_find)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int i = 0, vnum = 0, cmd_no = 0, room = 0, mnum = 0, tt = 0, ovnum = 0, kit = 0;
	struct kit_data *kitp;

	two_arguments(argument, arg1, arg2);

	if(!argument || !*arg1 || !*arg2)
	{
		message(ch, "Syntax: zcmdfind <type> <vnum/name>\r\n");
		return;
	}

	vnum = atoi(arg2);

	if(!strn_cmp(arg1, "object", strlen(arg1)))
	{
		for(i = 0;i < top_of_zone_table;i++)
		{
			for (cmd_no = 0; zone_table[i].cmd[cmd_no].command != 'S'; cmd_no++)
			{
				switch(zone_table[i].cmd[cmd_no].command)
				{

				case 'M':
					room = GET_ROOM_VNUM(zone_table[i].cmd[cmd_no].arg3);

					mnum = real_mobile(zone_table[i].cmd[cmd_no].arg1);

					if (mnum == -1)
						continue;

					if ( (kitp = mob_proto[mnum].mob_specials.primary_kit))
					{
						for(tt = 0; tt < NUM_WEARS; tt++)
						{
							if (kitp->equipment[0][tt] != NOTHING)
							{
								for(kit = 0; kit < NUM_OF_KITS; kit++)
								{
									if (kitp->equipment[kit][tt] == NOTHING)
										continue;

									ovnum = real_object(kitp->equipment[kit][tt]);

									if (ovnum < 0)
										continue;

									if (kitp->equipment[kit][tt] != vnum || !isname(arg2, obj_proto[ovnum].name))
										continue;

									message(ch, "%s loads on kit %d (%d%%) on mob %d (%d%%) in room %d.\r\n",
										obj_proto[ovnum].short_description, kitp->percent[kit][tt],
										kitp->vnum, mnum, zone_table[i].cmd[cmd_no].arg2, room);
								}
							}
						}
					}

					break;

				case 'E':
					if(GET_OBJ_VNUM(&obj_proto[zone_table[i].cmd[cmd_no].arg1]) == vnum ||
						is_name(arg2, (&obj_proto[zone_table[i].cmd[cmd_no].arg1])->name))
					{
						message(ch, "%s loads on a mob on a %d%% chance in room %d.\r\n",
						(&obj_proto[zone_table[i].cmd[cmd_no].arg1])->short_description,
						zone_table[i].cmd[cmd_no].arg2, room);
					}

					break;

				case 'O':
					if(GET_OBJ_VNUM(&obj_proto[zone_table[i].cmd[cmd_no].arg1]) == vnum ||
						is_name(arg2, (&obj_proto[zone_table[i].cmd[cmd_no].arg1])->name))
					{
						message(ch, "%s loads on the ground on a %d%% chance in room %d.\r\n",
							(&obj_proto[zone_table[i].cmd[cmd_no].arg1])->short_description,
							zone_table[i].cmd[cmd_no].arg2,
							GET_ROOM_VNUM(zone_table[i].cmd[cmd_no].arg3));
					}

					break;
				}
			}
		}
	}
}

// Serai - 06/18/04 - Copyover for Kinslayer.
ACMD(do_copyover)
{
	skip_spaces(&argument);

	if (str_cmp(argument, "now"))
	{
		send_to_char("To start a copyover, type \"copyover now\".  There is no timer.\r\n", ch);
		return;
	}

	send_to_all("Warning:  Copyover started.  You will not be disconnected.\r\n");

	sprintf(buf, "Copyover started by %s.", GET_NAME(ch));
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

	if (prep_execl())
	{
		// Serai - chdir(..) because CircleMUD does a chdir() into their data directory
		//         which is by default lib/ ... If you specify another directory copyover
		//         will probably fail.
		if (chdir("..") == -1)
		{
			sprintf(buf, "Copyover failed:  chdir(..) with error code %d.", errno);
		}
		else
		{
			execl(mudname, mudname, nullptr);
			sprintf(buf, "Copyover failed:  execl(%s, %s, nullptr) with error code %d.", mudname, mudname, errno);
		}

		if (chdir("lib") == -1)
		{
			log("execl() failed, and we can't find your data directory again...  Quitting.");
			circle_shutdown = 1;

			return;
		}

		mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
	}

	if (remove(".copyover") != 0)
		log("SYSERR: Could not remove(.copyover)!  Remove it manually!");

	send_to_all("Copyover failed.\r\n");

	return;
}

void reset_skills(char_data *ch)
{

	int i = 0;

	if(GET_LEVEL(ch) > 30)
	{

		if(IS_TROLLOC(ch))
			GET_PRACTICES(ch) = 30 * 4;
		
		else
			GET_PRACTICES(ch) = 30 * 5;

		GET_PRACTICES(ch) += (GET_LEVEL(ch) - 30);
	
		GET_SPRACTICES(ch) = (30 * 2) + (GET_LEVEL(ch) - 30);

	}

	else
	{
		if(IS_TROLLOC(ch))
			GET_PRACTICES(ch) = GET_LEVEL(ch) * 4;

		else
			GET_PRACTICES(ch) = GET_LEVEL(ch) * 5;

		GET_SPRACTICES(ch) = GET_LEVEL(ch) * 2;
	}

	for(i = 0;i < MAX_SKILLS;i++)
		GET_SKILL(ch, i) = 0;
}

/*
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}

ACMD(do_wiznoise)
{

	char arg1[MAX_INPUT_LENGTH], *msg = new char[MAX_STRING_LENGTH];
	int distance;

	half_chop(argument, arg1, msg);
	skip_spaces(&msg);

	if(!*argument)
	{
		send_to_char("Correct format: wiznoise <distance> <message>\r\n", ch);
		return;
	}

	if(!*msg)
	{
		send_to_char("What message do you want to be sent out?\r\n", ch);
		return;
	}

	distance = atoi(arg1);

	if(distance <= 0)
	{
		send_to_char("Distance must be above zero.\r\n", ch);
		return;
	}

	perform_group_noise(nullptr, ch->in_room, 0, distance, msg, -1);
	send_to_char("The noise has been sent!\r\n", ch);
}
*/

/* Tulon: 7-3-2004 */
ACMD(do_unbond)
{

	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if(!argument || !*arg)
	{
		message(ch, "Unbond who\r\n");
		return;
	}

	//unbond(arg);
	message(ch, OK);

}

ACMD(do_cqpurge)
{
}

ACMD(do_qval)
{
}

ACMD(do_wiznoise)
{
	char arg1[MAX_INPUT_LENGTH], *msg = new char[MAX_STRING_LENGTH];
	int distance;

	half_chop(argument, arg1, msg);
	skip_spaces(&msg);
	delete_doubledollar(msg);

	if(!*argument)
	{
		send_to_char("Correct format: wiznoise <distance> <message>\r\n", ch);
		delete[] msg;
		return;
	}

	if(!*msg)
	{
		send_to_char("What message do you want to be sent out?\r\n", ch);
		delete[] msg;
		return;
	}

	distance = atoi(arg1);

	if(distance <= 0)
	{
		send_to_char("Distance must be above zero.\r\n", ch);
		delete[] msg;
		return;
	}

	perform_group_noise(ch->in_room, distance, msg, -1);
	send_to_char("The noise has been sent!\r\n", ch);

	delete[] (msg);
	return;
}

//Coded by Tulon on February 6th, 2004. Calculates variouse memory types in the program.//
// Minor changes by Serai...
ACMD(do_memory)
{
	int number, total = 0, size = 0, i = 0;
	char_data *vict;
	struct obj_data *obj;
	struct trig_data *t;
	struct track_data *trax;
	class Ideas *idea;
	struct watch_data *watch;
	struct clan_player_data *pl;


	message(ch, "These are the different memory sizes for the game structures:\r\n\n", ch);

	for(number = 0, vict = character_list;vict;vict = vict->next)
		number++;

	/*CHARACTERS*/
	message(ch, "char_data:        Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(char_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(char_data)), COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(char_data) * number;


	/*OBJECTS*/
	for(number = 0, obj = object_list;obj;obj = obj->next)
		number++;

	message(ch, "obj_data:         Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(struct obj_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(struct obj_data)), COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(struct obj_data) * number;


	/*ROOMS*/
	message(ch, "room_data:        Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof( room_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), top_of_world + 1, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB((top_of_world + 1) * sizeof( room_data)),
		COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(room_data) * (top_of_world + 1);


	/*TRIGGERS*/
	for(number = 0, t = trigger_list;t;t = t->next_in_world)
		number++;

	message(ch, "trig_data:        Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof( trig_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof( trig_data)), COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(struct trig_data) * number;


	/* TRACKS */
	for(number = 0, trax = track_list;trax;trax = trax->next)
		number++;

	message(ch, "track_data:       Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(track_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(track_data)), COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(struct track_data) *number;

	/* WATCH LIST */
	for(number = 0, watch = watch_head;watch;watch = watch->next)
		number++;

	message(ch, "watch_data:       Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(watch_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(watch_data)), COLOR_NORMAL(ch, CL_NORMAL));

	total += sizeof(struct watch_data) *number;

	
	/* IDEAS */
	for(number = 0, idea = idea_list;idea;idea = idea->next)
	{
		size += sizeof(idea->idea);
		size += sizeof(idea->date);
		size += sizeof(idea->poster);
		number++;
	}

	message(ch, "idea_data:        Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof( Ideas), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(Ideas)), COLOR_NORMAL(ch, CL_NORMAL));


	total += size;
	total += sizeof(Ideas) *number;

	/* CLANS */
	for(number = 0, i = 0;i < NUM_CLANS;i++)
	{
		for(pl = clan_list[i].players;pl;pl = pl->next)
		{
			number++;
		}
	}

	message(ch, "clan_data:        Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(clan_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), NUM_CLANS, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(NUM_CLANS * sizeof(clan_data)), COLOR_NORMAL(ch, CL_NORMAL));

	message(ch, "clan_player_data: Single: [%s%5i%s] bytes, Number: [%s%5d%s], Total: [%s%10f%s] MB.\r\n",
		COLOR_GREEN(ch, CL_NORMAL), sizeof(clan_player_data), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), number, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(number * sizeof(clan_player_data)), COLOR_NORMAL(ch, CL_NORMAL));

	/* TOTAL */
	message(ch, "\r\nTotal:                    [%s%9d%s] bytes, [%s%13f%s] KB, [%s%10f%s] MB.\r\n\n",
		COLOR_GREEN(ch, CL_NORMAL), total, COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), KB(total), COLOR_NORMAL(ch, CL_NORMAL),
		COLOR_GREEN(ch, CL_NORMAL), MB(total), COLOR_NORMAL(ch, CL_NORMAL));

//	message(ch, "Other memory:\r\n");

//	message(ch, "obj_file_elem: Single size: [%s%8f%s] MB.\r\n",
//		COLOR_GREEN(ch, CL_NORMAL), (float) MB(sizeof(struct obj_file_elem)), COLOR_NORMAL(ch, CL_NORMAL));

//	message(ch, "char_file_u:   Single size: [%s%8f%s] MB.\r\n",
//		COLOR_GREEN(ch, CL_NORMAL), (float) MB(sizeof(struct char_file_u)), COLOR_NORMAL(ch, CL_NORMAL));
}


ACMD(do_wizdelete) 
{
	int count = 1;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	list<NoteData>::iterator cur;

	two_arguments(argument, arg1, arg2);

	if(!*arg1)
	{
		send_to_char("You can delete: Notes, Ideas, MobNotes\r\n", ch);
		return;
	}

	if(!strn_cmp(arg1, "notes", strlen(arg1)))
	{

		for(cur = notes.begin(); cur != notes.end(); cur++, count++)
		{		
			if(count == atoi(arg2))
			{
				notes.erase(cur);
				save_notes();
				message(ch, "Note %d has been removed.\r\n", count);
				sprintf(buf, "%s removed Note %d.", GET_NAME(ch), count);
				mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
				return;
			}
		}

		send_to_char("That is an invalid member of the note list.\r\n", ch);
		return;
	}

	else if(!strn_cmp(arg1, "mobnotes", strlen(arg1)))
	{
		for(cur = mob_notes.begin(); cur != mob_notes.end(); cur++, count++)
		{		
			if(count == atoi(arg2))
			{
				mob_notes.erase(cur);
				save_notes();
				message(ch, "Mob Note %d has been removed.\r\n", count);
				sprintf(buf, "%s removed Mob Note %d.", GET_NAME(ch), count);
				mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
				return;
			}
		}

		send_to_char("That is an invalid member of the note list.\r\n", ch);
		return;
	}

	else if(!strn_cmp(arg1, "ideas", strlen(arg1)))
	{

		class Ideas *idea, *temp;

		for(count = 1, idea = idea_list;idea;idea = idea->next, count++)
			if(count == atoi(arg2))
			{
				REMOVE_FROM_LIST(idea, idea_list, next);
				message(ch, "Idea number %d has been removed.\r\n", count);
				sprintf(buf, "%s removed idea %d.", GET_NAME(ch), count);
				mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
//				delete[] (idea);
				delete (idea);
				save_ideas();
				return;
			}
	}

	else 
	{
		send_to_char("Valid arguments: Notes, Ideas\r\n", ch);
		return;
	}

	message(ch, "Value number %d has not been found.\r\n", count);
}

ACMD(do_note)
{
	class NoteData cur;

	skip_spaces(&argument);

	if(GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch))
	{
		send_to_char("What!?!\r\n", ch);
		return;
	}

	cur.message = argument;
	cur.poster = GET_NAME(ch);
	cur.npc = IS_NPC(ch) ? 1 : 0;

	if(!cur.npc)
		notes.insert(notes.end(), cur);

	else
		mob_notes.insert(mob_notes.end(), cur);

	save_notes();

	if(!IS_NPC(ch))
	{
		sprintf(buf, "%s has added a note.", GET_NAME(ch));
		mudlog(buf, NRM, MAX(LVL_BLDER, GET_INVIS_LEV(ch)), TRUE);
	}

	send_to_char("You add the note.\r\n", ch);
}

void print_chest_log(char_data *ch, int vnum)
{
	bool found = FALSE;
	FILE *file;
	char line[MAX_STRING_LENGTH];
	int val = 0;

	if(!(file = fopen(CHEST_LOG_FILE, "r")))
	{
		mudlog("Error opening chest log file for read.", NRM, MAX(GET_LEVEL(ch), GET_INVIS_LEV(ch)), TRUE);
		return;
	}

	*buf = '\0';

	while(fgets(line, MAX_STRING_LENGTH, file))
	{
		val = atoi(line);

		if(fgets(line, MAX_STRING_LENGTH, file) == nullptr)
		{
			log("Line: %s.", line);
			break;
		}

		if((strlen(buf) + strlen(line)) > (MAX_STRING_LENGTH * 100))
		{
			log("Length: %d.", strlen(buf) + strlen(line));
			break;
		}

		if(val == vnum)
		{
			found = TRUE;
			sprintf(buf + strlen(buf), "%s", line);
		}
	}

	if(found == FALSE)
	{
		message(ch, "No entries in log for chest %d.\r\n", vnum);
	}

	else
	{
		if(ch->desc)
		{
			page_string(ch->desc, buf, TRUE);
		}
	}

	fclose(file);

}

ACMD(do_wizview)
 {
	int count = 1;
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	list<NoteData>::iterator cur;

	two_arguments(argument, arg, arg2);

	if(!*arg) 
	{
		send_to_char("You can view: Notes, MobNotes, Ideas\r\n", ch);
		return;
	}

	if(!strn_cmp(arg, "notes", strlen(arg)))
	{

		send_to_char("Note List:\r\n", ch);

		for(cur = notes.begin(); cur != notes.end(); cur++, count++)
		{
				sprintf(buf2, "%d:	%s -     %s\r\n", count, cur->poster.c_str(), cur->message.c_str());
				page_string(ch->desc, buf2, 1);
		}
	}

	else if(!strn_cmp(arg, "MobNotes", strlen(arg)))
	{
		send_to_char("Mob Note List:\r\n", ch);

		for(cur = mob_notes.begin();cur != mob_notes.end();cur++, count++)
		{

			sprintf(buf2, "%d:	%s -	%s\r\n", count, cur->poster.c_str(), cur->message.c_str());
			page_string(ch->desc, buf2, 1);
		}
	}

	else if(!strn_cmp(arg, "ideas", strlen(arg)))
	{
		class Ideas *idea;

		send_to_char("Idea List:\r\n", ch);

		for(count = 1, idea = idea_list;idea;idea = idea->next, count++)
		{
			sprintf(buf2, "%d:   %s: (%s), (%d) -		%s\r\n", count, idea->poster.c_str(), idea->date.c_str(), idea->room, idea->idea.c_str());
			page_string(ch->desc, buf2, 1);
		}
	}

	else if(!strn_cmp(arg, "chestlog", strlen(arg)))
	{
		print_chest_log(ch, atoi(arg2));
	}

	else
	{
		send_to_char("Invalid Option.\r\n", ch);
		return;
	}
}

ACMD(do_ipfind)
{

	int i = 0, count = 0;
	char_data *player1, *player2;
	player_index_element *entry;
	char name[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH], host[HOST_LENGTH+1];
	int lev;
	
	two_arguments(argument, name, level);

	if(!*name)
	{
		send_to_char("Usage: ipfind <name> <minimum level>\r\n", ch);
		return;
	}

	CREATE(player1, char_data, 1);
	CREATE(player2, char_data, 1);
	clear_char(player1);
	clear_char(player2);

	if(!player1->load(name))
	{
		message(ch, "There is no one by that name.\r\n");
		return;
	}

	if(!*level || !atoi(level))
		lev = 0;

	lev = atoi(level);
	message(ch, "%s's alts are:\r\n", CAP(name));

	for(entry = player_table;entry;entry = entry->next)
	{
		if (player2->load(entry->name))  
		{
			strcpy(host, player2->points.host);

			if(IS_LOCALHOST(host))
				break;

			if(!*host)
				continue;

			if(!str_cmp(host, player1->points.host) && GET_LEVEL(player2) >= lev)
			{
				if(count)
				{
					message(ch, ", ");
				}

				if(!(count % 5))
				{
					message(ch, "\r\n");
				}
			
				count++;
				message(ch, "%s", GET_NAME(player2));
			}
		}
	}

	free_char(player1);
	free_char(player2);

	message(ch, "\r\n");

}

ACMD(do_extra)
{
	sprintf(buf, "Random(1,6) : %d, Random(1,6) : %d\r\n", number(1,6), number(1,6));
	send_to_char(buf, ch);
}

ACMD(do_lag)
{

	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char_data *victim;
	int number = 0;

	two_arguments(argument, arg1, arg2);

	if(!argument || !*arg1 || !*arg2 || atoi(arg2) <= 0)
	{
		send_to_char("Hmm...\r\n", ch);
		return;
	}

	number = atoi(arg2);
	
	if(!(victim = get_char_vis(ch, arg1)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	message(ch, "%s lagged for %d seconds.\r\n", GET_NAME(victim), number);

	WAIT_STATE(victim, number * PASSES_PER_SEC);
}

void perform_pardon(char_data *ch, char_data *victim, int num)
{
	REMOVE_BIT_AR(GET_WARRANTS(victim), num);
	act("$N has been pardoned.", FALSE, ch, 0, victim, TO_CHAR);
	message(victim, "You have been pardoned by the %s clan!\r\n", clan_list[num].name);
	victim->save();
}

void perform_warrant(char_data *ch, char_data *victim, int num)
{
	SET_BIT_AR(GET_WARRANTS(victim), num);
	act("$N has been warranted.", FALSE, ch, 0, victim, TO_CHAR);
	message(victim, "You have been warranted by the %s clan!\r\n", clan_list[num].name);
	victim->save();
}

ACMD(do_disable)
{
	int count = 0, number = 0;
	char com[MAX_INPUT_LENGTH];

	one_argument(argument, com);

	if(!*argument)
	{
		send_to_char("Commands that are currently disabled:\r\n\n", ch);
		
		for(count = 0, number = 0;*complete_cmd_info[count].command != '\n';count++)
		{
			if(complete_cmd_info[count].disable == 1)
			{
				number++;
				sprintf(buf, "%d: %s\r\n", number, complete_cmd_info[count].command);
				send_to_char(buf, ch);
			}
		}

		return;
	}

	for(count = 0;*complete_cmd_info[count].command != '\n';count++)
		if(!strn_cmp( (const char *) complete_cmd_info[count].command, com, strlen(com)))
			if(complete_cmd_info[count].disable == 0 && str_cmp(complete_cmd_info[count].command, "enable"))
			{
				sprintf(buf, "Command %s has been disabled.\r\n", complete_cmd_info[count].command);
				send_to_char(buf, ch);
				complete_cmd_info[count].disable = 1;
				return;
			}

			else
			{
				send_to_char("That command is already disabled, or is invalid.\r\n", ch);
				return;
			}

	sprintf(buf, "%s is not a valid command!\r\n", com);
	send_to_char(buf, ch);
}

ACMD(do_enable)
{
	int count = 0;
	char com[MAX_INPUT_LENGTH];

	one_argument(argument, com);

	for(count = 0;*complete_cmd_info[count].command != '\n';count++)
		if(!strn_cmp( (const char *) complete_cmd_info[count].command, com, strlen(com)))
			if(complete_cmd_info[count].disable)
			{
				sprintf(buf, "Command %s is now enabled.\r\n", complete_cmd_info[count].command);
				send_to_char(buf, ch);
				complete_cmd_info[count].disable = 0;
				return;
			}

			else
			{
				send_to_char("That command is already avaliable.\r\n", ch);
				return;
			}

	sprintf(buf, "%s is not a valid command!\r\n", com);
	send_to_char(buf, ch);
}

ACMD(do_pardon)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char_data *victim;
    int count = 0;

	two_arguments(argument, arg, arg2);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("What!?!\r\n", ch);
		return;
	}
	
	if(!(victim = get_char_vis(ch, arg)))
	{
		send_to_char("They couldn't be found.\r\n", ch);
		return;
	}

	for(count = 1;count <= NUM_CLANS;count++)
		if(!strn_cmp(clan_list[count].name, arg2, strlen(arg2)))
		{
			perform_pardon(ch, victim, count);
			victim->save();

			sprintf(buf, "%s has pardoned %s.", GET_NAME(ch), GET_NAME(victim));
			mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			return;
		}

	send_to_char("That is an invalid clan.\r\n", ch);
	return;
}
	
ACMD(do_warrant) 
{
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char_data *victim;
    int count = 0;

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("What!?!\r\n", ch);
		return;
	}

	two_arguments(argument, arg, arg2);
	
	if(!(victim = get_char_vis(ch, arg)))
	{
		send_to_char("They couldn't be found.\r\n", ch);
		return;
	}

	if((count = clanByName(arg2)))
	{
		perform_warrant(ch, victim, count);
		victim->save();

		sprintf(buf, "%s warranted %s for clan %s.", GET_NAME(ch), GET_NAME(victim), clan_list[count].name);
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
	}
	else
		send_to_char("That is an invalid clan.\r\n", ch);
}

ACMD(do_zap)
{
	char_data *victim;
	int ammount = 0, i;
	char arg[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH];

	if(!*argument)
	{
		send_to_char("Usage: Delevel player level\r\n", ch);
		return;
	}

	two_arguments(argument, arg, level);

	if(!*level)
	{
		send_to_char("You must input a level\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, arg)))
	{
		send_to_char("They are not currently online.\r\n", ch);
		return;
	}

	ammount = atoi(level);

	if(ammount >= GET_LEVEL(victim))
	{
		act("Your argument MUST be lower than $S level.", FALSE, ch, nullptr, victim, TO_CHAR);
		return;
	}

	if(ammount <= 0)
	{
		send_to_char("Level drop cannot be below 1.\r\n", ch);
		return;
	}

	if(GET_LEVEL(victim) > GET_LEVEL(ch))
	{
		send_to_char("You'd be better off not doing that...\r\n", ch);
		return;
	}

	if(IS_NPC(victim))
	{
		send_to_char("Players only!!!\r\n", ch);
		return;
	}

	sprintf(buf, "%s lowered %s's level from %d to %d.", GET_NAME(ch), GET_NAME(victim), GET_LEVEL(victim), ammount);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

	for(i = GET_LEVEL(victim);i > ammount;i--)
		delevel(victim);

	GET_EXP(victim) = level_exp(GET_LEVEL(victim));


	sprintf(buf, "%s is now level %d\r\n", GET_NAME(victim), GET_LEVEL(victim));
	send_to_char(buf, ch);
	
	sprintf(buf, "ZAAAAP! You have been lowered to level %d!\r\n"
				 "Perhaps you should read the POLICY and re-read the RULES\r\n", GET_LEVEL(victim));
	send_to_char(buf, victim);
	victim->save();
}

/* Updates the legend position for ch. Tulon October 27, 2003. */
ACMD(do_legupdate)
{
	char_data *victim = 0;

	one_argument(argument, buf1);

	if(!*buf1)
	{
		send_to_char("But who?\r\n", ch);
		return;
	}

	victim = new char_data;
	clear_char(victim);

	if (victim->load(buf1))
	{
	
		update_legend(victim);
		send_to_char("Done!\r\n", ch);
		save_legends();
	}

	if(victim)
		delete(victim);
}


/* Swap an item from an offline character with the third argument. Tulon, October 2003 */
ACMD(do_swap)
{
	char_data *victim;
	struct obj_data *obj_to = nullptr;
	struct obj_data *obj_from, *obj;
	char arg[200], arg1[200], arg2[200], arg3[200];
	int i = 0, p;
	char text[300];

	half_chop(argument, arg1, arg);
	half_chop(arg, arg2, arg);
	one_argument(arg, arg3);

	if(!*arg1 || !*arg2 || !*arg3)
	{
		send_to_char("Correct format is: Swap, victim, object from, object to.\r\n", ch);
		return;
	}

	for(victim = character_list;victim;victim = victim->next)
		if(!str_cmp(GET_NAME(victim), arg1))
		{
			send_to_char("That character is already logged on!\r\n", ch);
			return;
		}

	

	victim = new char_data;
	clear_char(victim);

	if (victim->load(arg1))
	{
		Crash_load(victim, 0);

		for (obj_from = victim->carrying; obj_from; obj_from = obj_from->next_content)
		{
			if(isname(arg2, obj_from->name) || atoi(arg2) == GET_OBJ_VNUM(obj_from))
			{
				obj_from_char(obj_from);
				obj_to = read_object(real_object(atoi(arg3)), REAL);

				if(obj_to)
				{
					obj_to_char(obj_to, victim);
					i = 1;
					break;
				}
			}	
		}



		for(p = 0;p < NUM_WEARS;p++)
		{

			if(i != 1 && GET_EQ(victim, p))
			{
				obj = GET_EQ(victim, p);

				if(isname(arg2, obj->name) || atoi(arg2) == GET_OBJ_VNUM(obj))
				{
					obj_from = obj;
					obj_to_char(unequip_char(victim, p), victim);
					obj_from_char(obj_from);
					obj_to = read_object(real_object(atoi(arg3)), REAL);
		
					if(obj_to != nullptr)
					{
						obj_to_char(obj_to, victim);
						break;
					}
	
				}
			}
		}

	
		if(obj_to && obj_from)
		{
			sprintf(text, "%s was purged and replaced with %s on %s.\r\n", obj_from->short_description,
			obj_to->short_description, GET_NAME(victim));

			send_to_char(text, ch);
		}

		else
			send_to_char("That object was not found!\r\n", ch);
					
		Crash_crashsave(victim);

	}

	else
	{
		sprintf(buf, "Playerfile for character %s was not found.\r\n", arg1);
		send_to_char(buf, ch);
	}

	if(victim)
		delete(victim);

}

int compare_value(char *compare_type, int value, int compare)
{

	if(!compare_type)
		return 0;

	if(!str_cmp(compare_type, "=") && value == compare)
		return 1;

	else if(!str_cmp(compare_type, ">") && value > compare)
		return 1;

	else if(!str_cmp(compare_type, ">=") && value >= compare)
		return 1;

	else if(!str_cmp(compare_type, "<") && value < compare)
		return 1;

	else if(!str_cmp(compare_type, "<=") && value <= compare)
		return 1;

	else
		return 0;
}


/* Find a list of characters with the value of your argument. Tulon October 25, 2003. */
ACMD(do_pfind)
{
	char_data *victim = 0;
	player_index_element *entry;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
	int input = 0, type = 0, value = 0, count = 0, i = 0, p = 0;

	half_chop(argument, arg1, argument);
	two_arguments(argument, arg2, arg3);

	if(!*argument)
	{
		message(ch, "Valid Syntax: Pfind <type> <search form> <value>\r\n");
		return;
	}

	if(!*arg || !*arg2 || !*arg3)
	{
		message(ch, "Valid arguments: Level, Warnings, Weave, Questpoints.\r\n");
		return;
	}

	struct player_find {
		char name[MAX_INPUT_LENGTH];
	};
	
	struct player_find plfind[] =
	{
		{	"Level"			},
		{	"Questpoints"	},
		{	"Weavepoints"	},
		{	"Warnings"		}
	};

	for(p = 0;*plfind[p].name;p++)
	{
		if(!strn_cmp(arg1, plfind[p].name, strlen(arg1)))
		{
			type = p;
			break;
		}
	}//for
	input = atoi(arg3);

	CREATE(victim, char_data, 1);
	clear_char(victim); 

	for(entry = player_table;entry;entry = entry->next)
	{
		if (victim->load(entry->name))  
		{

			if(!strn_cmp(arg1, "level", strlen(arg1)))
				value = GET_LEVEL(victim);

			else if(!strn_cmp(arg1, "questpoints", strlen(arg1)))
				value = victim->totalQP();

			else if(!strn_cmp(arg1, "weavepoints", strlen(arg1)))
				value = GET_WP(victim);

			else if(!strn_cmp(arg1, "warnings", strlen(arg1)))
				value = GET_WARNINGS(victim);

			
			if(compare_value(arg2, value, input))
			{
				message(ch, "Player %s: %s = %d.\r\n", GET_NAME(victim), plfind[type].name, value);
				count++;
			}
		}
	}

	free_char(victim);
	message(ch, "\r\nThere were %d characters found.", count);
}

ACMD(do_warn)
{
	char_data *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);

	if(!*arg)
	{
		send_to_char("Who are you trying to warn", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, arg)))
	{
		message(ch, "Player '%s' was not found'\r\n", arg);
		return;
	}

	if(GET_LEVEL(victim) >= GET_LEVEL(ch))
	{
		message(ch, "You can't do that to %s! Might be risky!\r\n", GET_NAME(victim));
		return;
	}

	GET_WARNINGS(victim)++;
	message(ch, "%s now has %d warnings.\r\n", GET_NAME(victim), GET_WARNINGS(victim));
	message(victim, "You have been warned! You now has %d warning%s!\r\n", GET_WARNINGS(victim), GET_WARNINGS(victim) == 1 ? "" : "s");

	sprintf(buf, "%s has warned %s --Warning Number %d--", GET_NAME(ch), GET_NAME(victim), GET_WARNINGS(victim));
	mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}

ACMD(do_find)
{
	int count = 0, arg_vnum = -1;
	char fname[MAX_STRING_LENGTH], arg[200];
	
	char_data *victim = 0;
	obj_data *obj;
	obj_file_elem object;
	rent_info rent;
	player_index_element *entry;

	FILE *fl;
	
	one_argument(argument, arg);

	if (!*arg)
	{
		send_to_char("What object?\r\n", ch);
		return;
	}

	arg_vnum = atoi(arg);

	CREATE(victim, char_data, 1);
	clear_char(victim); 

	for(entry = player_table;entry;entry = entry->next)
	{
		if (victim->load(entry->name))
		{
			if (!get_filename(GET_NAME(victim), fname, CRASH_FILE))
				continue;

			if (!(fl = fopen(fname, "rb")))
				continue;

			if (!feof(fl))
				fread(&rent, sizeof(struct rent_info), 1, fl);

			while(!feof(fl))
			{
				fread(&object, sizeof(struct obj_file_elem), 1, fl);

				if(ferror(fl))
					break;

				if(!feof(fl))
				{
					obj = Obj_from_store(object);

					if(!obj || !obj->name)
						continue;
					
					if(isname(arg, obj->name) || arg_vnum == GET_OBJ_VNUM(obj))
					{
						message(ch, "Object %s is on player %s.\r\n", obj->short_description, GET_NAME(victim));
						count++;
					}

					extract_obj(obj);
				}
			}
			fclose(fl);
		}
	}

	message(ch, "\r\nThere are %d of this item in play.\r\n", count);

	free_char(victim);

}

ACMD(do_countdown)
{
	int count;

	one_argument(argument, arg);

	count = atoi(arg);

	if(count <= 0 && countdown > 0)
	{
		send_to_all("REBOOT CANCELLED.\r\n");
		countdown = -1;
		return;
	}

	if(count <= 0)
	{
		send_to_char("Reboot must be above 0 to start a reboot\r\n", ch);
		return;
	}

	countdown = count;

	message(ch, "Reboot will happen in %d minute%s.\r\n", count, (count > 1) ? "s" : "");
	sprintf(buf, "REBOOT IN %d MINUTE%s.\r\n", count, (count > 1) ? "S" : "");
	send_to_all(buf);

	sprintf(buf, "Reboot countdown of %d started by %s.", count, GET_NAME(ch));
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

	return;
}

ACMD(do_rank)
{
	char_data *victim;
	player_clan_data *cl;
	int i = 0;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	
	//Split into three arguments
	two_arguments(argument, arg1, arg2);

	if(GET_LEVEL(ch) < LVL_APPR && !IS_NPC(ch))
	{
		send_to_char("You cannot use this command!\r\n", ch);
		return;
	}

	if(!*arg1)
	{
		send_to_char("Rank who?\r\n", ch);
		return;
	}

	if(!*arg2)
	{
		send_to_char("Rank for which clan?\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, arg1)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	if(!(i = clanByName(arg2)))
	{
		message(ch, "There is no such clan.\r\n");
		return;
	}

	if(!(cl = victim->getClan(i)))
	{
		message(ch, "%s is not a member of that clan.\r\n", GET_NAME(victim));
		return;
	}

	if(cl->quest_points < rank_req[cl->rank])
	{
		send_to_char("They do not have enough quest points to rank.\r\n", ch);
		return;
	}

	++cl->rank;

	sprintf(buf, "%s raised %s's rank to %d.", GET_NAME(ch), GET_NAME(victim), cl->rank);
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

	victim->save();
	send_to_char("Ok.\r\n", ch);
}

ACMD(do_demote)
// demote function, imped by Stark
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int i = 0;
	char_data *victim;
	player_clan_data *cl;

	two_arguments(argument, arg1, arg2);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("You cannot use this command!\r\n", ch);
		return;
	}

	if(!*arg1)
	{
		send_to_char("Demote who?\r\n", ch);
		return;
	}

	if(!*arg2)
	{
		send_to_char("Demote from which clan?\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, buf1)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	if(!(i = clanByName(arg2)))
	{
		message(ch, "There is no such clan.\r\n");
		return;
	}

	if(!(cl = victim->getClan(i)))
	{
		message(ch, "%s is no in that clan.\r\n", GET_NAME(victim));
		return;
	}

	if(cl->rank <= 0)
	{
		message(ch, "You cannot lower someone's rank any lower than zero.\r\n");
		return;
	}

	--cl->rank;
	victim->save();

	sprintf(buf, "%s lowered %s's rank to %d.", GET_NAME(ch), GET_NAME(victim), cl->rank);
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
	send_to_char("Ok.\r\n", ch);

}	

ACMD(do_council)
{

	char_data *victim;
	char *type = "";

	if(GET_LEVEL(ch) < LVL_APPR && !IS_NPC(ch))
	{
		send_to_char("You can't do that!\r\n", ch);
		return;
	}

	if(!*argument)
	{
		send_to_char("Make who Council?\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if(!(victim = get_char_vis(ch, arg)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}
	

	if(PRF_FLAGGED(victim, PRF_COUNCIL))
	{
		REMOVE_BIT_AR(PRF_FLAGS(victim), PRF_COUNCIL);
		sprintf(buf, "%s has been removed from the Council.\r\n", GET_NAME(victim));
		send_to_char(buf, ch);
		type = "remove";
	}

	else
	{
		SET_BIT_AR(PRF_FLAGS(victim), PRF_COUNCIL);
		sprintf(buf, "%s has been added to the Council.\r\n", GET_NAME(victim));
		send_to_char(buf, ch);
		type = "add";
	}

	sprintf(buf, "%s %s %s council flag.", GET_NAME(ch), GET_NAME(victim), type);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

	victim->save();
}

ACMD(do_clan)
{

	char_data *victim;
	player_clan_data *cl;

	two_arguments(argument, buf1, buf2);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("You cannot use this command!\r\n", ch);
		return;
	}

	if(!*buf1)
	{
		send_to_char("Clan who?\r\n", ch);
		return;
	}

	if(!*buf2)
	{
		send_to_char("What clan?\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, buf1)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	/******** Add their player_clan_data entry *******/
	CREATE(cl, player_clan_data, 1);

	cl->clan = atoi(buf2);
	cl->rank = 1;
	cl->quest_points = 0;
	cl->clan_time = time(0);
	cl->rank_time = time(0);

	cl->next = victim->clans;
	victim->clans = cl;
	/*************************************************/
	
	if(atoi(buf2) > 0 && atoi(buf2) <= NUM_CLANS)
	{
		save_clan_quests(atoi(buf2));
		add_to_clan_list(victim, atoi(buf2));
	}

	victim->save();
	send_to_char("Ok.\r\n", ch);

	sprintf(buf, "%s clanned %s into clan %s.", GET_NAME(ch), GET_NAME(victim), clan_list[atoi(buf2)].name);
	mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);


}

ACMD(do_reset)
{
	char_data *victim;

	skip_spaces(&argument);

	if(!*arg)
	{
		send_to_char("Reset who?\r\n", ch);
		return;
	}

	else if (!(victim = get_char_vis(ch, argument)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	if(GET_LEVEL(victim) >= GET_LEVEL(ch))
	{
		send_to_char("Yeah right...\r\n", ch);
		return;
	}
  
	reset_skills(victim);
	send_to_char("You feel the loss of skill.\r\n", victim);
	send_to_char("Done.\r\n", ch);

	sprintf(buf, "%s reset %s's practices.", GET_NAME(ch), GET_NAME(victim));
	mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
}



ACMD(do_dig)
{
	/* Only works if you have Oasis OLC */
	extern void olc_add_to_save_list(int zone, cbyte type);

	char buf2[10];
	char buf3[10];
	char buf[80];
	int iroom = 0, rroom = 0, zone = 0;
	int dir = 0;
	struct descriptor_data *d = ch->desc; /* will save us some typing */
	/*  struct room_data *room; */

	two_arguments(argument, buf2, buf3);
	/* buf2 is the direction, buf3 is the room */


	iroom = atoi(buf3);

	rroom = real_room(iroom);

	if (!*buf2)
	{
		send_to_char("Format: dig <dir> <room number>\r\n", ch);
		return; 
	}
 
	else if (!*buf3)
	{
		send_to_char("Format: dig <dir> <room number>\r\n", ch);
		return; 
	}

	/* Main stuff */
	switch (*buf2)
	{
    
	case 'n':
	case 'N':
		dir = NORTH;
		break;

	case 'e':
	case 'E':
		dir = EAST;
		break;

	case 's':
	case 'S':
		dir = SOUTH;
		break;

	case 'w':
	case 'W':
		dir = WEST;
		break;

	case 'u':
	case 'U':
		dir = UP;
		break;

	case 'd':
	case 'D':
		dir = DOWN;
		break;

	}

	zone = world[IN_ROOM(ch)].zone;

	if (zone == NOWHERE)
	{
		message(ch, "You cannot link to a non-existing zone!\r\n");
		return;
	}

	if (!can_edit_zone(ch, zone))
	{
		message(ch, "You do not have permission to edit room #%d.\r\n", iroom);
		return;
	}

	if (rroom <= 0)
	{
		/*
		 * Give the descriptor an olc struct.
		 * This way we can let redit_save_internally handle the room adding.
		 */
		if (d->olc)
		{
			mudlog("SYSERR: do_dig: Player already had olc structure.", BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			delete (d->olc);
			d->olc = nullptr;
		}

		CREATE(d->olc, olc_data, 1);
		OLC_ZNUM(d) = zone;
		OLC_NUM(d) = iroom;
		CREATE(OLC_ROOM(d), room_data, 1);

		OLC_ROOM(d)->name = strdup("An unfinished room");
    
		/* Copy the room's description.*/
		OLC_ROOM(d)->description = strdup("You are in an unfinished room.\r\n");
		OLC_ROOM(d)->zone = OLC_ZNUM(d);
		OLC_ROOM(d)->number = NOWHERE;
    
		/*
		 * Save the new room to memory.
		 * redit_save_internally handles adding the room in the right place, etc.
		 */
		redit_save_internally(d);
		OLC_VAL(d) = 0;
    
		message(ch, "New room (%d) created.\r\n", iroom);
		cleanup_olc(d, CLEANUP_STRUCTS);
		/* 
		 * update rrnum to the correct room rnum after adding the room 
		 */
		rroom = real_room(iroom);
	}

	CREATE(world[rroom].dir_option[rev_dir[dir]], room_direction_data,1);
	world[rroom].dir_option[rev_dir[dir]]->general_description = nullptr;
	world[rroom].dir_option[rev_dir[dir]]->keyword = nullptr;
	world[rroom].dir_option[rev_dir[dir]]->to_room = ch->in_room;

	CREATE(world[ch->in_room].dir_option[dir], room_direction_data,1);
	world[ch->in_room].dir_option[dir]->general_description = nullptr;
	world[ch->in_room].dir_option[dir]->keyword = nullptr;
	world[ch->in_room].dir_option[dir]->to_room = rroom;

	/* Only works if you have Oasis OLC */
	olc_add_to_save_list((iroom / 100), OLC_SAVE_ROOM);

	sprintf(buf, "You make an exit %s to room %d.\r\n", buf2, iroom);
	send_to_char(buf, ch);
}


ACMD(do_echo)
{
	
	char_data *vict;
	skip_spaces(&argument);
	int count = 0;

	if (!*argument)
		send_to_char("Yes.. but what?\r\n", ch);
	
	else
	{
		for(count = 0, vict = world[ch->in_room].people;vict;vict = vict->next_in_room, count++)
		{		

			if (subcmd == SCMD_EMOTE)
			{
				if(!can_communicate(ch, vict))
						sprintf(buf, "%s makes some strange gestures with %s hands.\r\n", PERS(ch, vict), HSHR(ch));
				else
					sprintf(buf, "%s %s\r\n", PERS(ch, vict), argument);
			
				send_to_char(buf, vict);
			}

			else
			{
				sprintf(buf, "%s\r\n", argument);
				send_to_char(buf, vict);

				if(!count)
				{
					sprintf(buf, "%s made echo %s in room %d.", GET_NAME(ch), argument, GET_ROOM_VNUM(ch->in_room));
					mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
				}
			}
		}
	}
}

ACMD(do_award)
{

	char_data *victim;
	char *name = arg, *questpoints = buf2, clan_name[MAX_INPUT_LENGTH];
	int qp, clan_num;
	player_clan_data *cl;

	half_chop(argument, name, argument);
	two_arguments(argument, clan_name, questpoints);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

	if(!*name)
	{
		send_to_char("Syntax: <Player Name> <Clan Name> <Quest Points>\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, name)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	if(!(clan_num = clanByName(clan_name)))
	{
		message(ch, "There is no such clan.\r\n");
		return;
	}

	if(! (cl = victim->getClan(clan_num)))
	{
		message(ch, "%s is not a member of that clan.\r\n", GET_NAME(ch));
		return;
	}

	if((qp = atoi(questpoints)) == 0)
	{
		message(ch, "You cannot award zero questpoints.\r\n");
		return;
	}

	//By this point, everything should be good
	cl->quest_points += qp;
	sprintf(buf, "You award %s %d quest points.\r\n", GET_NAME(victim), qp);
	send_to_char(buf, ch);
	
	if(qp > 0)
		message(victim, "You gain some quest points.\r\n");
	else
		message(ch, "You lose some quest points.\r\n");

	sprintf(buf, "%s awarded %s %d quest points.", GET_NAME(ch), GET_NAME(victim), qp);
	mudlog(buf, NRM, MAX(LVL_BLDER, GET_INVIS_LEV(ch)), TRUE);
}

ACMD(do_awardd)
{

	char_data *victim;
	char *name = arg, *questpoints = buf2;
	int qp;

	two_arguments(argument, name, questpoints);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

	if(!*name)
	{
		send_to_char("Award Dark Points to who?\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, name)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	qp = atoi(questpoints);
	GET_DP(victim) += qp;
	message(ch, "You award %s %d Dark points\r\n", GET_NAME(victim), qp);
	

	sprintf(buf, "%s awarded %s %d Dark points.", GET_NAME(ch), GET_NAME(victim), qp);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

}

ACMD(do_awardh)
{

	char_data *victim;
	char *name = arg, *questpoints = buf2;
	int qp;

	two_arguments(argument, name, questpoints);

	if(GET_LEVEL(ch) < LVL_GOD && !IS_NPC(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

	if(!*name)
	{
		send_to_char("Award hidden QPs to who?\r\n", ch);
		return;
	}

	if(!(victim = get_char_vis(ch, name)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}

	qp = atoi(questpoints);
	GET_HIDDEN_QP(victim) += qp;
	message(ch, "You award %s %d hidden quest points\r\n", GET_NAME(victim), qp);
	

	sprintf(buf, "%s awarded %s %d hidden quest points.", GET_NAME(ch), GET_NAME(victim), qp);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

}

ACMD(do_send)
{
	char_data *vict;

	half_chop(argument, arg, buf);

	if (!*arg)
	{
		send_to_char("Send what to who?\r\n", ch);
		return;
	}
  
	if (!(vict = get_char_vis(ch, arg)))
	{
		send_to_char(NOPERSON, ch);
		return;
	}
  
	send_to_char(buf, vict);
	send_to_char("\r\n", vict);
  
	if (PRF_FLAGGED(ch, PRF_NOREPEAT))
		send_to_char("Sent.\r\n", ch);
  
	else
	{
		sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
		send_to_char(buf2, ch);
	}
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(char_data * ch, char *rawroomstr)
{
	int tmp;
	sh_int location;
	char_data *target_mob;
	struct obj_data *target_obj;
	char roomstr[MAX_INPUT_LENGTH];

	one_argument(rawroomstr, roomstr);

	if (!*roomstr)
	{
		send_to_char("You must supply a room number or name.\r\n", ch);
		return NOWHERE;
	}

	if (isdigit(*roomstr) && !strchr(roomstr, '.'))
	{
		tmp = atoi(roomstr);

		if ((location = real_room(tmp)) < 0)
		{
			send_to_char("No room exists with that number.\r\n", ch);
			return NOWHERE;
		}
	}

	else if ((target_mob = get_char_vis(ch, roomstr)))
		location = target_mob->in_room;

	else if ((target_obj = get_obj_vis(ch, roomstr)))
	{
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;

		else
		{
			send_to_char("That object is not available.\r\n", ch);
			return NOWHERE;
		}
	}

	else
	{
		send_to_char("No such creature or object around.\r\n", ch);
		return NOWHERE;
	}

	/* a location has been found -- if you're < GRGOD, check restrictions. */
	if (GET_LEVEL(ch) < LVL_GRGOD)
	{
		if (ROOM_FLAGGED(location, ROOM_GODROOM))
		{
			send_to_char("You are not godly enough to use that room!\r\n", ch);
			return NOWHERE;
		}

		if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
			world[location].people && world[location].people->next_in_room)
		{
			send_to_char("There's a private conversation going on in that room.\r\n", ch);
			return NOWHERE;
		}
		
		if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
			!House_can_enter(ch, GET_ROOM_VNUM(location)))
		{
			send_to_char("That's private property -- no trespassing!\r\n", ch);
			return NOWHERE;
		}
	}

	return location;
}



ACMD(do_at)
{
	char command[MAX_INPUT_LENGTH];
	int location, original_loc;

	half_chop(argument, buf, command);

	if (!*buf)
	{
		send_to_char("You must supply a room number or a name.\r\n", ch);
		return;
	}

	if (!*command)
	{
		send_to_char("What do you want to do there?\r\n", ch);
		return;
	}

	if ((location = find_target_room(ch, buf)) < 0)
		return;

	/* To prevent FN imms... */
	if(GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) <= LVL_BLDER || PLR_FLAGGED(ch, PLR_ZONE_BAN))
	{
		if(!can_edit_zone(ch, world[location].zone) && world[location].zone != 0)
		{
			send_to_char("You must be higher level to leave your zone!\r\n", ch);
			return;
		}
	}

	/* a location has been found. */
	original_loc = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, location);
	command_interpreter(ch, command);

	/* check if the char is still there */
	if (ch->in_room == location)
	{
		char_from_room(ch);
		char_to_room(ch, original_loc);
	}
}


ACMD(do_goto)
{
	sh_int location;

	if ((location = find_target_room(ch, argument)) < 0)
		return;
 
	/* To prevent FN imms... */
	if(GET_LEVEL(ch) >= LVL_IMMORT && GET_LEVEL(ch) <= LVL_BLDER || PLR_FLAGGED(ch, PLR_ZONE_BAN))
		if(!can_edit_zone(ch, world[location].zone) && world[location].zone != 0)
		{
			send_to_char("You must be higher level to leave your zone!\r\n", ch);
			return;
		}

	if (*ch->points.poofout)
		sprintf(buf, "$n %s", ch->points.poofout);
	
	else
		strcpy(buf, "$n steps back into another part of the wheel.");

	act(buf, TRUE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);

	if (*POOFIN(ch))
		sprintf(buf, "$n %s", POOFIN(ch));
 
	else
		strcpy(buf, "$n appears from no-where");

	act(buf, TRUE, ch, 0, 0, TO_ROOM);
	look_at_room(ch, 0);
}



ACMD(do_trans)
{
	struct descriptor_data *i;
	char_data *victim;

	one_argument(argument, buf);

	if (!*buf)
		send_to_char("Whom do you wish to transfer?\r\n", ch);

	else if (str_cmp("all", buf))
	{
		if (!(victim = get_char_vis(ch, buf)))
			send_to_char(NOPERSON, ch);
    
		else if (victim == ch)
			send_to_char("That doesn't make much sense, does it?\r\n", ch);

		else
		{
			if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim))
			{
				send_to_char("Go transfer someone your own size.\r\n", ch);
				return;
			}
      
			act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
			char_from_room(victim);
			char_to_room(victim, ch->in_room);
			act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
			act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
			look_at_room(victim, 0);

			sprintf(buf, "%s transfers %s to room vnum %d.", GET_NAME(ch), GET_NAME(victim), GET_ROOM_VNUM(victim->in_room));
			mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

		}
	} 

	else
	{			/* Trans All */
		if (GET_LEVEL(ch) < LVL_GRGOD)
		{
			send_to_char("I think not.\r\n", ch);
			return;
		}

		for (i = descriptor_list; i; i = i->next)
			if (STATE(i) == CON_PLAYING && i->character && i->character != ch)
			{
				victim = i->character;

				if (GET_LEVEL(victim) >= GET_LEVEL(ch))
					continue;

				act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
				char_from_room(victim);
				char_to_room(victim, ch->in_room);
				act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
				act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
				look_at_room(victim, 0);
			}

			send_to_char(OK, ch);
	}
}

ACMD(do_teleport)
{
	char_data *victim;
	sh_int target;

	two_arguments(argument, buf, buf2);

	if (!*buf)
		send_to_char("Whom do you wish to teleport?\r\n", ch);

	else if (!(victim = get_char_vis(ch, buf)))
		send_to_char(NOPERSON, ch);

	else if (victim == ch)
		send_to_char("Use 'goto' to teleport yourself.\r\n", ch);

	else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
		send_to_char("Maybe you shouldn't do that.\r\n", ch);

	else if (!*buf2)
		send_to_char("Where do you wish to send this person?\r\n", ch);

	else if ((target = find_target_room(ch, buf2)) >= 0)
	{
		send_to_char(OK, ch);
		act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
		char_from_room(victim);
		char_to_room(victim, target);
		act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
		act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
		look_at_room(victim, 0);
		sprintf(buf, "%s has teleported %s to %s.", GET_NAME(ch), GET_NAME(victim), world[victim->in_room].name);
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
	}
}

ACMD(do_vnum)
{
	two_arguments(argument, buf, buf2);

	if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj")))
	{
		send_to_char("Usage: vnum { obj | mob } <name>\r\n", ch);
		return;
	}

	if (is_abbrev(buf, "mob"))
		if (!vnum_mobile(buf2, ch))
			send_to_char("No mobiles by that name.\r\n", ch);

	if (is_abbrev(buf, "obj"))
		if (!vnum_object(buf2, ch))
			send_to_char("No objects by that name.\r\n", ch);
}

void do_stat_room(char_data * ch)
{
	struct extra_descr_data *desc;
	struct room_data *rm = &world[ch->in_room];
	int i, found = 0;
	struct obj_data *j = 0;
	char_data *k = 0;
	int type, dir_opt;

	sprintf(buf, "Room name: %s%s%s\r\n", COLOR_CYAN(ch, CL_NORMAL), rm->name,
	COLOR_NORMAL(ch, CL_NORMAL));

	send_to_char(buf, ch);

	type = rm->sector_type;

	sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
	zone_table[rm->zone].number, COLOR_GREEN(ch, CL_NORMAL), rm->number,
	COLOR_NORMAL(ch, CL_NORMAL), ch->in_room, sector_types[type]);

	send_to_char(buf, ch);

	sprintbitarray(rm->room_flags, room_bits, RF_ARRAY_MAX, buf2);
	sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
	(rm->func == nullptr) ? "None" : "Exists", buf2);

	send_to_char(buf, ch);

	send_to_char("Description:\r\n", ch);

	if (rm->description)
		send_to_char(rm->description, ch);

	else
		send_to_char("  None.\r\n", ch);

	if (rm->ex_description)
	{
		sprintf(buf, "Extra descs:%s", COLOR_CYAN(ch, CL_NORMAL));
		for (desc = rm->ex_description; desc; desc = desc->next)
		{
			strcat(buf, " ");
			strcat(buf, desc->keyword);
		}

		strcat(buf, COLOR_NORMAL(ch, CL_NORMAL));
		send_to_char(strcat(buf, "\r\n"), ch);
	}

	sprintf(buf, "Chars present:%s", COLOR_YELLOW(ch, CL_NORMAL));

	for (found = 0, k = rm->people; k; k = k->next_in_room)
	{
		if (!CAN_SEE(ch, k))
			continue;
		sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
		(!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
			
		strcat(buf, buf2);
		if (strlen(buf) >= 62)
		{

			if (k->next_in_room)
				send_to_char(strcat(buf, ",\r\n"), ch);

			else
				send_to_char(strcat(buf, "\r\n"), ch);

			*buf = found = 0;
		}
	}

	if (*buf)
		send_to_char(strcat(buf, "\r\n"), ch);

	send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);

	if (rm->contents)
	{
		sprintf(buf, "Contents:%s", COLOR_GREEN(ch, CL_NORMAL));
		for (found = 0, j = rm->contents; j; j = j->next_content)
		{
			if (!CAN_SEE_OBJ(ch, j))
				continue;
	
			sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
			strcat(buf, buf2);

			if (strlen(buf) >= 62)
			{
				if (j->next_content)
					send_to_char(strcat(buf, ",\r\n"), ch);

				else
					send_to_char(strcat(buf, "\r\n"), ch);
				*buf = found = 0;
			}
		}

		if (*buf)
			send_to_char(strcat(buf, "\r\n"), ch);
		send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);
	}

	for (i = 0; i < NUM_OF_DIRS; i++)
	{
		if (rm->dir_option[i]) {
			if (rm->dir_option[i]->to_room == NOWHERE)
				sprintf(buf1, " %sNONE%s", COLOR_CYAN(ch, CL_NORMAL), COLOR_NORMAL(ch, CL_NORMAL));

			else
				sprintf(buf1, "%s%5d%s", COLOR_CYAN(ch, CL_NORMAL),

			GET_ROOM_VNUM(rm->dir_option[i]->to_room), COLOR_NORMAL(ch, CL_NORMAL));
			dir_opt = rm->dir_option[i]->exit_info;
			sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
			COLOR_CYAN(ch, CL_NORMAL), dirs[i], COLOR_NORMAL(ch, CL_NORMAL), buf1, rm->dir_option[i]->key,

			rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None", exit_bits[dir_opt]);
			send_to_char(buf, ch);

			if (rm->dir_option[i]->general_description)
				strcpy(buf, rm->dir_option[i]->general_description);

			else
				strcpy(buf, "  No exit description.\r\n");

			send_to_char(buf, ch);
		}
	}

	/* check the room for a script */
	do_sstat_room(ch);
}

void do_stat_object(char_data * ch, struct obj_data * j)
{
	int i, vnum, found;
	struct obj_data *j2;
	struct extra_descr_data *desc;

	sprintf(buf, "%s did a statfind on object %s.", GET_NAME(ch), j->short_description);
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

	vnum = GET_OBJ_VNUM(j);
	sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", COLOR_YELLOW(ch, CL_NORMAL),
	((j->short_description) ? j->short_description : "<None>"),
	COLOR_NORMAL(ch, CL_NORMAL), j->name);

	send_to_char(buf, ch);
	
	if (GET_OBJ_RNUM(j) >= 0)
		strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
	
	else
		strcpy(buf2, "None");

	sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
	COLOR_GREEN(ch, CL_NORMAL), vnum, COLOR_NORMAL(ch, CL_NORMAL), GET_OBJ_RNUM(j), item_types[(int)GET_OBJ_TYPE(j)], buf2);
  
	send_to_char(buf, ch);
	sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
	send_to_char(buf, ch);

	if (j->ex_description)
	{
		sprintf(buf, "Extra descs:%s", COLOR_CYAN(ch, CL_NORMAL));
    
		for (desc = j->ex_description; desc; desc = desc->next)
		{
			strcat(buf, " ");
			strcat(buf, desc->keyword);
		}

		strcat(buf, COLOR_NORMAL(ch, CL_NORMAL));
		send_to_char(strcat(buf, "\r\n"), ch);
	}

	message(ch, "Creator: %s%s%s.%s\r\n", COLOR_BOLD(ch, CL_COMPLETE),
		COLOR_GREEN(ch, CL_COMPLETE), j->creator, COLOR_NORMAL(ch, CL_COMPLETE));

	send_to_char("Can be worn on: ", ch);
	sprintbitarray(j->obj_flags.wear_flags, wear_bits, TW_ARRAY_MAX, buf);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	send_to_char("Set char bits : ", ch);
	sprintbitarray( (int *) j->obj_flags.bitvector, affected_bits, TW_ARRAY_MAX, buf);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	send_to_char("Extra flags   : ", ch);
	sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
	object_weight(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
	send_to_char(buf, ch);

	strcpy(buf, "In room: ");
	if (j->in_room == NOWHERE)
		strcat(buf, "Nowhere");
	
	else
	{
		sprintf(buf2, "%d", GET_ROOM_VNUM(IN_ROOM(j)));
		strcat(buf, buf2);
	}
  
	/*
	* NOTE: In order to make it this far, we must already be able to see the
	*       character holding the object. Therefore, we do not need CAN_SEE().
	*/
	strcat(buf, ", In object: ");
	strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
	strcat(buf, ", Carried by: ");
	strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
	strcat(buf, ", Worn by: ");
	strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
	strcat(buf, "\r\n");
	send_to_char(buf, ch);

	switch (GET_OBJ_TYPE(j))
	{

	case ITEM_LIGHT:
		if (GET_OBJ_VAL(j, 2) == -1)
			strcpy(buf, "Hours left: Infinite");
		
		else
			sprintf(buf, "Hours left: [%d]", GET_OBJ_VAL(j, 2));
		break;
	
	case ITEM_SCROLL:
	case ITEM_POTION:
		sprintf(buf, "Spells: (Level %d) %s, %s, %s", GET_OBJ_VAL(j, 0),
		skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	    skill_name(GET_OBJ_VAL(j, 3)));
		break;
	
	case ITEM_WAND:
	case ITEM_STAFF:
		sprintf(buf, "Spell: %s at level %d, %d (of %d) charges remaining",
		skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
		break;
	
	case ITEM_WEAPON:
		sprintf(buf, "Todam: %dd%d, Message type: %d",
	    GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
		break;
	
	case ITEM_ARMOR:
		sprintf(buf, "AC-apply: [%d]", GET_OBJ_VAL(j, 0));
		break;
	
	case ITEM_TRAP:
		sprintf(buf, "Spell: %d, - Hitpoints: %d",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
		break;
	
	case ITEM_CONTAINER:
		sprintf(buf, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s",
	    GET_OBJ_VAL(j, 0), container_bits[GET_OBJ_VAL(j, 1)], GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)));
		break;
	
	case ITEM_DRINKCON:
	case ITEM_FOUNTAIN:
		sprinttype(GET_OBJ_VAL(j, 2), (const char **) drinks, buf2);
		sprintf(buf, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
		break;
	
	case ITEM_NOTE:
		sprintf(buf, "Tongue: %d", GET_OBJ_VAL(j, 0));
		break;
	
	case ITEM_KEY:
		strcpy(buf, "");
		break;
	
	case ITEM_FOOD:
		sprintf(buf, "Makes full: %d, Poisoned: %s", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
		break;
	
	case ITEM_MONEY:
		sprintf(buf, "Coins: %d", GET_OBJ_VAL(j, 0));
		break;
	
	default:
		sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
	
	}
	
	send_to_char(strcat(buf, "\r\n"), ch);

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

	if (j->contains)
	{
		sprintf(buf, "\r\nContents:%s", COLOR_GREEN(ch, CL_NORMAL));
		
		for (found = 0, j2 = j->contains; j2; j2 = j2->next_content)
		{
			sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
			strcat(buf, buf2);
			
			if (strlen(buf) >= 62)
			{
				if (j2->next_content)
					send_to_char(strcat(buf, ",\r\n"), ch);
				
				else
					send_to_char(strcat(buf, "\r\n"), ch);
	
				*buf = found = 0;
      
			}
		}

		if (*buf)
			send_to_char(strcat(buf, "\r\n"), ch);
    
		send_to_char(COLOR_NORMAL(ch, CL_NORMAL), ch);
  
	}
  
	found = 0; 
	send_to_char("Affections:", ch);
  
	for (i = 0; i < MAX_OBJ_AFFECT; i++)
		if (j->affected[i].modifier)
		{
			sprinttype(j->affected[i].location, (const char **) apply_types, buf2);
			sprintf(buf, "%s %+d to %s", found++ ? "," : "",
			j->affected[i].modifier, buf2);
			send_to_char(buf, ch);
		}
  
	if (!found)
		send_to_char(" None", ch);

	send_to_char("\r\n", ch);

	/* check the object for a script */
	do_sstat_object(ch, j);
}

void do_stat_character(char_data * ch, char_data * k)
{
	int i, i2, found = 0;
	string rank_time, clan_time;
	obj_data *j;
	follow_type *fol;
	affected_type *aff;
	player_clan_data *cl;


	sprintf(buf, "%s did a statfind on %s.", GET_NAME(ch), GET_NAME(k));
	mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

	switch (GET_SEX(k))
	{

	case SEX_NEUTRAL:
		strcpy(buf, "NEUTRAL-SEX");
		break;
	
	case SEX_MALE:
		strcpy(buf, "MALE");
		break;
	
	case SEX_FEMALE:
		strcpy(buf, "FEMALE");
		break;
	
	default:
		strcpy(buf, "ILLEGAL-SEX!!");
		break;
	}

  
	sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5d]\r\n",
	(!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	GET_NAME(k), GET_IDNUM(k), GET_ROOM_VNUM(IN_ROOM(k)));
	send_to_char(strcat(buf, buf2), ch);
  
	if (IS_MOB(k))
    message(ch, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
	k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));

	message(ch, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));

	message(ch, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));

	if (IS_NPC(k))
	{	/* Use GET_CLASS() macro? */
		strcpy(buf, "Monster Class: ");
		sprinttype(k->player.chclass, (const char **) npc_class_types, buf2);
	} 
  
	else
	{
		strcpy(buf, "Class: ");
		sprinttype(k->player.chclass, pc_class_types, buf2);
	}
  
	strcat(buf, buf2);

	sprintf(buf2, ", Lev: [%s%2d%s], XP: [%s%7d%s]\r\n",
	COLOR_YELLOW(ch, CL_NORMAL), GET_LEVEL(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_YELLOW(ch, CL_NORMAL), GET_EXP(k), COLOR_NORMAL(ch, CL_NORMAL));
	strcat(buf, buf2);
	send_to_char(buf, ch);

	if (!IS_NPC(k))
	{
		strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
		strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
		buf1[10] = buf2[10] = '\0';

		message(ch, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
		buf1, buf2, k->player.time.played / 3600,
		((k->player.time.played % 3600) / 60), age(k)->year);

		sprintf(buf, "Hometown: [%d], Speaks: [%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])",
		k->player.hometown, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
		GET_PRACTICES(k), int_app[GET_INT(k)].learn,
		wis_app[GET_WIS(k)].bonus);
		strcat(buf, "\r\n");
		send_to_char(buf, ch);
	}
  
	message(ch, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	"Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]\r\n",
	COLOR_CYAN(ch, CL_NORMAL), GET_STR(k), GET_ADD(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_CYAN(ch, CL_NORMAL), GET_INT(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_CYAN(ch, CL_NORMAL), GET_WIS(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_CYAN(ch, CL_NORMAL), GET_DEX(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_CYAN(ch, CL_NORMAL), GET_CON(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_CYAN(ch, CL_NORMAL), GET_CHA(k), COLOR_NORMAL(ch, CL_NORMAL));

	message(ch, "Total Quest Points [%s%d%s], Weave Points [%s%d%s], Legend [%s%d%s], Hidden Quest Points [%s%d%s].\r\n",
	COLOR_GREEN(ch, CL_NORMAL), k->totalQP(), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_GREEN(ch, CL_NORMAL), GET_WP(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_GREEN(ch, CL_NORMAL), k->getLegend(), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_GREEN(ch, CL_NORMAL), GET_HIDDEN_QP(k), COLOR_NORMAL(ch, CL_NORMAL));

	message(ch, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\n",
	COLOR_GREEN(ch, CL_NORMAL), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_GREEN(ch, CL_NORMAL), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), COLOR_NORMAL(ch, CL_NORMAL),
	COLOR_GREEN(ch, CL_NORMAL), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), COLOR_NORMAL(ch, CL_NORMAL));

	message(ch, "%s%sPLAYER'S CLANS:%s\r\n", COLOR_YELLOW(ch, CL_NORMAL), COLOR_BOLD(ch, CL_NORMAL), COLOR_NORMAL(ch, CL_NORMAL));
	for(cl = k->clans;cl;cl = cl->next)
	{
		rank_time = ctime(&cl->rank_time);
		clan_time = ctime(&cl->clan_time);

		rank_time[rank_time.length() - 1] = '\0';
		clan_time[clan_time.length() - 1] = '\0';

		message(ch, "Clan [%s%s%s], Rank [%s%d%s], Clanning Time [%s%s%s], Ranking Time [%s%s%s]\r\n",
			COLOR_GREEN(ch, CL_NORMAL), clan_list[cl->clan].name, COLOR_NORMAL(ch, CL_NORMAL),
			COLOR_GREEN(ch, CL_NORMAL), cl->rank, COLOR_NORMAL(ch, CL_NORMAL),
			COLOR_GREEN(ch, CL_NORMAL), clan_time.c_str(), COLOR_NORMAL(ch, CL_NORMAL),
			COLOR_GREEN(ch, CL_NORMAL), rank_time.c_str(), COLOR_NORMAL(ch, CL_NORMAL));
	}

	message(ch, "Coins: [%9d], Bank: [%9d] (Total: %d)\r\n",
	GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));

	message(ch, "ABS: [%d], Offense: [%2d], Dodge: [%2d], Parry: [%2d], Damroll: [%2d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	abs_find(k), offense_find(k), dodge_find(k), parry_find(k), k->points.damroll, GET_SAVE(k, 0),
	GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));

	sprinttype(GET_POS(k), (const char **) position_types, buf2);
	sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	(FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

	if (IS_NPC(k))
	{
		strcat(buf, ", Attack type: ");
		strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
	}
  
	if (k->desc)
	{
		sprinttype(STATE(k->desc), (const char **) connected_types, buf2);
		strcat(buf, ", Connected: ");
		strcat(buf, buf2);
	}
  
	send_to_char(strcat(buf, "\r\n"), ch);

	strcpy(buf, "Default position: ");
	sprinttype((k->mob_specials.default_pos), (const char **) position_types, buf2);
	strcat(buf, buf2);

	sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
	strcat(buf, buf2);
	send_to_char(buf, ch);

	if (IS_NPC(k))
	{
		sprintbitarray((int *) MOB_FLAGS(k), action_bits, PM_ARRAY_MAX, buf2);
		sprintf(buf, "NPC flags: %s%s%s\r\n", COLOR_CYAN(ch, CL_NORMAL), buf2, COLOR_NORMAL(ch, CL_NORMAL));
		send_to_char(buf, ch);
	} 
  
	else
	{
		sprintbitarray((int *) PLR_FLAGS(k), player_bits, PM_ARRAY_MAX, buf2);
		sprintf(buf, "PLR: %s%s%s\r\n", COLOR_CYAN(ch, CL_NORMAL), buf2, COLOR_NORMAL(ch, CL_NORMAL));
		send_to_char(buf, ch);
		sprintbitarray(PRF_FLAGS(k), preference_bits, PM_ARRAY_MAX, buf2);
		sprintf(buf, "PRF: %s%s%s\r\n", COLOR_GREEN(ch, CL_NORMAL), buf2, COLOR_NORMAL(ch, CL_NORMAL));
		send_to_char(buf, ch);
	}

	if (IS_MOB(k))
	{
		sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
		(mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
		k->mob_specials.damnodice, k->mob_specials.damsizedice);
		send_to_char(buf, ch);
	}
  
	sprintf(buf, "Carried: weight: %d, items: %d; ",
	carrying_weight(k), IS_CARRYING_N(k));

	for (i = 0, j = k->carrying; j; j = j->next_content, i++);
	sprintf(buf + strlen(buf), "Items in: inventory: %d, ", i);

	for (i = 0, i2 = 0; i < NUM_WEARS; i++)
		if (GET_EQ(k, i))
			i2++;
  
		sprintf(buf2, "eq: %d\r\n", i2);
	strcat(buf, buf2);
	send_to_char(buf, ch);

	if (!IS_NPC(k))
	{
		sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
		GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
		send_to_char(buf, ch);
	}

	sprintf(buf, "Master is: %s, Followers are:",
	((k->master) ? GET_NAME(k->master) : "<none>"));

	for (fol = k->followers; fol; fol = fol->next)
	{
		sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
		strcat(buf, buf2);
    
		if (strlen(buf) >= 62)
		{
			if (fol->next)
				send_to_char(strcat(buf, ",\r\n"), ch);
      
			else
				send_to_char(strcat(buf, "\r\n"), ch);
      
			*buf = found = 0;
		}
	}

	if (*buf)
		send_to_char(strcat(buf, "\r\n"), ch);

	/* Showing the bitvector */
	sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf2);
	sprintf(buf, "AFF: %s%s%s\r\n", COLOR_YELLOW(ch, CL_NORMAL), buf2, COLOR_NORMAL(ch, CL_NORMAL));
	send_to_char(buf, ch);

	/* Routine to show what spells a char is affected by */
  
	if (k->affected)
	{
		for (aff = k->affected; aff; aff = aff->next)
		{
			*buf2 = '\0';
			sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
			COLOR_CYAN(ch, CL_NORMAL), (aff->type >= 0 && aff->type <= MAX_SPELLS) ?
			spells[aff->type] : "TYPE UNDEFINED", COLOR_NORMAL(ch, CL_NORMAL));
      
			if (aff->modifier)
			{
				sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
				strcat(buf, buf2);
			}
      
			if (aff->bitvector)
			{
				if (*buf2)
					strcat(buf, ", sets ");
	
				else
					strcat(buf, "sets ");
	
				sprintf(buf2, affected_bits[aff->bitvector]);
				strcat(buf, buf2);
			}
     
			send_to_char(strcat(buf, "\r\n"), ch);
		}
	}

	/* check mobiles for a script */
  
	if (IS_NPC(k))
	{
		do_sstat_character(ch, k);
    
		if (SCRIPT_MEM(k))
		{
			struct script_memory *mem = SCRIPT_MEM(k);
			send_to_char("Script memory:\r\n  Remember             Command\r\n", ch);
      
			while (mem)
			{
				char_data *mc = find_char(mem->id);
        
				if (!mc) send_to_char("  ** Corrupted!\r\n", ch);
				
				else
				{
					if (mem->cmd) sprintf(buf,"  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          
					else sprintf(buf,"  %-20.20s <default>\r\n",GET_NAME(mc));
						send_to_char(buf, ch);
				}
      
				mem = mem->next;
			}
		}
	}

}

ACMD(do_statfind)
{
	char_data *victim = nullptr;
	struct obj_data *object = nullptr;
	int tmp;

	half_chop(argument, buf1, buf2);

	if (!*buf1) 
	{
		send_to_char("Stats on who or what?\r\n", ch);
		return;
	} 
	
	else if (is_abbrev(buf1, "room")) 
	{
		do_stat_room(ch);
	} 
	
	else if (is_abbrev(buf1, "mob")) 
	{
    
		if (!*buf2)
			send_to_char("Stats on which mobile?\r\n", ch);
    
		else 
		{
			if ((victim = get_char_vis(ch, buf2)))
				do_stat_character(ch, victim);
      
			else
				send_to_char("No such mobile around.\r\n", ch);
		}
	} 
	
	else if (is_abbrev(buf1, "player")) 
	{
    
		if (!*buf2) 
		{
			send_to_char("Stats on which player?\r\n", ch);
		} 
		
		else
		{
			if ((victim = get_player_vis(ch, buf2, 0)))
				do_stat_character(ch, victim);
      
			else
				send_to_char("No such player around.\r\n", ch);
		}
	} 
	
	else if (is_abbrev(buf1, "file"))
	{
    
		if (!*buf2) 
		{
			send_to_char("Stats on which player?\r\n", ch);
		} 
		
		else
		{
			victim = new char_data;
			clear_char(victim);
      
			if (victim->load(buf2))
			{
				if (GET_LEVEL(victim) > GET_LEVEL(ch))
					send_to_char("Sorry, you can't do that.\r\n", ch);
	
				else
					do_stat_character(ch, victim);
			} 

			else 
			{
				send_to_char("There is no such player.\r\n", ch);
			}

			delete(victim);
			victim = nullptr;
		}
	}

	else if (is_abbrev(buf1, "object"))
	{
    
		if (!*buf2)
			send_to_char("Stats on which object?\r\n", ch);

		else 
		{

			if ((object = get_obj_vis(ch, buf2)))
				do_stat_object(ch, object);
      
			else
				send_to_char("No such object around.\r\n", ch);
		}
	} 

	else 
	{

		if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
			do_stat_object(ch, object);

		else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
			do_stat_object(ch, object);

		else if ((victim = get_char_room_vis(ch, buf1)))
			do_stat_character(ch, victim);

		else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
			do_stat_object(ch, object);

		else if ((victim = get_char_vis(ch, buf1)))
			do_stat_character(ch, victim);

		else if ((object = get_obj_vis(ch, buf1)))
			do_stat_object(ch, object);

		else
			send_to_char("Nothing around by that name.\r\n", ch);
	}
}

void stop_snooping(char_data * ch)
{
	if (!ch->desc->snooping)
		send_to_char("You aren't snooping anyone.\r\n", ch);

	else
	{
		if(!IS_KING(ch))
		{
			sprintf(buf, "%s stops snooping %s.", GET_NAME(ch), GET_NAME(ch->desc->snooping->character));
			mudlog(buf, NRM, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
		}

		send_to_char("You stop snooping.\r\n", ch);
		ch->desc->snooping->snoop_by = nullptr;
		ch->desc->snooping = nullptr;
	}
}

ACMD(do_shutdown)
{
	char arg[MAX_INPUT_LENGTH];

	if (subcmd != SCMD_SHUTDOWN)
	{
		send_to_char("If you want to shut something down, say so!\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!*arg)
	{
		log("(GC) Shutdown by %s.", GET_NAME(ch));
		sprintf(buf, "Shutdown by %s.\r\n", GET_NAME(ch));
		send_to_all(buf);
		circle_shutdown = 1;

	}
}

ACMD(do_snoop)
{
	char_data *victim, *tch;

	if (!ch->desc)
		return;

	one_argument(argument, arg);

	if (!*arg)
		stop_snooping(ch);

	else if (!(victim = get_char_vis(ch, arg)))
		send_to_char("No such person around.\r\n", ch);

	else if (!victim->desc)
		send_to_char("There's no link.. nothing to snoop.\r\n", ch);

	else if (victim == ch)
		stop_snooping(ch);

	else if (victim->desc->snoop_by)
		send_to_char("Busy already. \r\n", ch);

	else if (victim->desc->snooping == ch->desc)
		send_to_char("Don't be stupid.\r\n", ch);

	else
	{
    
		if (victim->desc->original)
			tch = victim->desc->original;
    
		else
			tch = victim;

		if (GET_LEVEL(tch) >= GET_LEVEL(ch) && !IS_KING(ch))
		{
			send_to_char("You can't.\r\n", ch);
			return;
		}
    
		send_to_char(OK, ch);

		if(!IS_KING(ch))
		{
			sprintf(buf, "%s begins snooping %s.", GET_NAME(ch), GET_NAME(victim));
			mudlog(buf, NRM, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
		}

		if (ch->desc->snooping)
			ch->desc->snooping->snoop_by = nullptr;

		ch->desc->snooping = victim->desc;
		victim->desc->snoop_by = ch->desc;
	}
}

ACMD(do_switch)
{
	char_data *victim;

	one_argument(argument, arg);

	if (ch->desc->original)
		send_to_char("You're already switched.\r\n", ch);
  
	else if (!*arg)
		send_to_char("Switch with who?\r\n", ch);
  
	else if (!(victim = get_char_vis(ch, arg)))
		send_to_char("No such character.\r\n", ch);
  
	else if (ch == victim)
		send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  
	else if (victim->desc)
		send_to_char("You can't do that, the body is already in use!\r\n", ch);
  
	else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
		send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  
	else
	{
		send_to_char(OK, ch);

		sprintf(buf, "%s has switched into %s.", GET_NAME(ch), GET_NAME(victim));
		mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);

		ch->desc->character = victim;
		ch->desc->original = ch;

		victim->desc = ch->desc;
		ch->desc = nullptr;
	}
}

ACMD(do_return)
{
	if (ch->desc && ch->desc->original)
	{
		send_to_char("You return to your original body.\r\n", ch);

		/* JE 2/22/95 */
		/* if someone switched into your original body, disconnect them */
		if (ch->desc->original->desc)
			 STATE(ch->desc->original->desc) = CON_DISCONNECT;

		ch->desc->character = ch->desc->original;
		ch->desc->original = nullptr;

		ch->desc->character->desc = ch->desc;
		ch->desc = nullptr;
	}
}

/* ACMD(do_load)
{
	char_data *mob;
	struct obj_data *obj;
	int number, r_num;

	two_arguments(argument, buf, buf2);

	if (!*buf || !*buf2 || !isdigit(*buf2))
	{
		send_to_char("Usage: load { obj | mob } <number>\r\n", ch);
		return;
	}
  
	if ((number = atoi(buf2)) < 0)
	{
		send_to_char("A NEGATIVE number??\r\n", ch);
		return;
	}
  
	if (is_abbrev(buf, "mob"))
	{
		if ((r_num = real_mobile(number)) < 0)
		{
			send_to_char("There is no monster with that number.\r\n", ch);
			return;
		}
    
		mob = read_mobile(r_num, REAL);
		char_to_room(mob, ch->in_room);

		act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
		act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
		
		sprintf(buf, "%s has created %s.", GET_NAME(ch), GET_NAME(mob));
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

		
		load_mtrigger(mob);
	} 
	
	else if (is_abbrev(buf, "obj"))
	{
		if ((r_num = real_object(number)) < 0)
		{
			send_to_char("There is no object with that number.\r\n", ch);
			return;
		}

		obj = read_object(r_num, REAL);

		sprintf(obj->creator, "%s, normal load", GET_NAME(ch));

		if (load_into_inventory)
			obj_to_char(obj, ch);
    
		else
			obj_to_room(obj, ch->in_room);
    
		act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
		act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
		
		sprintf(buf, "%s has created %s.", GET_NAME(ch), obj->short_description);
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
		
		load_otrigger(obj);
	} 
	
	else
		send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}
*/
 ACMD(do_mbload)
{
	char_data *mob;
	int number, r_num;

	one_argument(argument, buf);

	if (!*buf || !isdigit(*buf))
	{
		send_to_char("Usage: mbload <number>\r\n", ch);
		return;
	}
  
	if ((number = atoi(buf)) < 0)
	{
		send_to_char("A NEGATIVE number??\r\n", ch);
		return;
	}
  
	
	
		if ((r_num = real_mobile(number)) < 0)
		{
			send_to_char("There is no monster with that number.\r\n", ch);
			return;
		}
    
		mob = read_mobile(r_num, REAL);
		char_to_room(mob, ch->in_room);

		act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
		act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
		
		sprintf(buf, "%s has created %s.", GET_NAME(ch), GET_NAME(mob));
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

		
		load_mtrigger(mob);
	 
	

}


 ACMD(do_oload)
{
	
	struct obj_data *obj;
	int number, r_num;

	one_argument(argument, buf);

	if (!*buf || !isdigit(*buf))
	{
		send_to_char("Usage: oload <number>\r\n", ch);
		return;
	}
  
	if ((number = atoi(buf)) < 0)
	{
		send_to_char("A NEGATIVE number??\r\n", ch);
		return;
	}
  
	

		if ((r_num = real_object(number)) < 0)
		{
			send_to_char("There is no object with that number.\r\n", ch);
			return;
		}

		obj = read_object(r_num, REAL);

		sprintf(obj->creator, "%s, normal load", GET_NAME(ch));

		if (load_into_inventory)
			obj_to_char(obj, ch);
    
		else
			obj_to_room(obj, ch->in_room);
    
		act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
		act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
		
		sprintf(buf, "%s has created %s.", GET_NAME(ch), obj->short_description);
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
		
		load_otrigger(obj);
	} 
	

ACMD(do_vstat)
{
	char_data *mob;
	struct obj_data *obj;
	int number, r_num;

	two_arguments(argument, buf, buf2);

	if (!*buf || !*buf2 || !isdigit(*buf2))
	{
		send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
		return;
	}
  
	if ((number = atoi(buf2)) < 0)
	{
		send_to_char("A NEGATIVE number??\r\n", ch);
		return;
	}
  
	if (is_abbrev(buf, "mob"))
	{
		if ((r_num = real_mobile(number)) < 0)
		{
			send_to_char("There is no monster with that number.\r\n", ch);
			return;
		}
    
		mob = read_mobile(r_num, REAL);
		char_to_room(mob, 0);
		do_stat_character(ch, mob);
		extract_char(mob);
	} 
	
	else if (is_abbrev(buf, "obj"))
	{
		if ((r_num = real_object(number)) < 0)
		{
			send_to_char("There is no object with that number.\r\n", ch);
			return;
		}
    
		obj = read_object(r_num, REAL);
		do_stat_object(ch, obj);
		extract_obj(obj);
	} 
	
	else
		send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}

/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
	char_data *vict, *next_v;
	struct obj_data *obj, *next_o;

	one_argument(argument, buf);

	if (*buf)
	{
		if ((vict = get_char_room_vis(ch, buf)))
		{
			if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict)))
			{
				send_to_char("Fuuuuuuuuu!\r\n", ch);
				return;
			}

			if(!str_cmp(GET_NAME(vict), "Tulon"))
			{
				send_to_char("You can't strike the dictator! He is just much more... superb than you!\r\n", ch);
				return;
			}
      
			act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

			if (!IS_NPC(vict))
			{
				sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
				mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
	
				if (vict->desc)
				{
					STATE(vict->desc) = CON_CLOSE;
					vict->desc->character = nullptr;
					vict->desc = nullptr;
				}
			}
      
			extract_char(vict);
		} 
		
		else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents)))
		{

			if(IS_OBJ_STAT(obj, ITEM_CHEST) && GET_LEVEL(ch) < LVL_IMPL)
			{
				message(ch, "You CANNOT destroy a chest as a level %d -EVER-\r\n", ch);
				return;
			}

			act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
			extract_obj(obj);
		} 
		
		else
		{
			send_to_char("Nothing here by that name.\r\n", ch);
			return;
		}

		send_to_char(OK, ch);
	} 
	
	else
	{			/* no argument. clean out the room */
		act("$n gestures... You are surrounded by scorching flames!",
		FALSE, ch, 0, 0, TO_ROOM);
		send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

		for (vict = world[ch->in_room].people; vict; vict = next_v)
		{
			next_v = vict->next_in_room;
      
			if (IS_NPC(vict))
				extract_char(vict);
		}

		for (obj = world[ch->in_room].contents; obj;obj = next_o)
		{
			
			next_o = obj->next_content;

			if(obj)
				if(IS_OBJ_STAT(obj, ITEM_CHEST))
					continue;

			extract_obj(obj);
	
		}	
	}
}

const char *logtypes[] =
{
	"off",
	"brief",
	"normal",
	"complete",
	"\n"
};

ACMD(do_syslog)
{
	int tp;

	one_argument(argument, arg);

	if (!*arg)
	{
		tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
		(PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
		sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
		send_to_char(buf, ch);
		return;
	}
  
	if (((tp = search_block(arg, ((const char **) logtypes), FALSE)) == -1))
	{
		send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
		return;
	}
  
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);
  
	if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
	if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);

	sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
	send_to_char(buf, ch);
}



ACMD(do_advance)
{
	char_data *victim;
	char *name = arg, *level = buf2;
	int newlevel, oldlevel, i = 0;

	two_arguments(argument, name, level);

	if (*name)
	{
		if (!(victim = get_char_vis(ch, name)))
		{
			send_to_char("That player is not here.\r\n", ch);
			return;
		}
	} 
	
	else {
		send_to_char("Advance who?\r\n", ch);
		return;
	}

	if (GET_LEVEL(ch) <= GET_LEVEL(victim))
	{
		send_to_char("Maybe that's not such a great idea.\r\n", ch);
		return;
	}
  
	if (IS_NPC(victim))
	{
		send_to_char("NO!  Not on NPC's.\r\n", ch);
		return;
	}
  
	if (!*level || (newlevel = atoi(level)) <= 0)
	{
		send_to_char("That's not a level!\r\n", ch);
		return;
	}
  
	if (newlevel > LVL_IMPL)
	{
		sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
		send_to_char(buf, ch);
		return;
	}
  
	if (newlevel > GET_LEVEL(ch))
	{
		send_to_char("Yeah, right.\r\n", ch);
		return;
	}
  
	if (newlevel == GET_LEVEL(victim))
	{
		send_to_char("They are already at that level.\r\n", ch);
		return;
	}
  
	oldlevel = GET_LEVEL(victim);
	
	if (newlevel < GET_LEVEL(victim))
	{
		message(ch, "%s's level is higher than %d.\r\n", GET_NAME(victim), newlevel);
		return;
	} 
	
	else
	{
		act("You gain some levels thanks to some nice immortal!", FALSE, ch, 0, victim, TO_VICT);
	}

	send_to_char(OK, ch);

	for(i = GET_LEVEL(victim);i < newlevel;i++)
	{
		GET_LEVEL(victim)++;
		advance_level(victim, FALSE);
	}

	message(ch, "You raise %s's level from %d to %d.\r\n", GET_NAME(victim), newlevel, oldlevel);
	victim->save();

	sprintf(buf, "%s raised %s's level from %d to %d.", GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);

	GET_EXP(victim) = level_exp(GET_LEVEL(victim));
}

ACMD(do_restore)
{
	char_data *vict;
	int i;

	one_argument(argument, buf);
  
	if (!*buf)
		send_to_char("Whom do you wish to restore?\r\n", ch);
  
	else if (!(vict = get_char_vis(ch, buf)))
		send_to_char(NOPERSON, ch);
  
	else
	{
		GET_HIT(vict) = GET_MAX_HIT(vict);
		GET_MANA(vict) = GET_MAX_MANA(vict);
		GET_MOVE(vict) = GET_MAX_MOVE(vict);
		GET_SHADOW(vict) = GET_MAX_SHADOW(vict);

		if ((GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_IMMORT))
		{
			for (i = 1; i <= MAX_SKILLS; i++)
				SET_SKILL(vict, i, 100);

		if (GET_LEVEL(vict) >= LVL_GOD)
		{
			vict->real_abils.str_add = 100;
			vict->real_abils.intel = 25;
			vict->real_abils.wis = 25;
			vict->real_abils.dex = 25;
			vict->real_abils.str = 25;
			vict->real_abils.con = 25;
			vict->real_abils.cha = 25;
		}
      
		vict->aff_abils = vict->real_abils;
	}
    
	update_pos(vict);
	send_to_char(OK, ch);
	act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);

	sprintf(buf, "%s restored %s's status points.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

	}
}

void perform_immort_vis(char_data *ch)
{
	if (GET_INVIS_LEV(ch) == 0 && !AFF_FLAGGED(ch, AFF_HIDE | AFF_INVISIBLE))
	{
		send_to_char("You are already fully visible.\r\n", ch);
		return;
	}
   
	GET_INVIS_LEV(ch) = 0;
	appear(ch);
	send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(char_data *ch, int level)
{
	char_data *tch;

	if (IS_NPC(ch))
		return;

	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
	{
		
		if (tch == ch)
			continue;
    
		if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
			act("$n steps back into the shadows, and is gone.", FALSE, ch, 0,
			tch, TO_VICT);
    
		if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
			act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
			tch, TO_VICT);
	}

	GET_INVIS_LEV(ch) = level;
	sprintf(buf, "Your invisibility level is %d.\r\n", level);
	send_to_char(buf, ch);
}

ACMD(do_invis)
{
	int level;

	if (IS_NPC(ch))
	{
		send_to_char("You can't do that!\r\n", ch);
		return;
	}

	one_argument(argument, arg);
  
	if (!*arg)
	{
		
		if (GET_INVIS_LEV(ch) > 0)
			perform_immort_vis(ch);
    
		else
			perform_immort_invis(ch, GET_LEVEL(ch));
	} 
	
	else
	{
		level = atoi(arg);
    
		if (level > GET_LEVEL(ch))
			send_to_char("You can't go invisible above your own level.\r\n", ch);
    
		else if (level < 1)
			perform_immort_vis(ch);
    
		else
			perform_immort_invis(ch, level);
	}
}

ACMD(do_gecho)
// EDITED BY STARK ON OCT 2003 TO TELL IMMS WHO DID WHAT GECHO
{
	struct descriptor_data *pt;

	skip_spaces(&argument);
	delete_doubledollar(argument);

	if (!*argument)
		send_to_char("That must be a mistake...\r\n", ch);
  
	else
	{
		sprintf(buf, "(GC) %s has broadcasted the gecho: %s", GET_NAME(ch), argument);
		mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    sprintf(buf, "%s\r\n", argument);
		for (pt = descriptor_list; pt; pt = pt->next)
			if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch)
				send_to_char(buf, pt->character);
    
			if (PRF_FLAGGED(ch, PRF_NOREPEAT))
				send_to_char(OK, ch);
    
			else
				send_to_char(buf, ch);
	}
}

ACMD(do_poofset)
{

	skip_spaces(&argument);

	if(subcmd == SCMD_POOFIN)
		strcpy(POOFIN(ch), argument);

	else if(subcmd == SCMD_POOFOUT)
		strcpy(POOFOUT(ch), argument);

	else
		return;
	
	send_to_char(OK, ch);
}

ACMD(do_dc)
{
	struct descriptor_data *d;
	int num_to_dc;

	one_argument(argument, arg);
  
	if (!(num_to_dc = atoi(arg)))
	{
		send_to_char("Usage: DC <user number> (type USERS for a list)\r\n", ch);
		return;
	}
  
	for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

	if (!d)
	{
		send_to_char("No such connection.\r\n", ch);
		return;
	}
  
	if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch))
	{
		if (!CAN_SEE(ch, d->character))
			send_to_char("No such connection.\r\n", ch);
    
		else
			send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    
		return;
	}

  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor.
   *
   * It is a much more logical extension for a CON_DISCONNECT to be used
   * for in-game socket closes and CON_CLOSE for out of game closings.
   * This will retain the stability of the close_me hack while being
   * neater in appearance. -gg 12/1/97
   */
	STATE(d) = CON_DISCONNECT;
	sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
	send_to_char(buf, ch);
	log("(GC) Connection closed by %s.", GET_NAME(ch));

	sprintf(buf, "%s has closed %s's socket.", GET_NAME(ch), GET_NAME(d->character));
	mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

}

ACMD(do_wizlock)
{
	int value;
	const char *when;

	one_argument(argument, arg);

	if (*arg)
	{
		value = atoi(arg);

		if (value < 0 || value > GET_LEVEL(ch))
		{
			send_to_char("Invalid wizlock value.\r\n", ch);
			return;
		}

		circle_restrict = value;
		when = "now";
	}

	else
		when = "currently";

	switch (circle_restrict)
	{

	case 0:
		sprintf(buf, "The game is %s completely open.\r\n", when);
		break;
  
	case 1:
		sprintf(buf, "The game is %s closed to new players.\r\n", when);
		break;
  
	default:
		sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
		circle_restrict, when);
		break;
	}

	send_to_char(buf, ch);
}

ACMD(do_date)
{
	char *tmstr;
	time_t mytime;
	int d, h, m;

	if (subcmd == SCMD_DATE)
		mytime = time(0);
  
	else
		mytime = boot_time;

	tmstr = (char *) asctime(localtime(&mytime));
	*(tmstr + strlen(tmstr) - 1) = '\0';

	if (subcmd == SCMD_DATE)
		sprintf(buf, "Current machine time: %s\r\n", tmstr);
  
	else
	{
		mytime = time(0) - boot_time;
		d = mytime / 86400;
		h = (mytime / 3600) % 24;
		m = (mytime / 60) % 60;

		sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
		((d == 1) ? "" : "s"), h, m);
	}

	send_to_char(buf, ch);
}

ACMD(do_last)
{
	char_data *vict;

	one_argument(argument, arg);
  
	if (!*arg)
	{
		send_to_char("For whom do you wish to search?\r\n", ch);
		return;
	}

	if(!playerExists(arg))
	{
		send_to_char("There is no such player.\r\n", ch);
		return;
	}

	CREATE(vict, char_data, 1);
	clear_char(vict);

	if(!vict->load(arg))
	{
		send_to_char("Error loading player.", ch);
		free_char(vict);
	}

	if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL))
	{
		send_to_char("You are not sufficiently godly enough for that!\r\n", ch);
		extract_char(vict);
		return;
	}

	message(ch, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
		GET_IDNUM(vict), GET_LEVEL(vict), class_abbrevs[(int) GET_CLASS(vict)],
		GET_NAME(vict), vict->points.host, ctime(&LAST_LOGON(vict)));

	sprintf(buf, "%s checked %s's last login.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

	free_char(vict);
}

ACMD(do_force)
{
	struct descriptor_data *i, *next_desc;
	char_data *vict, *next_force;
	char to_force[MAX_INPUT_LENGTH + 2];

	half_chop(argument, arg, to_force);

	sprintf(buf1, "$n has forced you to '%s'.", to_force);

	if (!*arg || !*to_force)
		send_to_char("Whom do you wish to force do what?\r\n", ch);
  
	else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg)))
	{
    
		if (!(vict = get_char_vis(ch, arg)))
			send_to_char(NOPERSON, ch);
    
		else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
			send_to_char("No, no, no!\r\n", ch);

		else
		{
			send_to_char(OK, ch);
			act(buf1, TRUE, ch, nullptr, vict, TO_VICT);

			sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
			mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

			command_interpreter(vict, to_force);
		}
	} 
	
	else if (!str_cmp("room", arg))
	{
		send_to_char(OK, ch);
		sprintf(buf, "(GC) %s forced room %d to %s",
		GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);
		mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

		for (vict = world[ch->in_room].people; vict; vict = next_force)
		{
			next_force = vict->next_in_room;
      	
			if (GET_LEVEL(vict) >= GET_LEVEL(ch))
				continue;
      
			act(buf1, TRUE, ch, nullptr, vict, TO_VICT);
			command_interpreter(vict, to_force);
		}
	} 
	
	else
	{ /* force all */
		send_to_char(OK, ch);
		sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
		mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

		for (i = descriptor_list; i; i = next_desc)
		{
			next_desc = i->next;

		if (STATE(i) != CON_PLAYING || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))	
			continue;
      
		strcpy(GET_FORCED(vict), GET_NAME(ch));
		act(buf1, TRUE, ch, nullptr, vict, TO_VICT);
		command_interpreter(vict, to_force);
		strcpy(GET_FORCED(vict), "");
		}
	}
}

ACMD(do_wiznet)
{
	struct descriptor_data *d;
	char emote = FALSE;
	char any = FALSE;
	int level = LVL_IMMORT;

	skip_spaces(&argument);
	delete_doubledollar(argument);

	if (!*argument) 
	{
		send_to_char(	"Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
						"       wiznet @<level> *<emotetext> | wiz @\r\n", ch);
		return;
	}
  
	switch (*argument)
	{
  
	case '*':
		emote = TRUE;
  
	case '#':
		one_argument(argument + 1, buf1);
    
		if (is_number(buf1)) 
		{
			half_chop(argument+1, buf1, argument);
			level = MAX(atoi(buf1), LVL_IMMORT);
      
			if (level > GET_LEVEL(ch)) 
			{
				send_to_char("You can't wizline above your own level.\r\n", ch);
				return;
			}
		} 
		
		else if (emote)
			argument++;
		
		break;
 
	case '@':
    
		for (d = descriptor_list; d; d = d->next) 
		{
			if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
			!PRF_FLAGGED(d->character, PRF_NOWIZ) &&
			(CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) 
			{
	
				if (!any) 
				{
					strcpy(buf1, "Gods online:\r\n");
					any = TRUE;
				}
	
				sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
	
				if (PLR_FLAGGED(d->character, PLR_WRITING))
					strcat(buf1, " (Writing)\r\n");
	
				else if (PLR_FLAGGED(d->character, PLR_MAILING))
					strcat(buf1, " (Writing mail)\r\n");
	
				else
					strcat(buf1, "\r\n");

			}
		}
    
		any = FALSE;
    
		for (d = descriptor_list; d; d = d->next) 
		{
			if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
			PRF_FLAGGED(d->character, PRF_NOWIZ) &&
			CAN_SEE(ch, d->character))
			{
	
				if (!any) 
				{
					strcat(buf1, "Gods offline:\r\n");
					any = TRUE;
	
				}
				sprintf(buf1 + strlen(buf1), "  %s\r\n", GET_NAME(d->character));
			}
		}
    
		send_to_char(buf1, ch);
		return;
  
	case '\\':
		++argument;
		break;
  
	default:
		break;
	}
  
	if (PRF_FLAGGED(ch, PRF_NOWIZ)) 
	{
		send_to_char("You are offline!\r\n", ch);
		return;
	}
  
	skip_spaces(&argument);

  
	if (!*argument)
	{
		send_to_char("Don't bother the gods like that!\r\n", ch);
		return;
	}
  
	if (level > LVL_IMMORT)
	{
		sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
		emote ? "<--- " : "", argument);
		sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
		argument);
	} 
	
	else
	{
		sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
		argument);
		sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
	}

	for (d = descriptor_list; d; d = d->next) 
	{
    
		if ((STATE(d) == CON_PLAYING) && (GET_LEVEL(d->character) >= level) &&
		(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
		(!PLR_FLAGGED(d->character, PLR_WRITING) ||
		!PLR_FLAGGED(d->character, PLR_MAILING))
		&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) 
		{
      
			send_to_char(COLOR_CYAN(d->character, CL_NORMAL), d->character);
      
			if (CAN_SEE(d->character, ch))
				send_to_char(buf1, d->character);
      
			else
				send_to_char(buf2, d->character);
      
			send_to_char(COLOR_NORMAL(d->character, CL_NORMAL), d->character);
		}
	}

	if (PRF_FLAGGED(ch, PRF_NOREPEAT))
		send_to_char(OK, ch);
}

ACMD(do_zreset)
{
	int i, j;

	one_argument(argument, arg);
  
	if (!*arg) 
	{
		send_to_char("You must specify a zone.\r\n", ch);
		return;
	}
  
	if (*arg == '*') 
	{
		for (i = 0; i <= top_of_zone_table; i++)
			reset_zone(i);
			
		send_to_char("Reset world.\r\n", ch);
		sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
		mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
		return;
	}	
	
	else if (*arg == '.')
		i = world[ch->in_room].zone;
  
	else 
	{
		j = atoi(arg);
    
		for (i = 0; i <= top_of_zone_table; i++)
			if (zone_table[i].number == j)
				break;
	}
  
	if (i >= 0 && i <= top_of_zone_table)
	{
		reset_zone(i);
		sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
		zone_table[i].name);
		send_to_char(buf, ch);
		sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
		mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
	} 
	
	else
		send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_tell_mute)
{

	char_data *vict;
	char arg[MAX_INPUT_LENGTH];
	long result = 0;

	one_argument(argument, arg);

	if(!*arg)
	{
		send_to_char("But who are you wanting to do this to?\r\n", ch);
		return;
	}

	if(!(vict = get_char_vis(ch, arg)))
	{
		message(ch, "There is no player by that name.\r\n");
		return;
	}

	if (GET_LEVEL(vict) > GET_LEVEL(ch) || (IS_KING(vict) && !IS_KING(ch)))
	{
		message(ch, "That isn't such a good idea...\r\n");
		return;
	}
		
	result = PRF_TOG_CHK(vict, PRF_TELL_MUTE);
	sprintf(buf, "(GC) Tell Mute %s for %s by %s.", ONOFF(result),
	GET_NAME(vict), GET_NAME(ch));
	mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
	message(ch, "%s is now mute to tells, and cannot send them.\r\n", GET_NAME(vict));
	send_to_char("You have lost your ability to send tells.\r\n", vict);

}

ACMD(do_reroll)
{

	char_data *vict;
	char name[MAX_INPUT_LENGTH], type[MAX_INPUT_LENGTH];

	two_arguments(argument, name, type);

	if(!argument || !*name)
	{
		send_to_char("Who do you wish to re-roll?\r\n", ch);
		return;
	}

	if(!*type)
	{
		send_to_char("Possible re-roll options: Moves, Stats.\r\n", ch);
		return;
	}

	if (!(vict = get_char_vis(ch, name)))
	{
		send_to_char("There is no such player.\r\n", ch);
		return;
	}

	if(!strn_cmp(type, "stats", strlen(type)))
	{
		send_to_char("Stats re-rolled...\r\n", ch);
		roll_real_abils(vict);
		
		sprintf(buf2, "(GC) %s has rerolled %s's stats.", GET_NAME(ch), GET_NAME(vict));
		mudlog(buf2, NRM, MAX(LVL_BLDER, GET_INVIS_LEV(ch)), TRUE);

		sprintf(buf, "New stats: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d.\r\n",
		GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
		GET_DEX(vict), GET_CON(vict));
		send_to_char(buf, ch);
	}

	else if(!strn_cmp(type, "moves", strlen(type)))
	{

		send_to_char("Moves re-rolled...\r\n", ch);
		roll_moves(vict);
		
		sprintf(buf, "(GC) %s has rerolled %s's moves.", GET_NAME(ch), GET_NAME(vict));
		mudlog(buf, NRM, MAX(LVL_BLDER, GET_INVIS_LEV(ch)), TRUE);

		sprintf(buf, "Rerolled moves to %d.\r\n", GET_MAX_MOVE(vict));
		send_to_char(buf, ch);
	}

	else
	{
		send_to_char("Possible re-roll options: Moves, Stats.\r\n", ch);
		return;
	}

	return;
}

ACMD(do_wizutil)
{
	char_data *vict;
	long result;

	one_argument(argument, arg);

  
	if (!*arg)
		send_to_char("Yes, but for whom?!?\r\n", ch);
  
	else if (!(vict = get_char_vis(ch, arg)))
		send_to_char("There is no such player.\r\n", ch);
  
	else if (IS_NPC(vict))
		send_to_char("You can't do that to a mob!\r\n", ch);
  
	else if (GET_LEVEL(vict) > GET_LEVEL(ch) || IS_KING(vict) && !IS_KING(ch))
		send_to_char("Hmmm...you'd better not.\r\n", ch);
  
	else
	{
    
		switch (subcmd)
		{
    
		case SCMD_NOTITLE:
			result = PLR_TOG_CHK(vict, PLR_NOTITLE);
			sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
			GET_NAME(vict), GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
			strcat(buf, "\r\n");
			send_to_char(buf, ch);
			break;
    
		case SCMD_SQUELCH:
			result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
			sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
			GET_NAME(vict), GET_NAME(ch));
			mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
			strcat(buf, "\r\n");
			send_to_char(buf, ch);
			break;
    
		case SCMD_FREEZE:
      
			if (ch == vict)
			{
				send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
				return;
			}
      
			if (PLR_FLAGGED(vict, PLR_FROZEN)) 
			{
				send_to_char("Your victim is already pretty cold.\r\n", ch);
				return;
			}
      
			SET_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
			GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
			send_to_char("A bitter wind suddenly rises and drains the heat from your body!\r\nYou feel frozen!\r\n",
			vict);
			send_to_char("Frozen.\r\n", ch);
			act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
			sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
			mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
			break;
    
		case SCMD_THAW:
      
			if (!PLR_FLAGGED(vict, PLR_FROZEN)) 
			{
				send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
				return;
			}
      
			if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) 
			{
				sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
				GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
				send_to_char(buf, ch);
				return;
			}
      
			sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
			mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
			REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
			send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
			send_to_char("Thawed.\r\n", ch);
			act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
			break;
    
		case SCMD_UNAFFECT:
      
			if (vict->affected) 
			{
				while (vict->affected)
					affect_remove(vict, vict->affected);
	
				send_to_char(	"There is a brief flash of light!\r\n"
								"You feel slightly different.\r\n", vict);
				send_to_char("All spells removed.\r\n", ch);
				REMOVE_BIT_AR(PRF_FLAGS(vict), PRF_HOLYLIGHT);
			} 
			
			else 
			{
				send_to_char("Your victim does not have any affections!\r\n", ch);
				return;
			}
      
			break;

		case SCMD_ZONE_BAN:

			if(GET_LEVEL(vict) < LVL_IMMORT || GET_LEVEL(vict) >= LVL_IMPL)
			{
				send_to_char("The level range must be between 100 and 104.\r\n", ch);
				return;
			}

			if(!PLR_FLAGGED(vict, PLR_ZONE_BAN))
			{
				message(ch, "%s now cannot leave %s zones.\r\n", GET_NAME(vict), HSHR(vict));
				SET_BIT_AR(PLR_FLAGS(vict), PLR_ZONE_BAN);
				sprintf(buf, "%s has zone banned %s from leaving %s zones.", GET_NAME(ch), GET_NAME(vict), HSHR(vict));
				mudlog(buf, BRF, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
			}

			else
			{
				message(ch, "%s can now leave %s zones.\r\n", GET_NAME(vict), HSHR(vict));
				REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_ZONE_BAN);
				sprintf(buf, "%s has removed %s's zone ban.", GET_NAME(ch), GET_NAME(vict));
				mudlog(buf, BRF, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
			}

			break;
    
		default:
			log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
			break;
		}
    
		vict->save();
	}
}

/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
	sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
	bufptr, zone_table[zone].number, zone_table[zone].name,
	zone_table[zone].age, zone_table[zone].lifespan,
	zone_table[zone].reset_mode, zone_table[zone].top);
}

ACMD(do_show)
{
	int i, j, k, l, con;
	char self = 0;
	char_data *vict;
	struct obj_data *obj;
	char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];

	struct show_struct
	{
		const char *cmd;
		int level;
	};
  
	struct show_struct fields[] =
	{
		{ "nothing",	LVL_IMMORT  },		/* 0 */
		{ "zones",		LVL_IMMORT },		
		{ "player",		LVL_GOD },
		{ "rent",		LVL_GOD },
		{ "stats",		LVL_IMMORT },
		{ "errors",		LVL_GOD },			/* 5 */
		{ "death",		LVL_GOD },
		{ "godrooms",	LVL_GOD },
		{ "shops",		LVL_IMMORT },
		{ "houses",		LVL_GOD },
		{ "buffers",	LVL_GOD },			/* 10 */
		{ "\n",			0 }
	};

	skip_spaces(&argument);

	if (!*argument)
	{
		strcpy(buf, "Show options:\r\n");

		for (j = 0, i = 1; fields[i].level; i++)
			if (fields[i].level <= GET_LEVEL(ch))
				sprintf(buf + strlen(buf), "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));

			strcat(buf, "\r\n");
			send_to_char(buf, ch);
			return;
	}

	strcpy(arg, two_arguments(argument, field, value));

	for (l = 0; *(fields[l].cmd) != '\n'; l++)
		if (!strncmp(field, fields[l].cmd, strlen(field)))
			break;

	if (GET_LEVEL(ch) < fields[l].level)
	{
		send_to_char("You are not godly enough for that!\r\n", ch);
		return;
	}

	if (!strcmp(value, "."))
		self = 1;

	buf[0] = '\0';

	switch (l)
	{

	case 1:			/* zone */
		/* tightened up by JE 4/6/93 */

		if (self)
			print_zone_to_buf(buf, world[ch->in_room].zone);

		else if (*value && is_number(value))
		{
			for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
				if (i <= top_of_zone_table)
					print_zone_to_buf(buf, i);

				else
				{
					send_to_char("That is not a valid zone.\r\n", ch);
					return;
				}
		}

		else
			for (i = 0; i <= top_of_zone_table; i++)
				print_zone_to_buf(buf, i);

			page_string(ch->desc, buf, TRUE);
		
		break;

	case 2:			/* player */
		if (!*value)
		{
			send_to_char("A name would help.\r\n", ch);
			return;
		}

		CREATE(vict, char_data, 1);
		clear_char(vict);

		if (!vict->load(value))
		{
			send_to_char("There is no such player.\r\n", ch);
			free_char(vict);
			return;
		}


		sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", GET_NAME(vict),
			genders[GET_SEX(vict)], GET_LEVEL(vict), class_abbrevs[GET_CLASS(vict)]);

		sprintf(buf,
			"%sAu: %-8d  Bal: %-8d  Exp: %-8d  Practices: %-3d  Spell Practices: %-3d\r\n",
			buf, GET_GOLD(vict), GET_BANK_GOLD(vict), GET_EXP(vict),
			GET_PRACTICES(vict), GET_SPRACTICES(vict));

		strcpy(birth, ctime(&vict->player.time.birth));

		sprintf(buf,
			"%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
			buf, birth, ctime(&LAST_LOGON(vict)), (int) (vict->player.time.played / 3600),
			(int) (vict->player.time.played / 60 % 60));

		send_to_char(buf, ch);

		free_char(vict);
		break;

	case 3:
		Crash_listrent(ch, value);
		break;

	case 4:
		i = 0;
		j = 0;
		k = 0;
		con = 0;

		for (vict = character_list; vict; vict = vict->next)
		{
			if (IS_NPC(vict))
				j++;
			
			else if (CAN_SEE(ch, vict))
			{
				i++;

				if (vict->desc)
					con++;
			}
		}

		for (obj = object_list; obj; obj = obj->next)
			k++;

		strcpy(buf, "Current stats:\r\n");
		sprintf(buf + strlen(buf), "  %5d players in game  %5d connected\r\n",
			i, con);

		sprintf(buf + strlen(buf), "  %5d registered\r\n",
			top_id + 1);
		
		sprintf(buf + strlen(buf), "  %5d mobiles          %5d prototypes\r\n",
			j, top_of_mobt + 1);
		
		sprintf(buf + strlen(buf), "  %5d objects          %5d prototypes\r\n",
			k, top_of_objt + 1);

		sprintf(buf + strlen(buf), "  %5d rooms            %5d zones\r\n",
			top_of_world + 1, top_of_zone_table + 1);

		sprintf(buf + strlen(buf), "  %5d large bufs\r\n",
			buf_largecount);

		sprintf(buf + strlen(buf), "  %5d buf switches     %5d overflows\r\n",
			buf_switches, buf_overflows);

		sprintf(buf + strlen(buf), "  %5d buf cache hits   %5d buf cache misses\r\n",
			buffer_cache_stat(BUFFER_CACHE_HITS), buffer_cache_stat(BUFFER_CACHE_MISSES));

		send_to_char(buf, ch);
		break;

	case 5:
		strcpy(buf, "Errant Rooms\r\n------------\r\n");

		for (i = 0, k = 0; i <= top_of_world; i++)
			for (j = 0; j < NUM_OF_DIRS; j++)
				if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
					sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++k, GET_ROOM_VNUM(i),
					world[i].name);

		page_string(ch->desc, buf, TRUE);
		break;

	case 6:
		strcpy(buf, "Death Traps\r\n-----------\r\n");

		for (i = 0, j = 0; i <= top_of_world; i++)
			if (ROOM_FLAGGED(i, ROOM_DEATH))
				sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++j,	
				GET_ROOM_VNUM(i), world[i].name);

		page_string(ch->desc, buf, TRUE);
		break;

	case 7:
		strcpy(buf, "Godrooms\r\n--------------------------\r\n");

		for (i = 0, j = 0; i < top_of_world; i++)
			if (ROOM_FLAGGED(i, ROOM_GODROOM))
				sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
					++j, GET_ROOM_VNUM(i), world[i].name);

		page_string(ch->desc, buf, TRUE);
		break;

	case 8:
		show_shops(ch, value);
		break;

	case 9:
		hcontrol_list_houses(ch);
		break;

	case 10:
		show_buffers(ch, -1, 1);
		show_buffers(ch, -1, 2);
		show_buffers(ch, -1, 0);

	default:
		send_to_char("Sorry, I don't understand that.\r\n", ch);
		break;
	}
}


/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT_AR(flagset, flags); \
	else if (off) REMOVE_BIT_AR(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


	/* The set options available */
	struct set_struct
	{
		const char *cmd;
		int level;
		int pcnpc;
		int type;
	}
  
	set_fields[] =
	{
		{ "brief",		LVL_GOD, 	PC, 	BINARY	},  // 0 //
		{ "invstart", 	LVL_GOD, 	PC, 	BINARY	},  // 1 //
		{ "title",		LVL_GOD, 	PC,		MISC	},
		{ "maxhit",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "maxmana", 	LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "maxmove", 	LVL_GRGOD, 	BOTH, 	NUMBER	},	// 5 //
		{ "hit", 		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "mana",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "move",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "str",	    LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "int", 		LVL_GRGOD, 	BOTH, 	NUMBER	},	// 10 //
		{ "wis", 		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "dex", 		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "con", 		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "cha",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "gold",		LVL_GRGOD, 	BOTH, 	NUMBER	},	// 15 //
		{ "bank",		LVL_GOD, 	PC, 	NUMBER	},
		{ "exp", 		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "damroll", 	LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "invis",		LVL_GRGOD, 	PC, 	NUMBER	},
		{ "nohassle", 	LVL_GRGOD, 	PC, 	BINARY	},	// 20 //
		{ "frozen",		LVL_FREEZE, PC, 	BINARY	},
		{ "practices", 	LVL_GRGOD, 	PC, 	NUMBER	},
		{ "drunk",		LVL_GRGOD, 	BOTH, 	MISC	},
		{ "hunger",		LVL_GRGOD, 	BOTH, 	MISC	},
		{ "thirst",		LVL_GRGOD, 	BOTH, 	MISC	},	// 25 //
		{ "darkfriend",	LVL_APPR, 	PC, 	BINARY	},
		{ "murderer",	LVL_GOD, 	PC, 	BINARY	},
		{ "level",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "room",		LVL_GRGOD, 	BOTH, 	NUMBER	},
		{ "roomflag", 	LVL_GRGOD, 	PC, 	BINARY	},	// 30 //
		{ "siteok",		LVL_GRGOD, 	PC, 	BINARY	},
		{ "deleted", 	LVL_IMPL, 	PC, 	BINARY	},
		{ "class",		LVL_GRGOD, 	BOTH, 	MISC	},
		{ "nowizlist", 	LVL_GOD, 	PC, 	BINARY	},
		{ "loadroom", 	LVL_GRGOD, 	PC, 	MISC	},	// 35 //
		{ "color",		LVL_GOD, 	PC, 	BINARY	},
		{ "idnum",		LVL_IMPL, 	PC, 	NUMBER	},
		{ "password",	LVL_IMPL, 	PC, 	MISC	},
		{ "nodelete", 	LVL_GOD, 	PC, 	BINARY	},
		{ "sex", 		LVL_GRGOD, 	BOTH, 	MISC	},	// 40 //
		{ "age",		LVL_GRGOD,	BOTH,	NUMBER	},
		{ "qp", 		LVL_GRGOD, 	PC, 	NUMBER	}, 
		{ "race",		LVL_GOD,    BOTH,   MISC	},
		{ "ob",			LVL_GOD,    BOTH,   NUMBER	},
		{ "db",			LVL_GOD,    BOTH,   NUMBER	},	// 45 //
		{ "pb",			LVL_GOD,    BOTH,   NUMBER	},
		{ "abs",		LVL_GOD,    BOTH,   NUMBER	},
		{ "clan",		LVL_GOD,    PC,     NUMBER	},
		{ "mood",	    LVL_GRGOD,  PC,     NUMBER	},
		{ "notell",		LVL_APPR,   PC,     BINARY	},	// 50 //
		{ "weave",		LVL_GRGOD,  PC,     NUMBER	},
		{ "shadow",		LVL_GRGOD,  PC,     NUMBER	},
		{ "maxshadow",	LVL_GRGOD,  PC,     NUMBER	},
		{ "masterweapon",LVL_APPR,	PC,		NUMBER	},
		{ "taint",      LVL_GRGOD,  PC,     NUMBER	},	// 55 //
		{ "logger",		LVL_IMPL,	PC,		BINARY	},
		{ "whoisextra",LVL_GRGOD,	PC,		MISC	},
		{ "\n", 0, BOTH, MISC }
	};

void list_set(char_data *ch)
{
	int i = 0;

	for(i = 0;set_fields[i].cmd != "\n";i++)
		if(GET_LEVEL(ch) >= set_fields[i].level)
		{ 
			sprintf(buf, "Field: %s,    Min Level: %d\r\n", set_fields[i].cmd, set_fields[i].level);
			send_to_char(buf, ch);
		}
}

int perform_set(char_data *ch, char_data *vict, int mode,
		char *val_arg, int file)
{
  
	int i, on = 0, off = 0, value = 0;
	char output[MAX_STRING_LENGTH];

	// Check to make sure all the levels are correct //
	if (GET_LEVEL(ch) != LVL_IMPL)
	{
		if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch)
		{
			send_to_char("Maybe that's not such a great idea...\r\n", ch);
			return 0;
		}
	}
  
	if (GET_LEVEL(ch) < set_fields[mode].level)
	{
		send_to_char("You are not godly enough for that!\r\n", ch);
		return 0;
	}

	/* Make sure the PC/NPC is correct */
	if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC))
	{
		send_to_char("You can't do that to a beast!\r\n", ch);
		return 0;
	} 
	
	else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC))
	{
		send_to_char("That can only be done to a beast!\r\n", ch);
		return 0;
	}

	// Find the value of the argument //
	if (set_fields[mode].type == BINARY)
	{
		if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
			on = 1;
    
		else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
			off = 1;
    
		if (!(on || off))
		{
			send_to_char("Value must be 'on' or 'off'.\r\n", ch);
			return 0;
		}
    
		sprintf(output, "%s %s for %s.", set_fields[mode].cmd, ONOFF(on),
		GET_NAME(vict));
	}		
	
	else if (set_fields[mode].type == NUMBER)
	{
		value = atoi(val_arg);
		sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
		set_fields[mode].cmd, value);
	} 
	
	else
	{
		strcpy(output, "Okay.");  /* can't use OK macro here 'cause of \r\n */
	}

	switch (mode)
	{
  
	case 0:
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
		break;
  
	case 1:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
		break;
  
	case 2:
		set_title(vict, val_arg);
		sprintf(output, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
		break;
  
	case 3:
		vict->points.max_hit = RANGE(1, 5000);
		affect_total(vict);
		break;
  
	case 4:
		vict->points.max_mana = RANGE(1, 5000);
		affect_total(vict);
		break;
  
	case 5:
		vict->points.max_move = RANGE(1, 5000);
		affect_total(vict);
		break;
  
	case 6:
		vict->points.hit = RANGE(-9, vict->points.max_hit);
		affect_total(vict);
		break;
  
	case 7:
		vict->points.mana = RANGE(0, vict->points.max_mana);
		affect_total(vict);
		break;
  
	case 8:
		vict->points.move = RANGE(0, vict->points.max_move);
		affect_total(vict);
		break;
  
	case 9:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.str = value;
		affect_total(vict);
		break;
  
	case 10:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.intel = value;
		affect_total(vict);
		break;
  
	case 11:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.wis = value;
		affect_total(vict);
		break;
  
	case 12:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.dex = value;
		affect_total(vict);
		break;
	
	case 13:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.con = value;
		affect_total(vict);
		break;
  
	case 14:
    
		if (IS_NPC(vict) || GET_LEVEL(vict) >= LVL_GRGOD)
			RANGE(3, 25);
    
		else
			RANGE(3, 19);
    
		vict->real_abils.cha = value;
		affect_total(vict);
		break;
  
	case 15:
		GET_GOLD(vict) = RANGE(0, 100000000);
		break;
  
	case 16:
		GET_BANK_GOLD(vict) = RANGE(0, 100000000);
		break;
  
	case 17:
		vict->points.exp = RANGE(0, 50000000);
		break;
  
	case 18:
		vict->points.damroll = RANGE(-20, 20);
		affect_total(vict);
		break;
  
	case 19:
    
		if (GET_LEVEL(ch) < LVL_IMPL && ch != vict)
		{
			send_to_char("You aren't godly enough for that!\r\n", ch);
			return 0;
		}
    
		GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
		break;
  
	case 20:
    
		if (GET_LEVEL(ch) < LVL_IMPL && ch != vict)
		{
			send_to_char("You aren't godly enough for that!\r\n", ch);
			return 0;
		}
    
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
		break;
  
	case 21:
    
		if (ch == vict)
		{
			send_to_char("Better not -- could be a long winter!\r\n", ch);
			return 0;
		}
    
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
		break;
  
	case 22:
    
		GET_PRACTICES(vict) = RANGE(0, 100);
		break;
  
	case 23:
	case 24:
	case 25:
    
		if (!str_cmp(val_arg, "off"))
		{
			GET_COND(vict, (mode - 23)) = (char) -1; /* warning: magic number here */
			sprintf(output, "%s's %s now off.", GET_NAME(vict), set_fields[mode].cmd);
		} 
		
		else if (is_number(val_arg))
		{
			value = atoi(val_arg);
			RANGE(0, 24);
			GET_COND(vict, (mode - 23)) = (char) value; /* and here too */
			sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
			set_fields[mode].cmd, value);
		} 
		
		else
		{
			send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
			return 0;
		}
    
		break;
  
	case 26:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DARKFRIEND);
		break;
  
	case 27:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MURDERER);
		break;
  
	case 28:
    
		if (value > GET_LEVEL(ch) || value > LVL_IMPL)
		{
			send_to_char("You can't do that.\r\n", ch);
			return 0;
		}
    
		RANGE(0, LVL_IMPL);
		vict->player.level = (cbyte) value;
		check_autowiz(vict);
		break;
  
	case 29:
    
		if ((i = real_room(value)) < 0)
		{
			send_to_char("No room exists with that number.\r\n", ch);
			return 0;
		}
    
		if (IN_ROOM(vict) != NOWHERE)	/* Another Eric Green special. */
			char_from_room(vict);
			
		char_to_room(vict, i);
		break;
  
	case 30:
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
		break;
  
	case 31:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
		break;
  
	case 32:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
		break;
  
	case 33:
		
		if ((i = parse_class(*val_arg)) == RACE_UNDEFINED)
		{
			send_to_char("That is not a class.\r\n", ch);
			return 0;
		}
    
		GET_CLASS(vict) = i;
		break;
  
	case 34:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
		break;
  
	case 35:
		break;
  
	case 36:
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_1);
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_2);
		break;
  
	case 37:
    
		GET_IDNUM(vict) = value;
		break;
  
	case 38:

		strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);
		*(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
		sprintf(output, "Password changed to '%s'.", val_arg);
		break;
  
	case 39:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
		break;
  
	case 40:
    
		if (!str_cmp(val_arg, "male"))
			vict->player.sex = SEX_MALE;
    
		else if (!str_cmp(val_arg, "female"))
			vict->player.sex = SEX_FEMALE;
    
		else if (!str_cmp(val_arg, "neutral"))
			vict->player.sex = SEX_NEUTRAL;
    
		else
		{
			send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
			return 0;
		}
    
		break;
  
	case 41:	/* set age */
		
		if (value < 2 || value > 200)
		{	/* Arbitrary limits. */
			send_to_char("Ages 2 to 200 accepted.\r\n", ch);
			return 0;
		}
    
		/*
		NOTE: May not display the exact age specified due to the integer
		division used elsewhere in the code.  Seems to only happen for
		some values below the starting age (17) anyway. -gg 5/27/98
		*/
    
		ch->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
		break;
  
	case 42:
    
		message(ch, "Command disabled.\r\n");
		break;
  
	case 43:
    
		if ((i = parse_race(*val_arg)) == RACE_UNDEFINED)
		{
			send_to_char("That is not a race.\r\n", ch);
			return 0;
		}
    
		GET_RACE(vict) = i;
		break;
  
	case 44:
		GET_OB(vict) = RANGE(0, 1000);
		break;
  
	case 45:
		GET_DB(vict) = RANGE(0, 1000);
		break;
  
	case 46:
		GET_PB(vict) = RANGE(0, 1000);
		break;
  
	case 47:
		GET_ABS(vict) = RANGE(0, 90);
		break;
  
	case 48:
		message(ch, "Command disabled.\r\n");
		//GET_CLAN(vict) = RANGE(0, NUM_CLANS);
		break;
  
	case 49:
		GET_MOOD(vict) = atoi(val_arg);
		break;
  
	case 50:
		SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOTELL);
		send_to_char("Ok.\r\n", ch);
		break;
  
	case 51:
		GET_WP(vict) = RANGE(0, 100000);
		break;

	case 52:
		GET_SHADOW(vict) = RANGE(0, 5000);
		break;

	case 53:
		GET_MAX_SHADOW(vict) = RANGE(0, 5000);
		break;

	case 54:
		if(atoi(val_arg) < 0)
		{
			send_to_char("The value must be above zero\r\n", ch);
			return 0;
		}
		
		else
			GET_MASTER_WEAPON(vict) = atoi(val_arg);
		
		break;

	case 55:
		if (value < 0 || value > TAINT_MAX)
		{
			send_to_char("Taint must be above 0 and below TAINT_MAX.\r\n", ch);
			return (0);
		}

		GET_TAINT(vict) = value;
		break;

	case 56:
		SET_OR_REMOVE(PLR_FLAGS(vict), PLR_LOGGER);
		break;

	case 57:
		if(!str_cmp(val_arg, "nullptr"))
		{
			GET_WHOIS_EXTRA(vict)[0] = '\0';
			message(ch, "%s's whois extra is now Null.\r\n", GET_NAME(vict));
		}

		else
		{
			strcpy(GET_WHOIS_EXTRA(vict), val_arg);
			message(ch, "%s's whois extra is now: %s", GET_NAME(vict), GET_WHOIS_EXTRA(vict));
		}

		break;
  
	default:
		send_to_char("Can't set that!\r\n", ch);
		return 0;
	}

	strcat(output, "\r\n");
	send_to_char(CAP(output), ch);

	if(!IS_KING(ch))
	{
		sprintf(buf, "%s: \"set %s%s %s %s\"",
			GET_NAME(ch), file ? "file " : "", GET_NAME(vict), set_fields[mode].cmd, val_arg);

		mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
	}

	return 1;
}


ACMD(do_set)
{
	char_data *vict = nullptr, *cbuf = nullptr;
	char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
	val_arg[MAX_INPUT_LENGTH];
	int mode = -1, len = 0, player_i = 0, retval;
	char is_file = 0, is_mob = 0, is_player = 0;

	half_chop(argument, name, buf);

	if (!strcmp(name, "file"))
	{
		is_file = 1;
		half_chop(buf, name, buf);
	} 
	
	else if (!str_cmp(name, "player"))
	{
		is_player = 1;
		half_chop(buf, name, buf);
	} 
	
	else if (!str_cmp(name, "mob"))
	{
		is_mob = 1;
		half_chop(buf, name, buf);
	}
  
	half_chop(buf, field, buf);
	strcpy(val_arg, buf);

	if (!*name || !*field)
	{
		send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
		list_set(ch);
		return;
	}

	/* find the target */
	if (!is_file)
	{
		if (is_player)
		{
			if (!(vict = get_player_vis(ch, name, 0)))
			{
				send_to_char("There is no such player.\r\n", ch);
				return;
			} 
		} 

		else
		{
			if (!(vict = get_char_vis(ch, name))) 
			{
				send_to_char("There is no such creature.\r\n", ch);
				return;
			}
		}
	}		

	else if (is_file)
	{
		/* try to load the player off disk */
		CREATE(cbuf, char_data, 1);
		clear_char(cbuf);
    
		if (cbuf->load(name))
		{
      
			if (GET_LEVEL(cbuf) >= GET_LEVEL(ch))
			{
				free_char(cbuf);
				send_to_char("Sorry, you can't do that.\r\n", ch);
				return;
			}
      
			vict = cbuf;
		}

		else
		{
			free_char(cbuf);
			send_to_char("There is no such player.\r\n", ch);
			return;
		}
	}
	/* find the command in the list */
	len = strlen(field);
  
	for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
		if (!strncmp(field, set_fields[mode].cmd, len))
			break;

	/* perform the set */
	retval = perform_set(ch, vict, mode, val_arg, is_file);

	/* save the character if a change was made */
	if (retval && !IS_NPC(vict))
	{
		vict->save();
	}

	/* free the memory if we allocated it earlier */
	if (is_file)
		free_char(cbuf);
}

ACMD(do_tedit)
{
	int l, i;
	char field[MAX_INPUT_LENGTH];

	struct editor_struct
	{
		char *cmd;
		char level;
		char *buffer;
		int  size;
		char *filename;
	} fields[] =
	{
    
		/* edit the lvls to your own needs */
		{ "credits",	LVL_IMPL,	credits,	2400,	CREDITS_FILE},
		{ "news",	LVL_GRGOD,	news,		8192,	NEWS_FILE},
		{ "motd",	LVL_GRGOD,	motd,		2400,	MOTD_FILE},
		{ "imotd",	LVL_IMPL,	imotd,		2400,	IMOTD_FILE},
		{ "help",       LVL_GRGOD,	help,		2400,	HELP_PAGE_FILE},
		{ "info",	LVL_GRGOD,	info,		8192,	INFO_FILE},
		{ "background",	LVL_IMPL,	background,	8192,	BACKGROUND_FILE},
		{ "handbook",   LVL_IMPL,	handbook,	8192,   HANDBOOK_FILE},
		{ "policies",	LVL_IMPL,	policies,	8192,	POLICIES_FILE},
		{ "startup",	LVL_IMPL,	startup,	8192,	STARTUP_FILE},
		{ "\n",		0,		nullptr,		0,	nullptr }
	};

	if (ch->desc == nullptr)
	{
		send_to_char("Get outta here you linkdead head!\r\n", ch);
		return;
	}

	if (GET_LEVEL(ch) < LVL_GRGOD)
	{
		send_to_char("You do not have text editor permissions.\r\n", ch);
		return;
	}

	half_chop(argument, field, buf);

	if (!*field)
	{
		strcpy(buf, "Files available to be edited:\r\n");
		i = 1;

		for (l = 0; *fields[l].cmd != '\n'; l++)
		{

			if (GET_LEVEL(ch) >= fields[l].level)
			{
				sprintf(buf, "%s%-11.11s", buf, fields[l].cmd);
        
				if (!(i % 7)) strcat(buf, "\r\n");
					i++;
			}
		}

		if (--i % 7) strcat(buf, "\r\n");
			if (i == 0) strcat(buf, "None.\r\n");
				
			send_to_char(buf, ch);
			return;		
	}

	for (l = 0; *(fields[l].cmd) != '\n'; l++)
		if (!strncmp(field, fields[l].cmd, strlen(field)))
			break;

	if (*fields[l].cmd == '\n')
	{
		send_to_char("Invalid text editor option.\r\n", ch);
		return;
	}

	if (GET_LEVEL(ch) < fields[l].level)
	{
		send_to_char("You are not godly enough for that!\r\n", ch);
		return;
	}

	switch (l)
	{
		case 0:
			ch->desc->str = &credits;
			break;

		case 1:
			ch->desc->str = &news;
			break;

		case 2:
			ch->desc->str = &motd;
			break;

		case 3:
			ch->desc->str = &imotd;
			break;

		case 4:
			ch->desc->str = &help;
			break;

		case 5:
			ch->desc->str = &info;
			break;

		case 6:
			ch->desc->str = &background;
			break;

		case 7:
			ch->desc->str = &handbook;
			break;

		case 8:
			ch->desc->str = &policies;
			break;

		case 9:
			ch->desc->str = &startup;
			break;

		default:

			send_to_char("Invalid text editor option.\r\n", ch);
			return;
	}
  
	/* set up editor stats */
	send_to_char("\x1B[H\x1B[J", ch);
	send_to_char("Edit file below: (/s saves /h for help)\r\n", ch);
	ch->desc->backstr = nullptr;
  
	if (fields[l].buffer)
	{
		send_to_char(fields[l].buffer, ch);
		ch->desc->backstr = str_dup(fields[l].buffer);
	}
  
	ch->desc->max_str = fields[l].size;
	ch->desc->mail_to = 0;
	CREATE(ch->desc->olc, struct olc_data, 1);
	OLC_STORAGE(ch->desc) = str_dup(fields[l].filename);
	act("$n begins editing a scroll.", TRUE, ch, 0, 0, TO_ROOM);
	SET_BIT_AR(PLR_FLAGS(ch), PLR_WRITING);
	STATE(ch->desc) = CON_TEXTED;
}

ACMD(do_peace)
{
  char_data *vict, *next_v;
  send_to_room("Everything is quite peaceful now.\r\n", ch->in_room);

    for (vict = world[IN_ROOM(ch)].people; vict; vict = next_v) {
      next_v = vict->next_in_room;  
      if (GET_LEVEL(vict) >= GET_LEVEL(ch)) 
        continue;
      stop_fighting(vict);  
      GET_POS(vict) = POS_SITTING; 
    }
    stop_fighting(ch);
    GET_POS(ch) = POS_STANDING;  
}

