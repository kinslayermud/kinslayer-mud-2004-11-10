/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "dg_scripts.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "interpreter.h"
#include "comm.h"

extern struct descriptor_data *descriptor_list;
extern sh_int find_target_room(char_data * ch, char *rawroomstr);
extern struct index_data *mob_index;
extern struct room_data *world;
extern int dg_owner_purged;
extern int top_of_world;
extern int top_of_zone_table;
extern struct zone_data *zone_table;
extern FILE *player_fl;
void die(char_data * ch, char_data * killer);

void reset_skills(char_data *ch);

void sub_write(char *arg, char_data *ch, cbyte find_invis, int targets);

/*
 * Local functions.
 */

/* attaches mob's name and vnum to msg and sends it to script_log */
void mob_log(char_data *mob, const char *msg)
{
    char buf[MAX_INPUT_LENGTH + 100];

    void script_log(const char *msg);

    sprintf(buf, "Mob (%s, VNum %d): %s",
	    GET_SHORT(mob), GET_MOB_VNUM(mob), msg);
    script_log(buf);
}
/*
** macro to determine if a mob is permitted to use these commands
*/
#define MOB_OR_IMPL(ch) \
  (IS_NPC(ch) && (!(ch)->desc || GET_LEVEL((ch)->desc->original)>=LVL_IMPL))



/* mob commands */

ACMD(do_mreset)
{
	char_data *victim;

	skip_spaces(&argument);

	if(!MOB_OR_IMPL(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

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

/* Change one's quest value. */
ACMD(do_mqval)
{
}

ACMD(do_mzreset)
{
	int i, j;

	one_argument(argument, arg);

	if(!MOB_OR_IMPL(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}
  
	if (!*arg) 
	{
		send_to_char("You must specify a zone.\r\n", ch);
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

ACMD(do_mhorse_purge)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *rider;

	one_argument(argument, arg);

	if(!MOB_OR_IMPL(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

	if(!*arg)
	{
		send_to_char("Who's mount do you want to purge?\r\n", ch);
		return;
	}

	if(!(rider = get_char_room_vis(ch, arg)))
	{
		send_to_char("There is no one here by that name.\r\n", ch);
		return;
	}

	if(!MOUNT(rider))
	{
		message(ch, "%s is not riding anything.\r\n", GET_NAME(rider));
		return;
	}

	extract_char(MOUNT(rider));
	return;
}


/* prints the argument to all the rooms aroud the mobile */
ACMD(do_masound)
{
	sh_int was_in_room;
	int  door;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}
  
	if (!*argument)
	{
		mob_log(ch, "masound called with no argument");
		return;
	}

	skip_spaces(&argument);

	was_in_room = IN_ROOM(ch);

	for (door = 0; door < NUM_OF_DIRS; door++)
	{
		struct room_direction_data *exit;

		if (((exit = world[was_in_room].dir_option[door]) != nullptr) &&
			exit->to_room != NOWHERE && exit->to_room != was_in_room)
		{
			IN_ROOM(ch) = exit->to_room;
			sub_write(argument, ch, TRUE, TO_ROOM);
		}
	}

	IN_ROOM(ch) = was_in_room;
}

ACMD(do_mdamage)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char_data *victim;
	int damage;

	if(!MOB_OR_IMPL(ch))
	{
		send_to_char("What?!?\r\n", ch);
		return;
	}

	two_arguments(argument, arg1, arg2);

	if(!(victim = get_char_room_vis(ch, arg1)))
		return;

	if(!*argument)
	{
		mob_log(ch, "mdamage called without an argument.");
		send_to_char("But mdamage who?\r\n", ch);
		return;
	}

	if(!*arg2)
	{
		send_to_char("mdamage how much?\r\n", ch);
		return;
	}

	damage = atoi(arg2);

	GET_HIT(victim) -= damage;

	if(GET_HIT(victim) <= 0)
		die(victim, ch);
}

/* lets the mobile kill any player or mobile without murder*/
ACMD(do_mkill)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *victim;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	one_argument(argument, arg);
    
	if (!*arg)
	{
		mob_log(ch, "mkill called with no argument");
		return;
	}

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "mkill: victim (%s) not found",arg);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_room_vis(ch, arg)))
	{
		sprintf(buf, "mkill: victim (%s) not found",arg);
		mob_log(ch, buf);
		return;
	}

	if (victim == ch)
	{
		mob_log(ch, "mkill: victim is self");
		return;
	}

	if (FIGHTING(ch))
	{
		mob_log(ch, "mkill: already fighting");
		return;
	}

	hit(ch, victim, TYPE_UNDEFINED);
	return;
}

/*
 * lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them
 */
ACMD(do_mjunk)
{
	char arg[MAX_INPUT_LENGTH];
	int pos;
	obj_data *obj;
	obj_data *obj_next;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}


	one_argument(argument, arg);
  
	if (!*arg)
	{
		mob_log(ch, "mjunk called with no argument");
		return;
	}

	if (find_all_dots(arg) != FIND_INDIV)
	{
		if ((obj=get_object_in_equip_vis(ch,arg,ch->equipment,&pos))!= nullptr)
		{
			unequip_char(ch, pos);
			extract_obj(obj);
			return;
		}

		if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying)) != nullptr )
			extract_obj(obj);

		return;
    }

	else
	{
		for (obj = ch->carrying; obj != nullptr; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (arg[3] == '\0' || isname(arg+4, obj->name))
			{
				extract_obj(obj);
			}
		}

		while ((obj=get_object_in_equip_vis(ch,arg,ch->equipment,&pos)))
		{
			unequip_char(ch, pos);
			extract_obj(obj);
		}   
	}

	return;
}


