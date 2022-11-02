/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Coparancopyright (C) 1990, 1991.               *
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
#include "constants.h"
#include "screen.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct memory_data *mlist;
extern int pk_allowed;
extern int count_fighting(char_data *ch);

/* extern functions */
void raw_kill(char_data * ch, char_data * killer);

/* local functions */
ACMD(do_assist);
ACMD(do_hit);
ACMD(do_kill);
ACMD(do_shoot);
ACMD(do_backstab);
ACMD(do_source);
ACMD(do_release);
ACMD(do_order);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_rescue);
ACMD(do_kick);

int too_scared(char_data *ch);
int abs_find(char_data *ch);
int parry_find(char_data *ch);
int offense_find(char_data *ch);
int dodge_find(char_data *ch);
int Crash_load(char_data *ch, int show);
int do_simple_move(char_data *ch, int dir, int need_specials_check, int air_ok);
int room_visibility(char_data *ch, char_data *vict);
int move_damage(char_data *ch, char_data *vict, int low, int high);
int can_communicate(char_data *speaker, char_data *receiver);
int can_move(char_data *ch, int dir, int need_specials_check);
void dmg(char_data *ch, char_data *vict, int low, int high, int same_room);
void obj_from_char(struct obj_data *obj);
void obj_to_char(obj_data *object, char_data *ch);
void stop_riding(char_data *ch);
void remove_player_clan(char_data *ch, int number);
void save_player_clan(char_data *ch, int number);
void perform_flee(char_data *ch);

extern char *mood_type(int mood);

int too_scared(char_data *ch)
{
	int chance;

	chance = number(1, 2);

	if(!AFF_FLAGGED(ch, AFF_PARANOIA))
		return 0;

	if(chance == 1)
		return 0;

	else
	{
		send_to_char("What?!? You're too scared to even think about that!!\r\n", ch);
		return 1;
		
	}
}

char_data *faceoff(char_data *attacker, char_data *prey)
{

	char_data *target;

	if(!FIGHTING(prey))
		return prey;

	for(target = world[IN_ROOM(prey)].people;target;target = target->next_in_room)
	{
		if(target->master == nullptr && prey->master == nullptr)
			continue;

		if(CAN_SEE(attacker, target) && (target->master == prey->master || target == prey->master || target->master == prey))
		{
			if(count_fighting(target) < count_fighting(prey)
				&& !MOB_FLAGGED(target, MOB_MOUNT) && !MOB_FLAGGED(target, MOB_SHADOW_MOUNT))
			{
				return target;
			}
		}
	}

	return prey;
}

void perform_bash(char_data *ch, char_data *vict)
 {
	int off;
	double def;
	struct obj_data *wielded;

	GET_TARGET(ch) = nullptr;

	if(!vict)
	{
		send_to_char("They must have already left...\r\n", ch);
		return;
	}

	wielded = GET_EQ(ch, WEAR_WIELD);

	/*Make the roll to determine who wins.*/
	off = number((int) (object_weight(wielded) * 3.8), (int) (object_weight(wielded) * 7.7));
	off += (int) (offense_find(ch) / 1.5);

	if(IS_CHANNELER(ch) && !IS_NPC(ch))
		off -= 30;

	if(GET_OBJ_VAL(wielded, 3) == 11)
		off = 0;

	if(ch->getClan(CLAN_GHOBHLIN))
		off += number(1, 25);

	if(ch->getClan(CLAN_BLADEMASTERS))
		off += number(10, 35);

	def = number(dodge_find(vict), dodge_find(vict) * 3.0);
	
	if(def < 110)
		def = 110;

	off = (int) ((float) off * ((float) GET_SKILL(ch, SKILL_BASH) / 100));

	//Testing balance
	if(PLR_FLAGGED(vict, PLR_LOGGER))
		def /= 2;

	if (off < def)
	{
		damage(ch, vict, 0, SKILL_BASH);
		change_pos(ch, POS_SITTING);
		
		WAIT_STATE(ch, PULSE_VIOLENCE / 2);


	} 
  
	else
	{
		if (damage(ch, vict, 1, SKILL_BASH) > 0)
		{	/* -1 = dead, 0 = miss */
			change_pos(vict, POS_SITTING);
			WAIT_STATE(vict, PULSE_VIOLENCE * 2);
			IS_BASHED(vict) = 1;
		}
	}
}


