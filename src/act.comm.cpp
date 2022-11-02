/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "dg_scripts.h"
#include "olc.h"

/* extern variables */
extern int level_can_shout;
extern float SHOUT_HEALTH_PERCENT;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern char_data *character_list;
extern struct clan_data clan_list[50];
void check_other(char_data *ch, char *speach);
int can_communicate(char_data *speaker, char_data *receiver);

/* local functions */
void perform_tell(char_data *ch, char_data *vict, char *arg);
int is_tell_ok(char_data *ch, char_data *vict);
char *PERS(char_data *ch, char_data *vict);
char *DARKNESS_CHECK(char_data *ch, char_data *vict);

ACMD(do_say);
ACMD(do_tell);
ACMD(do_reply);
ACMD(do_spec_comm);
ACMD(do_write);
ACMD(do_page);
ACMD(do_gen_comm);
ACMD(do_speak);

//Just re-defined //
ACMD(do_clan);
ACMD(do_rank);
ACMD(do_warrant);
ACMD(do_pardon);
ACMD(do_note);

/* Determine whether we scramble the communication text or not...  Galnor October 27, 2003 */
int can_communicate(char_data *speaker, char_data *receiver)
{

	if(!IS_TROLLOC(speaker) && !IS_TROLLOC(receiver))
		return 1;

	else if(IS_TROLLOC(speaker) && IS_TROLLOC(receiver))
		return 1;

	else if((IS_TROLLOC(speaker) && PLR_FLAGGED(receiver, PLR_DARKFRIEND)) ||
		(IS_TROLLOC(receiver) && PLR_FLAGGED(speaker, PLR_DARKFRIEND)))
		return 1;

	else if(IS_FADE(speaker) || IS_FADE(receiver) || IS_DREADLORD(speaker) || IS_DREADLORD(receiver) || IS_GREYMAN(speaker) || IS_GREYMAN(receiver))
		return 1;

	else if(GET_LEVEL(speaker) >= LVL_IMMORT || GET_LEVEL(receiver) >= LVL_IMMORT)
		return 1;

	else if(IS_NPC(speaker) || IS_NPC(receiver))
		return 1;

	else
		return 0;
}

// Check other speech related triggers. Galnor //
void check_other(char_data *ch, char *speach)
{
	char_data *victim, *master;
	player_clan_data *cl;
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int i = 0;

	two_arguments(speach, arg1, arg2);

	if(!ch || !PRF_FLAGGED(ch, PRF_COUNCIL))
		return;

	for(master = world[ch->in_room].people;master;master = master->next_in_room)
	{

		if(!(victim = get_char_vis(master, arg2)))
			break;

		 if(!MOB_FLAGGED(master, MOB_AWARD))
			 continue;

		/* Mob with the award flag was found. Now we have to see if they share a similar clan. */

		for(i = 0;i < NUM_CLANS;++i)
		{
			if((cl = ch->getClan(i)) && (master->getClan(i)))
				break;
		}

		if(!str_cmp(arg1, "clan"))
		{
			sprintf(buf, "%s %d", arg2, cl->clan);				
			do_clan(master, buf, 0, 0);
	
			do_say(master, "It has been done.", 0, 0);
			victim->save();
				
			sprintf(buf, "I %sed %s at the command of %s. Clan being %s, %s's rank is %d.",
			arg1, GET_NAME(victim), GET_NAME(ch), clan_list[cl->clan].name, GET_NAME(ch), cl->rank);
			do_note(master, buf, 0, 0);
			break;
		}

		if(!str_cmp(arg1, "declan"))
		{
			sprintf(buf, "%s 0", GET_NAME(victim));
			do_clan(master, buf, 0, 0);
			do_say(master, "It has been done.", 0, 0);
			victim->save();

			sprintf(buf, "I %sed %s at the command of %s. Clan being %s, %s's rank is %d.",
			arg1, GET_NAME(victim), GET_NAME(ch), clan_list[cl->clan].name, GET_NAME(ch), cl->rank);

			do_note(master, buf, 0, 0);
			break;
		}

		if(!str_cmp(arg1, "rank") && victim->getClan(i) && victim != ch)
		{
			sprintf(buf, "%s", GET_NAME(victim));
			do_rank(master, buf, 0, 0);
			do_say(master, "It has been done.", 0, 0);
			sprintf(buf, "I %sed %s at the command of %s. Clan being %s, %s's rank is %d.",
			arg1, GET_NAME(victim), GET_NAME(ch), clan_list[cl->rank].name, GET_NAME(ch), cl->rank);
			do_note(master, buf, 0, 0);
			break;
		}
/*
		if(!str_cmp(arg1, "warrant") && (victim = get_char_vis(master, arg2)))
			if(victim != ch && GET_RACE(ch) == GET_RACE(victim))
			{
				sprintf(buf, "%s %s", GET_NAME(victim), clan_list[GET_CLAN(ch)].name);
				do_warrant(master, buf, 0, 0);
				do_say(master, "It has been done.", 0, 0);
				sprintf(buf, "I %sed %s at the command of %s. Clan being %s, %s's rank is %d.",
				arg1, GET_NAME(victim), GET_NAME(ch), clan_list[GET_CLAN(ch)].name, GET_NAME(ch), GET_RANK(ch));
				do_note(master, buf, 0, 0);
				break;
			}
*/
	}
}

