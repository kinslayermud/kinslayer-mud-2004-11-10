/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __ACT_OTHER_C__

#include "conf.h"
#include "sysdep.h"
#include "stdio.h"

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "house.h"
#include "dg_scripts.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct memory_data *mlist;
extern const struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char *class_abbrevs[];
extern int free_rent;
extern int pt_allowed;
extern int max_filesize;
extern int nameserver_is_slow;
extern int top_of_world;
extern int auto_save;
extern class Ideas *idea_list;
extern struct index_data *obj_index;

extern FILE *player_fl;

/* extern procedures */
void list_skills(char_data * ch, char_data *teacher);
void appear(char_data * ch);
void perform_immort_vis(char_data *ch);
SPECIAL(shop_keeper);
ACMD(do_gen_comm);
void die(char_data * ch, char_data * killer);
void Crash_rentsave(char_data * ch, int cost);
void write_aliases(char_data *ch);
void add_to_watch(char_data *ch);

char *mood_type(int mood);

//Declarations //
ACMD(do_say);

/* local functions */
ACMD(do_quit);
ACMD(do_save);
ACMD(do_not_here);
ACMD(do_sneak);
ACMD(do_hide);
ACMD(do_steal);
ACMD(do_effuse);
ACMD(do_practice);
ACMD(do_visible);
int perform_group(char_data *ch, char_data *vict);
void print_group(char_data *ch);
ACMD(do_group);
ACMD(do_ungroup);
ACMD(do_split);
ACMD(do_use);
ACMD(do_mark);
ACMD(do_notice);
ACMD(do_ignore);
ACMD(do_wimpy);
ACMD(do_display);
ACMD(do_gen_write);
ACMD(do_gen_tog);
ACMD(do_scalp);
ACMD(do_follow);
ACMD(do_self_delete);


void perform_steal(char_data *ch, char_data *vict, struct obj_data *obj, char *obj_name)
{
	
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;	
	
	
	/* 101% is a complete failure */
	percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

	if (GET_POS(vict) < POS_SLEEPING)
		percent = -1;		/* ALWAYS SUCCESS */

	if (!pt_allowed && !IS_NPC(vict))
		pcsteal = 1;

	if(IS_NPC(vict) && MOB_FLAGGED(vict, MOB_AWARD))
	{
		do_say(vict, "You'll pay for that!", 0, 0);
		hit(vict, ch, TYPE_UNDEFINED);
		return;
	}

	if(FIGHTING(vict))
	{
		message(ch, "%s is fighting! He'll surely notice you trying that!\r\n", GET_NAME(vict));
		return;
	}

	/* NO NO With Imp's and Shopkeepers, and if player thieving is not allowed */
	if (GET_LEVEL(vict) >= LVL_IMMORT || pcsteal ||
	GET_MOB_SPEC(vict) == shop_keeper)
		percent = 101;		/* Failure */

	if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold"))
	{

		if (!(obj = get_obj_in_list_vis(vict, obj_name, vict->carrying)))
		{
			for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
				
				if (GET_EQ(vict, eq_pos) &&
				(isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
				CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos)))
				{
					
					obj = GET_EQ(vict, eq_pos);
					break;
				}
      
				if (!obj)
				{
					act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR);
					return;
				} 
				
				else
				{			/* It is equipment */
					
					if ((GET_POS(vict) > POS_STUNNED))
					{
						send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
						return;
					} 
					
					else
					{
						act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
						act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
						obj_to_char(unequip_char(vict, eq_pos), ch);
					}
				}
			} 
		
		else
		{			/* obj found in inventory */

			percent += object_weight(obj);	/* Make heavy harder */

			if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL)))
			{
				ohoh = TRUE;
				act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
				act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
				act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
			} 
			
			else
			{			/* Steal the item */
				if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
				{
					if ((carrying_weight(ch) + object_weight(obj)) < CAN_CARRY_W(ch))
					{
						obj_from_char(obj);
						obj_to_char(obj, ch);
						send_to_char("Got it!\r\n", ch);
					}
				} 
				
				else
					send_to_char("You cannot carry that much.\r\n", ch);
			}
		}
	} 
	
	else
	{			/* Steal some coins */
 
		if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL)))
		{
			ohoh = TRUE;
			act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
			act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
			act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
		} 
		
		else
		{
			/* Steal some gold coins */
			gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
			gold = MIN(1782, gold);
      
			if (gold > 0)
			{
				GET_GOLD(ch) += gold;
				GET_GOLD(vict) -= gold;
        
				if (gold > 1)
				{
					sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
					send_to_char(buf, ch);
				} 
				
				else
					send_to_char("You manage to swipe a solitary gold coin.\r\n", ch);
			} 
			
			else
			{
				send_to_char("You couldn't get any gold...\r\n", ch);
			}
		}
	}

	if (ohoh && IS_NPC(vict) && AWAKE(vict))
		hit(vict, ch, TYPE_UNDEFINED);
}