void perform_backstab(char_data *ch, char_data *vict)
{
	float offense, defense;

	GET_TARGET(ch) = nullptr;

	if(!vict)
	{
		send_to_char("They must have already left...\r\n", ch);
		return;
	}

	/****** Decide Stab Roll **************/

	offense = number(120, 270);
	offense *= (float) GET_SKILL(ch, SKILL_BACKSTAB) / 100;
	
	if (AFF_FLAGGED(ch, AFF_HIDE))
		offense *= (float) GET_SKILL(ch, SKILL_HIDE) / 100;

	else
		offense = 0;
	
	if(GET_LEVEL(ch) < 30)
		offense -= (30 - GET_LEVEL(ch)) * 10;

	if(IS_NPC(vict))
		defense = number(GET_LEVEL(vict) * 4, GET_LEVEL(vict) * 6);

	else
	{
		defense = number(100, 200);
		defense += GET_SKILL(vict, SKILL_BACKSTAB) / 5;
	}

	/****************************************/

	if (AWAKE(vict) && (defense > offense) || MOB_FLAGGED(vict, MOB_AWARE))
	{
		damage(ch, vict, 0, SKILL_BACKSTAB);
		WAIT_STATE(ch, PULSE_VIOLENCE);
	}
	
	else
		hit(ch, vict, SKILL_BACKSTAB);

}

void perform_assist(char_data *ch, char_data *helpee)
{
	
	char_data *opponent;


	if(!ch || !helpee || FIGHTING(ch))
		return;

	if (FIGHTING(helpee))
		opponent = FIGHTING(helpee);
    
	else
	{
		for (opponent = world[ch->in_room].people;
			opponent && (FIGHTING(opponent) != helpee);
			opponent = opponent->next_in_room);
	}

	if (!opponent)
			act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    
	else if (!CAN_SEE(ch, opponent))
		act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);

	else
	{
		opponent = faceoff(ch, opponent);
		send_to_char("You join the fight!\r\n", ch);
		act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
		act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
		set_fighting(ch, opponent);
	}
}

void perform_charge(char_data *ch, char_data *victim, int type)
{

	int defense = 0, offense = 0, skill = 0;

	skill = (type == SCMD_CHARGE ? SKILL_CHARGE : SKILL_SKEWER);

	if(!victim)
	{
		send_to_char("They must have left already...\r\n", ch);
		return;
	}

	/* Find the defense of the victim */
	defense += number(dodge_find(victim) / 2, dodge_find(victim));
	defense += (60 - (GET_WEIGHT(victim) / 10));
	defense += (250 - GET_HEIGHT(victim));
	defense +=  (GET_DEX(victim));
	defense += number(1, 100);

	if(IS_NPC(victim))
	{
		defense += number(GET_LEVEL(victim) * 4, GET_LEVEL(victim) * 5);
	}

	/* Find the offense of the attacker */

	offense += number(offense_find(ch), offense_find(ch) * 2);
	offense = (int) ((float) offense * ((float) GET_SKILL(ch, skill) / 100));
	offense += (30 - object_weight(GET_EQ(ch, WEAR_WIELD)));
	offense += number(1, 100);

	if(GET_LEVEL(ch) < 30)
	{
		offense -= (30 - GET_LEVEL(ch)) * 5;
	}

	/* The attacker fails. */
	if(defense > offense || (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_AWARE)))
	{
		damage(ch, victim, 0, skill);
		WAIT_STATE(ch, PULSE_VIOLENCE);
	}

	/* The attacker succeeds. */
	else
	{
		WAIT_STATE(ch, PULSE_VIOLENCE);
		WAIT_STATE(victim, PULSE_VIOLENCE);
		hit(ch, victim, skill);
	}
}