ACMD(do_say)
{
	char_data *vict;
	int r;
	char scramble[MAX_INPUT_LENGTH], temp_buf[320],
		say_type[MAX_INPUT_LENGTH], self[MAX_INPUT_LENGTH];

	skip_spaces(&argument);

	if(GET_CLASS(ch) == CLASS_FADE)
	{
		strcpy(say_type, "rasps");
		strcpy(self, "rasp");
	}

	else if(GET_CLASS(ch) == CLASS_GREYMAN)
	{
		strcpy(say_type, "breathes");
		strcpy(self, "breathe");
	}

	else
	{
		strcpy(say_type, "says");
		strcpy(self, "say");
	}

  	strcpy(scramble, argument);
	for (r = 0; argument[r]; r++)
	{
		if (scramble[r] != ' ')
			scramble[r] = number(56, 122);
	}

	if (!*argument)
		send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  
	else
	{
		for(vict = world[ch->in_room].people;vict;vict = vict->next_in_room)
		{
			if(vict != ch)
			{

				if(!can_communicate(ch, vict))
					sprintf(temp_buf, "%s %s '%s'\r\n", PERS(ch, vict), say_type, scramble);
				else
					sprintf(temp_buf, "%s %s '%s'\r\n", PERS(ch, vict), say_type, argument);

				send_to_char(temp_buf, vict);
			}
		}
 
		if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
			send_to_char(OK, ch);
    
		else
		{
			sprintf(temp_buf, "You %s '%s'", self, argument);
			act(temp_buf, FALSE, ch, 0, argument, TO_CHAR);
			noise_addup(ch->in_room, 1, TYPE_SPEACH);
		}
	}

	/* trigger check */
	speech_mtrigger(ch, argument);
	speech_wtrigger(ch, argument);
	check_other(ch, argument);
}


