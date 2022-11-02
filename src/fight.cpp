/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "dg_scripts.h"

/* Structures */
char_data *combat_list = nullptr;	/* head of l-list of fighting chars */
char_data *next_combat_list = nullptr;
extern struct battle tg;

/* External structures */
extern struct legend_data legend[8];
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern const struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern struct memory_data *mlist;
extern int pk_allowed;		/* see config.c */
extern int auto_save;		/* see config.c -- not used in this file */
extern int max_exp_gain;	/* see config.c */
extern int max_exp_loss;	/* see config.c */
extern int top_of_world;
extern int max_npc_corpse_time, max_pc_corpse_time;
extern char *find_suf(int number);
extern char *month_name[];

int abs_find(char_data *ch);
int parry_find(char_data *ch);
int dodge_find(char_data *ch);
int offense_find(char_data *ch);
int move_damage(char_data *ch, char_data *vict, int low, int high);

/* External procedures */
char *fread_action(FILE * fl, int nr);
ACMD(do_flee);
int backstab_add(int level);
int thaco(int ch_class, int level);
int ok_damage_shopkeeper(char_data * ch, char_data * victim);

/* local functions */
void perform_fear(char_data *ch, char_data *victim);
void check_fighting(void);
void perform_group_gain(char_data * ch, int base, char_data * victim);
void dam_message(int dam, char_data * ch, char_data * victim, int w_type);
void appear(char_data * ch);
void load_messages(void);
void make_corpse(char_data * ch);
void change_alignment(char_data * ch, char_data * victim);
void death_cry(char_data * ch);
void raw_kill(char_data * ch, char_data * killer);
void die(char_data * ch, char_data * killer);
void group_gain(char_data * ch, char_data * victim);
void solo_gain(char_data * ch, char_data * victim);
char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural);
void perform_violence(void);
void gain_wp(char_data *ch, char_data *vict);
void check_legend(char_data *ch);
void move_char_circle(char_data *ch);
void move_mob_circle(char_data *ch);
void save_battle();
void update_legend(char_data *ch);
void list_legends(char_data *ch);
void hit_all_fighting_char(char_data *ch);
void stop_riding(char_data *ch);
void stop_riding(char_data *ch, char_data *mount);
void interupt_timer(char_data *ch);
int count_fighting(char_data *ch);
int level_exp(int level);
struct fight_noise *fn = nullptr;

ACMD(do_note);

void interupt_timer(char_data *ch)
{
	if(IS_NPC(ch) || !ch->desc)
		return;

	if(ch->desc->timer <= 0)
		return;

	send_to_char("You are interupted and stop what you are doing.\r\n", ch);
	strcpy(LAST_COMMAND(ch), "");
	ch->desc->timer = 0;
	ch->desc->command_ready = 0;
}

void save_legends(void)
{
	int i;
	FILE *file;

	if(!(file = fopen(LEGEND_FILE, "w")))
	{
		log("Legend file is not opening in save_legends() in fight.c!");
		return;
	}

	for(i = 1;i <= 12;i++)
		fprintf(file, "~%s %d\n", legend[i].person, legend[i].ammount);
	
	fclose(file);

}

/* Galnor 10/26/2003- Checks to see if char is on the legends list */
int has_old_entry(char_data *ch)
{
	
	int i = 1;

	for(i = 1;i <= 12;i++)
		if(!str_cmp(GET_NAME(ch), legend[i].person))
			return 1;

	return 0;
}

void bump_list_up(int num)
{
	int i = num;

	for(i = num;i <= 11;i++)
	{
		strcpy(legend[i].person, legend[i + 1].person);
		legend[i].ammount = legend[i + 1].ammount;
		strcpy(legend[i + 1].person, "");
		legend[i + 1].ammount = 0;
	}
}

/* Galnor October 26th 2003. Purge one entry of the legend list. */
void purge_old_entry(char_data *ch)
{
	int i;

	for(i = 1;i <= 12;i++)
	{
		if(!str_cmp(GET_NAME(ch), legend[i].person)) 
		{
			strcpy(legend[i].person, "");
			legend[i].ammount = 0;
			bump_list_up(i);
		}
	}
}

/* Implemented by Galnor on October 26th 2003. This is used to update the legends list */
void lower_list(int num) 
{

	char current[MAX_INPUT_LENGTH];
	char next[MAX_INPUT_LENGTH];
	int curwp = 0, nextwp = 0, i = 0;

	/* Start the loop wherever num is */
	for(i = num;i <= 11;i++) 
	{

		if(i == 12)
			return;

	/* Grab the current legend name and weave points */
		if(i == num) {
			strcpy(current, legend[i].person);
			curwp = legend[i].ammount;
		}

	/* Now we need to grab the information next on the list */
		strcpy(next, legend[i + 1].person);
		nextwp = legend[i + 1].ammount;

	/* Ok, we finished grabbing the information we need. Now we need to copy over with the new information */
		strcpy(legend[i + 1].person,  current);
		legend[i + 1].ammount = curwp;

	/* The next part of the list is now updated. Now all we need to do is move the */
		strcpy(current, next);
		curwp = nextwp;

	/*And loop around and do it again */
	}
}

/* Galnor October 26th 2003. Update legend for particulare player (ch). */
void update_legend(char_data *ch) 
{
	int i;

	if(has_old_entry(ch))
		purge_old_entry(ch);

	for(i = 1;i <= 12;i++)
	{
		if(GET_WP(ch) > legend[i].ammount) 
		{
				lower_list(i);

			strcpy(legend[i].person, GET_NAME(ch));
			legend[i].ammount = GET_WP(ch);
			return;
		}
	}

	GET_LEGEND(ch) = -1;
}

