/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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
#include "dg_scripts.h"

/*   external vars  */
extern struct room_data *world;
extern char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct spell_info_type spell_info[];
extern const struct int_app_type int_app[];
extern char *class_types[];
extern int top_of_world;
extern const char *spells[];

/* Spell voids */

void fireball(char_data * ch);
void cast_fireball(char_data * ch);
void raw_kill(char_data *ch, char_data *killer);
void add_follower(char_data * ch, char_data * leader);
ACMD(do_drop);
ACMD(do_gen_door);
ACMD(do_say);
ACMD(do_tell);
ACMD(do_yell);
ACMD(do_gen_comm);
ACMD(do_follow);
ACMD(do_assist);

int cmd_yell;
/* local functions */
const char *how_good(int percent);
void list_skills(char_data * ch);
void sort_spells(void);
void npc_steal(char_data * ch, char_data * victim);
SPECIAL(guild);
SPECIAL(mayor);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(guild_guard);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(cityguard);
SPECIAL(pet_shops);
SPECIAL(bank);
SPECIAL(guardian);
SPECIAL(taim);
SPECIAL(angry);
SPECIAL(call_gate);
SPECIAL(mc);
SPECIAL(fade);
SPECIAL(trolloc);
SPECIAL(yurian);

#define R_ROOM(zone, num) (real_room(((zone)*100)+(num)))
/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

char_data *find_npc_by_name(char_data * chAtChar,
		const char *pszName, int iLen)
{
	char_data *ch;

	for (ch = world[chAtChar->in_room].people; ch; ch = ch->next_in_room)
		if (IS_NPC(ch))
			if (!strncmp(pszName, ch->player.short_descr, iLen))
				return (ch);

	return nullptr;
}

#define CLASS_WARRIOR		0
#define CLASS_THIEF		1
#define CLASS_RANGER		2
#define CLASS_CHANNELER		3
#define CLASS_FADE		4
#define CLASS_DREADLORD		5
#define CLASS_BLADEMASTER	6
#define CLASS_GREYMAN		7
#define CLASS_DRAGHKAR		8

//Use like this: skill_cost[MY_CLASS][SKILL_CLASS]  -- Tulon/3/11/2004
int skill_cost[NUM_CLASSES][NUM_CLASSES] =
{
	//W   T   R   C   F   DL   B   G   DK
	{ 1,  3,  2,  0,  0,  0,   0,  0,  0}, //Warriors
	{ 3,  1,  2,  0,  0,  0,   0,  0,  0}, //Thieves
	{ 2,  2,  1,  0,  0,  0,   0,  0,  0}, //Ramgers
	{ 3,  2,  2,  1,  0,  0,   0,  0,  0}, //Channelers
	{ 1,  2,  1,  0,  1,  0,   0,  0,  0}, //Fades
	{ 2,  2,  2,  1,  0,  1,   0,  0,  0}, //Dreadlords
	{ 1,  2,  1,  0,  0,  0,   1,  0,  0}, //Blademasters
	{ 2,  1,  2,  0,  0,  0,   0,  1,  0}, //Greymen
	{ 0,  0,  0,  0,  0,  0,   0,  0,  1}  //Draghkar

};

//Function put to use by Tulon on 3-11-2004
int calc_price(char_data *ch, int number)
{
	
	int skill_class = spell_info[number].class_type;

	return (skill_cost[(int) GET_CLASS(ch)][skill_class]);
		

}

//NPC damage. Used mainly for special MOBs who we don't want to run through the main damage function//
void dmg(char_data *ch, char_data *vict, int low, int high, int same_room)
{

	
	int dam;
	dam = number(low, high);

	if(IN_ROOM(ch) != IN_ROOM(vict) && same_room)
		return;

	GET_HIT(vict) -= dam;
	
	if(GET_HIT(vict) <= 0)
		raw_kill(vict, ch);

}


int spell_sort_info[MAX_SKILLS+1];