/* prints the message to everyone in the room other than the mob and victim */
ACMD(do_mechoaround)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *victim;
	char *p;

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char( "Huh?!?\r\n", ch );
		return;
	}

	p = one_argument(argument, arg);
	skip_spaces(&p);
    
	if (!*arg)
	{
		mob_log(ch, "mechoaround called with no argument");
		return;
	}

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "mechoaround: victim (%s) does not exist", arg);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_room_vis(ch, arg)))
	{
		sprintf(buf, "mechoaround: victim (%s) does not exist", arg);
		mob_log(ch, buf);
		return;
	}

	sub_write(p, victim, TRUE, TO_ROOM);
}


/* sends the message to only the victim */
ACMD(do_msend)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *victim;
	char *p;

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char( "Huh?!?\r\n", ch );
		return;
	}

	p = one_argument(argument, arg);
	skip_spaces(&p);

	if (!*arg)
	{
		mob_log(ch, "msend called with no argument");
		return;
	}

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "msend: victim (%s) does not exist",arg);
			mob_log(ch, buf);
			return;
		}
	}
	
	else if (!(victim = get_char_room_vis(ch, arg)))
	{
		sprintf(buf, "msend: victim (%s) does not exist",arg);
		mob_log(ch, buf);
		return;
	}
  
	sub_write(p, victim, TRUE, TO_CHAR);
}


/* prints the message to the room at large */
ACMD(do_mecho)
{
	char *p;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char( "Huh?!?\r\n", ch );
		return;
	}
  
	if (!*argument)
	{
		mob_log(ch, "mecho called with no arguments");
		return;
	}
	
	p = argument;
	skip_spaces(&p);
  
	sub_write(p, ch, TRUE, TO_ROOM);
}


/*
 * lets the mobile load an item or mobile.  All items
 * are loaded into inventory, unless it is NO-TAKE. 
 */
