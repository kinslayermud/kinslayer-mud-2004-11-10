/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
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
#include "interpreter.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "constants.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct room_data *world;

/* local functions */
void spello(int spl, int max_mana, int min_mana, int mana_change, int minpos, int targets, int violent, int routines);
int mag_manacost(char_data * ch, int spellnum);
void unused_spell(int spl);
void mag_assign_spells(void);
int mag_damage(int level, char_data *ch, char_data *victim, int spellnum, int savetype);
int room_visibility(char_data *ch, char_data *vict);

ACMD(do_cast);
ACMD(do_weaves);

char_data *randomTarget(char_data *ch);



/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

const char *spells[] =
{
  "!RESERVED!",			/* 0 - reserved */

  /* SPELLS */

  "Agility",		/* 1 */
  "Teleport",
  "Bless",
  "Blindness",
  "Call Lightning",
  "Chill Touch",		
  "Control Weather",	
  "Create Food",
  "Create Water",
  "Cure Blind",		/* 10 */
  "Cure Critic",
  "Cure Light",
  "Earthquake",
  "Enchant Weapon",
  "Fireball",	
  "Locate Object",
  "Poison",
  "Sanctuary",
  "Sleep",
  "Strength",		/* 20 */
  "Remove Poison",
  "Regen",
  "Air Scythe",
  "Night Vision",
  "Shield",
  "Haste",
  "Locate Life",
  "Tornado",
  "Restore",
  "Balefire",		/* 30 */
  "Gate",
  "Bond",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",		/* 40 */
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",		/* 50 */
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",
  "!UNUSED!",		/* 60 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 65 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 70 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 75 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 80 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 85 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 90 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 95 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 100 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 105 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 110 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 115 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 120 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 125 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 130 */

  /* SKILLS */

  "Backstab",			/* 131 */
  "Bash",
  "Hide",
  "Kick",
  "Pick",
  "Rescue",
  "Sneak",
  "Steal",
  "Track",
  "Attack",
  "Bow",				/* 140 */
  "Dodge", "Shield Parry", "Notice", "Ride", "Search",			/* 145 */
  "Fade", "Crafting", "Effusion", "Fear", "Clubs",				/* 150 */
  "Axes", "Long Blades", "Short Blades", "Staves", "Spears",	/* 155 */
  "Chains", "Vitality", "Charge", "Skewer", "Lance",			/* 160 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 165 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 170 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 175 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 180 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 185 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 190 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 195 */
  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",	/* 200 */

  /* OBJECT SPELLS AND NPC SPELLS/SKILLS */

  "identify",			/* 201 */
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",

  "\n"				/* the end */
};


int mag_manacost(char_data * ch, int spellnum)
{
	float mana;
	struct obj_data *angreal;

	mana = MAX(SINFO.mana_max - (SINFO.mana_change *
	(GET_LEVEL(ch) - SINFO.min_level)),
	SINFO.mana_min);

	/* If player is carrying an angreal, subtract the mana cost. */
	if ( (angreal = GET_EQ(ch, WEAR_HOLD)) != nullptr)
	{
		if(GET_OBJ_TYPE(angreal) == ITEM_ANGREAL && GET_OBJ_VAL(angreal, 2) != 0)
		{
			/* Turn this into a percent. */
			mana *= ((float) ((float)GET_OBJ_VAL(angreal, 0) / 100));
		}
	}

	return (int) mana;
}


const char *skill_name(int num)
{
	int i = 0;

	if (num <= 0)
	{
		
		if (num == -1)
			return "UNUSED";
    
		else
			return "UNDEFINED";
	}

	while (num && *spells[i] != '\n')
	{
		num--;
		i++;
	}

	if (*spells[i] != '\n')
		return spells[i];
  
	else
		return "UNDEFINED";
}

	 
int find_skill_num(char *name)
{
	int index = 0, ok;
	char *temp, *temp2;
	char first[256], first2[256];

	while (*spells[++index] != '\n')
	{
		if (is_abbrev(name, spells[index]))
			return index;

		ok = 1;
		/* It won't be changed, but other uses of this function elsewhere may. */
		temp = any_one_arg((char *)spells[index], first);
		temp2 = any_one_arg(name, first2);
    
		while (*first && *first2 && ok)
		{
      
			if (!is_abbrev(first2, first))
				ok = 0;
      
			temp = any_one_arg(temp, first);
			temp2 = any_one_arg(temp2, first2);
		}

		if (ok && !*first2)
			return index;
	}

	return -1;
}	



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(char_data * caster, char_data * cvict,
	     struct obj_data * ovict, int spellnum, int level, int casttype, char *argument)
{
	int savetype;
	struct obj_data *angreal;