void sort_spells(void)
{
	int a, b, tmp;

	/* initialize array */
	for (a = 1; a < MAX_SKILLS; a++)
		spell_sort_info[a] = a;

	/* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
	for (a = 1; a < MAX_SKILLS - 1; a++)
		for (b = a + 1; b < MAX_SKILLS; b++)
			if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0)
			{
				tmp = spell_sort_info[a];
				spell_sort_info[a] = spell_sort_info[b];
				spell_sort_info[b] = tmp;
			}
}


const char *how_good(int percent)
{
	if (percent < 0)
		return " error)";
  
	if (percent == 0)
		return " (not learned)";
  
	if (percent <= 10)
		return " (awful)";
  
	if (percent <= 20)
		return " (bad)";
  
	if (percent <= 40)
		return " (poor)";
  
	if (percent <= 55)
		return " (average)";
  
	if (percent <= 70)
		return " (fair)";
  
	if (percent <= 80)
		return " (good)";
  
	if (percent <= 85)
		return " (very good)";

	return " (superb)";
}

const char *prac_types[] =
{
	"spell",
	"skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[4][NUM_CLASSES];

#define LEARNED(ch) 99
#define SPLSKL(ch)	(prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

void know_spell(char_data * ch)
{
	GET_SKILL(ch, SPELL_FIREBALL) = 99;
}

void list_skills(char_data * ch, char_data *teacher)
{
	int i, sortpos, has_pracs;

	has_pracs = GET_PRACTICES(ch);
	
	if(IS_CHANNELER(ch) || IS_DREADLORD(ch))
		has_pracs += GET_SPRACTICES(ch);
    
	if(teacher)
		message(ch, "It will cost you %d practice%s to practice here.\r\n", 
		skill_cost[(int) GET_CLASS(ch)][(int) GET_CLASS(teacher)], skill_cost[(int) GET_CLASS(ch)][(int) GET_CLASS(teacher)] > 0 ? "s" : "");

	if(has_pracs <= 0)
		strcpy(buf, "You have no practice sessions remaining.\r\n");
	  
	if(has_pracs > 0)
	{
		sprintf(buf, "You have %d practice session%s remaining",
		GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));
		
		if(IS_CHANNELER(ch) || IS_DREADLORD(ch))
			sprintf(buf + strlen(buf), ", and %d spell practice%s remaining.\r\n",
			GET_SPRACTICES(ch), (GET_SPRACTICES(ch) == 1 ? "" : "s"));

	
		else
			sprintf(buf + strlen(buf), ".\r\n");
	}



	sprintf(buf + strlen(buf), "You know of the following %ss:\r\n", SPLSKL(ch));
	strcpy(buf2, buf);

	for (sortpos = 1; sortpos <= MAX_SKILLS; sortpos++)
	{
		i = spell_sort_info[sortpos];
    
		if (strlen(buf2) >= MAX_STRING_LENGTH - 32)
		{
			strcat(buf2, "**OVERFLOW**\r\n");
			break;
		}

		if(GET_SKILL(ch, i) <= 0 && !teacher)
			continue;

		if(!can_practice(ch, i))
			continue;

		if((teacher) && (GET_CLASS(teacher) != spell_info[i].class_type))
			continue;

		sprintf(buf, "%-20s %s, ( %d%% )\r\n", spells[i], how_good(GET_SKILL(ch, i)), GET_SKILL(ch, i));
		strcat(buf2, buf);
	}

	page_string(ch->desc, buf2, 1);
}


SPECIAL(angry)
{
	char_data *cook = (char_data *) me;

	if(cmd)
		return FALSE;

	if(GET_POS(cook) == POS_STANDING)
	{
    
		switch(number(1, 10))
		{
    
		case 1:
			act("$n cuts some meat, and while doing it cuts $s finger.", FALSE, cook, 0, 0, TO_ROOM);
    
		case 2:
		case 3:
			act("$n screams and throws a knife at the wall.", FALSE, cook, 0, 0, TO_ROOM);
			break;
    
		case 7:
			act("$n mutters about a hard days work.", FALSE, cook, 0, 0, TO_ROOM);
    
		case 10:
			command_interpreter(cook, "yell Fresh meat! Some with blood, but I'll take care of that!");
			break;
		}
	}

	if(GET_HOUR == 1 && GET_POS(cook) != POS_SLEEPING)
	{
		act("$n says 'Time to sleep... FINALLY!'", FALSE, cook, 0, 0, TO_ROOM);
		act("$n unfolds a bed and instantly drops down asleep.", FALSE, cook, 0, 0, TO_ROOM);
		change_pos(cook, POS_SLEEPING);
		return FALSE;
    }

    else if (GET_HOUR == 4 && GET_POS(cook) != POS_STANDING)
	{
		change_pos(cook, POS_STANDING);
		act("$n groans as $e wakes up and gets back to work.", FALSE, ch, 0, 0, TO_ROOM);
		return FALSE;
    }

	return 0;
}

int black_tower_weave[11] =
{

	0,
	SPELL_NIGHT_VISION,
	SPELL_SHIELD,
	SPELL_HASTE,
	SPELL_LOCATE_LIFE,
	SPELL_TORNADO,
	SPELL_RESTORE,
	SPELL_GATE,
	-1,
	-1,
	SPELL_BALEFIRE,
};

/* Return 0 if this skill is not to be listed/practicable by a person */
int can_practice(char_data *ch, int skill)
{
	int i = 0, num = 0;
	player_clan_data *cl;


	if (skill <= 0 || GET_LEVEL(ch) < spell_info[skill].min_level)
		return 0;

	if(!IS_CHANNELER(ch) && !IS_DREADLORD(ch) && skill <= MAX_SPELLS)
		return 0;

	for(i = 1;i <= 10;i++)
	{

		num = black_tower_weave[i];

		if(num == skill)
		{
			if((cl = ch->getClan(CLAN_BLACK_TOWER)) || (cl = ch->getClan(CLAN_CHOSEN)) && cl->rank >= i)
			{
				return 1;
			}

			else
			{
				return 0;
			}
		}
	}

	if(skill == SKILL_FADE && !IS_FADE(ch))
		return 0;

	if(skill == SKILL_EFFUSION && !IS_GREYMAN(ch))
		return 0;

	if(skill == SKILL_FEAR && !IS_FADE(ch))
		return 0;

	if(skill == SKILL_CHARGE && !CAN_RIDE(ch))
		return 0;

	if(skill == SKILL_SKEWER && CAN_RIDE(ch))
		return 0;

	if(skill == SPELL_BOND && !ch->AES_SEDAI())
		return 0;

	return 1;

}

SPECIAL(guild)
{
	int skill_num, percent, price;
	char_data *teacher = (char_data *) me;

	if (IS_NPC(ch) || !CMD_IS("practice"))
		return 0;

	if(IS_CHANNELER(ch) && IS_CHANNELER(teacher) && GET_SEX(ch) != GET_SEX(teacher))
		return 0;

	skip_spaces(&argument);

	if (!*argument)
	{
		list_skills(ch, teacher);
		return 1;
	}

	skill_num = find_skill_num(argument);

	if (GET_PRACTICES(ch) <= 0 && skill_num > MAX_SPELLS)
	{
		send_to_char("You do not seem to be able to practice now.\r\n", ch);
		return 1;
	}

	price = calc_price(ch, skill_num);

	if(!can_practice(ch, skill_num) || ((teacher) && (GET_CLASS(teacher) != spell_info[skill_num].class_type)))
	{
		do_say(teacher, "I do not teach that skill here.", 0, 0);
		return 1;
	}
  
	if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
	{
		send_to_char("You are already learned in that area.\r\n", ch);
		return 1;
	}
	
	if(skill_num <= MAX_SPELLS && (IS_CHANNELER(ch) || IS_DREADLORD(ch)))
	{
		if(GET_SPRACTICES(ch) > 0)
			GET_SPRACTICES(ch) -= 1;
	  
		else
		{
			send_to_char("You cannot practice spells right now\r\n", ch);
			return 0;
		}
	}

	else
		GET_PRACTICES(ch) -= price;

	send_to_char("You practice for a while. ", ch);

	percent = GET_SKILL(ch, skill_num);
	percent += prac_add(ch, percent, skill_num);

	SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

	if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
		send_to_char("You are now learned in that area.\r\n", ch);

	else
		message(ch, "The skill %s is now practiced to %d%%.\r\n", spells[skill_num], GET_SKILL(ch, skill_num));

	return 1;
}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)