ACMD(do_mload)
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
		per_room[MAX_INPUT_LENGTH], per_zone[MAX_INPUT_LENGTH] ,per_mud[MAX_INPUT_LENGTH];
	
	int number = 0, rnum = 0;
	char_data *mob;
	obj_data *object;


  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if( ch->desc && GET_LEVEL(ch->desc->original) < LVL_IMPL)
		return;
  
	half_chop(argument, arg1, argument);
	half_chop(argument, arg2, argument);
	half_chop(argument, per_room, argument);
	half_chop(argument, per_zone, argument);
	half_chop(argument, per_mud, argument);
  
	if (!*arg1 || !*arg2 || !is_number(arg2) || ((number = atoi(arg2)) < 0))
	{
		mob_log(ch, "mload: bad syntax");
		return;
	}

	rnum = real_mobile(number);

	if (is_abbrev(arg1, "mob"))
	{


		if((*per_room && count_mobs_room(rnum, IN_ROOM(ch)) >= atoi(per_room)) ||
			(*per_zone && count_mobs_zone(rnum, world[IN_ROOM(ch)].zone) >= atoi(per_zone)) ||
			(*per_mud && count_mobs_mud(rnum) >= atoi(per_mud)))
		{

			return;
		}

		if ((mob = read_mobile(number, VIRTUAL)) == nullptr)
		{
			mob_log(ch, "mload: bad mob vnum");
			return;
		}

		char_to_room(mob, IN_ROOM(ch));
		load_mtrigger(mob);
	}

	else if (is_abbrev(arg1, "obj"))
	{
		if ((object = read_object(number, VIRTUAL)) == nullptr)
		{
			mob_log(ch, "mload: bad object vnum");
			return;
		}
	
		if (CAN_WEAR(object, ITEM_WEAR_TAKE))
		{
			obj_to_char(object, ch);
		}

		else
		{
			obj_to_room(object, IN_ROOM(ch));
		}

		if(ch->desc && ch->desc->original)
		{
			sprintf(object->creator, "Mload and Switched: %s on mob %d",
				GET_NAME(ch->desc->original), GET_MOB_VNUM(ch));
		}

		else if(!str_cmp(GET_FORCED(ch), ""))
			sprintf(object->creator, "Forced Mload by %s on mob %d", GET_FORCED(ch), GET_MOB_VNUM(ch));

		else
			sprintf(object->creator, "Mload: Mob %d",
				GET_MOB_VNUM(ch));

		load_otrigger(object);
	}

	else
		mob_log(ch, "mload: bad type");
}


/*
 * lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room.  It can purge
 *  itself, but this will be the last command it does.
 */
ACMD(do_mpurge)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *victim;
	obj_data  *obj;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;

	one_argument(argument, arg);
  
	if (!*arg)
	{
		/* 'purge' */
		char_data *vnext;
		obj_data  *obj_next;

		for (victim = world[IN_ROOM(ch)].people; victim; victim = vnext)
		{
			vnext = victim->next_in_room;

			if (IS_NPC(victim) && victim != ch)
				extract_char(victim);
		}

		for (obj = world[IN_ROOM(ch)].contents; obj; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj);
		}
    
		return;
	}

	if (*arg == UID_CHAR)
		victim = get_char(arg);

	else
		victim = get_char_room_vis(ch, arg);

	if (victim == nullptr)
	{
		if ((obj = get_obj_vis(ch, arg)))
		{
			extract_obj(obj);
		}

		else 
			mob_log(ch, "mpurge: bad argument");
	
		return;
	}
    
	if (!IS_NPC(victim))
	{
		mob_log(ch, "mpurge: purging a PC");
		return;
	}

	if (victim == ch)
		dg_owner_purged = 1;
    
	extract_char(victim);
}


/* lets the mobile goto any location it wishes that is not private */
ACMD(do_mgoto)
{
	char arg[MAX_INPUT_LENGTH];
	sh_int location;
  
	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (!*arg)
	{
		mob_log(ch, "mgoto called with no argument");
		return;
	}

	if ((location = find_target_room(ch, arg)) == NOWHERE)
	{
		mob_log(ch, "mgoto: invalid location");
		return;
	}

	if (FIGHTING(ch))
		stop_fighting(ch);

	char_from_room(ch);
	char_to_room(ch, location);
}