	if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
		return 0;

	if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC) && casttype != CAST_POTION)
	{
		send_to_char("Your magic fizzles out and dies.\r\n", caster);
		act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
		return 0;
	}
  
	if (ROOM_FLAGGED(caster->in_room, ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)))
	{
		send_to_char(	"A flash of white light fills the room, dispelling your "
						"violent magic!\r\n", caster);
		
		act(			"White light from no particular source suddenly fills the room, "
						"then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
		return 0;
	}
  
	/* determine the type of saving throw */
  
	switch (casttype)
	{
  
	case CAST_STAFF:
	case CAST_SCROLL:
	case CAST_POTION:
	case CAST_WAND:
		savetype = SAVING_ROD;
		break;
  
	case CAST_SPELL:
		savetype = SAVING_SPELL;
		break;
	
	default:
		savetype = SAVING_BREATH;
		break;
	}


	/*
	 * Hm, target could die here.  Wonder if we should move this down lower to
	 * give the other spells a chance to go off first? -gg 6/24/98
	 */
  
	if (IS_SET(SINFO.routines, MAG_DAMAGE))
		if (mag_damage(level, caster, cvict, spellnum, savetype) < 0)
			return -1;

  
	if (IS_SET(SINFO.routines, MAG_AFFECTS))
		mag_affects(level, caster, cvict, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
		mag_unaffects(level, caster, cvict, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_POINTS))
		mag_points(level, caster, cvict, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
		mag_alter_objs(level, caster, ovict, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_GROUPS))
		mag_groups(level, caster, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_MASSES))
		mag_masses(level, caster, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_AREAS))
		mag_areas(level, caster, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_SUMMONS))
		mag_summons(level, caster, ovict, spellnum, savetype);

	if (IS_SET(SINFO.routines, MAG_CREATIONS))
		mag_creations(level, caster, spellnum);

	if (IS_SET(SINFO.routines, MAG_MANUAL))
		switch (spellnum)
		{
    
		case SPELL_CREATE_WATER:
			MANUAL_SPELL(spell_create_water);
			break;
		
		case SPELL_ENCHANT_WEAPON:
			MANUAL_SPELL(spell_enchant_weapon);
			break;
		
		case SPELL_LOCATE_OBJECT:
			MANUAL_SPELL(spell_locate_object);
			break;
		
		case SPELL_TELEPORT:
			MANUAL_SPELL(spell_teleport);
			break;

		case SPELL_GATE:
			MANUAL_SPELL(spell_gate);
			break;
		
		case SPELL_REGEN:
			MANUAL_SPELL(spell_regen);
			break;

		case SPELL_RESTORE:
			MANUAL_SPELL(spell_restore);
			break;

		case SPELL_LOCATE_LIFE:
			MANUAL_SPELL(spell_locate_life);
			break;

		case SPELL_TORNADO:
			MANUAL_SPELL(spell_tornado);
			break;

		case SPELL_BOND:
			MANUAL_SPELL(spell_bond);
			break;

	}

	/* Subtract a charge from an angreal if it is not set to unlimited charges (-1) */
	if ( (angreal = GET_EQ(caster, WEAR_HOLD)) != nullptr)
	{
		if(GET_OBJ_TYPE(angreal) == ITEM_ANGREAL && GET_OBJ_VAL(angreal, 2) > 0)
		{
			GET_OBJ_VAL(angreal, 2) -= 1;
		}
	}

	return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */

void mag_objectmagic(char_data * ch, struct obj_data * obj,
		          char *argument)
{
	int i, k;
	char_data *tch = nullptr;
	struct obj_data *tobj = nullptr;

	one_argument(argument, arg);

	k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
	FIND_OBJ_EQUIP, ch, &tch, &tobj);

  
	switch (GET_OBJ_TYPE(obj))
	{
  
	case ITEM_POTION:
		
		tch = ch;
		act("You quaff $p.", FALSE, ch, obj, nullptr, TO_CHAR);
    
		if (obj->action_description)
			act(obj->action_description, FALSE, ch, obj, nullptr, TO_ROOM);
    
		else
			act("$n quaffs $p.", TRUE, ch, obj, nullptr, TO_ROOM);

		for (i = 1; i < 4; i++)
			if (!(call_magic(ch, ch, nullptr, GET_OBJ_VAL(obj, i),
			GET_OBJ_VAL(obj, 0), CAST_POTION, nullptr)))
				break;

    
		if (obj != nullptr)
			extract_obj(obj);
    
		break;
  
	default:
		log("SYSERR: Unknown object_type %d in mag_objectmagic.",
		GET_OBJ_TYPE(obj));
		break;
  
	}
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(char_data * ch, char_data * tch,
	           struct obj_data * tobj, int spellnum, char *argument)
{
	if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE)
	{
		log("SYSERR: cast_spell trying to call spellnum %d/%d.\n", spellnum,
		TOP_SPELL_DEFINE);
		return 0;
	}
    
	if (GET_POS(ch) < SINFO.min_position)
	{
    
		switch (GET_POS(ch))
		{
      
		case POS_SLEEPING:
			send_to_char("You dream about great magical powers.\r\n", ch);
			break;
    
		case POS_RESTING:
			send_to_char("You cannot concentrate while resting.\r\n", ch);
			break;
    
		case POS_SITTING:
			send_to_char("You can't do this sitting!\r\n", ch);
			break;
    
		case POS_FIGHTING:
			send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
			break;
    
		default:
			send_to_char("You can't do much of anything like this!\r\n", ch);
			break;
		}
    
		return 0;
	}
  
	if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY))
	{
		send_to_char("You can only cast this spell upon yourself!\r\n", ch);
		return 0;
	}
  
	if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF))
	{
		send_to_char("You cannot cast this spell upon yourself!\r\n", ch);
		return 0;
	}
  
	if (IS_SET(SINFO.routines, MAG_GROUPS) && !AFF_FLAGGED(ch, AFF_GROUP))
	{
		send_to_char("You can't cast this spell if you're not in a group!\r\n",ch);
		return 0;
	}
  
	send_to_char(OK, ch);

	return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL, argument));

}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */


void perform_cast(char_data *ch, char_data *tch, int target, int spellnum, struct obj_data *tobj, char *argument)
{

	int mana;
	mana = mag_manacost(ch, spellnum);
  
	if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT))
	{
		send_to_char("You haven't the energy to cast that spell!\r\n", ch);
		return;
	}

	/* You throws the dice and you takes your chances.. 101% is total failure */
	if (number(0, 113) > GET_SKILL(ch, spellnum))
	{
		if (!tch || !skill_message(0, ch, tch, spellnum))
			send_to_char("You lost your concentration!\r\n", ch);
    
		if (mana > 0)
			GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana / 2)));
    
		if (SINFO.violent && tch && IS_NPC(tch))
			hit(tch, ch, TYPE_UNDEFINED);
	} 
	
	else
	{ /* cast spell returns 1 on success; subtract mana & set waitstate */
		if (cast_spell(ch, tch, tobj, spellnum, argument))
		{
			if (mana > 0)
				GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
		}
	}
}

char *randomChannel[] =
{
	"fireball", /* 0 */
	"call lightning",
	"air scythe" /* 2 */
};

int numberOfSpells = 2;

char_data *randomTarget(char_data *ch)
{
	int target, count = 0;
	char_data *temp;

	for(temp = world[IN_ROOM(ch)].people; temp; temp = temp->next_in_room)
		if (room_visibility(ch, temp))
			++count;

	target = number(1, count);

	for(temp = world[IN_ROOM(ch)].people; temp; temp = temp->next_in_room)
		if (room_visibility(ch, temp) && --target == 0)
			return (temp);

	return (nullptr); /* should never get here */
}