SPECIAL(pet_shops)
{
	char buf[MAX_STRING_LENGTH], pet_name[256];
	int pet_room;
	char_data *pet;

	pet_room = ch->in_room + 1;

	if (CMD_IS("list"))
	{
		send_to_char("Available pets are:\r\n", ch);

		for (pet = world[pet_room].people; pet; pet = pet->next_in_room)
		{
			sprintf(buf, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
			send_to_char(buf, ch);
		}

		return (TRUE);
	}

	else if (CMD_IS("buy"))
	{

		argument = one_argument(argument, buf);
		argument = one_argument(argument, pet_name);

		if (!(pet = get_char_room(buf, pet_room)))
		{
			send_to_char("There is no such pet!\r\n", ch);
			return (TRUE);
		}

		if (GET_GOLD(ch) < PET_PRICE(pet))
		{
			send_to_char("You don't have enough gold!\r\n", ch);
			return (TRUE);
		}

		GET_GOLD(ch) -= PET_PRICE(pet);

		pet = read_mobile(GET_MOB_RNUM(pet), REAL);
		GET_EXP(pet) = 0;

		if (*pet_name)
		{
			sprintf(buf, "%s %s", pet->player.name, pet_name);
			/* free(pet->player.name); don't free the prototype! */
			pet->player.name = str_dup(buf);

			sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
			pet->player.description, pet_name);

			/* free(pet->player.description); don't free the prototype! */
			pet->player.description = str_dup(buf);
		}

		char_to_room(pet, ch->in_room);
		add_follower(pet, ch);
		load_mtrigger(pet);

		send_to_char("May you enjoy your pet.\r\n", ch);
		act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

		return 1;
	}

	/* All commands except list and buy */
	return 0;
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
	int amount;

	if (CMD_IS("balance"))
	{
		if (GET_BANK_GOLD(ch) > 0)
			sprintf(buf, "Your current balance is %d coins.\r\n",
			GET_BANK_GOLD(ch));

		else
			sprintf(buf, "You currently have no money deposited.\r\n");
			send_to_char(buf, ch);

			return 1;
	}
	
	else if (CMD_IS("deposit"))
	{
		if ((amount = atoi(argument)) <= 0)
		{
			send_to_char("How much do you want to deposit?\r\n", ch);
			return 1;
		}

		if (GET_GOLD(ch) < amount)
		{
			send_to_char("You don't have that many coins!\r\n", ch);
			return 1;
		}

		GET_GOLD(ch) -= amount;
		GET_BANK_GOLD(ch) += amount;
		sprintf(buf, "You deposit %d coins.\r\n", amount);
		send_to_char(buf, ch);
		act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
		return 1;
	}

	else if (CMD_IS("withdraw"))
	{
		if ((amount = atoi(argument)) <= 0)
		{
			send_to_char("How much do you want to withdraw?\r\n", ch);
			return 1;
		}

		if (GET_BANK_GOLD(ch) < amount)
		{
			send_to_char("You don't have that many coins deposited!\r\n", ch);
			return 1;
		}

		GET_GOLD(ch) += amount;
		GET_BANK_GOLD(ch) -= amount;
		sprintf(buf, "You withdraw %d coins.\r\n", amount);
		send_to_char(buf, ch);
		act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
		return 1;
	}
	
	else
		return 0;
}