/* lets the mobile do a command at another location. Very useful */
ACMD(do_mat)
{
	char arg[MAX_INPUT_LENGTH];
	sh_int location;
	sh_int original;

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	argument = one_argument( argument, arg );

	if (!*arg || !*argument)
	{
		mob_log(ch, "mat: bad argument");
		return;
	}

	if ((location = find_target_room(ch, arg)) == NOWHERE)
	{
		mob_log(ch, "mat: invalid location");
		return;
	}

	original = IN_ROOM(ch);
	char_from_room(ch);
	char_to_room(ch, location);
	command_interpreter(ch, argument);
  
	/*
	 * See if 'ch' still exists before continuing!
	 * Handles 'at XXXX quit' case.
	 */
	if (IN_ROOM(ch) == location)
	{
		char_from_room(ch);
		char_to_room(ch, original);
	}
}

/*
 * lets the mobile transfer people.  the all argument transfers
 * everyone in the current room to the specified location
 */
ACMD(do_mteleport)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	sh_int target;
	char_data *vict, *next_ch;

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	argument = two_arguments(argument, arg1, arg2);

	if (!*arg1 || !*arg2)
	{
		mob_log(ch, "mteleport: bad syntax");
		return;
	}

	target = find_target_room(ch, arg2);

	if (target == NOWHERE)
		mob_log(ch, "mteleport target is an invalid room");

	else if (!str_cmp(arg1, "all"))
	{
		if (target == IN_ROOM(ch))
		{
			mob_log(ch, "mteleport all target is itself");
			return;
		}

		for (vict = world[IN_ROOM(ch)].people; vict; vict = next_ch)
		{
			next_ch = vict->next_in_room;

			if (GET_LEVEL(vict)<LVL_IMMORT)
			{
				char_from_room(vict);
				char_to_room(vict, target);
			}
		}
	}

	else
	{
		if (*arg1 == UID_CHAR)
		{
			if (!(vict = get_char(arg1)))
			{
				sprintf(buf, "mteleport: victim (%s) does not exist",arg1);
				mob_log(ch, buf);
				return;
			}
		}
		
		else if (!(vict = get_char_vis(ch, arg1)))
		{
			sprintf(buf, "mteleport: victim (%s) does not exist",arg1);
			mob_log(ch, buf);
			return;
		}

		if (GET_LEVEL(vict)<LVL_IMMORT)
		{
			char_from_room(vict);
			char_to_room(vict, target);
		}
	}
}

/*
 * lets the mobile force someone to do something.  must be mortal level
 * and the all argument only affects those in the room with the mobile
 */