void update_slew(char_data *ch, char_data *victim)
{

	const char *suf = find_suf(time_info.day + 1);

	sprintf(buf, "Slew %s on the %d%s Day of the %s, Year %d.",
	GET_NAME(victim), time_info.day + 1, suf, month_name[(int) time_info.month], time_info.year);

	strcpy(GET_SLEW_MESSAGE(ch), buf);
}

void true_weave_gain(char_data *ch, int ammount) 
{

	GET_WP(ch) += ammount;
	send_to_char("The Wheel weaves around you more tightly.\r\n", ch);
	update_legend(ch);

}

void gain_wp(char_data *ch, char_data *vict) 
{
	int wp_gain = 0, group_members = 1;
	char_data *master;
	struct follow_type *f;

	if(!vict || !ch || vict == ch)
		return;

	if(GET_LEVEL(ch) < 15 || GET_LEVEL(vict) < 15)
		return;

	if(IS_NPC(vict) || IS_NPC(ch))
		return;

	wp_gain += 7;

	wp_gain += GET_WP(vict) / 5;

	update_slew(ch, vict);

	if(GET_RACE(ch) != GET_RACE(vict)) 
	{
		if(IS_HUMAN(ch))
			tg.humanwp += wp_gain;
		
		else
			tg.trollwp += wp_gain;
	}

	if(GET_RACE(ch) == GET_RACE(vict))
		wp_gain = 1;

	GET_WP(vict) -= wp_gain;

	update_legend(vict);

	if (!(master = ch->master))
		master = ch;

	if (!AFF_FLAGGED(master, AFF_GROUP) || (master->in_room != ch->in_room) || !ch->followers)
		true_weave_gain(ch, wp_gain);

	for (f = master->followers; f; f = f->next)
		if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
		{
			update_slew(f->follower, vict);
			group_members++;
		}

		wp_gain /= group_members;

	for (f = master->followers; f; f = f->next)
		if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
			true_weave_gain(f->follower, wp_gain);

	if(ch->master)
		true_weave_gain(ch->master, wp_gain);

	if(master->followers && ch == master)
		true_weave_gain(ch, wp_gain);

	if(GET_WP(vict) < 0)
		GET_WP(vict) = 0;

	send_to_char("The Wheel weaves around you less tightly.\r\n", vict);
}

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
	{"hit",			"hits"},			/* 0 */
	{"sting",		"stings"},
	{"whip",		"whips"},
	{"slash",		"slashes"},
	{"bite",		"bites"},
	{"bludgeon",	"bludgeons"},		/* 5 */
	{"crush",		"crushes"},
	{"pound",		"pounds"},
	{"claw",		"claws"},
	{"maul",		"mauls"},
	{"thrash",		"thrashes"},		/* 10 */
	{"pierce",		"pierces"},
	{"blast",		"blasts"},
	{"punch",		"punches"},
	{"stab",		"stabs"},		
	{"smash",		"smashes"},			/* 15 */
	{"strike",		"strikes"},
	{"hack",		"hacks"},
	{"smite",		"smites"},
	{"slice",		"slices"},
	{"cleave",		"cleaves"},			/* 20 */
};

	#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

	/* The Fight related routines */

void appear(char_data * ch)
{

	REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE);

	if (GET_LEVEL(ch) < LVL_IMMORT)
		act("A bright flash of light appears as $n steps in.", FALSE, ch, 0, 0, TO_ROOM);

}

void load_messages(void)
{
	FILE *fl;
	int i, type;
	struct message_type *messages;
	char chk[128];

	if (!(fl = fopen(MESS_FILE, "r")))
	{
		sprintf(buf2, "SYSERR: Error reading combat message file %s", MESS_FILE);
		perror(buf2);
		exit(1);
	}
  
	for (i = 0; i < MAX_MESSAGES; i++)
	{
		fight_messages[i].a_type = 0;
		fight_messages[i].number_of_attacks = 0;
		fight_messages[i].msg = 0;
	}

// Serai - 10/17/04 - changed "*chk == '\n'" to "ISNEWL(*chk)" for Linux compatability.
	fgets(chk, 128, fl);
	while (!feof(fl) && (ISNEWL(*chk) || *chk == '*'))
		fgets(chk, 128, fl);

	while (*chk == 'M')
	{
		fgets(chk, 128, fl);
		sscanf(chk, " %d\n", &type);
    
		for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
		(fight_messages[i].a_type); i++);
    
		if (i >= MAX_MESSAGES)
		{
			log("SYSERR: Too many combat messages.  Increase MAX_MESSAGES and recompile.");
			exit(1);
		}
    
		messages = new struct message_type;
		fight_messages[i].number_of_attacks++;
		fight_messages[i].a_type = type;
		messages->next = fight_messages[i].msg;
		fight_messages[i].msg = messages;

		messages->die_msg.attacker_msg = fread_action(fl, i);
		messages->die_msg.victim_msg = fread_action(fl, i);
		messages->die_msg.room_msg = fread_action(fl, i);
		messages->miss_msg.attacker_msg = fread_action(fl, i);
		messages->miss_msg.victim_msg = fread_action(fl, i);
		messages->miss_msg.room_msg = fread_action(fl, i);
		messages->hit_msg.attacker_msg = fread_action(fl, i);
		messages->hit_msg.victim_msg = fread_action(fl, i);
		messages->hit_msg.room_msg = fread_action(fl, i);
		messages->god_msg.attacker_msg = fread_action(fl, i);
		messages->god_msg.victim_msg = fread_action(fl, i);
		messages->god_msg.room_msg = fread_action(fl, i);
		fgets(chk, 128, fl);
// Serai - again, changed same as above        
		while (!feof(fl) && (ISNEWL(*chk) || *chk == '*'))
		fgets(chk, 128, fl);
	}

	fclose(fl);
}