ACMD(do_speak)
{

	struct descriptor_data *pt;
	char arg1[MAX_INPUT_LENGTH], *speach = new char[MAX_INPUT_LENGTH];
	int race;

	char *to[] =
	{
		"the Light",
		"the Dark",
		"your race",
		"your race",
		"your race",
		"your race"
	};

	half_chop(argument, arg1, speach);

	skip_spaces(&speach);
	delete_doubledollar(speach);

	if(!*arg1)
	{
		message(ch, "Valid Choises: Light, Dark, All\r\n");
		return;
	}

	if(!strn_cmp(arg1, "light", strlen(arg1)))
		race = RACE_HUMAN;

	else if(!strn_cmp(arg1, "dark", strlen(arg1)))
		race = RACE_TROLLOC;

	else if(!strn_cmp(arg1, "all", strlen(arg1)))
		race = -1;

	else
	{
		message(ch, "Valid Choises: Light, Dark, All\r\n");
		return;
	}

	if (!*speach)
		send_to_char("You begin to speak, but have nothing to say...\r\n", ch);
	

	else {
    
		for (pt = descriptor_list; pt; pt = pt->next)
		{

			if(!pt->character)
				continue;

			sprintf(buf, "%s%s$n speaks loudly to %s '%s'%s",
				COLOR_BOLD(ch, CL_SPARSE), COLOR_CYAN(ch, CL_SPARSE),
				to[(int) GET_RACE(pt->character)] ? to[(int) GET_RACE(pt->character)] : "your race",
				speach, COLOR_NORMAL(ch, CL_SPARSE));
			
			if(GET_LEVEL(pt->character) < LVL_IMMORT)
			{
				if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch &&
					(GET_RACE(pt->character) == race || race == -1))
				{
					act(buf, FALSE, ch, 0, pt->character, TO_VICT | TO_SLEEP);
				}
			}
			
			else
				if(pt->character != ch)
				{
					if(race == -1)
					{
						sprintf(buf, "%s%s$n speaks loudly to all creation '%s'%s",COLOR_BOLD(ch, CL_SPARSE),
							COLOR_CYAN(ch, CL_SPARSE), speach, COLOR_NORMAL(ch, CL_SPARSE));
						act(buf, FALSE, ch, 0, pt->character, TO_VICT | TO_SLEEP);
					}

					else
					{

						sprintf(buf, "%s%s$n speaks loudly to %s '%s'%s", COLOR_BOLD(ch, CL_SPARSE), 
						COLOR_CYAN(ch, CL_SPARSE), to[race], speach, COLOR_NORMAL(ch, CL_SPARSE));
						act(buf, FALSE, ch, 0, pt->character, TO_VICT | TO_SLEEP);
					}
				}

		}
		
		if (PRF_FLAGGED(ch, PRF_NOREPEAT))
			send_to_char(OK, ch);
    
		else
			message(ch, "%s%sYou speak loudly to %s '%s'%s\r\n",
			COLOR_BOLD(ch, CL_SPARSE), COLOR_CYAN(ch, CL_NORMAL), race == -1 ? "all creation" : to[race],
			speach, COLOR_NORMAL(ch, CL_NORMAL));
	}
}