ACMD(do_mforce)
{
	char arg[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;

	argument = one_argument(argument, arg);

	if (!*arg || !*argument)
	{
		mob_log(ch, "mforce: bad syntax");
		return;
	}

	if (!str_cmp(arg, "all"))
	{
		struct descriptor_data *i;
		char_data *vch;

		for (i = descriptor_list; i ; i = i->next)
		{
			if ((i->character != ch) && !i->connected &&
			(IN_ROOM(i->character) == IN_ROOM(ch)))
			{
				vch = i->character;
				
				if (GET_LEVEL(vch) < GET_LEVEL(ch) && CAN_SEE(ch, vch) &&
				GET_LEVEL(vch)<LVL_IMMORT)
				{
					command_interpreter(vch, argument);
				}
			}
		}
	}

	else
	{
		char_data *victim;

		if (*arg == UID_CHAR)
		{
			if (!(victim = get_char(arg)))
			{
				sprintf(buf, "mforce: victim (%s) does not exist",arg);
				mob_log(ch, buf);
				return;
			}
		}

		else if ((victim = get_char_vis(ch, arg)) == nullptr)
		{
			mob_log(ch, "mforce: no such victim");
			return;
		}

		if (victim == ch)
		{
			mob_log(ch, "mforce: forcing self");
			return;
		}

		if (GET_LEVEL(victim) < LVL_IMMORT)
		{
			strcpy(GET_FORCED(victim), GET_NAME(ch));
			command_interpreter(victim, argument);
			strcpy(GET_FORCED(victim), "");
		}
			
	}
}


/* increases the target's exp */
ACMD(do_mexp)
{
	char_data *victim;
	char name[MAX_INPUT_LENGTH], amount[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;  

	two_arguments(argument, name, amount);

	if (!*name || !*amount)
	{
		mob_log(ch, "mexp: too few arguments");
		return;
	}

	if (*name == UID_CHAR)
	{
		if (!(victim = get_char(name)))
		{
			sprintf(buf, "mexp: victim (%s) does not exist",name);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_vis(ch, name)))
	{
		sprintf(buf, "mexp: victim (%s) does not exist",name);
		mob_log(ch, buf);
		return;
	}
  
	gain_exp(victim, atoi(amount));      
}


/* increases the target's gold */
ACMD(do_mgold)
{
	char_data *victim;
	char name[MAX_INPUT_LENGTH], amount[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}
  
	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;  

	two_arguments(argument, name, amount);

	if (!*name || !*amount)
	{
		mob_log(ch, "mgold: too few arguments");
		return;
	}

	if (*name == UID_CHAR)
	{
		if (!(victim = get_char(name)))
		{
			sprintf(buf, "mgold: victim (%s) does not exist",name);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_vis(ch, name)))
	{
		sprintf(buf, "mgold: victim (%s) does not exist",name);
		mob_log(ch, buf);
		return;
	}

	if ((GET_GOLD(victim) += atoi(amount)) < 0)
	{
		mob_log(ch, "mgold subtracting more gold than character has");
		GET_GOLD(victim) = 0;
	}
}

/* hunt for someone */
ACMD(do_mhunt)
{
	char_data *victim;
	char arg[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;

	one_argument(argument, arg);

	if (!*arg)
	{
		mob_log(ch, "mhunt called with no argument");
		return;
    }


	if (FIGHTING(ch))
		return;

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "mhunt: victim (%s) does not exist", arg);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_vis(ch, arg)))
	{
		sprintf(buf, "mhunt: victim (%s) does not exist", arg);
		mob_log(ch, buf);
		return;
	}

	HUNTING(ch) = victim;


}

/* place someone into the mob's memory list */
ACMD(do_mremember)
{
	char_data *victim;
	struct script_memory *mem;
	char arg[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;

	argument = one_argument(argument, arg);

	if (!*arg)
	{
		mob_log(ch, "mremember: bad syntax");
		return;
	}

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "mremember: victim (%s) does not exist", arg);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_vis(ch, arg)))
	{
        sprintf(buf, "mremember: victim (%s) does not exist", arg);
		mob_log(ch, buf);
		return;
	}

	/* create a structure and add it to the list */
	CREATE(mem, struct script_memory, 1);

	if (!SCRIPT_MEM(ch))
		SCRIPT_MEM(ch) = mem;
		
	else
	{
		struct script_memory *tmpmem = SCRIPT_MEM(ch);

		while (tmpmem->next)
			tmpmem = tmpmem->next;

		tmpmem->next = mem;
	}

	/* fill in the structure */
	mem->id = GET_ID(victim);

	if (argument && *argument)
	{
		mem->cmd = strdup(argument);
	}
}