void change_pos(char_data *ch, int pos)
{

	GET_LAST_POS(ch) = GET_POS(ch);
	GET_POS(ch) = pos;
}

void update_pos(char_data * victim)
{
	if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
		return;
	
	else if (GET_HIT(victim) > 0)
		change_pos(victim, POS_STANDING);
	
	else if (GET_HIT(victim) <= -11)
		change_pos(victim, POS_DEAD);
	
	else if (GET_HIT(victim) <= -6)
		change_pos(victim, POS_MORTALLYW);
	
	else if (GET_HIT(victim) <= -3)
		change_pos(victim, POS_INCAP);
	
	else
		change_pos(victim, POS_STUNNED);
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(char_data * ch, char_data * vict)
{
	if (ch == vict)
		return;

	if (FIGHTING(ch))
	{
		core_dump();
		return;
	}

	ch->next_fighting = combat_list;
	combat_list = ch;

	if (AFF_FLAGGED(ch, AFF_SLEEP))
		affect_from_char(ch, SPELL_SLEEP, 0);

	FIGHTING(ch) = vict;
	change_pos(ch, POS_FIGHTING);

	if(!FIGHTING(vict))
		set_fighting(vict, ch);
}

/* remove a char from the list of fighting chars */
void stop_fighting(char_data * ch)
{
	char_data *temp, *t;

	if (ch == next_combat_list)
		next_combat_list = ch->next_fighting;

	REMOVE_FROM_LIST(ch, combat_list, next_fighting);
	ch->next_fighting = nullptr;
	FIGHTING(ch) = nullptr;
	
	if(GET_POS(ch) == POS_FIGHTING)
		GET_POS(ch) = POS_STANDING;
	
	update_pos(ch);

}

void make_corpse(char_data * ch)
{
	struct obj_data *corpse, *o;
	struct obj_data *money, *obj;
	int i, x, y;
	string type = " ";

	if(!ch->desc && !IS_NPC(ch))
		type = " torn up ";
	else if(GET_HIT(ch) <= -30)
		type = " bloody ";
	else if(GET_HIT(ch) <= -70)
		type = " sliced up ";
	else if(GET_HIT(ch) <= -100)
		type = " decimated ";
	else if(GET_HIT(ch) <= -150)
		type = " flayed ";
	else if(GET_HIT(ch) <= -200)
		type = " barely recognizable ";

	corpse = create_obj();

	corpse->item_number = NOTHING;
	corpse->in_room = NOWHERE;
	corpse->name = str_dup("corpse");

	sprintf(buf2, "The%scorpse of %s is lying here.", type.c_str(), GET_NAME(ch));
	corpse->description = str_dup(buf2);

	sprintf(buf2, "the corpse of %s", GET_NAME(ch));
	corpse->short_description = str_dup(buf2);
	sprintf(buf2, GET_NAME(ch));

	/* Added by Galnor in September of 2003. Patch added to give object scalp information. */ 
	if(!IS_NPC(ch))
		corpse->scalp.is_scalp = 1;

	corpse->scalp.race = GET_RACE(ch);
	corpse->scalp.name = GET_NAME(ch);
	corpse->scalp.level = IS_NPC(ch) ? 30 : GET_LEVEL(ch);
	corpse->scalp.scalped = 1;
	corpse->food_unit = GET_WEIGHT(ch) / 5;

	GET_CORPSE_DATA(corpse) = ch;

	GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
	for(x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++)
	{
		if (x < EF_ARRAY_MAX)
			GET_OBJ_EXTRA_AR(corpse, x) = 0;
    
		if (y < TW_ARRAY_MAX)
			corpse->obj_flags.wear_flags[y] = 0;
	}
  
	SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
	SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
	GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
	GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
	GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + carrying_weight(ch);
	GET_OBJ_RENT(corpse) = 100000;
  
	if (IS_NPC(ch))
		GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
  
	else
		GET_OBJ_TIMER(corpse) = max_pc_corpse_time;

	/* transfer character's inventory to the corpse */
	corpse->contains = ch->carrying;
  
	for (o = corpse->contains; o != nullptr; o = o->next_content)
		o->in_obj = corpse;
	
	object_list_new_owner(corpse, nullptr);

	/* transfer character's equipment to the corpse */
	for (i = 0; i < NUM_WEARS; i++) {
		if (GET_EQ(ch, i)) {
		obj = GET_EQ(ch, i);
		obj_to_obj(unequip_char(ch, i), corpse);
		}
	}

	/* transfer gold */
	if (GET_GOLD(ch) > 0) {
		if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc))
		{
			money = create_money(GET_GOLD(ch));
			obj_to_obj(money, corpse);
		}
    
		GET_GOLD(ch) = 0;
	}
  
	ch->carrying = nullptr;
	IS_CARRYING_N(ch) = 0;

	obj_to_room(corpse, ch->in_room);
}

/* When ch kills victim */
void change_alignment(char_data * ch, char_data * victim)
{
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
	GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 16;
}

void death_cry(char_data * ch)
{
	int door, was_in;

	act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
	was_in = ch->in_room;

	for (door = 0; door < NUM_OF_DIRS; door++)
	{
		if(EXIT(ch, door))
			if (CAN_GO(ch, door))
			{
				ch->in_room = world[was_in].dir_option[door]->to_room;
				act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
				ch->in_room = was_in;
		}
	}
}