ACMD(do_charge)
{
	char_data *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);


	if(IS_TROLLOC(ch) && subcmd == SCMD_CHARGE && !IS_FADE(ch) && !IS_DREADLORD(ch))
	{
		send_to_char("You can't charge! You would need a mount for that.\r\n", ch);
		return;
	}

	if((!IS_TROLLOC(ch) || (IS_FADE(ch) || IS_DREADLORD(ch))) && (subcmd == SCMD_SKEWER))
	{
		send_to_char("You can't run fast enough to do that.\r\n", ch);
		return;
	}

	if(!argument || !*arg)
	{
		send_to_char("Who do you want to do this to?\r\n", ch);
		return;
	}

	if (!(victim = get_char_room_vis(ch, arg)))
	{
		send_to_char("That person is not here.\r\n", ch);
		return;
	}

	if(ch->desc)
	{
		if(ch->desc->command_ready)
		{
			victim = GET_TARGET(ch);
		}
	}

	if(!room_visibility(ch, victim))
	{ //Checks to see if ch can see vict in ch's room //
		send_to_char("They must have left already...\r\n", ch);
		return;
	}

	if(too_scared(ch))
		return;

	if(subcmd == SCMD_CHARGE && !MOUNT(ch))
	{
		send_to_char("You will need to be riding to even attempt this.\r\n", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("You want to try to charge yourself?\r\n", ch);
		return;
	}

	if(FIGHTING(ch) && ch->desc && !ch->desc->command_ready)
	{
		send_to_char("No way! You're fighting for your life here!\r\n", ch);
		return;
	}
  
	if (!GET_EQ(ch, WEAR_WIELD))
	{
		send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
		return;
	}
  
	if ((subcmd == SCMD_CHARGE && GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 0) != WEAPON_SPEAR))
	{
		message(ch, "You will need a spear to even attempt this.\r\n");
		return;
	}

	if ((subcmd == SCMD_SKEWER && GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 0) != WEAPON_LANCE))
	{
		message(ch, "You will need a lance to even attempt this.\r\n");
		return;
	}

	if(FIGHTING(victim))
	{
		message(ch, "You might hit the wrong target!\r\n", GET_NAME(victim));
		return;
	}
	
	if(IS_BASHED(victim))
	{
		message(ch, "%s is stunned... You can't seem to reach the right spot!\r\n", GET_NAME(victim));
		return;
	}

	GET_TARGET(ch) = victim;

	if(ch->desc && !ch->desc->command_ready)
	{
		message(ch, "You begin charging towards %s.\r\n", GET_NAME(victim));
		message(victim, "%s begins charging towards you.\r\n", GET_NAME(ch));
		act("$n begins charging towards $N.", TRUE, ch, nullptr, victim, TO_NOTVICT);
	}

	if(ch->desc)
		ch->desc->command_ready = 1;

	if(!IS_NPC(ch) && ch->desc)
	{
		if(ch->desc->timer <= 0)
			perform_charge(ch, GET_TARGET(ch), subcmd);
	}

	else
	{
		if(IS_NPC(ch))
		{
			perform_charge(ch, victim, subcmd);

		}
	}
}


/*	Tulon: 4-9-2004. Basically here we have two types of bow and arrow shooting.
 *	You can shoot a target one room away for limited damage and less accuracy or
 *	shoot someone in the same room for more damage
 */

