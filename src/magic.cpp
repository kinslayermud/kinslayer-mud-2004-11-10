/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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
#include "db.h"
#include "dg_scripts.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern char_data *character_list;
extern struct index_data *obj_index;

extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;
extern char *spell_wear_off_msg[];
extern struct default_mobile_stats *mob_defaults;
extern struct apply_mod_defaults *apmd;

cbyte saving_throws(int class_num, int type, int level); /* class.c */
void clearMemory(char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(char_data * ch, char_data * leader);
extern struct spell_info_type spell_info[];

/* local functions */
int mag_materials(char_data * ch, int item0, int item1, int item2, int extract, int verbose);
int mag_savingthrow(char_data * ch, int type);
void affect_update();

/*
 * Saving throws are now in class.c (bpl13)
 */

int mag_savingthrow(char_data * ch, int type)
{
	int save;

	save = GET_LEVEL(ch);

	/* throwing a 0 is always a failure */
	if (MAX(1, save) < number(0, 99))
		return TRUE;
  
	else
		return FALSE;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update()
{
	struct affected_type *af, *next;
	char_data *i;

	for (i = character_list; i; i = i->next)
	{

		for (af = i->affected; af; af = next)
		{
			next = af->next;
      
			if (af->duration >= 1)
				af->duration--;
      
			else if (af->duration == -1)	/* No action */
				af->duration = -1;	/* GODs only! unlimited */
      
			else
			{
				if ((af->type > 0) && (af->type <= MAX_SPELLS))
					if (!af->next || (af->next->type != af->type) ||
					(af->next->duration > 0))
						if (*spell_wear_off_msg[af->type])
						{
							send_to_char(spell_wear_off_msg[af->type], i);
							send_to_char("\r\n", i);
						}
	
						affect_remove(i, af);
			}
		}
	}
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
	struct obj_data *tobj;
	struct obj_data *obj0 = nullptr, *obj1 = nullptr, *obj2 = nullptr;

	for (tobj = ch->carrying; tobj; tobj = tobj->next_content)
	{
		if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0))
		{
			obj0 = tobj;
			item0 = -1;
		} 
		
		else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1))
		{
			obj1 = tobj;
			item1 = -1;
		} 
		
		else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2))
		{
			obj2 = tobj;
			item2 = -1;
		}
	}
  
	if ((item0 > 0) || (item1 > 0) || (item2 > 0))
	{
		if (verbose)
		{
      
			switch (number(0, 2))
			{
      
			case 0:
				send_to_char("A wart sprouts on your nose.\r\n", ch);
				break;
      
			case 1:
				send_to_char("Your hair falls out in clumps.\r\n", ch);
				break;
      
			case 2:
				send_to_char("A huge corn develops on your big toe.\r\n", ch);
				break;
			}
		}
    
		return (FALSE);
	}
  
	if (extract)
	{
		if (item0 < 0)
		{
			obj_from_char(obj0);
			extract_obj(obj0);
		}
    
		if (item1 < 0)
		{
			obj_from_char(obj1);
			extract_obj(obj1);
		}
    
		if (item2 < 0)
		{
			obj_from_char(obj2);
			extract_obj(obj2);
		}
	}
  
	if (verbose)
	{
		send_to_char("A puff of smoke rises from your pack.\r\n", ch);
		act("A puff of smoke rises from $n's pack.", TRUE, ch, nullptr, nullptr, TO_ROOM);
	}
  
	return (TRUE);
}




/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 *
 * -1 = dead, otherwise the amount of damage done.
 */