ACMD(do_weaves)
{
	
	struct weave_data *weave;
	struct affected_type *af;

	send_to_char("The following weaves are attached to you:\r\n", ch);

	for(weave = ch->weaves;weave;weave = weave->next)
	{
		af = weave->spell;

		message(ch, "%s:		Target: %s. Tied: %s, Time Remaining: %d.\r\n",
			affected_bits[af->bitvector], GET_NAME(weave->target), weave->tied ? "Yes" : "No", af->duration);
	}
}

ACMD(do_cast)
{
	char_data *tch = nullptr;
	struct obj_data *tobj = nullptr;
	char *s, *t;
	char text[MAX_INPUT_LENGTH] = "";
	int spellnum, i, target = 0;
	bool tainted = false;

	/* get: blank, spell name, target name */
	s = strtok(argument, "'");

	if(!IS_CHANNELER(ch) && !IS_DREADLORD(ch) && !IS_NPC(ch))
	{
		if(GET_LEVEL(ch) < LVL_APPR)
		{
			send_to_char("You don't seem to be able to connect to the True Source.\r\n", ch);
			return;
		}
	}

	if(AFF_FLAGGED(ch, AFF_SHIELD))
	{
		send_to_char("You are blocked from the True Source!\r\n", ch);
		return;
	}

	if(!PRF_FLAGGED(ch, PRF_SOURCE) && !IS_NPC(ch))
	{
		message(ch, "You are not in touch with the True Source.\r\n");
		return;
	}

	if (s == nullptr)
	{
		if (GET_TAINT(ch))
		{
			if (GET_TAINT(ch) > 85 && (number(1, 300) <= GET_TAINT(ch)))
			{
				s = randomChannel[number(0, numberOfSpells)];
				tainted = true;
			}
		}

		if (!tainted)
		{
			send_to_char("Cast what where?\r\n", ch);
			return;
		}
	}

	if (!tainted)
		s = strtok(nullptr, "'");

	if (s == nullptr)
	{
		if (GET_TAINT(ch) && !tainted)
		{
			if (GET_TAINT(ch) > 85 && (number(1, 300) <= GET_TAINT(ch)))
			{
				s = randomChannel[number(0, numberOfSpells)];
				tainted = true;
			}
		}


		if (!tainted)
		{
			send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
			return;
		}
	}

	if (!tainted)
		t = strtok(nullptr, "\0");

	/* spellnum = search_block(s, spells, 0); */
	spellnum = find_skill_num(s);

	if ((spellnum < 1) || (spellnum > MAX_SPELLS)) 
	{
		send_to_char("Cast what?!?\r\n", ch);
		return;
	}
  
	if (GET_LEVEL(ch) < SINFO.min_level)
	{
		send_to_char("You do not know that spell!\r\n", ch);
		return;
	}	
  
	if (GET_SKILL(ch, spellnum) == 0 && !tainted)
	{
		send_to_char("You are unfamiliar with that spell.\r\n", ch);
		return;
	}
  
	/* Find the target */

	if (tainted)
	{
		if (IS_SET(SINFO.targets, TAR_CHAR_ROOM))
		{
			if (IN_ROOM(ch) != NOWHERE) /* paranoia of -1 index */
			{
				if (.8*TAINT_CALC(ch) > number(0, 100))
				{
					tch = ch;
				}
				else
				{
					tch = randomTarget(ch);

					/* only bad taint will channel violent on self or curative on others */
					if ( !tch || ((tch == ch && SINFO.violent) || (tch != ch && !SINFO.violent)) && .9*TAINT_CALC(ch) <= number(0, 100))
					{
						message(ch, "Whom do you wish to channel this weave upon?\r\n");
						return;
					}
				}

				switch(number(1, 3))
				{
				case 1:
					send_to_char("You feel as if someone else inside your head wants out!\r\n", ch);
					act("$n clutches $s head, muttering about the voices!", FALSE, ch, nullptr, nullptr, TO_ROOM);
					break;

				case 2:
					send_to_char("You mutter 'Must.... Kill....'\r\n", ch);
					act("$n mutters 'Must.... Kill....'", FALSE, ch, nullptr, nullptr, TO_ROOM);
					break;

				case 3:
					send_to_char("You struggle to regain control of Saidin!\r\n", ch);
					act("$n looks as if $e is fighting for control of $s body!", TRUE, ch, nullptr, nullptr, TO_ROOM);
					break;
				}

				if (GET_POS(ch) >= SINFO.min_position && CHECK_WAIT(ch) <= 4 RL_SEC)
					WAIT_STATE(ch, PULSE_VIOLENCE);
			}
		}
	}
	
	else
	{
		if (t != nullptr)
		{
			skip_spaces(&t);
			strcpy(text, t);
			one_argument(strcpy(arg, t), t);
			skip_spaces(&t);
		}

		if (IS_SET(SINFO.targets, TAR_IGNORE))
		{
			target = TRUE;
		} 
		
		else if (t != nullptr && *t)
		{
			if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM)))
			{
				if ((tch = get_char_room_vis(ch, t)) != nullptr)
					target = TRUE;
			}
    
			if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
				if ((tch = get_char_vis(ch, t)))
					target = TRUE;

    
				if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
					if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
						target = TRUE;

					if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP))
					{
						for (i = 0; !target && i < NUM_WEARS; i++)
							if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name))
							{
								tobj = GET_EQ(ch, i);
								target = TRUE;
							}
					}
    
					if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
						if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
							target = TRUE;

					if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
						if ((tobj = get_obj_vis(ch, t)))
							target = TRUE;
		
		} 
  		
		else
		{			/* if target string is empty */
			if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
				if (FIGHTING(ch) != nullptr)
				{
					tch = ch;
					target = TRUE;
				}
		
				if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
					if (FIGHTING(ch) != nullptr)
					{
						tch = FIGHTING(ch);
						target = TRUE;
					}
    
					/* if no target specified, and the spell isn't violent, default to self */
					if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
					!SINFO.violent)
					{
						tch = ch;
						target = TRUE;
					}
		 
					if (!target)
					{
						sprintf(buf, "Upon %s should the spell be cast?\r\n",
						IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
						"what" : "who");
						send_to_char(buf, ch);
						return;
					}	
		
					if (target && (tch == ch) && SINFO.violent)
					{
						send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
						return;
					}
		
					if (!target)
					{
						send_to_char("Cannot find the target of your spell!\r\n", ch);
						return;
					}
		}

		/* a change of targets */
		if (GET_TAINT(ch) >= 85 && number(1, 300) <= GET_TAINT(ch))
			tch = randomTarget(ch);


		if(!target)
		{
			message(ch, "Whom do you wish to channel this weave apon?\r\n");
			return;
		}

		if(ch->desc && ch->desc->command_ready)
			tch = GET_TARGET(ch);

		if(!room_visibility(ch, tch) && ch->desc && ch->desc->command_ready && tch)
		{ //Checks to see if ch can see vict in ch's room //
			send_to_char("They must have left already...\r\n", ch);
			return;
		}

		if(ch->desc)
		{
			if(!ch->desc->command_ready)
			{
				send_to_char("You begin to weave the appropriate flows.\r\n", ch);
				ch->desc->timer = SINFO.timer;
				GET_TARGET(ch) = tch;

				/* The command started successfully.  If MC add taint.  -Serai */
				/* The longer they channel (spell timer), the more taint. */
				if (GET_SEX(ch) == SEX_MALE && IS_CHANNELER(ch) && GET_TAINT(ch) < TAINT_MAX)
				{

					int strain = 0;

					//Add strain dependant on mana cost and spell timer.
					strain += (int) (SINFO.timer  / 2) + (SINFO.mana_max / 10);
					strain += (int) (GET_STRAIN(ch) * .10);
					strain += ((GET_MANA(ch) > GET_MAX_MANA(ch) / 3) ? 0 : (GET_MAX_MANA(ch) / 3) - GET_MANA(ch));


					if(GET_COND(ch, FULL) <= 4 || GET_COND(ch, THIRST) <= 4)
						strain = (int) (strain * .03);

					if(GET_COND(ch, DRUNK) >= 12)
						strain += GET_COND(ch, DRUNK) - 11;

					if(ch->getClan(CLAN_BLACK_TOWER))
						strain = (int) (strain * .75);

					GET_STRAIN(ch) = MIN(GET_STRAIN(ch), 300);

				}
			}
		}

		if(ch->desc)
			ch->desc->command_ready = 1;
	} /* end if(tainted)-else */

	if((ch->desc && ch->desc->timer <= 0) || tainted)
		perform_cast(ch, tch, target, spellnum, tobj, text);
}