void raw_kill(char_data *ch, char_data *killer) 
{ 

	stop_riding(ch);

	if(killer && !IS_NPC(killer) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(killer) < LVL_IMMORT)
		GET_DEATH_WAIT(ch) = 3;

	if (FIGHTING(ch))
		if(FIGHTING(FIGHTING(ch)))
			stop_fighting(FIGHTING(ch));

	if (FIGHTING(ch)) 
		stop_fighting(ch);

	// Unbond them.
	//unbond(GET_NAME(ch));
 
	while (ch->affected) 
		affect_remove(ch, ch->affected);

	if(ch->desc)
	{
		ch->desc->timer = 0.0;
		ch->desc->command_ready = 0;
	}
 
	death_cry(ch);
	send_to_char("You are dead! Sorry...\r\n", ch);
	act("$n is dead! R.I.P.", FALSE, ch, 0, 0, TO_ROOM);
	make_corpse(ch);
	Crash_crashsave(ch);

	death_mtrigger(ch, killer);

	if(IS_NPC(ch))
	{
		stop_fighting(ch);
		extract_char(ch);
	}

    else
		move_char_circle(ch);
}

void die(char_data * ch, char_data * killer)
{

	if(!ch)
		return;

	/* Prevent NPCs and Immortals from affecting the legend list and gaining weave points period... */
	if(killer && ch)
	{
		if(!IS_NPC(ch) && !IS_NPC(killer))
		{
			if(GET_LEVEL(ch) < LVL_IMMORT)
			{
				gain_wp(killer, ch);
				save_battle();
				save_legends();			
			}
		}
		killer->save();
	}

	if(killer && IS_NPC(killer))
		gain_exp(ch, -(level_exp(GET_LEVEL(ch)) - level_exp(GET_LEVEL(ch))));

	else
		gain_exp(ch, -(level_exp(GET_LEVEL(ch) + 1) - level_exp(GET_LEVEL(ch))) / 2);

	if(killer && FIGHTING(killer) == ch)
		stop_fighting(killer);

	ch->save();
	raw_kill(ch, killer);

}

void perform_group_gain(char_data * ch, int base,
char_data * victim)
{
	int share;

	share = MIN(max_exp_gain, MAX(1, base));

	sprintf(buf2, "You receive your share of experience.\r\n");
	send_to_char(buf2, ch);

	gain_exp(ch, share);
}

void group_gain(char_data * ch, char_data * victim)
{
	int tot_members, base, tot_gain;
	char_data *k;
	struct follow_type *f;

	if (!(k = ch->master))
		k = ch;

	if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room))
		tot_members = 1;
  
	else
		tot_members = 0;

	for (f = k->followers; f; f = f->next)
		if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
			tot_members++;

	/* round up to the next highest tot_members */
	tot_gain = (GET_EXP(victim) / 3) + tot_members - 1;

	/* prevent illegal xp creation when killing players */
	if (!IS_NPC(victim))
		tot_gain = MIN(max_exp_loss * 2 / 3, tot_gain);

	if (tot_members > 2)
		base = MAX(1, tot_gain / tot_members);
	
	else
		base = tot_gain;

	if (AFF_FLAGGED(k, AFF_GROUP) && k->in_room == ch->in_room)
		perform_group_gain(k, base, victim);

	for (f = k->followers; f; f = f->next)
		if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
			perform_group_gain(f->follower, base, victim);
}