ACMD(do_ignore)
{
	struct ignore_data *i, *temp;
	char arg[MAX_INPUT_LENGTH];
	int count = 0;

	one_argument(argument, arg);

	if(!argument || !*arg)
	{

		send_to_char("You are ignoring the following people:\r\n", ch);

		for(i = ch->ignores, count = 0;i;i = i->next, count++)
		{

			if(count)
				send_to_char(", ", ch);

			if(!(count % 5))
				send_to_char("\r\n", ch);

			message(ch, "%s", i->name);
		}

		return;

	}

	arg[0] = UPPER(arg[0]);

	if(!playerExists(arg))
	{
		send_to_char("There is no such player\r\n", ch);
		return;
	}

	for(i = ch->ignores;i;i = i->next)
	{
		if(!str_cmp(i->name, arg))
		{
			send_to_char("You will no longer ignore that one.\r\n", ch);
			sprintf(buf, "%s is no longer ignoring %s.", GET_NAME(ch), arg);
			mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
			REMOVE_FROM_LIST(i, ch->ignores, next);
			delete(i);
			save_ignores(ch);
			return;
		}
	}

	i = new ignore_data;
	strcpy(i->name, arg);
	i->next = ch->ignores;
	ch->ignores = i;

	save_ignores(ch);

	sprintf(buf, "%s is now ignoring %s.", GET_NAME(ch), arg);
	mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
	send_to_char("You will now ignore that person.\r\n", ch);
}


ACMD(do_self_delete)
{
	if(!PLR_FLAGGED(ch, PLR_DELETED))
	{
		if(GET_LEVEL(ch) > 30)
		{
			send_to_char("You must be level 30 or below in order to self delete.\r\n", ch);
			return;
		}

		send_to_char("Your delete flag is now set.\r\n"
		"Warning: Once you re-create this character, this cannot be un-done.\r\n", ch);
		SET_BIT_AR(PLR_FLAGS(ch), PLR_DELETED);
	}

	else
	{
		send_to_char("Your delete flag has been removed.\r\n", ch);
		REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_DELETED);
	}
}

ACMD(do_mark)
{

	char_data *victim;
	char person[MAX_INPUT_LENGTH];

	one_argument(argument, person);

	if(!IS_GREYMAN(ch))
	{
		send_to_char("You don't seem to have that kind of power...\r\n", ch);
		return;
	}

	if(!*argument && !*person)
	{
		send_to_char("Who do you wish to mark?\r\n", ch);
		return;
	}

	if(!(victim = get_char_room_vis(ch, person)))
	{
		send_to_char("There is no one by that name here.\r\n", ch);
		return;
	}

	if(GET_MARKED(ch))
		message(ch, "You release your focus on %s's location.\r\n", GET_NAME(GET_MARKED(ch)));

	message(ch, "You now focus on %s's location.\r\n", GET_NAME(victim));
	GET_MARKED(ch) = victim;
}

ACMD(do_effuse)
{

	struct affected_type af;

	if(!GET_SKILL(ch, SKILL_EFFUSION))
	{
		send_to_char("You do not know that skill.\r\n", ch);
		return;
	}

	if(!AFF_FLAGGED(ch, AFF_EFFUSION))
		send_to_char("You will now leave unseen.\r\n", ch);

	else if(AFF_FLAGGED(ch, AFF_EFFUSION))
	{
		send_to_char("You will stop trying to sneak away unnoticed\n\r", ch);
		affect_from_char(ch, 0, AFF_EFFUSION);
		return;
	}

	af.type = SKILL_EFFUSION;
	af.duration = 30;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_EFFUSION;
	affect_to_char(ch, &af);
}

/* Added by Tulon in October 2003. Command sets characters "notice" AFFECTION bit. */
ACMD(do_notice)
{
	
	struct affected_type af;

	if(!AFF_FLAGGED(ch, AFF_NOTICE))
		send_to_char("You will now look around more carefully.\r\n", ch);

	else if(AFF_FLAGGED(ch, AFF_NOTICE))
	{
		send_to_char("You look around less carefully.\n\r", ch);
		affect_from_char(ch, 0, AFF_NOTICE);
		return;
	}

	af.type = SKILL_NOTICE;
	af.duration = 20;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_NOTICE;
	affect_to_char(ch, &af);

}

int butcher_cost(char_data *ch)
{

	int skill = GET_SKILL(ch, SKILL_VITALITY);

	return(25 - (skill / 5));
}