void spell_level(int spell, int chclass, int level)
{
	int bad = 0;

	if (spell < 0 || spell > TOP_SPELL_DEFINE)
	{
		log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
		return;
	}

	if (chclass < 0 || chclass >= NUM_CLASSES)
	{
		log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(spell),
		chclass, NUM_CLASSES - 1);
		bad = 1;
	}

	if (level < 1 || level > LVL_IMPL)
	{
		log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),
		level, LVL_APPR);
		bad = 1;
	}

	if (!bad)    
		spell_info[spell].min_level = level;
}

/* Assign the spells on boot up */
void spello(int spl, int max_mana, int min_mana, int mana_change, int minpos,
	         int targets, int violent, int routines, int class_type, float timer)
{

	if(class_type == -1)
		spell_info[spl].class_type = CLASS_CHANNELER;

	else
		spell_info[spl].class_type = class_type;

	spell_info[spl].min_level = 1;
	spell_info[spl].timer = timer;
	spell_info[spl].mana_max = max_mana;
	spell_info[spl].mana_min = min_mana;
	spell_info[spl].mana_change = mana_change;
	spell_info[spl].min_position = minpos;
	spell_info[spl].targets = targets;
	spell_info[spl].violent = violent;
	spell_info[spl].routines = routines;
}


void unused_spell(int spl)
{

	spell_info[spl].min_level = LVL_IMPL + 1;
	spell_info[spl].timer = 0;
	spell_info[spl].mana_max = 0;
	spell_info[spl].mana_min = 0;
	spell_info[spl].mana_change = 0;
	spell_info[spl].min_position = 0;
	spell_info[spl].targets = 0;
	spell_info[spl].violent = 0;
	spell_info[spl].routines = 0;
}