SPECIAL(guardian)
{
	struct obj_data *obj;

	//load up specific objects for kits.
	int helmet = real_object(1);
	int shirt = real_object(30);
	int sleeves = real_object(60);
	int gloves = real_object(90);
	int weapon = real_object(330);
	int shield = real_object(870);
	int pants = real_object(120);
	int boots = real_object(150);

	int light = real_object(900);
	int skin = real_object(920);
	int sack = real_object(911);
	int meat = real_object(930);

	if (CMD_IS("kit")) //If the character types kit then it continues...
	{

		if(!PRF_FLAGGED(ch, PRF_KIT) && GET_LEVEL(ch) <= 5 || GET_LEVEL(ch) > 99)
		{

			SET_BIT_AR(PRF_FLAGS(ch), PRF_KIT);   //If the character has not had a kit yet, this will make sure that they don't get another.


			act("\nThe Guardian of Life tells you 'This is your kit, given to you once per life,", TRUE, ch, 0, 0, TO_CHAR);
			act("as long as you are under the level of 5'", TRUE, ch, 0, 0, TO_CHAR);
			act("$n is granted a kit.", FALSE, ch, 0, 0, TO_ROOM);
        
			obj = read_object(helmet, REAL);
			obj_to_char(obj, ch);

			obj = read_object(shirt, REAL);
			obj_to_char(obj, ch);

			obj = read_object(sleeves, REAL);
			obj_to_char(obj, ch);

			obj = read_object(gloves, REAL);
			obj_to_char(obj, ch);

			obj = read_object(meat, REAL);
			obj_to_char(obj, ch);

			obj = read_object(weapon, REAL);
			obj_to_char(obj, ch);

			obj = read_object(shield, REAL);
			obj_to_char(obj, ch);

			obj = read_object(light, REAL);
			obj_to_char(obj, ch);

			obj = read_object(pants, REAL);
			obj_to_char(obj, ch);

			obj = read_object(boots, REAL);
			obj_to_char(obj, ch);
		
			obj = read_object(skin, REAL);
			obj_to_char(obj, ch);

			obj = read_object(sack, REAL);
			obj_to_char(obj, ch);

			obj = read_object(meat, REAL);
			obj_to_char(obj, ch);

			GET_GOLD(ch) += 500;

			return 1;
		}

		else
		{
			send_to_char("You have had a kit once already!\n", ch);
			act("\nThe gods look shamefully towards you", FALSE, ch, 0, 0, TO_CHAR);
			act("The gods look shamefully towards $n", FALSE, ch, 0, 0, TO_ROOM);
			return 1;
		}

	}
	
	return 0;
}