/* Added by Tulon in September of 2003. Command gives meat from the corpse to selected player. */
ACMD(do_butcher)
{
	struct obj_data *corpse, *obj, *wielded;

	wielded = GET_EQ(ch, WEAR_WIELD);

	one_argument(argument, arg);

	if (!(corpse = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
	{
		send_to_char("You don't see that here.\r\n", ch);
		return;
	}

	if(!IS_CORPSE(corpse))
	{
		 send_to_char("That isn't a corpse!\r\n", ch);
		 return;
	}


	else
	{
		
		if(wielded && GET_OBJ_VAL(wielded, 3) != 11)
		{
			send_to_char("You need a piercing weapon to butcher.\r\n", ch);
			return;
		}

		if((corpse->food_unit - butcher_cost(ch)) < 0)
		{
			send_to_char("You can't carve any more food from it.\r\n", ch);
			return;
		}

		message(ch, "You lean over and butcher a piece of meat from the corpse of %s.\r\n", corpse->scalp.name);
		obj = read_object(real_object(930), REAL);
		obj_to_char(obj, ch);
		corpse->food_unit -= butcher_cost(ch);
	}
}

/* Added by Tulon in September of 2003. Command gives a scalp of selected corpse to the character. */
ACMD(do_scalp)
{
	struct obj_data *corpse, *scalpo;
	struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
	bool found = FALSE;
	char *type;

	one_argument(argument, arg);

	if(!arg)
	{
		for(corpse = world[ch->in_room].contents;corpse;corpse = corpse->next_content)
		{
			if(IS_CORPSE(corpse) && CAN_SEE_OBJ(ch, corpse))
			{
				found = true;
				break;
			}
		}
	}

	else
	{
		corpse = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents);
		found = true;
	}

	//We found no corpse -- No argument
	if(!found && !arg)
	{
		send_to_char("There is nothing here to scalp\r\n", ch);
		return;
	}

	//No object matched their argument
	if(!corpse)
	{
		send_to_char("You do not see that here\r\n", ch);
		return;
	}

	//Object is not a corpse
	if(!IS_CORPSE(corpse))
	{
		 send_to_char("That isn't a corpse!\r\n", ch);
		 return;  
	}

	//No weapon wielded
	if(!wielded)
	{
		send_to_char("You must use piercing weapons to scalp.\r\n", ch);
		return;
	}

	/* Only daggers can do this */
	if(wielded && GET_OBJ_VAL(wielded, 3) != 11)
	{
		send_to_char("You must use piercing weapons to scalp.\r\n", ch);
		return;
	}

	/* Minimum level for scalping */
	if(corpse->scalp.level < 25)
	{
		send_to_char("Why even bother scalping that puny thing?!\r\n", ch);
		return;
	}

	/* Only do this if it has NOT been scalped yet. */
	if(corpse->scalp.scalped)
	{
		corpse->scalp.scalped = 0;
		scalpo = create_obj();

		scalpo->item_number = NOTHING;
		scalpo->in_room = NOWHERE;
		scalpo->name = str_dup("scalp");

		if(corpse->scalp.is_scalp == 1)
		scalpo->scalp.is_scalp = 1;
		scalpo->scalp.race = corpse->scalp.race;
		scalpo->scalp.level = corpse->scalp.level;

		if(corpse->scalp.level > 40)
			type = "head";

		else
			type = "scalp";

		sprintf(buf2, "The bloody %s of %s.", type, corpse->scalp.name);
	
		scalpo->description = str_dup(buf2);

		sprintf(buf2, "a bloody %s of %s", type, corpse->scalp.name);	
		scalpo->short_description = str_dup(buf2);

		SET_BIT_AR(GET_OBJ_WEAR(scalpo), ITEM_WEAR_TAKE);
//		GET_OBJ_WEIGHT(scalpo) = 2.5;
		GET_OBJ_WEIGHT(scalpo) = 2;

		message(ch, "You sever a bloody %s from the corpse of %s.\r\n", type, corpse->scalp.name);
		sprintf(buf, "%s leans over and severs the bloody %s from the corpse of %s.", GET_NAME(ch), type,
		corpse->scalp.name);
		act(buf, TRUE, ch, 0, 0, TO_ROOM);
		obj_to_char(scalpo, ch);
	}

	else
		send_to_char("It has already been scalped!\r\n", ch);

	return;
}

/* Added by Tulon in September of 2003. Command mounts a character if successful. */
ACMD(do_ride)
{
	char_data *victim;
	char_data *riding;
	int percent;

	one_argument(argument, arg);
	percent = number(1, 50);

	if(!*argument)
	{
		send_to_char("Ride who?\r\n", ch);
		return;
	}

	if(IS_TROLLOC(ch) && !IS_FADE(ch) && !IS_DREADLORD(ch) && !IS_NPC(ch))
	{
		send_to_char("But it looks so delicious... Those 4 legs....\r\n", ch);
		return;
	}

	else
	{
		if(!(victim = get_char_room_vis(ch, arg)))
			send_to_char("They aren't here!\r\n", ch);
		
		else if(victim == ch)
			send_to_char("Yeah, that'd be fun...\r\n", ch);
		
		else if(MOUNT(ch))
			act("You're already riding $N", FALSE, ch, 0, victim, TO_CHAR);
		
		else if(RIDDEN_BY(ch))
			act("You can't ride something else... $N is mounted on you!", FALSE, ch, 0, victim, TO_CHAR);
		
		else if(RIDDEN_BY(victim) && RIDDEN_BY(victim)->in_room == victim->in_room)
		{
			sprintf(buf, "%s is already riding it", GET_NAME(RIDDEN_BY(victim)));
			send_to_char(buf, ch);
		}
		
		else if(!IS_HORSE(victim))
			act("You can't ride $M!", FALSE, ch, 0, victim, TO_CHAR);

		else if( (MOB_FLAGGED(victim, MOB_MOUNT) && IS_TROLLOC(ch)) || (MOB_FLAGGED(victim, MOB_SHADOW_MOUNT) && !IS_TROLLOC(ch)))
			act("$E would not like having your type on $M.", FALSE, ch, 0, victim, TO_CHAR);
		
		else if(ch->getClan(CLAN_WOLFBROTHER))
			send_to_char("You wouldn't even think about getting up on that thing!\r\n", ch);
		
		else if(SECT(ch->in_room) == SECT_INSIDE)
			send_to_char("You can't ride inside!\r\n", ch);

		else if(GET_SKILL(ch, SKILL_RIDE) < percent)
			act("You try to mount $M and fall right off of $S back!", FALSE, ch, 0, victim, TO_CHAR);

		else
		{
			riding = victim;
			MOUNT(ch) = riding;
			RIDDEN_BY(riding) = ch;
			act("You begin riding $N.", FALSE, ch, 0, victim, TO_CHAR);
			act("$n begins riding $N.", FALSE, ch, 0, victim, TO_NOTVICT);
			act("$n begins to ride on your back.", FALSE, ch, 0, victim, TO_VICT);
			do_follow(riding, "self", 0, 0);
			noise_addup(ch->in_room, 1, TYPE_MOUNT);
		}
	}

	return;
}

/* Added by Tulon in September of 2003. Command dismounts a player from their mount. */
ACMD(do_dismount)
{
	if(MOUNT(ch))
	{
		act("You stop riding $N.", FALSE, ch, 0, MOUNT(ch), TO_CHAR);
		act("$n dismounts from you.", FALSE, ch, 0, MOUNT(ch), TO_VICT);
		act("$n stops riding $N.", FALSE, ch, 0, MOUNT(ch), TO_NOTVICT);
		RIDDEN_BY(MOUNT(ch)) = nullptr;
		MOUNT(ch) = nullptr;
		noise_addup(ch->in_room, 1, TYPE_MOUNT);
		return;
	}
	
	else
	{
		send_to_char("You aren't riding anything\r\n", ch);
		return;
	}
	return;
}

ACMD(do_quit)
{
	sh_int save_room;
	struct descriptor_data *d, *next_d;

	if (IS_NPC(ch) || !ch->desc)
		return;
  
	if(AFF_FLAGGED(ch, AFF_NOQUIT)) 
	{
		send_to_char("Can't you feel the speed of your heartbeat?\r\n", ch);
		return;
	}

	if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT)
		send_to_char("You have to type quit--no less, to quit!\r\n", ch);


	else if (GET_POS(ch) == POS_FIGHTING)
		send_to_char("No way!  You're fighting for your life!\r\n", ch);
	
	else if (GET_POS(ch) < POS_STUNNED)
	{
		send_to_char("You die before your time...\r\n", ch);
		die(ch, nullptr);
	}
	
	else
	{
		if (!GET_INVIS_LEV(ch))
			act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    
		sprintf(buf, "%s has quit the game.", GET_NAME(ch));
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
		send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);


    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
      
			if (d == ch->desc)
				continue;
      
			if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
				STATE(d) = CON_DISCONNECT;
		}

		write_aliases(ch);
		add_to_watch(ch);
		save_room = ch->in_room;
   
		//if (free_rent)
		//	Crash_rentsave(ch, 0);

		if(GET_LEVEL(ch) >= LVL_IMMORT)
		{
			send_to_char("Your equipment has been saved.\r\n", ch);
			Crash_crashsave(ch);
		}

		else
			send_to_char("Your equipment has NOT BEEN SAVED\r\n", ch);
    
		extract_char(ch);		/* Char is saved in extract char */
		ch->save();
		Crash_crashsave(ch);
	}
}