void perform_tell(char_data *ch, char_data *vict, char *arg)
{

	struct ignore_data *i, *temp;

	send_to_char(COLOR_RED(ch, CL_NORMAL), vict);
	sprintf(buf, "$n tells you, '%s'", arg);
	act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
	send_to_char(COLOR_NORMAL(ch, CL_NORMAL), vict);	

	if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
		send_to_char(OK, ch);

	else
	{
		sprintf(buf, "You tell $N, '%s'", arg);
		act(buf, FALSE, ch, 0, vict, TO_CHAR);
	
		for(i = ch->ignores;i;i = i->next)
		{
			if(!str_cmp(i->name, GET_NAME(vict)))
			{
				sprintf(buf, "%s is no longer ignoring %s.", GET_NAME(ch), GET_NAME(vict));
				mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
				REMOVE_FROM_LIST(i, ch->ignores, next);
				delete(i);
				save_ignores(ch);
				break;
			}
		}
	}

	if (!IS_NPC(vict) && !IS_NPC(ch))
		GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

int is_tell_ok(char_data *ch, char_data *vict)
{
	if (ch == vict)
		send_to_char("You try to tell yourself something.\r\n", ch); 
  
	else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOTELL))
		send_to_char("You can't tell other people while you have notell on.\r\n", ch);
 
	else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
		send_to_char("The walls seem to absorb your words.\r\n", ch);
  
	else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
		act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);

	else if (PRF_FLAGGED(ch, PRF_TELL_MUTE))
		message(ch, "You are mute to tells. You shouldn't have abused them.\r\n");

	else if (PRF_FLAGGED(vict, PRF_TELL_MUTE))
		act("$E's is mute to tells... Try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  
	else if (PLR_FLAGGED(vict, PLR_WRITING))
		act("$E's writing a message right now; try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  
	else if ((!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_NOTELL)) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
		act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);

	else if(is_ignoring(vict, GET_NAME(ch)))
		act("$E is ignoring you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  
	else if(GET_RACE(vict) != GET_RACE(ch) && GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(vict) < LVL_IMMORT)
		send_to_char(NOPERSON, ch);
  
	else
		return TRUE;

	return FALSE;
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
	char_data *vict;

	half_chop(argument, buf, buf2);

	if (!*buf || !*buf2)
		send_to_char("Who do you wish to tell what??\r\n", ch);
  
	else if (!(vict = get_char_vis(ch, buf)))
		send_to_char(NOPERSON, ch);
  
	else if (is_tell_ok(ch, vict))
		perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
	char_data *tch = character_list;

	if (IS_NPC(ch))
		return;

	skip_spaces(&argument);

	if (GET_LAST_TELL(ch) == NOBODY)
		send_to_char("You have no-one to reply to!\r\n", ch);
  
	else if (!*argument)
		send_to_char("What is your reply?\r\n", ch);
  
	else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */
				     
    /*
     * XXX: A descriptor list based search would be faster although
     *      we could not find link dead people.  Not that they can
     *      hear tells anyway. :) -gg 2/24/98
     */
    
		while (tch != nullptr && (IS_NPC(tch) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
			tch = tch->next;

		if (tch == nullptr)
			send_to_char("They are no longer playing.\r\n", ch);
    
		else if (is_tell_ok(ch, tch))
			perform_tell(ch, tch, argument);
	}
}


ACMD(do_spec_comm)
{
	char_data *vict;
	const char *action_sing, *action_plur, *action_others;


	if (subcmd == SCMD_WHISPER)
	{
		action_sing = "whisper to";
		action_plur = "whispers to";
		action_others = "$n whispers something to $N.";
	} 
	
	else {
		action_sing = "ask";
		action_plur = "asks";
		action_others = "$n asks $N a question.";
	}

	half_chop(argument, buf, buf2);

	if (!*buf || !*buf2) {
		sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
		send_to_char(buf, ch);
	} 
	
	else if (!(vict = get_char_room_vis(ch, buf)))
		send_to_char(NOPERSON, ch);

	else if (!can_communicate(ch, vict))
	{
		message(ch, "Why would you try talking to %s?\r\n", GET_NAME(vict));
		return;
	}
  
	else if (vict == ch)
		send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  
	else {
		sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		
		if (PRF_FLAGGED(ch, PRF_NOREPEAT))
			send_to_char(OK, ch);
    
		else {
		sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
		act(buf, FALSE, ch, 0, 0, TO_CHAR);
		}
		
		act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
	}
}



#define MAX_NOTE_LENGTH 1000	/* arbitrary */

ACMD(do_write)
{
	struct obj_data *paper = 0, *pen = 0;
	char *papername, *penname, msg[MAX_STRING_LENGTH], cur[MAX_STRING_LENGTH], waste[MAX_STRING_LENGTH];
	int i = 0, ammount = 0;

	papername = buf1;
	penname = buf2;

	half_chop(argument, papername, argument);
	half_chop(argument, penname, msg);
	half_chop(msg, waste, msg);
	//skip_spaces(&msg);

	for(i = 0, ammount = atoi(waste);i < ammount && *waste;i++)
		if(*waste)
			half_chop(msg, waste, msg);

	if (!*papername)
	{		/* nothing was delivered */
		send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
		return;
	}
  
	if (*penname)
	{		/* there were two arguments */
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		{
			message(ch, "You have no %s.\r\n", papername);
			return;
		}
    
		if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
		{
			message(ch, "You have no %s.\r\n", penname);
			return;
		}
	} 
	
	else
	{		/* there was one arg.. let's see what we can find */
		if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
		{
			message(ch, "There is no %s in your inventory.\r\n", papername);
			return;
		}
    
		if (GET_OBJ_TYPE(paper) == ITEM_PEN)
		{	/* oops, a pen.. */
			pen = paper;
			paper = 0;
		} 
		
		else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
			send_to_char("That thing has nothing to do with writing.\r\n", ch);
			return;
		}
    
		/* One object was found.. now for the other one. */
		if (!GET_EQ(ch, WEAR_HOLD))
		{
			message(ch, "You can't write with %s %s alone.\r\n", AN(papername),
			papername);
			return;
		}
    
		if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD)))
		{
			send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
			return;
		}
    
		if (pen)
			paper = GET_EQ(ch, WEAR_HOLD);
    
		else
			pen = GET_EQ(ch, WEAR_HOLD);
	}


	/* ok.. now let's see what kind of stuff we've found */
	if (GET_OBJ_TYPE(pen) != ITEM_PEN)
		act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  
	else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
		act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  
	else if (paper->action_description && ch->desc && !IS_NPC(ch))
		send_to_char("There's something written on it already.\r\n", ch);
  
	else
	{
		/* we can write - hooray!
		this is the PERFECT code example of how to set up:
		a) the text editor with a message already loaed
		b) the abort buffer if the player aborts the message
		*/
     


	if (!ch->desc || IS_NPC(ch))
	{
		if(!*msg)
		{
			message(ch, "You must write a message right away if you are a mob!");
			return;
		}

		act("$n writes down a quick note.", TRUE, ch, 0, 0, TO_ROOM);
		
		if(paper->action_description)
		{
			strcpy(cur, paper->action_description);
			sprintf(cur + strlen(cur), "%s\r\n", msg);
		}

		else
			sprintf(cur, "%s\r\n", msg);

		paper->action_description = new char[MAX_STRING_LENGTH];
		paper->action_description = str_dup(cur);
		return;
	}




		ch->desc->backstr = nullptr;
		send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
		
		/* ok, here we check for a message ALREADY on the paper */
     
		if (paper->action_description)
		{
			/* we str_dup the original text to the descriptors->backstr */
			ch->desc->backstr = str_dup(paper->action_description);
			/* send to the player what was on the paper (cause this is already */
			/* loaded into the editor) */
			send_to_char(paper->action_description, ch);
		}
    
		act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
		/* assign the descriptor's->str the value of the pointer to the text */
		/* pointer so that we can reallocate as needed (hopefully that made */
		/* sense :>) */
		ch->desc->str = &paper->action_description;
		ch->desc->max_str = MAX_NOTE_LENGTH;
	}
}