void solo_gain(char_data * ch, char_data * victim)
{
	int exp;

	exp = MIN(max_exp_gain, GET_EXP(victim));

	/* Calculate level-difference bonus */
	if (IS_NPC(ch))
		exp += MAX(0, (exp * MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
	else
		exp += MAX(0, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);

	exp = MAX(exp, 1);

	sprintf(buf2, "You receive some experience points.\r\n");
	send_to_char(buf2, ch);

	if(GET_RACE(ch) == GET_RACE(victim))
		gain_exp(ch, 1000);
	
	else
		gain_exp(ch, exp);

}

char *replace_string(const char *str, const char *weapon_singular, const char *weapon_plural)
{
	static char buf[256];
	char *cp = buf;

	for (; *str; str++)
	{
		if (*str == '#')
		{
			
			switch (*(++str))
			{
				case 'W':
					for (; *weapon_plural; *(cp++) = *(weapon_plural++));
						break;
				
				case 'w':
					for (; *weapon_singular; *(cp++) = *(weapon_singular++));
						break;
				
				default:
					*(cp++) = '#';
					break;
			}
		} 
		
		else
			*(cp++) = *str;

		*cp = 0;
	}				/* For */

	return (buf);
}

/* message for doing damage with a weapon */
void dam_message(int dam, char_data * ch, char_data * victim,
		      int w_type)
{
	char *buf;
	int msgnum;

	static struct dam_weapon_type
	{
		const char *to_room;
		const char *to_char;
		const char *to_victim;
	}	dam_weapons[] =
	{

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

		{
			"$n tries to #w $N, but misses.",	/* 0: 0     */
			"You try to #w $N, but miss.",
			"$n tries to #w you, but misses."
		},

		{
			"$n barely tickles $N with his #w,",	/* 7: 19..23 */
			"You barely tickle $N with your #w.",
			"$n barely tickles you with his #w."
		},

		{
			"$n tickles $N as $e #W $M.",	/* 1: 1..2  */
			"You tickle $N as you #w $M.",
			"$n tickles you as $e #W you."
		},

		{
			"$n barely #W $N.",		/* 2: 3..4  */
			"You barely #w $N.",
			"$n barely #W you."
		},

		{
			"$n #W $N.",			/* 3: 5..6  */
			"You #w $N.",
			"$n #W you."
		},

		{
			"$n #W $N hard.",			/* 4: 7..10  */
			"You #w $N hard.",
			"$n #W you hard."
		},

		{
			"$n #W $N very hard.",		/* 5: 11..14  */
			"You #w $N very hard.",
			"$n #W you very hard."
		},

		{
			"$n #W $N extremely hard.",	/* 6: 15..19  */
			"You #w $N extremely hard.",
			"$n #W you extremely hard."
		},


		{
			"$n #W $N into bloody fragments!",	/* 8: > 23   */
			"You #w $N into bloody fragments!",
			"$n #W you into bloody fragments!"
		}
	};


	w_type -= TYPE_HIT;		/* Change to base of table with text */

	if		(dam == 0)
		msgnum = 0;
	
	else if	(dam <= 2)
		msgnum = 1;
	
	else if	(dam <= 4)
		msgnum = 2;
	
	else if	(dam <= 6)
		msgnum = 3;
	
	else if	(dam <= 10)
		msgnum = 4;
	
	else if	(dam <= 14)
		msgnum = 5;
	
	else if	(dam <= 19)
		msgnum = 6;
	
	else if	(dam <= 23)
		msgnum = 7;
	
	else
		msgnum = 8;

	/* damage message to onlookers */
	buf = replace_string(dam_weapons[msgnum].to_room,
	attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, nullptr, victim, TO_NOTVICT);

	/* damage message to damager */
	send_to_char(COLOR_GREEN(ch, CL_COMPLETE), ch);
	buf = replace_string(dam_weapons[msgnum].to_char,
	attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, nullptr, victim, TO_CHAR);
	send_to_char(COLOR_NORMAL(ch, CL_COMPLETE), ch);

	/* damage message to damagee */
	send_to_char(COLOR_RED(victim, CL_COMPLETE), victim);
	buf = replace_string(dam_weapons[msgnum].to_victim,
	attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
	act(buf, FALSE, ch, nullptr, victim, TO_VICT | TO_SLEEP);
	send_to_char(COLOR_NORMAL(victim, CL_COMPLETE), victim);
}

/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
// Small fixes by Serai for "dodge/parry" and multiple messages sometimes.
int skill_message(int dam, char_data * ch,char_data * vict, int attacktype)
{
	int i, j, nr, db, pb;
	struct message_type *msg;
	struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
	char mesg[1024], *temp;

	for (i = 0; i < MAX_MESSAGES; i++)
	{
		if (fight_messages[i].a_type == attacktype)
		{
			nr = dice(1, fight_messages[i].number_of_attacks);			
			
			for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
				msg = msg->next;

			if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT))
			{
				act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
				act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
				act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);

				return (1);
			} 
	  
			else if (dam != 0)
			{
        /*
         * Don't send redundant color codes for TYPE_SUFFERING & other types
         * of damage without attacker_msg.
         */


				if (GET_POS(vict) == POS_DEAD)
				{
					if (msg->die_msg.attacker_msg)
					{
						send_to_char(COLOR_GREEN(ch, CL_COMPLETE), ch);
						act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
						send_to_char(COLOR_NORMAL(ch, CL_COMPLETE), ch);
					}

					send_to_char(COLOR_RED(vict, CL_COMPLETE), vict);
					act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
					send_to_char(COLOR_NORMAL(vict, CL_COMPLETE), vict);

					act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
				}
				
				
				else
				{
					if (msg->hit_msg.attacker_msg)
					{
						send_to_char(COLOR_GREEN(ch, CL_COMPLETE), ch);
						act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
						send_to_char(COLOR_NORMAL(ch, CL_COMPLETE), ch);
					}

					send_to_char(COLOR_RED(vict, CL_COMPLETE), vict);
					act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
					send_to_char(COLOR_NORMAL(vict, CL_COMPLETE), vict);

					act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
				}
			} 
			
			else if (ch != vict)
			{	/* Dam == 0 */
				if (IS_WEAPON(attacktype))
				{
					attacktype -= TYPE_HIT;
				
					pb = parry_find(vict);
					db = dodge_find(vict);
					nr = number(0, (pb + db));
					temp = replace_string("you try to #w $N, but $E %s the attack.",
					attack_hit_text[attacktype].singular, attack_hit_text[attacktype].plural);
					sprintf(mesg, temp, (nr >= pb ? "dodges" : "parries"));
					act(mesg, FALSE, ch, weap, vict, TO_CHAR);

					temp = replace_string("$n tries to #w you, but you %s the attack.",
					attack_hit_text[attacktype].singular, attack_hit_text[attacktype].plural);
					sprintf(mesg, temp, (nr >= pb ? "dodge" : "parry"));
					act(mesg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);

					temp = replace_string("$n tries to #w $N, but $E %s the attack.",
					attack_hit_text[attacktype].singular, attack_hit_text[attacktype].plural);
					sprintf(mesg, temp, (nr >= pb ? "dodges" : "parries"));
					act(mesg, FALSE, ch, weap, vict, TO_NOTVICT);
				}
				
				else
				{
					if (msg->miss_msg.attacker_msg)
					{
						send_to_char(COLOR_GREEN(ch, CL_COMPLETE), ch);
						act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
						send_to_char(COLOR_NORMAL(ch, CL_COMPLETE), ch);
					}

					send_to_char(COLOR_RED(vict, CL_COMPLETE), vict);
					act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
					send_to_char(COLOR_NORMAL(vict, CL_COMPLETE), vict);

					act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
				}
				
				return (1);
			}
		}
	}
	return (0);
}

int move_damage(char_data *ch, char_data *vict, int low, int high)
{

	if(!vict || !ch)
		return 0;

	GET_MOVE(vict) -= number(low, high);

	if(GET_MOVE(vict) < 0)
		GET_MOVE(vict) = 0;

	return 1;
}

/*
 * Alert: As of bpl14, this function returns the following codes:
 *	< 0	Victim died.
 *	= 0	No damage.
 *	> 0	How much damage done.
 */