int mag_damage(int level, char_data *ch, char_data *victim, int spellnum, int savetype)
{
	int dam = 0;
	struct obj_data *angreal;

	if (victim == nullptr || ch == nullptr)
		return 0;

	switch (spellnum)
	{
		/* Mostly mages */
  
	case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
		
		if (IS_DREADLORD(ch))
			dam = dice(1, 8) + 3;
    
		else
			dam = dice(1, 6) + 3;

		if(ch->AES_SEDAI())
		{
			dam = (int) (dam * 1.5);
		}
    
		break;
  
	case SPELL_FIREBALL:
    
		if (IS_DREADLORD(ch))
			dam = number(60, 110) + 10;
    
		else
			dam = number(50, 105) + 10;
		
		break;

	case SPELL_BALEFIRE:
    
		dam = number(250, 300) + 10;
		break;
  
	case SPELL_CALL_LIGHTNING:
    
		dam = dice(15, 9) + 7;
		break;

	case SPELL_EARTHQUAKE:
		
		dam = dice(6, 8) + level;
		break;

	case SPELL_AIR_SCYTHE:

		dam = number(35, 55);

		if(ch->AES_SEDAI())
		{
			dam = (int) (dam * 1.5);
		}

		break;

	}

	if ( (angreal = GET_EQ(ch, WEAR_HOLD)) != nullptr)
	{
		if(GET_OBJ_TYPE(angreal) == ITEM_ANGREAL && GET_OBJ_VAL(angreal, 2) != 0)
		{
			/* Turn this into a percent. */
			dam = (int) ((float) dam * ((float) ((float)GET_OBJ_VAL(angreal, 1) / 100)));
		}
	}

	/* and finally, inflict the damage */
	return damage(ch, victim, dam, spellnum);
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 5	/* change if more needed */

void mag_affects(int level, char_data * ch, char_data * victim,
		      int spellnum, int savetype)
{
	struct affected_type af[MAX_SPELL_AFFECTS];
	struct affect_type_not_saved *al;
	char_data *temp;
	bool accum_affect = FALSE, accum_duration = FALSE;
	const char *to_vict = nullptr, *to_room = nullptr;
	int i;


	if (victim == nullptr || ch == nullptr)
		return;

	for (i = 0; i < MAX_SPELL_AFFECTS; i++)
	{
		af[i].type = spellnum;
		af[i].bitvector = 0;
		af[i].modifier = 0;
		af[i].location = APPLY_NONE;
	}

	switch (spellnum)
	{

  
	case SPELL_CHILL_TOUCH:
		
		af[0].location = APPLY_STR;
		af[0].duration = 4;
    
		af[0].modifier = -1;
		accum_duration = TRUE;
		to_vict = "You feel your strength wither!";
		break;

	case SPELL_AGILITY:
		
		af[0].location = APPLY_STR;
		af[0].duration = GET_LEVEL(victim);
		af[0].modifier = 0;
		af[0].bitvector = AFF_AGILITY;
		to_vict = "You feel your body's agility increase.";
		to_room = "$n appears to be ready to move more quickly now.";
		break;



	case SPELL_BLESS:
    
		af[0].location = APPLY_HITROLL;
		af[0].modifier = 2;
		af[0].duration = 6;

		af[1].location = APPLY_SAVING_SPELL;
		af[1].modifier = -1;
		af[1].duration = 6;

		accum_duration = TRUE;
		to_vict = "You feel righteous.";
		break;

	case SPELL_BLINDNESS:
    
		if (MOB_FLAGGED(victim,MOB_NOBLIND) || mag_savingthrow(victim, savetype))
		{
			send_to_char("You fail.\r\n", ch);
			return;
		}

		af[0].location = APPLY_HITROLL;
		af[0].modifier = -4;
		af[0].duration = 2;
		af[0].bitvector = AFF_BLIND;

		to_room = "$n seems to be blinded!";
		to_vict = "You have been blinded!";
		break;

	case SPELL_POISON:
    
		if (mag_savingthrow(victim, savetype))
		{
			send_to_char(NOEFFECT, ch);
			return;
		}

		af[0].location = APPLY_STR;
		af[0].duration = 2;
		af[0].modifier = -2;
		af[0].bitvector = AFF_POISON;
		to_vict = "You feel very sick.";
		to_room = "$n gets violently ill!";
		break;

	case SPELL_HASTE:

		af[0].duration =	5;
		af[0].modifier =	0;
		af[0].location =	APPLY_NONE;
		af[0].bitvector =	AFF_HASTE;
		to_vict = "You feel your ability to fight become faster.";
		to_room = "$n looks ready to fight with haste.";
		break;

	case SPELL_SHIELD:

		if(!IS_CHANNELER(victim))
		{
			message(ch, "You fail.\r\n");
			return;
		}

		af[0].duration =	5;
		af[0].modifier =	0;
		af[0].location =	APPLY_NONE;
		af[0].bitvector =	AFF_SHIELD;
		to_vict = "You lose your ability to connect to the True Source!";
		act("$N has been shielded from the True Source.", FALSE, ch, 0, victim, TO_CHAR);
		break;

	case SPELL_NIGHT_VISION:

		af[0].duration =	GET_SKILL(ch, SPELL_NIGHT_VISION) / 5;
		af[0].modifier =	0;
		af[0].location =	APPLY_NONE;
		af[0].bitvector =	AFF_NIGHT_VISION;
		to_vict = "Your eyes begin to shine with a bright golden color.";
		to_room = "$n's eyes begin to shine with a bright golden color.";
		break;

	case SPELL_SANCTUARY:
    
		af[0].duration = 2;
		af[0].bitvector = AFF_SANCTUARY;

		accum_duration = TRUE;
		to_vict = "A white aura momentarily surrounds you.";
		to_room = "$n is surrounded by a white aura.";
		break;

	case SPELL_SLEEP:

		if (GET_LEVEL(victim) >= LVL_IMMORT)
		{
			temp = victim;
			victim = ch;
			ch = temp;

			act("You ward off an attempt to put you to sleep!", FALSE, ch, nullptr, nullptr, TO_CHAR);
			act("$n rebounds the attempt to put $s asleep!", TRUE, ch, nullptr, nullptr, TO_ROOM);
		}
    
		if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(victim)) 
			return;
    
		if (MOB_FLAGGED(victim, MOB_NOSLEEP))
			return;
    
		if (mag_savingthrow(victim, savetype))
			return;

		af[0].duration = 4 + (GET_LEVEL(ch) / 4);
		af[0].bitvector = AFF_SLEEP;

		if (GET_POS(victim) > POS_SLEEPING)
		{
			act("You feel very sleepy...  Zzzz......", FALSE, victim, nullptr, nullptr, TO_CHAR);
			act("$n goes to sleep.", TRUE, victim, nullptr, nullptr, TO_ROOM);
			change_pos(victim, POS_SLEEPING);
		}
    
		break;

	case SPELL_STRENGTH:
	  
		if(victim == ch)
		{
			to_vict = "You cannot weave this on your self!";
		}

		else
		{
			af[0].location = APPLY_STR;
			af[0].duration = (GET_LEVEL(ch) / 2) + 4;
			af[0].modifier = 1;
			af[0].bitvector = AFF_STRENGTH;
			accum_duration = TRUE;
			accum_affect = TRUE;
			to_vict = "You feel stronger!";
			break;
		}
	}

	/*
	 * If this is a mob that has this affect set in its mob file, do not
	 * perform the affect.  This prevents people from un-sancting mobs
	 * by sancting them and waiting for it to fade, for example.
	 */
  
	 if (IS_NPC(victim) && !affected_by_spell(victim, spellnum))
		for (i = 0; i < MAX_SPELL_AFFECTS; i++)
			if (AFF_FLAGGED(victim, af[i].bitvector))
			{
				send_to_char(NOEFFECT, ch);
				return;
			}

	/*
	 * If the victim is already affected by this spell, and the spell does
	 * not have an accumulative effect, then fail the spell.
	 */
  
	if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect))
	{
		send_to_char(NOEFFECT, ch);
		return;
	}

  
	for (i = 0; i < MAX_SPELL_AFFECTS; i++)
	{
		if (af[i].bitvector || (af[i].location != APPLY_NONE))
		{
			affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);
		}
	}

 
	/* Add the non-saving affection to the non-saving affection list. */
	al = new affect_type_not_saved;
	al->caster = ch;
	al->affect = af;
	add_affection_to_list(victim, al);

	/* Now attach the weave to the caster. */
	attach_weave(ch, victim, victim->affected);

	/* Send the messages for the spell. */
	if (to_vict != nullptr)
		act(to_vict, FALSE, victim, nullptr, ch, TO_CHAR);
  
	if (to_room != nullptr)
		act(to_room, TRUE, victim, nullptr, ch, TO_ROOM);
}