#define skillo(skill, class_type) spello(skill, 0, 0, 0, 0, 0, 0, 0, class_type, 0);


/*
 * Arguments for spello calls:
 *
 * spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
 *
 * spellnum:  Number of the spell.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).
 *
 * maxmana :  The maximum mana this spell will take (i.e., the mana it
 * will take when the player first gets the spell).
 *
 * minmana :  The minimum mana this spell will take, no matter how high
 * level the caster is.
 *
 * manachng:  The change in mana for the spell from level to level.  This
 * number should be positive, but represents the reduction in mana cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

/*
 * NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
 * In order to make this cleaner, as well as to make adding new classes
 * much easier, spell levels are now assigned in class.c.  You only need
 * a spello() call to define a new spell; to decide who gets to use a spell
 * or skill, look in class.c.  -JE 5 Feb 1996
 */

void mag_assign_spells(void)
{
	int i;

	/* Do not change the loop below */
	for (i = 1; i <= TOP_SPELL_DEFINE; i++)
		unused_spell(i);
	/* Do not change the loop above */

	spello(SPELL_AGILITY, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, -1, 6);

	spello(SPELL_BLESS, 35, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS, -1, 4);

	spello(SPELL_BLINDNESS, 35, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS, -1, 4);

	spello(SPELL_CALL_LIGHTNING, 40, 30, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, -1, 6);

	spello(SPELL_CHILL_TOUCH, 30, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS, -1, 4);

	spello(SPELL_CONTROL_WEATHER, 75, 25, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL, -1, 10);

	spello(SPELL_CREATE_FOOD, 30, 5, 4, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS, -1, 3);

	spello(SPELL_CREATE_WATER, 30, 5, 4, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL, -1, 3);

	spello(SPELL_CURE_BLIND, 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS, -1, 2);

	spello(SPELL_CURE_CRITIC, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS, -1, 8);

	spello(SPELL_CURE_LIGHT, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS, -1, 4);

	spello(SPELL_EARTHQUAKE, 40, 25, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS, -1, 6);

	spello(SPELL_ENCHANT_WEAPON, 150, 100, 10, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL, -1, 12);

	spello(SPELL_FIREBALL, 40, 35, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, -1, 5);

	spello(SPELL_LOCATE_OBJECT, 25, 20, 1, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL, -1, 10);

	spello(SPELL_POISON, 50, 20, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS, -1, 6.5);

	spello(SPELL_SANCTUARY, 110, 85, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, -1, 8);

	spello(SPELL_SLEEP, 40, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS, -1, 6);

	spello(SPELL_STRENGTH, 35, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, -1, 6);

	spello(SPELL_TELEPORT, 70, 70, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL, -1, 15);

	spello(SPELL_REMOVE_POISON, 40, 8, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS, -1, 6);
	
	spello(SPELL_REGEN, 60, 45, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL, -1, 6);

	spello(SPELL_AIR_SCYTHE, 35, 25, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, -1, 3);

	spello(SPELL_NIGHT_VISION, 50, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, -1, 5.5);

	spello(SPELL_SHIELD, 45, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS, -1, 6);

	spello(SPELL_HASTE, 60, 55, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS, -1, 8);

	spello(SPELL_LOCATE_LIFE, 75, 50, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL, -1, 15);

	spello(SPELL_TORNADO, 40, 30, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL, -1, 7);

	spello(SPELL_RESTORE, 90, 60, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL, -1, 20);

	spello(SPELL_BALEFIRE, 212, 190, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE, -1, 7);

	spello(SPELL_GATE, 170, 170, 3, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_MANUAL, -1, 15);

	spello(SPELL_BOND, 1, 1, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL, -1, 20);
  
  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

	// Warriors
	skillo(SKILL_BASH,			CLASS_WARRIOR);
	skillo(SKILL_RESCUE,		CLASS_WARRIOR);
	skillo(SKILL_RIDE,			CLASS_WARRIOR);
	skillo(SKILL_PARRY,			CLASS_WARRIOR);
	skillo(SKILL_CLUB,			CLASS_WARRIOR);
	skillo(SKILL_AXE,			CLASS_WARRIOR);
	skillo(SKILL_LONG_BLADE,	CLASS_WARRIOR);
	skillo(SKILL_SPEAR,			CLASS_WARRIOR);
	skillo(SKILL_CHAIN,			CLASS_WARRIOR);
	skillo(SKILL_CHARGE,		CLASS_WARRIOR);
	skillo(SKILL_SKEWER,		CLASS_WARRIOR);
	skillo(SKILL_LANCE,			CLASS_WARRIOR);
	
	//Thieves
	skillo(SKILL_SNEAK,			CLASS_THIEF);
	skillo(SKILL_STEAL,			CLASS_THIEF);
	skillo(SKILL_PICK_LOCK,		CLASS_THIEF);
	skillo(SKILL_HIDE,			CLASS_THIEF);
	skillo(SKILL_BACKSTAB,		CLASS_THIEF);
	skillo(SKILL_DODGE,			CLASS_THIEF);
	skillo(SKILL_ATTACK,		CLASS_THIEF);
	skillo(SKILL_SHORT_BLADE,	CLASS_THIEF);

	//Rangers
	skillo(SKILL_TRACK,			CLASS_RANGER);
	skillo(SKILL_KICK,			CLASS_RANGER);
	skillo(SKILL_NOTICE,		CLASS_RANGER);
	skillo(SKILL_SEARCH,		CLASS_RANGER);
	skillo(SKILL_STAFF,			CLASS_RANGER);
	skillo(SKILL_VITALITY,		CLASS_RANGER);
	skillo(SKILL_BOW,			CLASS_RANGER);

	/* Other */

	//Fades
	skillo(SKILL_FADE,			CLASS_FADE);

	//Greymen
	skillo(SKILL_EFFUSION,		CLASS_GREYMAN);
}