ACMD(do_save)
{
	if (IS_NPC(ch) || !ch->desc)
		return;

	/* Only tell the char we're saving if they actually typed "save" */
  
	if (cmd)
	{
		
		/*
		 * This prevents item duplication by two PC's using coordinated saves
		 * (or one PC with a house) and system crashes. Note that houses are
		 * still automatically saved without this enabled.
		 */


		message(ch, "Saving %s.\r\n", GET_NAME(ch));
	}


	write_aliases(ch);
	ch->save();
	Crash_crashsave(ch);

}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}



ACMD(do_sneak)
{
  struct affected_type af;
  cbyte percent;

  if(!AFF_FLAGGED(ch, AFF_SNEAK))
  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);

  else if(AFF_FLAGGED(ch, AFF_SNEAK)) {
  send_to_char("Okay, you will now stop sneaking.\n\r", ch);
  affect_from_char(ch, 0, AFF_SNEAK);
  return;
  }


  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = 25;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}



ACMD(do_hide)
{
	cbyte percent;

	if(MOUNT(ch) != nullptr) {
		send_to_char("You can't do that while riding!\r\n", ch);
		return;
	}

	send_to_char("You attempt to hide yourself.\r\n", ch);

	if (AFF_FLAGGED(ch, AFF_HIDE))
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

	percent = number(1, 101);	/* 101% is a complete failure */

	if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
		return;

	SET_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
}