/* remove someone from the list */
ACMD(do_mforget)
{
	char_data *victim;
	struct script_memory *mem, *prev;
	char arg[MAX_INPUT_LENGTH];

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc && (GET_LEVEL(ch->desc->original) < LVL_IMPL))
		return;

	one_argument(argument, arg);
  
	if (!*arg)
	{
		mob_log(ch, "mforget: bad syntax");
		return;
	}

	if (*arg == UID_CHAR)
	{
		if (!(victim = get_char(arg)))
		{
			sprintf(buf, "mforget: victim (%s) does not exist", arg);
			mob_log(ch, buf);
			return;
		}
	}

	else if (!(victim = get_char_vis(ch, arg)))
	{
		sprintf(buf, "mforget: victim (%s) does not exist", arg);
		mob_log(ch, buf);
		return;
	}

	mem = SCRIPT_MEM(ch);
	prev = nullptr;

	while (mem)
	{
		if (mem->id == GET_ID(victim))
		{
			if (mem->cmd)
				delete[] (mem->cmd);

			if (prev==nullptr)
			{
				SCRIPT_MEM(ch) = mem->next;
				delete[] (mem);
				mem = SCRIPT_MEM(ch);
			}

			else
			{
				prev->next = mem->next;
				delete[] (mem);
				mem = prev->next;
			}
		}

		else
		{
			prev = mem;
			mem = mem->next;
		}
	}
}

/* transform into a different mobile */
ACMD(do_mtransform)
{
	char arg[MAX_INPUT_LENGTH];
	char_data *m, tmpmob;
	obj_data *obj[NUM_WEARS];
	int pos;

	if (!MOB_OR_IMPL(ch))
	{
		send_to_char("Huh?!?\r\n", ch);
		return;
	}

	if (ch->desc)
	{
		send_to_char("You've got no VNUM to return to, dummy! try 'switch'\r\n",
		ch);
		return;
	}

	one_argument(argument, arg);

	if (!*arg)
		mob_log(ch, "mtransform: missing argument");

	else if (!isdigit(*arg)) 
		mob_log(ch, "mtransform: bad argument");

	else
	{
		m = read_mobile(atoi(arg), VIRTUAL);

		if (m==nullptr)
		{
			mob_log(ch, "mtransform: bad mobile vnum");
			return;
		}

		/* move new obj info over to old object and delete new obj */

		for (pos = 0; pos < NUM_WEARS; pos++)
		{
			if (GET_EQ(ch, pos))
				obj[pos] = unequip_char(ch, pos);

			else
				obj[pos] = nullptr;
		}

		/* put the mob in the same room as ch so extract will work */
		char_to_room(m, IN_ROOM(ch));

		memcpy(&tmpmob, m, sizeof(*m));
		tmpmob.id = ch->id;
		tmpmob.affected = ch->affected;
		tmpmob.carrying = ch->carrying;
		tmpmob.proto_script = ch->proto_script;
		tmpmob.script = ch->script;
		tmpmob.memory = ch->memory;
		tmpmob.next_in_room = ch->next_in_room;
		tmpmob.next = ch->next;
		tmpmob.next_fighting = ch->next_fighting;
		tmpmob.followers = ch->followers;
		tmpmob.master = ch->master;

		GET_WAS_IN(&tmpmob) = GET_WAS_IN(ch);
		GET_HIT(&tmpmob) = GET_HIT(ch);
		GET_MAX_HIT(&tmpmob) = GET_MAX_HIT(ch);
		GET_EXP(&tmpmob) = GET_EXP(ch);
		GET_GOLD(&tmpmob) = GET_GOLD(ch);
		GET_POS(&tmpmob) = GET_POS(ch);
		IS_CARRYING_N(&tmpmob) = IS_CARRYING_N(ch);
		FIGHTING(&tmpmob) = FIGHTING(ch);
		HUNTING(&tmpmob) = HUNTING(ch);
		memcpy(ch, &tmpmob, sizeof(*ch));

		for (pos = 0; pos < NUM_WEARS; pos++)
		{
			if (obj[pos])
				equip_char(ch, obj[pos], pos);
		}

	extract_char(m);
		
	}
}