ACMD(do_shoot)
{
	char_data *victim;
	struct obj_data *weapon = nullptr;
	int room_to = 0, old_room = 0, i = 0;
	bool found = false;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], *dm = nullptr;

	weapon = GET_EQ(ch, WEAR_WIELD);

	if(!weapon || GET_OBJ_VAL(weapon, 0) != WEAPON_BOW)
	{
		send_to_char("You can only shoot with a bow.\r\n", ch);
		return;
	}

	two_arguments(argument, arg1, arg2);

	if(!*arg1)
	{
		send_to_char("In which direction do you want to shoot, and whom?\r\n", ch);
		return;
	}

	/*
	if(!*arg2)
	{
		send_to_char("Who in that direction do you want to shoot?\r\n", ch);
		return;
	}

	*/


	if(!(victim = get_char_room_vis(ch, arg1)) && !*arg2)
	{
		send_to_char("There is no one here by that name.\r\n", ch);
		return;
	}

	if(ch->desc && ch->desc->command_ready)
		victim = GET_TARGET(ch);

	if(!room_visibility(ch, victim) && !*arg2)
	{ //Checks to see if ch can see vict are in the same room //
		send_to_char("They must have left already...\r\n", ch);
		return;
	}

	/* There was no victim found inside of the shooters room, so we need to try a directional shot */
	if(!victim)
	{

		while(*dirs[i] != '\n')
		{
			if(!strn_cmp(arg1, dirs[i], strlen(arg1)))
			{
				found = true;
				break;
			}

			i++;
		}

		if(!world[ch->in_room].dir_option[i] || !found)
		{
			send_to_char("There is no such exit.\r\n", ch);
			return;
		}

		room_to = world[ch->in_room].dir_option[i]->to_room;

		if(i == DOWN || i == UP)
			dm = "";

		else
			dm = "to the ";

		if(ch->desc && !ch->desc->command_ready)
		{
			message(ch, "You pull an arrow back and aim %sward.\r\n", dirs[i]);
			sprintf(buf, "%s aims an arrow %s%s.", GET_NAME(ch), dm, dirs[i]);
			act(buf, FALSE, ch, 0, 0, TO_ROOM);
			
			ch->desc->command_ready = 1;
		}
	}

	/* A victim was found from the room search, so let's shoot them instead */
	else
	{

		if(ch->desc && !ch->desc->command_ready)
		{
			message(ch, "You pull an arrow back onto your bow string and aim at %s.\r\n", GET_NAME(victim));
			act("$n pulls an arrow back into $s bow and aims at you.", TRUE, ch, nullptr, victim, TO_VICT);
			act("$n aims an arrow at $N.", TRUE, ch, nullptr, victim, TO_NOTVICT);
			
			ch->desc->command_ready = 1;
			ch->desc->timer = 2.0;

			GET_TARGET(ch) = victim;
		}
	}

	if((ch->desc && ch->desc->timer <= 0) || IS_NPC(ch))
	{

		if(!GET_TARGET(ch))
		{

			old_room = ch->in_room;

			char_from_room(ch);
			char_to_room(ch, room_to);

			victim = get_char_room_vis(ch, arg2);

			char_from_room(ch);
			char_to_room(ch, old_room);

			if(!victim)
			{
				send_to_char(NOPERSON, ch);
				return;
			}

			message(ch, "You let loose your arrow as it flies %s%s.\r\n", dm, dirs[i]);

			if(i == DOWN)
				dm = "above";

			else if(i == UP)
				dm = "below";

			else
				dm = "the ";

			if((number(1, 130) + (FIGHTING(victim) ? 10 : 0) > GET_SKILL(ch, SKILL_BOW)))
			{

				sprintf(buf, "An arrow from %s%s hits the ground.\r\n", dm, (i != UP && i != DOWN) ? dirs[rev_dir[i]] : "");
				send_to_room(buf, victim->in_room);
				return;
			}

			message(victim, "%sAn arrow from %s%s strikes you!%s\r\n", COLOR_RED(victim, CL_COMPLETE), dm,
				(i != UP && i != DOWN) ? dirs[rev_dir[i]] : "", COLOR_NORMAL(victim, CL_COMPLETE));
			
			sprintf(buf, "An arrow shoots through the air from %s%s and hits %s.", dm, (i != UP && i != DOWN) ? dirs[rev_dir[i]] : "", GET_NAME(victim));
			act(buf, TRUE, victim, 0, 0, TO_NOTVICT);

			dmg(ch, victim, 10, 20, FALSE);
		}

		else
		{

			if((number(1, 120) + (FIGHTING(victim) ? 10 : 0) > GET_SKILL(ch, SKILL_BOW)))
			{
				i = 0;
				victim = nullptr;

				if(number(1, 3) == 1)
				{
					message(ch, "You shoot your arrow which lands on the ground.\r\n");
					act("$n shoots an arrow which lands on the ground.", TRUE, ch, 0, 0, TO_ROOM);
					return;
				}
				
				while(!victim || victim == ch)
				{
					if(i++ >= 100)
						break;

					for(victim = world[ch->in_room].people;victim;victim = victim->next)
					{
						if(number(1, 3) == 1)
							break;
					}
				}
			}

			if(i >= 100)
			{
				send_to_char("You lose focus on your target and are not able to shoot anyone.\r\n", ch);
				return;
			}

			message(ch, "You let loose your arrow, which hits %s.\r\n", GET_NAME(victim));

			message(victim, "%sYou are hit by an arrow which was shot from %s.%s\r\n", COLOR_RED(victim, CL_COMPLETE),
				GET_NAME(ch), COLOR_NORMAL(victim, CL_COMPLETE));

			act("$n lets go of $s arrow which hits $N.", TRUE, ch, nullptr, victim, TO_NOTVICT);

			dmg(ch, victim, 35, 60, TRUE);
		}
	}
}