ACMD(do_steal)
{
	char_data *vict;
	struct obj_data *obj = nullptr;
	char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];

	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
		send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
		return;
	}

	argument = one_argument(argument, obj_name);
	one_argument(argument, vict_name);

	if (!(vict = get_char_room_vis(ch, vict_name))) {
		send_to_char("Steal what from who?\r\n", ch);
		return;
	} 
  
	else if (vict == ch) {
		send_to_char("Come on now, that's rather stupid!\r\n", ch);
		return;
	}


	if(!IS_NPC(ch) || ch->desc)
		ch->desc->command_ready = 1;

	if(ch->desc)
		if(ch->desc->timer <= 0)
			perform_steal(ch, vict, obj, obj_name);

}



ACMD(do_practice)
{
	one_argument(argument, arg);
    list_skills(ch, nullptr);
}



ACMD(do_visible)
{
	if (GET_LEVEL(ch) >= LVL_IMMORT) {
		perform_immort_vis(ch);
		return;
	}

	if AFF_FLAGGED(ch, AFF_INVISIBLE) {
		appear(ch);
		send_to_char("You break the spell of invisibility.\r\n", ch);
	} 
	
	else
		send_to_char("You are already visible.\r\n", ch);
}


int perform_group(char_data *ch, char_data *vict)
{
	if (AFF_FLAGGED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
		return 0;

	SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
  
	if (ch != vict)
		act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  
	act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
	act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
	return 1;
}


void print_group(char_data *ch)
{
	char_data *k;
	struct follow_type *f;

	if (!AFF_FLAGGED(ch, AFF_GROUP))
		send_to_char("But you are not the member of a group!\r\n", ch);
  
	else
	{
		send_to_char("Your group consists of:\r\n", ch);

		k = (ch->master ? ch->master : ch);

		if (AFF_FLAGGED(k, AFF_GROUP))
		{
			sprintf(buf, "     $N (Head of group)");
			act(buf, FALSE, ch, 0, k, TO_CHAR);
		}

		for (f = k->followers; f; f = f->next)
		{
			if (!AFF_FLAGGED(f->follower, AFF_GROUP))
				continue;

			sprintf(buf, "      $N");
			act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
		}
	}
}



ACMD(do_group)
{
	char_data *vict;
	struct follow_type *f;
	int found;

	one_argument(argument, buf);

	if (!*buf)
	{
		print_group(ch);
		return;
	}

	if (ch->master)
	{
		act("You can not enroll group members without being head of a group.",
		FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if (!str_cmp(buf, "all"))
	{
		perform_group(ch, ch);
    
		for (found = 0, f = ch->followers; f; f = f->next)
			found += perform_group(ch, f->follower);
			
		if (!found)
			send_to_char("Everyone following you is already in your group.\r\n", ch);
	return;
	}

	if (!(vict = get_char_room_vis(ch, buf)))
		send_to_char(NOPERSON, ch);
 
	else if ((vict->master != ch) && (vict != ch))
		act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  
	else
	{
		
		if (!AFF_FLAGGED(vict, AFF_GROUP))
			perform_group(ch, vict);
    
		else
		{
			if (ch != vict)
				act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      
			act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
			act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
			REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
		}
	}
}



ACMD(do_ungroup)
{
	struct follow_type *f, *next_fol;
	char_data *tch;

	one_argument(argument, buf);

	if (!*buf)
	{
		if (ch->master || !(AFF_FLAGGED(ch, AFF_GROUP)))
		{
			send_to_char("But you lead no group!\r\n", ch);
			return;
		}

		sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));

		for (f = ch->followers; f; f = next_fol)
		{
			next_fol = f->next;

			if (AFF_FLAGGED(f->follower, AFF_GROUP))
			{
				REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
				send_to_char(buf2, f->follower);

				stop_follower(f->follower);
			}
		}

		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
		send_to_char("You disband the group.\r\n", ch);
		return;
	}

	if (!(tch = get_char_room_vis(ch, buf)))
	{
		send_to_char("There is no such person!\r\n", ch);
		return;
	}

	if (tch->master != ch)
	{
		send_to_char("That person is not following you!\r\n", ch);
		return;
	}

	if (!AFF_FLAGGED(tch, AFF_GROUP))
	{
		send_to_char("That person isn't in your group.\r\n", ch);
		return;
	}

	REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);

	act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
	act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
	act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
}