SPECIAL(yurian)
{
	char_data *yurian = (char_data *) me;
	char_data *vict;

	if(cmd)
		return FALSE;

	if(FIGHTING(yurian))
	{
		vict = FIGHTING(ch);
	}
	else
	{
		return FALSE;
	}

	if (number(1, 15) > 10)
	{
		hit(yurian, vict, TYPE_UNDEFINED);
	}

	else if(number(1, 4) == 4)
	{
		if(number(1, 3) == 3 && FIGHTING(yurian))
			cast_spell(yurian, vict, nullptr, SPELL_FIREBALL, 0);
	}

	else if(number(1, 4) == 4)
	{
		act("Your body is BURNED by a huge blast of flames sent by $n!!!", 1, yurian, nullptr, yurian, TO_VICT);
		act("$n looks deeply into $N, and shoots a huge blast of flames at him!", 1, yurian, nullptr, vict, TO_NOTVICT);
		act("You blast a huge sheet of flame towards $N, scorching his body!.", 1, yurian, nullptr, vict, TO_CHAR);
		dmg(yurian, vict, 80, 150, TRUE);
	}

	else if(number(1, 6) == 5)
	{
		act("$n weaves around himself, looking a bit more healed.", 1, yurian, nullptr, nullptr, TO_ROOM);
		act("You weave around yourself, healing your body.", 1, yurian, nullptr, nullptr, TO_CHAR);
		GET_HIT(yurian) += number(20, 50);
	}

	else
	{
		return FALSE;
	}

	return TRUE;
}