int damage(char_data * ch, char_data * victim, int dam, int attacktype)
{
	if (GET_POS(victim) <= POS_DEAD)
	{
		log("SYSERR: Attempt to damage corpse '%s' in room #%d by '%s'.",
		GET_NAME(victim), GET_ROOM_VNUM(IN_ROOM(victim)), GET_NAME(ch));
		die(victim, ch);
		return 0;			/* -je, 7/7/92 */
	}

	/* peaceful rooms */
	if (ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
	{
		send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
		return 0;
	}

	/* You can't damage an immortal! */
	if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
		dam = 0;

	if (victim != ch)
	{
		/* Start the attacker fighting the victim */
		if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == nullptr))
			set_fighting(ch, victim);

		/* Start the victim fighting the attacker */
		if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == nullptr))
		{
			set_fighting(victim, ch);
			
			if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch))
				remember(victim, ch);
		}
	}

	/* If you attack a pet, it hates your guts */
	if (victim->master == ch)
		stop_follower(victim);

	/* If the attacker is invisible, he becomes visible */
	if (AFF_FLAGGED(ch, AFF_INVISIBLE))
		appear(ch);

	/* Cut damage in half if victim has sanct, to a minimum 1 */
	if (AFF_FLAGGED(victim, AFF_SANCTUARY) && dam >= 2)
		dam = (int) ((double) dam * .75);

	/* Set the maximum damage per round and subtract the hit points */
	dam = MAX(MIN(dam, 5000), 0);
	GET_HIT(victim) -= dam;

	/* Gain exp for the hit */
	if (ch != victim)
		gain_exp(ch, GET_LEVEL(victim) * dam);

	update_pos(victim);

	if(victim->desc && (!str_cmp(GET_COMMAND(victim), "channel") || !str_cmp(GET_COMMAND(victim), "backstab"))
		&& victim->desc->timer > 0 && dam > 0)
	{
		interupt_timer(victim);
	}

  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   * 
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */
  
	if (!IS_WEAPON(attacktype))
		skill_message(dam, ch, victim, attacktype);
  
	else
	{
		if (GET_POS(victim) == POS_DEAD || dam == 0)
		{
			if (!skill_message(dam, ch, victim, attacktype))
				dam_message(dam, ch, victim, attacktype);
		} 
		
		else
		{
			dam_message(dam, ch, victim, attacktype);
		}
	}

	/* Use send_to_char -- act() doesn't send message if you are DEAD. */
  
	switch (GET_POS(victim))
	{
		case POS_MORTALLYW:
			act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
				send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
				break;
  
		case POS_INCAP:
			act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
			send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
			break;
  
		case POS_STUNNED:
			act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
			send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
			break;
  
		default:			/* >= POSITION SLEEPING */
			if (dam > (GET_MAX_HIT(victim) / 4))
			{
				act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);
				act("$n grimaces in pain.", FALSE, victim, 0, 0, TO_ROOM);
			}


		if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4))
		{
			if(dam > 0)
			{
				sprintf(buf2, "You wish that your wounds would stop BLEEDING so much!\r\n");
				send_to_char(buf2, victim);
			}
			
			if (ch != victim && MOB_FLAGGED(victim, MOB_WIMPY))
				do_flee(victim, nullptr, 0, 0);
		}
    
		if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
			GET_HIT(victim) < GET_WIMP_LEV(victim) && GET_HIT(victim) > 0)
		{
			send_to_char("You wimp out, and attempt to flee!\r\n", victim);
			do_flee(victim, nullptr, 0, 0);
		}
		
		break;
	}

	/* stop someone from fighting if they're stunned or worse */
	if ((GET_POS(victim) <= POS_STUNNED) && (FIGHTING(victim) != nullptr))
		stop_fighting(victim);

	/* Uh oh.  Victim died. */
	if (GET_POS(victim) == POS_DEAD)
	{
		if ((ch != victim) && (IS_NPC(victim) || victim->desc))
		{
			if (AFF_FLAGGED(ch, AFF_GROUP))
				group_gain(ch, victim);
      
			else
				solo_gain(ch, victim);
		}

		if (!IS_NPC(victim))
		{
			sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
			world[victim->in_room].name);
			mudlog(buf2, BRF, LVL_IMMORT, TRUE);
      
			if (MOB_FLAGGED(ch, MOB_MEMORY))
				forget(ch, victim);
		}
    
		die(victim, ch);
		return -1;
  
	}
	return dam;
}
	