ACMD(do_split)
{
	int amount, num, share;
	char_data *k;
	struct follow_type *f;

	if (IS_NPC(ch))
		return;

	one_argument(argument, buf);

	if (is_number(buf))
	{
		amount = atoi(buf);

		if (amount <= 0)
		{
			send_to_char("Sorry, you can't do that.\r\n", ch);
			return;
		}

		if (amount > GET_GOLD(ch))
		{
			send_to_char("You don't seem to have that much gold to split.\r\n", ch);
			return;
		}

		k = (ch->master ? ch->master : ch);

		if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room))
			num = 1;

		else
			num = 0;

		for (f = k->followers; f; f = f->next)
			if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
			(!IS_NPC(f->follower)) &&
			(f->follower->in_room == ch->in_room))
				num++;

		if (num && AFF_FLAGGED(ch, AFF_GROUP))
			share = amount / num;

		else
		{
			send_to_char("With whom do you wish to share your gold?\r\n", ch);
			return;
		}

		GET_GOLD(ch) -= share * (num - 1);

		if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room)
		&& !(IS_NPC(k)) && k != ch)
		{
			GET_GOLD(k) += share;
			sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
			amount, share);
			send_to_char(buf, k);
		}

		for (f = k->followers; f; f = f->next)
		{
			if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
			(!IS_NPC(f->follower)) &&
			(f->follower->in_room == ch->in_room) &&
			f->follower != ch)
			{
				GET_GOLD(f->follower) += share;
				sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
				amount, share);
				send_to_char(buf, f->follower);
			}
		}

		sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
		amount, num, share);
		send_to_char(buf, ch);
	}

	else
	{
		send_to_char("How many coins do you wish to split with your group?\r\n", ch);
		return;
	}
}



ACMD(do_use)
{
	struct obj_data *mag_item;
	int equipped = 1;

	half_chop(argument, arg, buf);
  
	if (!*arg)
	{
		sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
		send_to_char(buf2, ch);
		return;
	}

	for(mag_item = ch->carrying;mag_item;mag_item = mag_item->next_content)
	{

		if(GET_OBJ_VNUM(mag_item) == 952 && isname(arg, mag_item->name) && subcmd == SCMD_USE)
		{

			act("$n holds $p into the air and it glows with a huge flash of light.", FALSE, ch, mag_item, nullptr, TO_NOTVICT);
			act("$n digs $p into the ground and backs away from it.", FALSE, ch, mag_item, nullptr, TO_NOTVICT);
			act("You hold $p into the air and smash it into the ground and back away,", FALSE, ch, mag_item, nullptr, TO_CHAR);
			obj_from_char(mag_item);
			obj_to_room(read_object(real_object(953), REAL), ch->in_room);
			return;
		}
	}


  
	mag_item = GET_EQ(ch, WEAR_HOLD);

	if (!mag_item || !isname(arg, mag_item->name))
	{

    
		switch (subcmd)
		{
    
		case SCMD_RECITE:
		case SCMD_QUAFF:
			
			equipped = 0;
      
			if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying)))
			{
				sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
				send_to_char(buf2, ch);
				return;
			}
      
			break;
    
		case SCMD_USE:
			sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
			send_to_char(buf2, ch);
			return;
    
		default:
			log("SYSERR: Unknown subcmd %d passed to do_use.", subcmd);
			return;
		}
	}
  
	switch (subcmd)
	{
  
	case SCMD_QUAFF:
    
		if (GET_OBJ_TYPE(mag_item) != ITEM_POTION)
		{
			send_to_char("You can only quaff potions.", ch);
			return;
		}
    
		break;
  
	case SCMD_RECITE:
    
		if (GET_OBJ_TYPE(mag_item) != ITEM_SCROLL)
		{
			send_to_char("You can only recite scrolls.", ch);
			return;
		}
    
		break;
  
	case SCMD_USE:
    
		if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
		(GET_OBJ_TYPE(mag_item) != ITEM_STAFF))
		{
			send_to_char("You can't seem to figure out how to use it.\r\n", ch);
			return;
		}
    
		break;
	}

	mag_objectmagic(ch, mag_item, buf);
}



ACMD(do_wimpy)
{
	int wimp_lev;

	/* 'wimp_level' is a player_special. -gg 2/25/98 */
	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);

	if (!*arg)
	{
		if (GET_WIMP_LEV(ch))
		{
			sprintf(buf, "Your current wimp level is %d hit points.\r\n",
			GET_WIMP_LEV(ch));
			send_to_char(buf, ch);
			return;
		} 
		
		else
		{
			send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
			return;
		}
	}
  
	if (isdigit(*arg))
	{
		if ((wimp_lev = atoi(arg)))
		{
			if (wimp_lev < 0)
				send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
      
			else if (wimp_lev > GET_MAX_HIT(ch))
				send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      
			else if (wimp_lev > (GET_MAX_HIT(ch) / 2))
				send_to_char("You can't set your wimp level above half your hit points.\r\n", ch);
      
			else
			{
				sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
				wimp_lev);
				send_to_char(buf, ch);
				GET_WIMP_LEV(ch) = wimp_lev;
			}
		} 
		
		else
		{
			send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
			GET_WIMP_LEV(ch) = 0;
		}
	} 
	
	else
		send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);

	return;

}