/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, char_data * ch, int spellnum, int savetype)
{
	char_data *k;

	if (ch == nullptr)
		return;

	if (!AFF_FLAGGED(ch, AFF_GROUP))
		return;
  
	if (ch->master != nullptr)
		k = ch->master;
  
	else
		k = ch;
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, char_data * ch, int spellnum, int savetype)
{
	char_data *tch, *tch_next;

	for (tch = world[ch->in_room].people; tch; tch = tch_next)
	{
		tch_next = tch->next_in_room;
    
		if (tch == ch)
			continue;

	}
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(int level, char_data * ch, int spellnum, int savetype)
{
	char_data *tch, *next_tch;
	const char *to_char = nullptr, *to_room = nullptr;

	if (ch == nullptr)
		return;

	/*
	 * to add spells to this fn, just add the message here plus an entry
	 * in mag_damage for the damaging part of the spell.
	 */
  
	switch (spellnum)
	{
  
	case SPELL_EARTHQUAKE:
		to_char = "You gesture and the earth begins to shake all around you!";
		to_room ="$n gracefully gestures and the earth begins to shake violently!";
		break;
	}

	if (to_char != nullptr)
		act(to_char, FALSE, ch, nullptr, nullptr, TO_CHAR);
	
	if (to_room != nullptr)
		act(to_room, FALSE, ch, nullptr, nullptr, TO_ROOM);
  

	for (tch = world[ch->in_room].people; tch; tch = next_tch)
	{
		next_tch = tch->next_in_room;

		/*
		 * The skips: 1: the caster
		 *            2: immortals
		 *            3: if no pk on this mud, skips over all players
		 *            4: pets (charmed NPCs)
		 * players can only hit players in CRIMEOK rooms 4) players can only hit
		 * charmed mobs in CRIMEOK rooms
		 */

		if (tch == ch)
			continue;
		
		if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
			continue;
    
		if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch))
			continue;
    

		/* Doesn't matter if they die here so we don't check. -gg 6/24/98 */
		mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
	}
}