void hit(char_data * ch, char_data * victim, int type)
{
	struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
	struct affected_type af;

	int w_type, dam, dice_ob, noquit;
	int victim_parry, count;
	float fraction_abs;

	if(!ch || !victim)
		return;

	if(((count_fighting(victim) >= 5 && !IS_NPC(ch)) ||
		(ROOM_FLAGGED(ch->in_room, ROOM_TUNNEL) && count_fighting(victim) >= 2)) && FIGHTING(ch) != victim)
	{
		send_to_char("You'd probably get killed before you reached him it is so crowded!\r\n", ch);
		return;
	}

	if(IS_NPC(victim) && GET_RACE(victim) == GET_RACE(ch))
	{
		for(player_clan_data *cl = victim->clans;cl;cl = cl->next)
		{
			SET_BIT_AR(GET_WARRANTS(ch), cl->clan);
		}
	}

	/* check if the character has a fight trigger */
	fight_mtrigger(ch);

	if(MOUNT(ch) == victim)
		stop_riding(ch);

	/* Do some sanity checking, in case someone flees, etc. */
	if(victim)
	{
		if(victim->in_room != ch->in_room)
			return;
	}

  	if (AFF_FLAGGED(ch, AFF_HIDE))
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

	/* Find the weapon type (for display purposes only) */
	if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
		w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  
	else
	{
		if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))  
			w_type = ch->mob_specials.attack_type + TYPE_HIT;
    
		else
			w_type = TYPE_HIT;
	}

	//Cheesy way of balancing abs a bit... //
	fraction_abs = number(abs_find(victim) - 5, abs_find(victim));
  
	if(fraction_abs <= 0)
		fraction_abs += number(1, 9);

	fraction_abs -= number(0, MAX(1, (int) fraction_abs / 10));

	if(AFF_FLAGGED(ch, AFF_NOQUIT))
		affect_from_char(ch, 0, AFF_NOQUIT);

	//Add no quit once a char hits another... //
  
	if(IS_NPC(victim) || IS_NPC(ch))
		noquit = 1;

	else
		noquit = 5;

	af.type = SKILL_SNEAK;
	af.duration = noquit;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_NOQUIT;
	affect_to_char(ch, &af);
	affect_to_char(victim, &af);

	dice_ob = number(offense_find(ch), (int) (offense_find(ch) * 2 * .65));

	/* Needs a bit more balancing first... Galnor

	This was used to make a random chance of hitting each part of the body, to make a difference on where
	you put a piece of equipment with a different Absorbing Point Percentage. */

	/*

	i = number(1, 35);
  
	if(i >= 1)
		part = WEAR_BODY;

	if(i > 10)
		part = WEAR_LEGS 
	  ;
	if(i > 18)
		part = WEAR_ARMS;

	if(i > 25)
		part = WEAR_HEAD;

	if(part > 31)
		part = WEAR_HANDS;
  
	if(part > 33)
		part = WEAR_FEET;
	*/	


	/*This was moved up, and is returned here so that abs does not affect damage on a stab */
	if (type == SKILL_BACKSTAB)
	{
		dam = dice(GET_OBJ_VAL(wielded, 1) + 3, GET_OBJ_VAL(wielded, 2) + 3);
		dam *= (GET_DEX(ch) / 2);
		dam += backstab_add(GET_LEVEL(ch)) * 2;

		if(IS_GREYMAN(ch))
			dam += number(number(1, 20), 60);

		if(ch->getClan(CLAN_KOBOL))
			dam += number(15, 40);

		if (AFF_FLAGGED(victim, AFF_HIDE) && dam > 0)
			REMOVE_BIT_AR(AFF_FLAGS(victim), AFF_HIDE);

		damage(ch, victim, dam, SKILL_BACKSTAB);
		return;
	}

	if(type == SKILL_CHARGE || type == SKILL_SKEWER)
	{
		dam = dice(GET_OBJ_VAL(wielded, 1) + 2, GET_OBJ_VAL(wielded, 2) + 2);
		dam *= GET_STR(ch) / 2;
		dam += GET_LEVEL(ch) / 2;

		damage(ch, victim, dam, type);
		return;
	}

	/* Subtract parry by 25% for each person on the victim */

	victim_parry = parry_find(victim);

	if(count_fighting(victim) > 1)
	{
		for(count = 2;count <= count_fighting(victim);count++)
		{
			victim_parry = (int) ((double) victim_parry * .70);
		}
	}

	/* decide whether this is a hit or a miss */
	if (dice_ob < victim_parry + dodge_find(victim))
	{
		/* the attacker missed the victim */
		damage(ch, victim, 0, w_type);
	} 
	
	else
	{
		/* okay, we know the guy has been hit.  now calculate damage. */

		/* Start with the damage bonuses: the damroll and strength apply */
		dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
		dam += GET_DAMROLL(ch);

		if (wielded)
			/* Add weapon-based damage if a weapon is being wielded */
			dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));

		else
		{
		/* If no weapon, add bare hand damage instead */
      
			if (IS_NPC(ch))
			{
				dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
			} 
			
			else
			{
				dam += number(0, 2);	/* Max 2 bare hand damage for players */
			}
		}

		// Boost up damage for Warders VS trollocs
		if(IS_WARDER(ch) && IS_TROLLOC(victim))
			dam = (int) (dam * 1.2);

		//Testing damage multiplier differences, for bonuses or maluses with unused flag.
		if(PLR_FLAGGED(victim, PLR_LOGGER))
			dam = (int) (dam * 1.7);

		dam = dam - (int) (((float) fraction_abs / 100) * (float) dam);

		/* Damage times 1 + 1 per possition under POS_FIGHTING. */
		if (GET_POS(victim) < POS_FIGHTING)
			dam = (int) ((float) dam * (.5 + (POS_FIGHTING - GET_POS(victim))));

		/* check if the victim has a hitprcnt trigger */
		hitprcnt_mtrigger(victim);

		if(!victim)
			return;

		/* at least 1 hp damage min per hit */
		dam = MAX(1, dam);

		damage(ch, victim, dam, w_type);
	}
}

/* Added by Galnor on November 3, 2003. Hit's everyone fighting you except FIGHTING(ch) */

void hit_all_fighting_char(char_data *ch)
{
	
	char_data *vict, *temp;

	for(vict = world[ch->in_room].people;vict;vict = temp)
	{
		temp = vict->next_in_room;
		if(vict)
			if(FIGHTING(vict))
				if(FIGHTING(vict) == ch && vict != FIGHTING(ch))
					hit(ch, vict, TYPE_UNDEFINED);
	}
}


void perform_fear(char_data *ch, char_data *victim)
{

	struct affected_type af;

	if(victim == nullptr)
		return;

	if(AFF_FLAGGED(victim, AFF_PARANOIA) || victim->getClan(CLAN_BLADEMASTERS))
		return;

	act("You stare into $n's eyeless face, unable to pull your eyes away as you feel your bones freeze.",
		TRUE, ch, nullptr, victim, TO_VICT);
	act("You stare into $N's eyes and watch $M shake with fear.", TRUE, ch, nullptr, victim, TO_CHAR);
	act("$N begins to shiver in paranoia as $E stares into the eyeless face of $n.", TRUE, ch, nullptr, victim, TO_NOTVICT);

	af.type = SKILL_NOTICE;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_PARANOIA;
	affect_to_char(victim, &af);
}