ACMD(do_page)
{
	struct descriptor_data *d;
	char_data *vict;

	half_chop(argument, arg, buf2);

	if (IS_NPC(ch))
		send_to_char("Monsters can't page.. go away.\r\n", ch);
	
	else if (!*arg)
		send_to_char("Whom do you wish to page?\r\n", ch);
	
	else {
		sprintf(buf, "\007*%s* %s\r\n", GET_NAME(ch), buf2);
    
		if (!str_cmp(arg, "all"))
		{
			if (GET_LEVEL(ch) > LVL_GOD)
			{
	
				for (d = descriptor_list; d; d = d->next)
					if (STATE(d) == CON_PLAYING && d->character)
						act(buf, FALSE, ch, 0, d->character, TO_VICT);
			} 
			
			else
				send_to_char("You will never be godly enough to do that!\r\n", ch);
      
			return;
		}
    
		if ((vict = get_char_vis(ch, arg)) != nullptr)
		{
			act(buf, FALSE, ch, 0, vict, TO_VICT);
			if (PRF_FLAGGED(ch, PRF_NOREPEAT))
				send_to_char(OK, ch);
      
			else
				act(buf, FALSE, ch, 0, vict, TO_CHAR); 
			return;
		} 
		
		else
			send_to_char("There is no such person in the game!\r\n", ch);
	}
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
	struct descriptor_data *i;
	char color_on[24];
	char scramble[MAX_INPUT_LENGTH];
	int r = 0;
	char name[MAX_INPUT_LENGTH];

	/* Array of flags which must _not_ be set in order for comm to be heard */
	int channels[] = {
//		0,
		PRF_NOYELL,
		PRF_NOSHOUT,
		PRF_NOCHAT,
		PRF_NONARR,
		0
	};

		/*
		com_msgs:	[0] Message if you can't perform the action because of noshout
					[1] name of the action
					[2] message if you're not on the channel
					[3] a color string.
		*/
  
	const char *com_msgs[][4] = {
		{"You cannot yell!!\r\n",
		"yell",
		"",
		YELLOW},

		{"You cannot shout!!\r\n",
		"shout",
		"Turn off your noshout flag first!\r\n",
		YELLOW},

		{"You cannot chat!!\r\n",
		"chat",
		"You aren't even on the channel!\r\n",
		YELLOW},

		{"You cannot narrate!!\r\n",
		"narrate",
		"You aren't even on the channel!\r\n",
		YELLOW}
	};
	
	if(GET_LEVEL(ch) >= LVL_IMMORT)
	{
		send_to_char("Use speak!\r\n", ch);
		return;
	}

	if (PLR_FLAGGED(ch, PLR_NOSHOUT))
	{
		send_to_char(com_msgs[subcmd][0], ch);
		return;
	}
  
	if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
	{
		send_to_char("The walls seem to absorb your words.\r\n", ch);
		return;
	}
  
	/* level_can_shout defined in config.c */
	if (GET_LEVEL(ch) < level_can_shout)
	{
		sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
		level_can_shout, com_msgs[subcmd][1]);
		send_to_char(buf1, ch);
		return;
	}
  
	/* make sure the char is on the channel */
	if (PRF_FLAGGED(ch, channels[subcmd]))
	{
		send_to_char(com_msgs[subcmd][2], ch);
		return;
	}
  
	/* skip leading spaces */
	skip_spaces(&argument);

	/* make sure that there is something there to say! */
	if (!*argument)
	{
		sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
		com_msgs[subcmd][1], com_msgs[subcmd][1]);
		send_to_char(buf1, ch);
		return;
	}
  
	if (subcmd == SCMD_SHOUT && !IS_NPC(ch))
	{
		if (GET_HIT(ch) < GET_MAX_HIT(ch) * SHOUT_HEALTH_PERCENT)
		{
			send_to_char("You're too exhausted to shout.\r\n", ch);
			return;
		}
		
		else
			GET_HIT(ch) = MAX(1, (int) (GET_MAX_HIT(ch) * SHOUT_HEALTH_PERCENT));
	}
  
	/* set up the color on code */
	strcpy(color_on, com_msgs[subcmd][3]);

	/* first, set up strings to be given to the communicator */
	if (PRF_FLAGGED(ch, PRF_NOREPEAT))
		send_to_char(OK, ch);
  
	else
	{
		if (COLOR_LEV(ch) >= CL_COMPLETE)
			sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1],
			argument, NORMAL);
    
		else
			sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
			act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
	}

	/* now send all the strings out */
	for (i = descriptor_list; i; i = i->next)
	{

		if(i->character && is_ignoring(i->character, GET_NAME(ch)))
			continue;

		if (STATE(i) == CON_PLAYING && i != ch->desc && i->character &&
		!PRF_FLAGGED(i->character, channels[subcmd]) &&
		!PLR_FLAGGED(i->character, PLR_WRITING) &&
		!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF))
		
		{
			/* To prevent crossrace communication */
			if(GET_LEVEL(ch) < LVL_IMMORT && GET_LEVEL(i->character) < LVL_IMMORT)
			{
				if(subcmd == SCMD_NARRATE &&
				GET_RACE(i->character) != GET_RACE(ch))
					continue;
		
				if(subcmd == SCMD_CHAT &&
				GET_RACE(i->character) != GET_RACE(ch))
					continue;
			}

		strcpy(scramble, argument);

		for (r = 0; argument[r]; r++)
		{
			if (scramble[r] != ' ')
				scramble[r] = number(56, 122);
		}

		strcpy(name, DARKNESS_CHECK(i->character, ch));

		if((IS_DARK(IN_ROOM(i->character)) || IS_DARK(IN_ROOM(ch))) && !CAN_SEE_IN_DARK(i->character))
			strcpy(name, "Someone");

		if(!can_communicate(ch, i->character))
			sprintf(buf, "%s %ss, '%s'", name, com_msgs[subcmd][1], scramble);
	
		else
			sprintf(buf, "%s %ss, '%s'", name, com_msgs[subcmd][1], argument);

		if (subcmd == SCMD_YELL &&
		((world[ch->in_room].zone != world[i->character->in_room].zone) ||
		GET_POS(i->character) < POS_RESTING))
			continue;

		
		if (COLOR_LEV(i->character) >= CL_NORMAL)
			send_to_char(color_on, i->character);
      
		act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      
		if (COLOR_LEV(i->character) >= CL_NORMAL)
			send_to_char(NORMAL, i->character);
		}
	}
}