/* Connecting to the True Source in order to channel. */
ACMD(do_source)
{
	
	char_data *victim;

	if(!IS_CHANNELER(ch) && !IS_DREADLORD(ch) && GET_LEVEL(ch) < LVL_GRGOD)
	{
		if(subcmd == SCMD_SEIZE)
			message(ch, "You come out empty handed.\r\n");

		else if(subcmd == SCMD_EMBRACE)
			message(ch, "There is nothing for you to embrace!\r\n");

		return;
	}

	if(subcmd == SCMD_EMBRACE && GET_SEX(ch) == SEX_MALE)
	{

		message(ch, "If you did that, saidin would burn you alive!\r\n");
		return;
	}

	if(subcmd == SCMD_SEIZE && GET_SEX(ch) == SEX_FEMALE)
	{
		message(ch, "You have to embrace saidar and surrender to it in order to wield it.\r\n");
		return;
	}

	if(PRF_FLAGGED(ch, PRF_SOURCE))
	{
		message(ch, "You are in touch with the True Source already. Can you not feel the surge of power through you?\r\n");
		return;
	}

	if(subcmd == SCMD_EMBRACE)
		message(ch, "You surrender yourself to saidar and feel it fill your body.\r\n");
		
	else if(subcmd == SCMD_SEIZE)
		message(ch, "You grasp for saidin and feel it surge through your body.\r\n");

	else
		message(ch, "What's that? How did you get here? Please repord this.\r\n");

	SET_BIT_AR(PRF_FLAGS(ch), PRF_SOURCE);

	for(victim = world[IN_ROOM(ch)].people;victim;victim = victim->next_in_room)
	{
		if(GET_SEX(ch) == GET_SEX(victim) && victim != ch &&
			(IS_CHANNELER(victim) || IS_DREADLORD(victim) || GET_LEVEL(victim) >= LVL_GRGOD))
		{
			message(victim, "You feel a strange sensation from somewhere nearby.\r\n");
		}
	}
}

ACMD(do_release)
{

	char_data *victim;

	if(!IS_CHANNELER(ch) && !IS_DREADLORD(ch) && GET_LEVEL(ch) < LVL_GRGOD)
	{
		message(ch, "And you want to release what?\r\n");
		return;
	}

	if(!PRF_FLAGGED(ch, PRF_SOURCE))
	{
		message(ch, "You aren't even connected to the True Source.");
		return;
	}

	message(ch, "You release the One Power.\r\n");

	REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_SOURCE);

	for(victim = world[IN_ROOM(ch)].people;victim;victim = victim->next_in_room)
	{
		if(GET_SEX(ch) == GET_SEX(victim) && victim != ch &&
			(IS_CHANNELER(victim) || IS_DREADLORD(victim) || GET_LEVEL(victim) >= LVL_GRGOD))
		{
			message(victim, "A strange sensation from nearby gets weaker.\r\n");
		}
	}

}

ACMD(do_assist)
{
	char_data *helpee;

	if (FIGHTING(ch))
	{
		send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
		return;
	}
  
	one_argument(argument, arg);

	if (!*arg)
		send_to_char("Whom do you wish to assist?\r\n", ch);
  
	else if (!(helpee = get_char_room_vis(ch, arg)))
		send_to_char(NOPERSON, ch);

	else if(too_scared(ch))
		return;
  
	else if (helpee == ch)
		send_to_char("You can't help yourself any more than this!\r\n", ch);
  
	else
		perform_assist(ch, helpee);
}


ACMD(do_hit)
{
	char_data *vict;

	one_argument(argument, arg);

	if (!*arg)
		send_to_char("Hit who?\r\n", ch);
  
	else if (!(vict = get_char_room_vis(ch, arg)))
		send_to_char("They don't seem to be here.\r\n", ch);

	else if(too_scared(ch))
		return;
  
	else if (vict == ch)
	{
		send_to_char("You hit yourself...OUCH!.\r\n", ch);
		act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
	}
  
	else
	{

		vict = faceoff(ch, vict);

		if ((GET_POS(ch) == POS_STANDING) &&  !FIGHTING(ch))
			hit(ch, vict, TYPE_UNDEFINED);
		
		else
			send_to_char("You do the best you can!\r\n", ch);
	}

}