/* These mobiles do not exist. */
#define MOB_MONSUM_I		130
#define MOB_MONSUM_II		140
#define MOB_MONSUM_III		150
#define MOB_GATE_I		160
#define MOB_GATE_II		170
#define MOB_GATE_III		180

/* Defined mobiles. */
#define MOB_ELEMENTAL_BASE	20	/* Only one for now. */
#define MOB_CLONE		10
#define MOB_ZOMBIE		11
#define MOB_AERIALSERVANT	19


void mag_summons(int level, char_data * ch, struct obj_data * obj,
		      int spellnum, int savetype)
{
	return;
}


void mag_points(int level, char_data * ch, char_data * victim,
		     int spellnum, int savetype)
{
	int hit = 0;
	int move = 0;

	if (victim == nullptr)
		return;

	if(victim == ch)
		return;

	switch (spellnum)
	{
  
	case SPELL_CURE_LIGHT:
		hit = dice(1, 8) + 1 + (level / 4);
		send_to_char("You feel better.\r\n", victim);
		break;
  
	case SPELL_CURE_CRITIC:
		hit = dice(3, 8) + 3 + (level / 4);
		send_to_char("You feel a lot better!\r\n", victim);
		break;
	}
  
	GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
	GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
	update_pos(victim);
}


void mag_unaffects(int level, char_data * ch, char_data * victim,
		        int spellnum, int type)
{
	int spell = 0;
	const char *to_vict = nullptr, *to_room = nullptr;

	if (victim == nullptr)
		return;

	switch (spellnum)
	{

		case SPELL_CURE_BLIND:
			spell = SPELL_BLINDNESS;
			to_vict = "Your vision returns!";
			to_room = "There's a momentary gleam in $n's eyes.";
			break;

		default:
			log("SYSERR: unknown spellnum %d passed to mag_unaffects.", spellnum);
			return;
	}
  
	if (!affected_by_spell(victim, spell))
	{
		send_to_char(NOEFFECT, ch);
		return;
	}

	affect_from_char(victim, spell, 0);
  
	if (to_vict != nullptr)
		act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  
	if (to_room != nullptr)
		act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, char_data * ch, struct obj_data * obj,
		         int spellnum, int savetype)
{
	const char *to_char = nullptr, *to_room = nullptr;

	if (obj == nullptr)
		return;

	switch (spellnum)
	{
    
	case SPELL_BLESS:
		if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
		(object_weight(obj) <= 5 * GET_LEVEL(ch)))
		{
			SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BLESS);
			to_char = "$p glows briefly.";
		}
      
		break;
    
	case SPELL_POISON:
      
		if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
		(GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
		(GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3))
		{
			GET_OBJ_VAL(obj, 3) = 1;
			to_char = "$p steams briefly.";
		}
      
		break;
    
	case SPELL_REMOVE_POISON:
	
		if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
		(GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
		(GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3))
		{
			GET_OBJ_VAL(obj, 3) = 0;
			to_char = "$p steams briefly.";
		}
      
		break;
	}

	if (to_char == nullptr)
		send_to_char(NOEFFECT, ch);
  
	else
		act(to_char, TRUE, ch, obj, 0, TO_CHAR);

	if (to_room != nullptr)
		act(to_room, TRUE, ch, obj, 0, TO_ROOM);
	
	else if (to_char != nullptr)
		act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, char_data * ch, int spellnum)
{
	struct obj_data *tobj;
	int z;

	if (ch == nullptr)
		return;
  
	level = MAX(MIN(level, LVL_IMPL), 1);

	switch (spellnum)
	{
  
	case SPELL_CREATE_FOOD:
		z = 930;
		break;
  
	default:
		send_to_char("Spell unimplemented, it would seem.\r\n", ch);
		return;
	}

	if (!(tobj = read_object(z, VIRTUAL)))
	{
		send_to_char("I seem to have goofed.\r\n", ch);
		log("SYSERR: spell_creations, spell %d, obj %d: obj not found",
		spellnum, z);
		return;
	}

	obj_to_char(tobj, ch);
	act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
	act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
	load_otrigger(tobj);
}