SPECIAL(taim)
{

	char_data *taim = (char_data *) me;
	char_data *vict;
	int i = 0;

	/* If we're here because of command input or if Taim is not fighting, return the function */
	if (cmd || GET_POS(taim) != POS_FIGHTING)
		return FALSE;

	/* Random chance of Taim hitting the victim again */
	if(number(1, 5) == 5 && FIGHTING(taim)) {
		do_say(taim, "Fool, you shall die!", 0, 0);
		hit(taim, FIGHTING(taim), TYPE_UNDEFINED);
	}

	/* Check for each person inside Taim's room, if they are fighting him, there is a 50% chance he chooses
	 * that person to weave against. If the chance fails, check the next person fighting Taim.
	 */
	for(vict = world[taim->in_room].people;vict;vict = vict->next_in_room) {
		i = number(1, 2);
		
		if(i == 2)
			continue;
		  
		if(FIGHTING(vict) != taim)
			continue;
		
		if(vict != nullptr)
			break;
	}

	if(vict == nullptr || !vict && FIGHTING(vict) != taim)
		return 0;

	if(number(1, 3) == 3)
		if (cast_spell(taim, vict, nullptr, SPELL_FIREBALL, 0) < 0)
			return FALSE;

	if(number(1, 5) == 5)
		if (cast_spell(taim, vict, nullptr, SPELL_FIREBALL, 0) < 0)
			return FALSE;

	if(number(1, 5) == 5)
		if (cast_spell(taim, vict, nullptr, SPELL_FIREBALL, 0) < 0)
			return FALSE;

	switch (number(1, 8))
	{
		case 1:
		case 2:
		case 3:
		case 4:
			if (GET_LEVEL(vict) < LVL_IMMORT)
			{
				act("$n throws a ball of energy, which hits your body!", 1, taim, 0, vict, TO_VICT);
				act("$n throws a ball of energy at $N, which scorches $s body!", 1, taim, 0, vict, TO_NOTVICT);
				act("You throw a ball of energy at $N, causing him to scream in pain.", 1, taim, 0, vict, TO_CHAR);
				dmg(taim, vict, 20, 80, TRUE);
			}
			break;
  
		case 6:
			cast_spell(taim, vict, nullptr, SPELL_FIREBALL, 0);
			break;
		case 7:
			if(GET_HIT(taim) < GET_MAX_HIT(taim) / 5)
			{
				act("$n weaves around himself, and looks alot better.", 1, taim, 0, 0, TO_ROOM);
				act("You feel the curing power from your hands reach inside and cure your wounds.", 1, taim, 0, 0, TO_CHAR);
				GET_HIT(taim) += number(30, 60);
			}
			break;
		case 8:
			if(GET_LEVEL(vict) < LVL_GOD)
			{
				act("Your body is BURNED by a huge blast of flames sent by $n!!!", 1, taim, 0, vict, TO_VICT);
				act("$n looks deeply into $N, and shoots a huge blast of flames at him!", 1, taim, 0, vict, TO_NOTVICT);
				act("You blast a huge sheet of flame towards $N, scorching his body!.", 1, taim, 0, vict, TO_CHAR);
				dmg(taim, vict, 60, 90, TRUE);
			}
			break;
		default:
			return 0;
	}

	return TRUE;

}


SPECIAL(mc)
{

	char_data *vict;
	char_data *taim;
	char_data *us;
	char_data *mc = (char_data *) me;

	if (!AWAKE(ch))
		return FALSE;

	if ((!cmd) && (taim = find_npc_by_name(ch, "Mazrim Taim", 11)))
		if (!ch->master)
			do_follow(ch, "Mazrim Taim", 0, 0);

	if (!cmd && GET_POS(ch) != POS_FIGHTING)
		do_assist(ch, "Mazrim Taim", 0, 0);

	us = find_npc_by_name(ch, "dark channeler", 11);

	if (!cmd && GET_POS(ch) != POS_FIGHTING)
		do_assist(ch, "dark channeler", 0, 0);

	vict = FIGHTING(mc);

	if(cmd || !FIGHTING(mc))
		return 0;

	if(vict == nullptr)
		return 0;

	if(GET_POS(mc) == POS_FIGHTING)
	{
		switch(number(1, 5))
		{
			case 1:
				cast_spell(mc, vict, nullptr, SPELL_FIREBALL, 0);
				break;
			
			case 3:
				cast_spell(mc, mc, nullptr, SPELL_AGILITY, 0);
				break;   
		}
	}

	return 0;
}

SPECIAL(fade)
{
	char_data *trolloc;
	char_data *fade = (char_data *) me;

	if (!AWAKE(ch))
		return FALSE;


	trolloc = find_npc_by_name(ch, "Trolloc Guard", 11);

	if (!cmd && GET_POS(ch) != POS_FIGHTING)
		do_assist(fade, "Trolloc Guard", 0, 0);


	return 0;
}

SPECIAL(trolloc)
{
	char_data *Mjuurk;

	if (!AWAKE(ch))
		return FALSE;

	if ((!cmd) && (Mjuurk = find_npc_by_name(ch, "Mjuurk", 11)))
	{
		if (!ch->master)
			do_follow(ch, "Mjuurk", 0, 0);
	}

	if (!cmd && GET_POS(ch) != POS_FIGHTING)
		do_assist(ch, "Mjuurk", 0, 0);

	return 0;
}