ACMD(do_kill)
{
	char_data *vict;

	if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch))
	{
		do_hit(ch, argument, cmd, subcmd);
		return;
	}

	if(too_scared(ch))
		return;

	one_argument(argument, arg);

	if (!*arg)
		send_to_char("Kill who?\r\n", ch);
	
	else
	{
		if (!(vict = get_char_room_vis(ch, arg)))
			send_to_char("They aren't here.\r\n", ch);
    
		else if (ch == vict)
			send_to_char("Your mother would be so sad.. :(\r\n", ch);

		else if(IS_KING(vict))
		{
			send_to_char("You can't strike the dictator! He is just much more... superb than you!\r\n", ch); //We know its true! :)
			return;
		}
    
		else
		{
			act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
			act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
			act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
			stop_riding(vict);
			raw_kill(vict, ch);
		}
	}
}

ACMD(do_backstab)
{
	char_data *vict;

	one_argument(argument, buf);

	if (!(vict = get_char_room_vis(ch, buf)))
	{
		send_to_char("Backstab who?\r\n", ch);
		return;
	}

	if(ch->desc)
	{
		if(ch->desc->command_ready)
		{
			vict = GET_TARGET(ch);
		}
	}

	if(!room_visibility(ch, vict))
	{ //Checks to see if ch can see vict in ch's room //
		send_to_char("They must have left already...\r\n", ch);
		return;
	}

	if(MOUNT(ch))
	{
		send_to_char("You can't do that while riding!\r\n", ch);
		return;
	}

	if(too_scared(ch))
		return;

	if (vict == ch)
	{
		send_to_char("How can you sneak up on yourself?\r\n", ch);
		return;
	}

	if(FIGHTING(ch) && ch->desc && !ch->desc->command_ready)
	{
		send_to_char("No way! You're fighting for your life here!\r\n", ch);
		return;
	}
  
	if (!GET_EQ(ch, WEAR_WIELD))
	{
		send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
		return;
	}
  
	if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)
	{
		send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
		return;
	}

	if(FIGHTING(vict) && ((ch->desc && ch->desc->command_ready) || !IS_NPC(FIGHTING(vict))))
	//if(FIGHTING(vict) && ch->desc && ch->desc->command_ready)
	{
		message(ch, "You can't sneak up on a fighting target.\r\n", GET_NAME(vict));
		return;
	}

	
	if(IS_BASHED(vict))
	{
		message(ch, "%s is stunned... You can't seem to reach the right spot!\r\n", GET_NAME(vict));
		return;
	}

	GET_TARGET(ch) = vict;
		

	if(ch->desc)
		ch->desc->command_ready = 1;

	if(!IS_NPC(ch) && ch->desc)
	{
		if(ch->desc->timer <= 0)
			perform_backstab(ch, GET_TARGET(ch));
	}

	else
		if(IS_NPC(ch))
			perform_backstab(ch, vict);
}

char *orders[] =
{
	"follow",
	"\n",
};

int order_command(char *order)
{
	int i = 0;

	for(i = 0;orders[i] != "\n";i++)
		if(!str_cmp(order, orders[i]))
			return 1;

	return 0;
}


int order_ok(char_data *ch, char_data *victim, char *order)
{

	int i = 0;

	if(victim->master != ch)
		return 0;

	if(!IS_NPC(victim))
		return 0;

	for(i = 0;*complete_cmd_info[i].command != '\n';i++)
	{
		if(!strn_cmp(complete_cmd_info[i].command, order, strlen(order)))
			if(!order_command((char *) complete_cmd_info[i].command))
			{
				continue;
			}

			else
				return 1;
	}

	return 0;
	
}

ACMD(do_order)
{
	char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
	bool found = FALSE;
	int org_room;
	char_data *vict;
	struct follow_type *k;

	half_chop(argument, name, message);

	if (!*name || !*message)
		send_to_char("Order who to do what?\r\n", ch);
  
	else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
		send_to_char("That person isn't here.\r\n", ch);
  
	else if (ch == vict)
		send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

	else
	{

		if(too_scared(ch))
			return;

		if (vict)
		{

			if(!can_communicate(ch, vict))
			{
				send_to_char("They won't listen... Why try?\r\n", ch);
				return;
			}

			sprintf(buf, "$N orders you to '%s'", message);
			act(buf, FALSE, vict, 0, ch, TO_CHAR);
			act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

		if (!order_ok(ch, vict, message))
			act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      
		else
		{
			send_to_char(OK, ch);
			command_interpreter(vict, message);
		}
	} 
		
		else
		{			/* This is order "followers" */
			sprintf(buf, "$n issues the order '%s'.", message);
			act(buf, FALSE, ch, 0, vict, TO_ROOM);

			org_room = ch->in_room;

		for (k = ch->followers; k; k = k->next)
		{
			if (org_room == k->follower->in_room)
			{
				found = TRUE;
				command_interpreter(k->follower, message);
			}
		}
      
		if (found)
			send_to_char(OK, ch);
      
		else
			send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
		}
	}
}