ACMD(do_display)
{
	size_t i;

	if (IS_NPC(ch))
	{
		send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
		return;
	}
  
	skip_spaces(&argument);

	if (!*argument)
	{
		send_to_char("Usage: prompt { { H | M | V } | all | none }\r\n", ch);
		return;
	}
  
	if ((!str_cmp(argument, "on")) || (!str_cmp(argument, "all")))
	{ 
		SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
		SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
		SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	} 
	
	else 
	{
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);

		for (i = 0; i < strlen(argument); i++)
		{
      
			switch (LOWER(argument[i]))
			{
			
			case 'h':
				SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
				break;
			
			case 'm':
				SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
				break;
      
			case 'v':
				SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
				break;
      
			default:
				send_to_char("Usage: prompt { { H | M | V } | all | none }\r\n", ch);
				return;
				break;
			}
		}
	}

	send_to_char(OK, ch);
}



ACMD(do_gen_write)
{
	FILE *fl;
	char *tmp;
	const char *filename;
	char type[MAX_INPUT_LENGTH], text[MAX_INPUT_LENGTH];
	struct stat fbuf;
	time_t ct;

	ct = time(0);
	tmp = asctime(localtime(&ct));
	tmp += 4;
	
	sprintf(tmp, "%6.6s", tmp);

	switch (subcmd)
	{
  
	case SCMD_BUG:
		filename = BUG_FILE;
		strcpy(type, "Bug");
		break;
  
	case SCMD_TYPO:
		filename = TYPO_FILE;
		strcpy(type, "Typo");
		break;
  
	case SCMD_IDEA:
		class Ideas *idea, *temp;

		idea = new Ideas;

		idea->poster = GET_NAME(ch);
		idea->date = tmp;
		idea->room = GET_ROOM_VNUM(IN_ROOM(ch));
		idea->idea = argument;

		for(temp = idea_list;temp->next;temp = temp->next);

		temp->next = idea;
		idea->next = nullptr;

		filename = IDEA_FILE;
		strcpy(type, "Idea");
		break;
  
	default:
		return;
	}

	if (IS_NPC(ch))
	{
		send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
		return;
	}

	skip_spaces(&argument);
	delete_doubledollar(argument);

	if (!*argument)
	{
		send_to_char("That must be a mistake...\r\n", ch);
		return;
	}

	if (stat(filename, &fbuf) < 0)
	{
		perror("Error statting file");
		return;
	}
  
	if (fbuf.st_size >= max_filesize)
	{
		send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
		return;
	}
  
	if (!(fl = fopen(filename, "a")))
	{
		perror("do_gen_write");
		send_to_char("Could not open the file.  Sorry.\r\n", ch);
		return;
	}
  
	fprintf(fl, "%-8s: (%s) [%5d] %s~\n", GET_NAME(ch), (tmp),
	GET_ROOM_VNUM(IN_ROOM(ch)), argument);
	fclose(fl);
	sprintf(text, "%s %s: %s", GET_NAME(ch), type, argument);
	mudlog(text, NRM, LVL_BLDER, TRUE);
	send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1


ACMD(do_gen_tog)
{
	long result;

	const char *tog_messages[][2] =
	{
		{"Nohassle disabled.\r\n",
		"Nohassle enabled.\r\n"},
		{"Brief mode off.\r\n",
		"Brief mode on.\r\n"},
		{"Compact mode off.\r\n",
		"Compact mode on.\r\n"},
		{"You can now hear tells.\r\n",
		"You are now deaf to tells.\r\n"},
		{"You can now hear narrates.\r\n",
		"You are now deaf to narrates.\r\n"},
		{"You can now hear chats.\r\n",
		"You are now deaf to chats.\r\n"},
		{"You can now hear shouts.\r\n",
		"You are now deaf to shouts.\r\n"},
		{"You can now hear yells.\r\n",
		"You are now deaf to chat yells.\r\n"},
		{"You can now hear the Wiz-channel.\r\n",
		"You are now deaf to the Wiz-channel.\r\n"},
		{"You will no longer see the room flags.\r\n",
		"You will now see the room flags.\r\n"},
		{"You will now have your communication repeated.\r\n",
		"You will no longer have your communication repeated.\r\n"},
		{"HolyLight mode off.\r\n",
		"HolyLight mode on.\r\n"},
		{"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
		"Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
		{"Autoexits disabled.\r\n",
		"Autoexits enabled.\r\n"},
		{"Buildwalk now OFF.\r\n",
		"Buildwalk now ON.\r\n"}
	};


	if (IS_NPC(ch))
		return;

	switch (subcmd)
	{

	case SCMD_NOHASSLE:
		result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
		break;

	case SCMD_BRIEF:
		result = PRF_TOG_CHK(ch, PRF_BRIEF);
		break;

	case SCMD_COMPACT:
		result = PRF_TOG_CHK(ch, PRF_COMPACT);
		break;

	case SCMD_NOTELL:
		result = PRF_TOG_CHK(ch, PRF_NOTELL);
		break;
  
	case SCMD_NONARRATE:
		result = PRF_TOG_CHK(ch, PRF_NONARR);
		break;
  
	case SCMD_NOCHAT:
		result = PRF_TOG_CHK(ch, PRF_NOCHAT);
		break;
  
	case SCMD_NOYELL:
		result = PRF_TOG_CHK(ch, PRF_NOYELL);
		break;

	case SCMD_NOWIZ:
		result = PRF_TOG_CHK(ch, PRF_NOWIZ);
		break;
  
	case SCMD_ROOMFLAGS:
		result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
		break;

	case SCMD_NOREPEAT:
		result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
		break;

	case SCMD_HOLYLIGHT:
		result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
		break;

	case SCMD_SLOWNS:
		result = (nameserver_is_slow = !nameserver_is_slow);
		break;

	case SCMD_AUTOEXIT:
		result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
		break;

	case SCMD_BUILDWALK:
		result = PRF_TOG_CHK(ch, PRF_BUILDWALK);
		break;

	default:
		log("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
		return;
	}

	if (result)
		send_to_char(tog_messages[subcmd][TOG_ON], ch);

	else
		send_to_char(tog_messages[subcmd][TOG_OFF], ch);

	return;
}


ACMD(do_change)
{

  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];


	skip_spaces(&argument);
	delete_doubledollar(argument);
	one_argument(argument, arg1);

	if (!*argument)
	{
		send_to_char("Usage: Title, Mood, Password, Spam\r\n", ch);
		return;
	}

	
	if(!strn_cmp(arg1, "title", strlen(arg1)))
	{

		half_chop(argument, arg1, arg2);

		if (IS_NPC(ch))
			send_to_char("Your title is fine... go away.\r\n", ch);

		if (PLR_FLAGGED(ch, PLR_NOTITLE))
			send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
		else if (strstr(arg2, "(") || strstr(arg2, ")"))

			send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
		else if (strlen(arg2) > MAX_TITLE_LENGTH) 
		{
			sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
				MAX_TITLE_LENGTH);

			send_to_char(buf, ch);
		}

		else 
		{
			set_title(ch, arg2);
			sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
			send_to_char(buf, ch);
			
		}
	}

	else if(!strn_cmp(arg1, "mood", strlen(arg1)))
	{

		two_arguments(argument, arg1, arg2);

		if(GET_MOOD(ch) == MOOD_BERSERK && FIGHTING(ch))
		{
			send_to_char("You are too berserk and in battle to do that now!\r\n", ch);
			return;
		}

		if(!str_cmp(arg2, "wimpy"))
			GET_MOOD(ch) = MOOD_WIMPY;
		
		else if(!str_cmp(arg2, "normal"))
			GET_MOOD(ch) = MOOD_NORMAL;

		else if(!str_cmp(arg2, "brave"))
			GET_MOOD(ch) = MOOD_BRAVE;

		else if(!str_cmp(arg2, "berserk"))
			GET_MOOD(ch) = MOOD_BERSERK;

		else
		{
			send_to_char("Current moods are: wimpy, normal, brave, berserk.\r\n", ch);
			return;
		}

		sprintf(buf, "Mood changed to: %s\r\n", mood_type(GET_MOOD(ch)));
		send_to_char(buf, ch);
	}
	
	else if(!strn_cmp(arg1, "password", strlen(arg1)))
	{

		send_to_char("This option is disabled.\r\n", ch);
		return;

		/*
		two_arguments(argument, arg1, arg2);

		if(!str_cmp(arg2, ""))
		{
			send_to_char("Password cannot be nullptr.\r\n", ch);
			return;
		}

		strcpy(GET_PASSWD(ch), arg2);

		sprintf(buf, "Your password has been changed to: %s.\r\n", arg2);
		send_to_char(buf, ch);
		*/
	}

	else if(!strn_cmp(arg1, "spam", strlen(arg1)))
	{

		two_arguments(argument, arg1, arg2);

		if(!str_cmp(arg2, "complete"))
		{
			send_to_char("Your spam has been turned to COMPLETE.\r\n", ch);
			SET_BIT_AR(PRF_FLAGS(ch), PRF_SPAM);
		}

		else if(!str_cmp(arg2, "none"))
		{
			send_to_char("Your spam has been turned NONE.\r\n", ch);
			REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_SPAM);
		}

		else
		{
			send_to_char("You can only change your spam to COMPLETE or NONE.\r\n", ch);
			return;
		}
	}

	else if(!strn_cmp(arg1, "description", strlen(arg1)))
	{

		if(ch->desc)
		{

			act("$n begins editing $s description.", FALSE, ch, nullptr, nullptr, TO_ROOM);
			send_to_char("Enter the new text you'd like others to see when they look at you.\r\n", ch);
			send_to_char("(/s saves /h for help)\r\n", ch);
			ch->desc->str = &ch->player.description;
			ch->desc->max_str = EXDSCR_LENGTH;
		}

		else
		{
			send_to_char("You don't have this ability!\r\n", ch);
			return;
		}
	}
}