void perform_poison(char_data *ch, struct obj_data *wielded, char_data *victim) {

	struct affected_type af;

	if(victim == nullptr)
		return;

	if(AFF_FLAGGED(victim, AFF_POISON))
		return;

	if(IN_ROOM(ch) != IN_ROOM(victim))
		return;

	act("You slash into $N's body deeply, injecting poison from your weapon into $S body!", TRUE, ch, nullptr, victim, TO_CHAR);
	act("$n slashes into deeply into your body, injecting a tainted poison inside of you!", TRUE, ch, nullptr, victim, TO_VICT);
	act("$n slashes into $N deeply, injecting poison into $S body!", TRUE, ch, nullptr, victim, TO_NOTVICT);

	af.type = SKILL_NOTICE;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(victim, &af);
}
	

void check_fighting(void)
{
	char_data *ch;
	struct obj_data *weapon;

	for(ch = combat_list;ch;ch = next_combat_list)
	{
		next_combat_list = ch->next_fighting;

		if(!FIGHTING(ch))
			continue;

		weapon = GET_EQ(ch, WEAR_WIELD);
		
		if(weapon && IS_OBJ_STAT(weapon, ITEM_POISON) && number(1, 40) == 1)
			perform_poison(ch, weapon, FIGHTING(ch));

		if(IS_FADE(ch) &&
		number(GET_SKILL(ch, SKILL_FEAR), 2000) <= GET_SKILL(ch, SKILL_FEAR) && FIGHTING(ch))
			perform_fear(ch, FIGHTING(ch));
	}
}

struct fight_noise
{
	int room;
	struct fight_noise *next;
};

void add_noise_room(int room)
{
	struct fight_noise *new_room = NEW(fight_noise, 1);

	new_room->room = room;
	new_room->next = fn;
	fn = new_room;
}

void kill_noise_list(void)
{
	struct fight_noise *cur, *cur2;

	cur = fn;
	while(cur)
	{
		cur2 = cur->next;
		delete(cur);
		cur = cur2;
	}

	fn = nullptr;
}

void fighting_noise(int room, int distance)
{
	struct fight_noise *cur;

	for(cur = fn;cur;cur = cur->next)
		if(cur->room == room)
			return;

	add_noise_room(room);
	perform_group_noise(room, distance, 0, TYPE_BATTLE);
}

/* control the fights going on.  Called every 4 seconds from comm.c. */
void perform_violence(void)
{
	char_data *ch, *i;
	struct obj_data *wielded;
	int percent, prob, num;

	for (ch = combat_list; ch; ch = next_combat_list)
	{
		next_combat_list = ch->next_fighting;

		if(!IS_NPC(ch))
		{
			if(!ch->desc) 
				return;

		/* Checks the characters timer to see whether to pass the round or go on to hit() *Galnor* */
			if(CHECK_WAIT(ch))
				continue;
		
			if(ch->desc->timer < 0.0)
				ch->desc->timer = 0.0;
		
			if(ch->desc->timer > 0.0)
				continue;
		}

		if(IS_NPC(ch) && GET_MOB_WAIT(ch))
			continue;

		if (GET_POS(ch) < POS_FIGHTING)
			change_pos(ch, POS_FIGHTING);
      
		if (GET_POS(ch) < POS_FIGHTING)
			continue;

		if (FIGHTING(ch) == nullptr || ch->in_room != FIGHTING(ch)->in_room)
		{
			stop_fighting(FIGHTING(ch));  
			stop_fighting(ch);
			continue;
		}

		if(IS_NPC(ch))
			if(MOB_FLAGGED(ch, MOB_NOFIGHT))
				continue;

		wielded = GET_EQ(ch, WEAR_WIELD);

		if(wielded)
			if(IS_OBJ_STAT(wielded, ITEM_CHAIN))
				hit_all_fighting_char(ch);

		for(num = 0, i = world[ch->in_room].people;i;i = i->next_in_room)
			if(FIGHTING(i))
				num++;

		//fighting_noise(ch->in_room, num);
		if(!FIGHTING(ch))
			return;
			
		hit(ch, FIGHTING(ch), TYPE_UNDEFINED);

		if(wielded)
		{
			if(GET_OBJ_VAL(wielded, 3) == 11 || IS_FADE(ch) || ch->getClan(CLAN_BLADEMASTERS)
				|| AFF_FLAGGED(ch, AFF_HASTE) || IS_WARDER(ch))
			{
				percent = number(1, 150);	/* random for attack */

				if(AFF_FLAGGED(ch, AFF_HASTE))
					prob = 99;
				
				else
					prob = GET_SKILL(ch, SKILL_ATTACK);

				if(percent < prob && FIGHTING(ch) != nullptr)
					hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
		
			}
		}

		/* XXX: Need to see if they can handle "" instead of nullptr. */
		if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != nullptr)
			(mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
	}

	//kill_noise_list();
}

/* Added by Galnor in October of 2003. Counts the number of people on character. */
int count_fighting(char_data *ch)
 {
	int count = 0;
	char_data *victim;

	for(victim = world[ch->in_room].people;victim;victim = victim->next_in_room)
	{
		if(FIGHTING(victim) == ch)
			count++;
	}

	return count;
}

/* Added by Galnor in September of 2003. Saves battle struct info to disk. */
void save_battle()
 {
	FILE *file;

	if(!(file = fopen(BATTLE_FILE, "w")))
	{
		log("ERROR OPENING BATTLE FILE!");
		return;
	}

	fprintf(file, "%d %d", tg.humanwp, tg.trollwp);
	fclose(file);
}