void perform_flee(char_data *ch)
{
	int attempt, loss, oldroom;
	char_data *was_fighting, *vict;

	FLEE_GO(ch) = FALSE;

	if (GET_POS(ch) < POS_FIGHTING) 
	{
		send_to_char("You are in pretty bad shape, unable to flee!\r\n", ch);
		return;
	}

	attempt = GET_DIRECTION(ch);	/* Select a random direction */
    
	if (CAN_GO(ch, attempt) &&
	!ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH))
	{
      
		oldroom = IN_ROOM(ch);
		vict = FIGHTING(ch);
		was_fighting = FIGHTING(ch);

		if (do_simple_move(ch, attempt, FALSE, TRUE))
		{
			send_to_char("You flee head over heels.\r\n", ch);
			

			// half a second for fighting people, .25 seconds for non engaged.
			if(FIGHTING(ch))
				FLEE_LAG(ch) += 3;

			else
				FLEE_LAG(ch) += 2;

			FLEE_LAG(ch) = MIN(15, FLEE_LAG(ch)); //Set the max flee lag.

			if(PLR_FLAGGED(ch, PLR_LOGGER) && FLEE_LAG(ch) < 15) //Testing different amounts
				FLEE_LAG(ch)++;
	
			if (was_fighting)
			{

				if(!IS_NPC(ch))
				{
					loss = GET_MAX_HIT(was_fighting) - GET_HIT(was_fighting);
					gain_exp(ch, -loss);
				}
	  
				if(FIGHTING(ch))
					stop_fighting(ch);

				for(vict = world[oldroom].people;vict;vict = vict->next_in_room)
				{
					if(FIGHTING(vict))
						if(FIGHTING(vict) == ch)
							stop_fighting(vict);
				}

			}
		} 
			
		else
			act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      
		return;
	}
	
	send_to_char("PANIC!  You couldn't escape!\r\n", ch);

}



ACMD(do_flee)
{
	int i;
	bool room_found = FALSE;

	if(GET_MOOD(ch) == MOOD_BERSERK)
	{
		send_to_char("You are BERSERK! Can you not feel the MADNESS?!", ch);
		return;
	}

	//send_to_char("You panic and attempt to flee!\r\n", ch);
	act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);

	if(FLEE_GO(ch) == FALSE)
	{
		for (i = 0; i < 4; i++) 
		{

			GET_DIRECTION(ch) = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */

			if (CAN_GO(ch, GET_DIRECTION(ch)) && !ROOM_FLAGGED(EXIT(ch, GET_DIRECTION(ch))->to_room, ROOM_DEATH))
			{

				if(can_move(ch, GET_DIRECTION(ch), FALSE)) 
				{
					room_found = TRUE;
					break;
				}
			}
		}
	}

	if(FLEE_GO(ch) || (!FIGHTING(ch) && room_found == TRUE)) 
	{
		perform_flee(ch);
		return;
	}

	if(room_found == TRUE)
	{
		FLEE_GO(ch) = TRUE;	
		WAIT_STATE(ch, (PULSE_VIOLENCE / 16) * FLEE_LAG(ch));
		return;
	}

	send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}

ACMD(do_bash)
{
	char_data *vict;

	one_argument(argument, arg);

	if (!GET_SKILL(ch, SKILL_BASH))
	{
		send_to_char("You have no idea how.\r\n", ch);
		return;
	}
  
	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL))
	{
		send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
		return;
	}

	if(too_scared(ch))
		return;

	if (!GET_EQ(ch, WEAR_WIELD)) 
	{
		send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
		return;
	}

	if(!*arg && FIGHTING(ch))
		vict = FIGHTING(ch);
  
	if (!(vict = get_char_room_vis(ch, arg)))
	{
		if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
		{
			vict = FIGHTING(ch);
		} 
	
		else
		{
			send_to_char("Bash who?\r\n", ch);
			return;
		}
	}

	// Requested by Someone (Jonlin?), next few lines of code by Serai - 02/10/04
	if (FIGHTING(ch) && FIGHTING(ch) != vict && FIGHTING(vict) != ch)
	{
		send_to_char("You can't bash someone else while engaged!\r\n", ch);
		return;
	}

	if(ch->desc && ch->desc->command_ready)
		vict = GET_TARGET(ch);

	if(!room_visibility(ch, vict))
	{ //Checks to see if ch can see vict in ch's room //
		send_to_char("They must have left already...\r\n", ch);
		return;
	}
  
	if (vict == ch)
	{
		send_to_char("Aren't we funny today...\r\n", ch);
		return;
	}

	if(IS_BASHED(vict))
	{
		send_to_char("They already seem stunned!\r\n", ch);
		return;
	}

	GET_TARGET(ch) = vict;

	if(!IS_NPC(ch))
		ch->desc->command_ready = 1;

	if(ch->desc)
	{
		if(ch->desc->timer <= 0)
			perform_bash(ch, vict);
	}

	else
		if(IS_NPC(ch))
			perform_bash(ch, vict);
}



ACMD(do_rescue)
{
	char_data *vict, *tmp_ch;
	int percent, prob;

	one_argument(argument, arg);

	if (!(vict = get_char_room_vis(ch, arg)))
	{
		send_to_char("Whom do you want to rescue?\r\n", ch);
		return;
	}

	if(GET_RACE(vict) != GET_RACE(ch))
	{
		send_to_char("Why would you rescue someone that is not of your kind?\r\n", ch);
		return;
	}
  
	if (vict == ch)
	{
		send_to_char("What about fleeing instead?\r\n", ch);
		return;
	}
  
	if (FIGHTING(ch) == vict)
	{
		send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
		return;
	}

	if(too_scared(ch))
		return;

	for (tmp_ch = world[ch->in_room].people; tmp_ch &&
	(FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

	if (!tmp_ch)
	{
		act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
		return;
	}
  
	if (!GET_SKILL(ch, SKILL_RESCUE))
		send_to_char("But you have no idea how!\r\n", ch);
  
	else
	{
		percent = number(1, 101);	/* 101% is a complete failure */
		prob = GET_SKILL(ch, SKILL_RESCUE);

		if (percent > prob)
		{
			send_to_char("You fail the rescue!\r\n", ch);
			return;
		}
    
		send_to_char("Banzai!  To the rescue...\r\n", ch);
		act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
		act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

		if (FIGHTING(vict) == tmp_ch)
			stop_fighting(vict);
    
		if (FIGHTING(tmp_ch))
			stop_fighting(tmp_ch);
    
		if (FIGHTING(ch))
			stop_fighting(ch);

		set_fighting(ch, tmp_ch);
		set_fighting(tmp_ch, ch);

		WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
	}

}

void perform_kick(char_data *ch, char_data *vict)
{

	int percent = 0, prob = 0;
	
	if(!vict || !ch)
		return;
	
	vict = faceoff(ch, vict);
	percent = number(1, dodge_find(vict));
	prob = GET_SKILL(ch, SKILL_KICK);

	if (percent > prob)
		damage(ch, vict, 0, SKILL_KICK); 

	else
	{
		move_damage(ch, vict, GET_SKILL(ch, SKILL_KICK) / 6, GET_SKILL(ch, SKILL_KICK) / 4);
		damage(ch, vict, GET_LEVEL(ch) / 4, SKILL_KICK);
	}

	WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}

ACMD(do_kick)
{
	char_data *vict;

	if (!GET_SKILL(ch, SKILL_KICK))
	{
		send_to_char("You have no idea how.\r\n", ch);
		return;
	}
  
	one_argument(argument, arg);

	if (!(vict = get_char_room_vis(ch, arg)))
		if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
			vict = FIGHTING(ch);
		
		else
		{
			send_to_char("Kick who?\r\n", ch);
			return;
		}

	if (vict == ch)
	{
		send_to_char("Aren't we funny today...\r\n", ch);
		return;
	}

	if(too_scared(ch))
		return;

	if(!IS_NPC(ch))
		ch->desc->command_ready = 1;

	if(ch->desc)
	{
		if(ch->desc->timer <= 0)
			perform_kick(ch, vict);

	}

	else
		if(IS_NPC(ch))
			perform_kick(ch, vict);
}
