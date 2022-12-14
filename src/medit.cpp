/************************************************************************
 * OasisOLC - medit.c						v1.5	*
 * Copyright 1996 Harvey Gilpin.					*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "shop.h"
#include "olc.h"
#include "handler.h"
#include "dg_olc.h"

/*
 * Set this to 1 for debugging logs in medit_save_internally.
 */
#if 0
#define DEBUG
#endif

/*
 * Set this to 1 as a last resort to save mobiles.
 */
#if 0
#define I_CRASH
#endif

/*-------------------------------------------------------------------*/

/*
 * External variable declarations.
 */
extern struct clan_data clan_list[50];
extern struct index_data *mob_index;
extern char_data *mob_proto;
extern struct kit_data *kit_index;
extern struct shop_data *shop_index;
extern struct descriptor_data *descriptor_list;
extern char_data *character_list;
extern struct zone_data *zone_table;
extern struct player_special_data dummy_mob;
extern struct attack_hit_type attack_hit_text[];
extern int top_of_mobt;
extern int top_of_zone_table;
extern char *action_bits[];
extern char *affected_bits[];
extern char *position_types[];
extern char *genders[];
extern char *aggros[];
extern char *class_types[];
extern const char *pc_race_types[];
extern int top_shop;
#if defined(OASIS_MPROG)
extern const char *mobprog_types[];
#endif

/*-------------------------------------------------------------------*/

/*
 * Handy internal macros.
 */

#define GET_NDD(mob) ((mob)->mob_specials.damnodice)
#define GET_SDD(mob) ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)
#define GET_ATTACK(mob) ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop) ((shop)->keeper)
#if defined(OASIS_MPROG)
#define GET_MPROG(mob)		(mob_index[(mob)->nr].mobprogs)
#define GET_MPROG_TYPE(mob)	(mob_index[(mob)->nr].progtypes)
#endif

/*-------------------------------------------------------------------*/

/*
 * Function prototypes.
 */
void medit_clan(struct descriptor_data *d);
void medit_parse(struct descriptor_data *d, char *arg);
void medit_disp_menu(struct descriptor_data *d);
void medit_setup_new(struct descriptor_data *d);
void medit_setup_existing(struct descriptor_data *d, int rmob_num);
void medit_save_internally(struct descriptor_data *d);
void medit_save_to_disk(int zone_num);
void init_mobile(char_data *mob);
void copy_mobile(char_data *tmob, char_data *fmob);
void medit_disp_positions(struct descriptor_data *d);
void medit_disp_mob_flags(struct descriptor_data *d);
void medit_disp_aff_flags(struct descriptor_data *d);
void medit_disp_attack_types(struct descriptor_data *d);
void medit_race(struct descriptor_data *d);
void medit_class(struct descriptor_data *d);
#if defined(OASIS_MPROG)
void medit_disp_mprog(struct descriptor_data *d);
void medit_change_mprog(struct descriptor_data *d);
const char *medit_get_mprog_type(struct mob_prog_data *mprog);
#endif

extern struct kit_data *real_kit(int);

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/

/*
 * Free a mobile structure that has been edited.
 * Take care of existing mobiles and their mob_proto!
 */

void medit_free_mobile(char_data *mob)
{
	int i;
	/*
	 * Non-prototyped mobile.  Also known as new mobiles.
	 */
  
	if (!mob)
		return;
  
	else if (GET_MOB_RNUM(mob) == -1)
	{
    
		if (mob->player.name)
			delete(mob->player.name);
    
		if (mob->player.title)
			delete(mob->player.title);
    
		if (mob->player.short_descr)
			delete(mob->player.short_descr);
    
		if (mob->player.long_descr)
			delete(mob->player.long_descr);

		if (mob->player.description)
			delete(mob->player.description);
	} 
	
	else if ((i = GET_MOB_RNUM(mob)) > -1)
	{	/* Prototyped mobile. */
    
		if (mob->player.name && mob->player.name != mob_proto[i].player.name)
			delete(mob->player.name);
    
		if (mob->player.title && mob->player.title != mob_proto[i].player.title)
			delete(mob->player.title);
    
		if (mob->player.short_descr && mob->player.short_descr != mob_proto[i].player.short_descr)
			delete(mob->player.short_descr);
    
		if (mob->player.long_descr && mob->player.long_descr != mob_proto[i].player.long_descr)
			delete(mob->player.long_descr);
    
		if (mob->player.description && mob->player.description != mob_proto[i].player.description)
			delete(mob->player.description);
	}
  
	while (mob->affected)
		affect_remove(mob, mob->affected);

	delete(mob);
}

void medit_setup_new(struct descriptor_data *d)
{
	char_data *mob;

	/*
	 * Allocate a scratch mobile structure.  
	 */
  
	CREATE(mob, char_data, 1);

	init_mobile(mob);

	GET_MOB_RNUM(mob) = -1;
	/*
	 * Set up some default strings.
	 */
  
	GET_ALIAS(mob) = str_dup("mob unfinished");
	GET_SDESC(mob) = str_dup("the unfinished mob");
	GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
	GET_DDESC(mob) = str_dup("It looks unfinished.\r\n");

#if defined(OASIS_MPROG)
	OLC_MPROGL(d) = nullptr;
	OLC_MPROG(d) = nullptr;
#endif

	OLC_MOB(d) = mob;
	OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */
	OLC_ITEM_TYPE(d) = MOB_TRIGGER;

	medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

void medit_setup_existing(struct descriptor_data *d, int rmob_num)
{
	char_data *mob;
#if defined(OASIS_MPROG)
	MPROG_DATA *temp;
	MPROG_DATA *head;
#endif

	/*
	 * Allocate a scratch mobile structure. 
	 */
  
	CREATE(mob, char_data, 1);

	copy_mobile(mob, mob_proto + rmob_num);

#if defined(OASIS_MPROG)
	/*
	 * I think there needs to be a brace from the if statement to the #endif
	 * according to the way the original patch was indented.  If this crashes,
	 * try it with the braces and report to greerga@van.ml.org on if that works.
	 */
  
	if (GET_MPROG(mob))
		CREATE(OLC_MPROGL(d), MPROG_DATA, 1);
  
	head = OLC_MPROGL(d);
  
	for (temp = GET_MPROG(mob); temp;temp = temp->next)
	{
		OLC_MPROGL(d)->type = temp->type;
		OLC_MPROGL(d)->arglist = str_dup(temp->arglist);
		OLC_MPROGL(d)->comlist = str_dup(temp->comlist);
    
		if (temp->next)
		{
			CREATE(OLC_MPROGL(d)->next, MPROG_DATA, 1);
			OLC_MPROGL(d) = OLC_MPROGL(d)->next;
		}
	}
  
	OLC_MPROGL(d) = head;
	OLC_MPROG(d) = OLC_MPROGL(d);
#endif

	OLC_MOB(d) = mob;
	OLC_ITEM_TYPE(d) = MOB_TRIGGER;
	dg_olc_script_copy(d);
	medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

/*
 * Copy one mob struct to another.
 */
void copy_mobile(char_data *tmob, char_data *fmob)
{
	struct trig_proto_list *proto, *fproto;

	/*
	* Free up any used strings.
	*/
  
	if (GET_ALIAS(tmob))
		delete(GET_ALIAS(tmob));
  
	if (GET_SDESC(tmob))
		delete(GET_SDESC(tmob));
  
	if (GET_LDESC(tmob))
		delete(GET_LDESC(tmob));
  
	if (GET_DDESC(tmob))
		delete(GET_DDESC(tmob));

	/* delete the old script list */
	proto = tmob->proto_script;
  
	while (proto)
	{
		fproto = proto;
		proto = proto->next;
		delete(fproto);
	}	

	/*
	* Copy mob over.
	*/
	*tmob = *fmob;

	/*
	* Reallocate strings.
	*/
	GET_ALIAS(tmob) = str_dup((GET_ALIAS(fmob) && *GET_ALIAS(fmob)) ? GET_ALIAS(fmob) : "undefined");
	GET_SDESC(tmob) = str_dup((GET_SDESC(fmob) && *GET_SDESC(fmob)) ? GET_SDESC(fmob) : "undefined");
	GET_LDESC(tmob) = str_dup((GET_LDESC(fmob) && *GET_LDESC(fmob)) ? GET_LDESC(fmob) : "undefined");
	GET_DDESC(tmob) = str_dup((GET_DDESC(fmob) && *GET_DDESC(fmob)) ? GET_DDESC(fmob) : "undefined");

	/* copy the new script list */
	if (fmob->proto_script)
	{
		fproto = fmob->proto_script;
		CREATE(proto, struct trig_proto_list, 1);
		tmob->proto_script = proto;
    
		do
		{
			proto->vnum = fproto->vnum;
			fproto = fproto->next;
      
			if (fproto)
			{ 
				CREATE(proto->next, struct trig_proto_list, 1);
				proto = proto->next;
			}
		
		} 
		while (fproto);
	}
}

/*-------------------------------------------------------------------*/

/*
 * Ideally, this function should be in db.c, but I'll put it here for
 * portability.
 */
void init_mobile(char_data *mob)
{
	clear_char(mob);

	GET_HIT(mob) = GET_MANA(mob) = 1;
	GET_MAX_MANA(mob) = GET_MAX_MOVE(mob) = 100;
	GET_NDD(mob) = GET_SDD(mob) = 1;
	GET_WEIGHT(mob) = 200;
	GET_HEIGHT(mob) = 198;

	mob->real_abils.str = mob->real_abils.intel = mob->real_abils.wis = 11;
	mob->real_abils.dex = mob->real_abils.con = mob->real_abils.cha = 11;
	mob->aff_abils = mob->real_abils;

	SET_BIT_AR(MOB_FLAGS(mob), MOB_ISNPC);
	mob->player_specials = &dummy_mob;
}

/*-------------------------------------------------------------------*/

#define ZCMD zone_table[zone].cmd[cmd_no]

/*
 * Save new/edited mob to memory.
 */
void medit_save_internally(struct descriptor_data *d)
{
	int rmob_num, found = 0, new_mob_num = 0, zone, cmd_no, shop;
	char_data *new_proto;
	struct index_data *new_index;
	char_data *live_mob;
	struct descriptor_data *dsc;

	/* put the script into proper position */
	OLC_MOB(d)->proto_script = OLC_SCRIPT(d);
	
	/*
	 * Mob exists? Just update it.
	 */
  
	if ((rmob_num = real_mobile(OLC_NUM(d))) != -1) 
	{
		OLC_MOB(d)->proto_script = OLC_SCRIPT(d);
		copy_mobile((mob_proto + rmob_num), OLC_MOB(d));
		/*
		 * Update live mobiles.
		 */
    
		for (live_mob = character_list; live_mob; live_mob = live_mob->next)
			if (IS_MOB(live_mob) && GET_MOB_RNUM(live_mob) == rmob_num)
			{
				/*
				 * Only really need to update the strings, since these can
				 * cause protection faults.  The rest can wait till a reset/reboot.
				 */
	
				GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
				GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
				GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
				GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
			}
	}

	/*
	 * Mob does not exist, we have to add it.
	 */

	else
	{

#if defined(DEBUG)
		fprintf(stderr, "top_of_mobt: %d, new top_of_mobt: %d\n", top_of_mobt, top_of_mobt + 1);
#endif

		CREATE(new_proto, char_data, top_of_mobt + 2);
		CREATE(new_index, struct index_data, top_of_mobt + 2);

		for (rmob_num = 0; rmob_num <= top_of_mobt; rmob_num++)
		{
			if(!found)
			{		/* Is this the place? */

				/*	if ((rmob_num > top_of_mobt) || (mob_index[rmob_num].vnum > OLC_NUM(d))) {*/

				if (mob_index[rmob_num].vnum > OLC_NUM(d))
				{
					/*
					 * Yep, stick it here.
					 */
					found = TRUE;
#if defined(DEBUG)
					fprintf(stderr, "Inserted: rmob_num: %d\n", rmob_num);
#endif
					new_index[rmob_num].vnum = OLC_NUM(d);
					new_index[rmob_num].number = 0;
					new_index[rmob_num].func = nullptr;
					new_mob_num = rmob_num;
					GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
					copy_mobile((new_proto + rmob_num), OLC_MOB(d));

					/*
					 * Copy the mob that should be here on top.
					 */

					new_index[rmob_num + 1] = mob_index[rmob_num];
					new_proto[rmob_num + 1] = mob_proto[rmob_num];
					GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
				}

				else
				{
					/* Nope, copy over as normal. */
					new_index[rmob_num] = mob_index[rmob_num];
					new_proto[rmob_num] = mob_proto[rmob_num];
				}
			}

			else
			{ /* We've already found it, copy the rest over. */
				new_index[rmob_num + 1] = mob_index[rmob_num];
				new_proto[rmob_num + 1] = mob_proto[rmob_num];
				GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
			}
		}
#if defined(DEBUG)
		fprintf(stderr, "rmob_num: %d, top_of_mobt: %d, array size: 0-%d (%d)\n",
		rmob_num, top_of_mobt, top_of_mobt + 1, top_of_mobt + 2);
#endif
		if (!found)
		{
			/* Still not found, must add it to the top of the table. */
#if defined(DEBUG)
			fprintf(stderr, "Append.\n");
#endif
			new_index[rmob_num].vnum = OLC_NUM(d);
			new_index[rmob_num].number = 0;
			new_index[rmob_num].func = nullptr;
			new_mob_num = rmob_num;
			GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
			copy_mobile((new_proto + rmob_num), OLC_MOB(d));
		}

		/*
		 * Replace tables.
		 */
#if defined(DEBUG)
		fprintf(stderr, "Attempted free.\n");
#endif
#if !defined(I_CRASH)
		delete(mob_index);
		delete(mob_proto);
#endif
		mob_index = new_index;
		mob_proto = new_proto;
		top_of_mobt++;
#if defined(DEBUG)
		fprintf(stderr, "Free ok.\n");
#endif

		 /*
		 * Update live mobile rnums.
		 */

		for (live_mob = character_list; live_mob; live_mob = live_mob->next)
			if (GET_MOB_RNUM(live_mob) > new_mob_num)
				GET_MOB_RNUM(live_mob)++;

		/*
		 * Update zone table.
		 */
    
		for (zone = 0; zone <= top_of_zone_table; zone++)
			for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
				if (ZCMD.command == 'M')
					if (ZCMD.arg1 >= new_mob_num)
						ZCMD.arg1++;

		/*
		 * Update shop keepers.
		 */
    
		if (shop_index)
			for (shop = 0; shop <= top_shop; shop++)
 				if (SHOP_KEEPER(shop) >= new_mob_num)
					SHOP_KEEPER(shop)++;

		/*
		 * Update keepers in shops being edited and other mobs being edited.
		 */
    
		for (dsc = descriptor_list; dsc; dsc = dsc->next)
			if (dsc->connected == CON_SEDIT)
			{
				if (S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
					S_KEEPER(OLC_SHOP(dsc))++;
			}
			
			else if (dsc->connected == CON_MEDIT)
			{
				if (GET_MOB_RNUM(OLC_MOB(dsc)) >= new_mob_num)
					GET_MOB_RNUM(OLC_MOB(dsc))++;
			}
	}

#if defined(OASIS_MPROG)
	GET_MPROG(OLC_MOB(d)) = OLC_MPROGL(d);
	GET_MPROG_TYPE(OLC_MOB(d)) = (OLC_MPROGL(d) ? OLC_MPROGL(d)->type : 0);
  
	while (OLC_MPROGL(d))
	{
		GET_MPROG_TYPE(OLC_MOB(d)) |= OLC_MPROGL(d)->type;
		OLC_MPROGL(d) = OLC_MPROGL(d)->next;
	}
#endif

	olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/*-------------------------------------------------------------------*/

/*
 * Save ALL mobiles for a zone to their .mob file, mobs are all 
 * saved in Extended format, regardless of whether they have any
 * extended fields.  Thanks to Sammy for ideas on this bit of code.
 */
void medit_save_to_disk(int zone_num)
{
	int i, rmob_num, zone, top, p = 0;
	FILE *mob_file;
	char fname[64];
	char_data *mob;
	player_clan_data *cl;

#if defined(OASIS_MPROG)
	MPROG_DATA *mob_prog = nullptr;

#endif

	zone = zone_table[zone_num].number;
	top = zone_table[zone_num].top;

	sprintf(fname, "%s/%d.new", MOB_PREFIX, zone);
  
	if (!(mob_file = fopen(fname, "w")))
	{
		mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_BUILDER, TRUE);
		return;
	}

	/*
	 * Seach the database for mobs in this zone and save them.
	 */
  
	for (i = zone * 100; i <= top; i++)
	{
		if ((rmob_num = real_mobile(i)) != -1)
		{
			if (fprintf(mob_file, "#%d\n", i) < 0)
			{
				mudlog("SYSERR: OLC: Cannot write mob file!\r\n", BRF, LVL_BUILDER, TRUE);
				fclose(mob_file);
				return;
			}
      
			mob = (mob_proto + rmob_num);

			/*
			 * Clean up strings.
			 */
      
			cl = mob->clans;

			strcpy(buf1, (GET_LDESC(mob) && *GET_LDESC(mob)) ? GET_LDESC(mob) : "undefined");
			strip_string(buf1);
			strcpy(buf2, (GET_DDESC(mob) && *GET_DDESC(mob)) ? GET_DDESC(mob) : "undefined");
			strip_string(buf2);

			fprintf(mob_file,	"%s~\n"
								"%s~\n"
								"%s~\n"
								"%s~\n"
								"%ld %ld %ld %ld %ld %ld %ld %ld %d E\n"
								"%d %d %d %dd%d+%d %dd%d+%d\n"
								"%d %d\n"
								"%d %d %d %d %d %d %d %d %d %d %d %d\n",
							(GET_ALIAS(mob) && *GET_ALIAS(mob)) ? GET_ALIAS(mob) : "undefined",
							(GET_SDESC(mob) && *GET_SDESC(mob)) ? GET_SDESC(mob) : "undefined",
							buf1, buf2, 
							MOB_FLAGS(mob)[0], MOB_FLAGS(mob)[1], MOB_FLAGS(mob)[2], MOB_FLAGS(mob)[3], 
							AFF_FLAGS(mob)[0], AFF_FLAGS(mob)[1], AFF_FLAGS(mob)[2], MOB_FLAGS(mob)[3], 
							GET_ALIGNMENT(mob),
							GET_LEVEL(mob), 0,
							0, GET_HIT(mob), GET_MANA(mob), GET_MOVE(mob),
							GET_NDD(mob), GET_SDD(mob), GET_DAMROLL(mob), GET_GOLD(mob),
							GET_EXP(mob), GET_POS(mob), GET_DEFAULT_POS(mob), GET_SEX(mob), GET_RACE(mob),
							GET_OB(mob), GET_DB(mob), GET_PB(mob), GET_MAX_MOVE(mob), cl->clan > 0 ? clan_list[cl->clan].name : "None",
							cl ? cl->rank : 0, GET_CLASS(mob), mob->mob_specials.primary_kit ? mob->mob_specials.primary_kit->vnum : 0);
			
			for(p = 0;p < NUM_AGGROS;p++)
				fprintf(mob_file, "%d%s", GET_AGGRO(mob, p), p < 19 ? " " : "");

			fprintf(mob_file, "\n");

							/*
							 * Deal with Extra stats in case they are there.
							 */
     
			if (GET_ATTACK(mob) != 0)
				fprintf(mob_file, "BareHandAttack: %d\n", GET_ATTACK(mob));
      
			if (GET_STR(mob) != 11)
				fprintf(mob_file, "Str: %d\n", GET_STR(mob));
      
			if (GET_ADD(mob) != 0)
				fprintf(mob_file, "StrAdd: %d\n", GET_ADD(mob));
      
			if (GET_DEX(mob) != 11)
				fprintf(mob_file, "Dex: %d\n", GET_DEX(mob));
      
			if (GET_INT(mob) != 11)
				fprintf(mob_file, "Int: %d\n", GET_INT(mob));
      
			if (GET_WIS(mob) != 11)
				fprintf(mob_file, "Wis: %d\n", GET_WIS(mob));
      
			if (GET_CON(mob) != 11)
				fprintf(mob_file, "Con: %d\n", GET_CON(mob));
      
			if (GET_CHA(mob) != 11)
				fprintf(mob_file, "Cha: %d\n", GET_CHA(mob));

			/*
			 * XXX: Add E-mob handlers here.
			 */

			fprintf(mob_file, "E\n");

			script_save_to_disk(mob_file, mob, MOB_TRIGGER);

#if defined(OASIS_MPROG)
	/*
	 * Write out the MobProgs.
	 */
      
	mob_prog = GET_MPROG(mob);
	
	while(mob_prog)
	{
		strcpy(buf1, mob_prog->arglist);
		strip_string(buf1);
		strcpy(buf2, mob_prog->comlist);
		strip_string(buf2);
		fprintf(mob_file, "%s %s~\n%s", medit_get_mprog_type(mob_prog),
					buf1, buf2);
		mob_prog = mob_prog->next;
		fprintf(mob_file, "~\n%s", (!mob_prog ? "|\n" : ""));
     }

#endif
    
		}
	}
  
	fprintf(mob_file, "$\n");
	fclose(mob_file);
	sprintf(buf2, "%s/%d.mob", MOB_PREFIX, zone);
  
	/*
	 * We're fubar'd if we crash between the two lines below.
	 */
  
	remove(buf2);
	rename(fname, buf2);

	olc_remove_from_save_list(zone_table[zone_num].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * Display positions. (sitting, standing, etc)
 */
void medit_disp_positions(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);
#endif
  
	for (i = 0; *position_types[i] != '\n'; i++)
	{
		message(d->character, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
	}
  
	send_to_char("Enter position number : ", d->character);
}

void medit_display_aggros(struct descriptor_data *d)
{

	int i = 0;
	
	get_char_cols(d->character);

	#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);

	#endif


	for(i = 0;aggros[i] != "\n";i++)
		message(d->character, "%d) %s%s%s\r\n", i + 1, grn, aggros[i], nrm);

	message(d->character, "Current aggros:");
	for(i = 0;i < NUM_AGGROS;i++)
		if(GET_AGGRO(OLC_MOB(d), i))
		{

			if(i)
				message(d->character, " ,");

			message(d->character, "%s%s%s", cyn, aggros[i], nrm);
		}
}


void medit_race(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);

#endif
  
	for (i = 0; *pc_race_types[i] != '\n'; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, pc_race_types[i]);
		send_to_char(buf, d->character);
	}

	send_to_char("Enter a race :", d->character);
}

void medit_class(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);

#endif
  
	for (i = 0; *class_types[i] != '\n'; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, class_types[i]);
		send_to_char(buf, d->character);
	}

	send_to_char("Enter a race :", d->character);
}

void medit_clan(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);

#endif
  
	for (i = 1; i <= NUM_CLANS; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, clan_list[i].name);
		send_to_char(buf, d->character);
	}

	send_to_char("Enter a clan :", d->character);
}

/*-------------------------------------------------------------------*/

#if defined(OASIS_MPROG)
/*
 * Get the type of MobProg.
 */
const char *medit_get_mprog_type(struct mob_prog_data *mprog)
{
	switch (mprog->type)
	{
  
	case IN_FILE_PROG:
	  return ">in_file_prog";
	
	case ACT_PROG:
	  return ">act_prog";
	
	case SPEECH_PROG:
	  return ">speech_prog";
	
	case RAND_PROG:
	  return ">rand_prog";
	
	case FIGHT_PROG:
	  return ">fight_prog";
	
	case HITPRCNT_PROG:
	  return ">hitprcnt_prog";
	
	case DEATH_PROG:
	  return ">death_prog";
	
	case ENTRY_PROG:
	  return ">entry_prog";
	
	case GREET_PROG:
	  return ">greet_prog";
	
	case ALL_GREET_PROG:
	  return ">all_greet_prog";
	
	case GIVE_PROG:
	  return ">give_prog";
	
	case BRIBE_PROG:
	  return ">bribe_prog";
	
	}
  
	return ">ERROR_PROG";
}

/*-------------------------------------------------------------------*/

/*
 * Display the MobProgs.
 */
void medit_disp_mprog(struct descriptor_data *d)
{
	struct mob_prog_data *mprog = OLC_MPROGL(d);

	OLC_MTOTAL(d) = 1;

#if defined(CLEAR_SCREEN)
	send_to_char("^[[H^[[J", d->character);
#endif
  
	while (mprog)
	{
		sprintf(buf, "%d) %s %s\r\n", OLC_MTOTAL(d), medit_get_mprog_type(mprog),
		(mprog->arglist ? mprog->arglist : "NONE"));
		send_to_char(buf, d->character);
		OLC_MTOTAL(d)++;
		mprog = mprog->next;
	}
  
	sprintf(buf,	"%d) Create New Mob Prog\r\n"
					"%d) Purge Mob Prog\r\n"
					"Enter number to edit [0 to exit]:  ",
				OLC_MTOTAL(d), OLC_MTOTAL(d) + 1);
  
	send_to_char(buf, d->character);
	OLC_MODE(d) = MEDIT_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProgs.
 */
void medit_change_mprog(struct descriptor_data *d)
{

#if defined(CLEAR_SCREEN)
	send_to_char("^[[H^[[J", d->character);
#endif
	
	sprintf(buf,	"1) Type: %s\r\n"
					"2) Args: %s\r\n"
					"3) Commands:\r\n%s\r\n\r\n"
					"Enter number to edit [0 to exit]: ",
				medit_get_mprog_type(OLC_MPROG(d)),
				(OLC_MPROG(d)->arglist ? OLC_MPROG(d)->arglist: "NONE"),
				(OLC_MPROG(d)->comlist ? OLC_MPROG(d)->comlist : "NONE"));

	send_to_char(buf, d->character);
	OLC_MODE(d) = MEDIT_CHANGE_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProg type.
 */
void medit_disp_mprog_types(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char("^[[H^[[J", d->character);
#endif

	for (i = 0; i < NUM_PROGS-1; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, mobprog_types[i]);
		send_to_char(buf, d->character);
	}
 
	send_to_char("Enter mob prog type : ", d->character);
	OLC_MODE(d) = MEDIT_MPROG_TYPE;
}
#endif

/*-------------------------------------------------------------------*/

/*
 * Display the gender of the mobile.
 */
void medit_disp_sex(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);
#endif
  
	for (i = 0; i < NUM_GENDERS; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
		send_to_char(buf, d->character);
	}
  
	send_to_char("Enter gender number : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display attack types menu.
 */
void medit_disp_attack_types(struct descriptor_data *d)
{
	int i;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);
#endif
  
	for (i = 0; i < NUM_ATTACK_TYPES; i++)
	{
		sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, attack_hit_text[i].singular);
		send_to_char(buf, d->character);
	}
  
	send_to_char("Enter attack type : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display mob-flags menu.
 */
void medit_disp_mob_flags(struct descriptor_data *d)
{
	int i, columns = 0;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char(".[H.[J", d->character);
#endif
  
	for (i = 0; i < NUM_MOB_FLAGS; i++)
	{
		sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, action_bits[i],
		!(++columns % 2) ? "\r\n" : "");
		send_to_char(buf, d->character);
	}
  
	sprintbitarray( (int *) MOB_FLAGS(OLC_MOB(d)), (char **) action_bits, PM_ARRAY_MAX, buf1);
	sprintf(buf, "\r\nCurrent flags : %s%s%s\r\nEnter mob flags (0 to quit) : ",
		cyn, buf1, nrm);
	send_to_char(buf, d->character);
}

/*
 * Display affection flags menu.
 */
void medit_disp_aff_flags(struct descriptor_data *d)
{
	int i, columns = 0;

	get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
	send_to_char("[H[J", d->character);
#endif
  
	for (i = 0; i < NUM_AFF_FLAGS; i++)
	{
		sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, affected_bits[i+1],
		!(++columns % 2) ? "\r\n" : "");
		send_to_char(buf, d->character);
	}
  
	sprintbitarray(AFF_FLAGS(OLC_MOB(d)), (char **) affected_bits, AF_ARRAY_MAX, buf1);
	sprintf(buf, "\r\nCurrent flags   : %s%s%s\r\nEnter aff flags (0 to quit) : ",
		cyn, buf1, nrm);
	send_to_char(buf, d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display main menu.
 */
void medit_disp_menu(struct descriptor_data *d)
{
	char_data *mob;
	char aggro_list[MAX_STRING_LENGTH];
	int i = 0;
	
	mob = OLC_MOB(d);
	get_char_cols(d->character);


	aggro_list[0] = '\0';
	for(i = 0;aggros[i] && aggros[i] != "\n";i++)
		if(GET_AGGRO(mob, i))
			sprintf(aggro_list + strlen(aggro_list), "%s ", aggros[i]);

	sprintf(buf,

#if defined(CLEAR_SCREEN)
	"[H[J"
#endif
	  
	"-- Mob Number:  [%s%d%s]\r\n"
	"%s1%s) Sex: %s%-7.7s%s	        %s2%s) Alias: %s%s\r\n"
	"%s3%s) S-Desc: %s%s\r\n"
	"%s4%s) L-Desc:-\r\n%s%s"
	"%s5%s) D-Desc:-\r\n%s%s"
	"%s6%s) Level:       [%s%4d%s], %s7%s) Offensive:    [%s%4d%s]\r\n"
	"%s8%s) Dodge:       [%s%4d%s], %s9%s) Parry:        [%s%4d%s]\r\n"
	"%sA%s) NumDamDice:  [%s%4d%s], %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
	"%sC%s) Num HP Dice: [%s%4d%s], %sD%s) Size HP Dice: [%s%4d%s],  %sE%s) HP Bonus:  [%s%5d%s]\r\n"
	"%sF%s) Damroll: [%s%8d%s], %sG%s) Exp:      [%s%8d%s],  %sH%s) Gold:   [%s%8d%s]\r\n",

		cyn, OLC_NUM(d), nrm,
		grn, nrm, yel, genders[(int)GET_SEX(mob)], nrm,
		grn, nrm, yel, GET_ALIAS(mob),
		grn, nrm, yel, GET_SDESC(mob),
		grn, nrm, yel, GET_LDESC(mob),
		grn, nrm, yel, GET_DDESC(mob),
		grn, nrm, cyn, GET_LEVEL(mob), nrm,
		grn, nrm, cyn, GET_OB(mob), nrm,
		grn, nrm, cyn, GET_DB(mob), nrm,
		grn, nrm, cyn, GET_PB(mob), nrm,
		grn, nrm, cyn, GET_NDD(mob), nrm,
		grn, nrm, cyn, GET_SDD(mob), nrm,
		grn, nrm, cyn, GET_HIT(mob), nrm,
		grn, nrm, cyn, GET_MANA(mob), nrm,
		grn, nrm, cyn, GET_MOVE(mob), nrm,
		grn, nrm, cyn, GET_DAMROLL(mob), nrm,
		grn, nrm, cyn, GET_EXP(mob), nrm,
		grn, nrm, cyn, GET_GOLD(mob), nrm
	);
  
	send_to_char(buf, d->character);

	sprintbitarray( (int *) MOB_FLAGS(mob), (char **) action_bits, PM_ARRAY_MAX, buf1);
	sprintbitarray( (int *) AFF_FLAGS(mob), (char **) affected_bits, AF_ARRAY_MAX, buf2);
	sprintf(buf,
	  
	"%sI%s) Position  : %s%s\r\n"
	"%sJ%s) Default   : %s%s\r\n"
	"%sK%s) Attack    : %s%s\r\n"
	"%sL%s) NPC Flags : %s%s\r\n"
	"%sM%s) AFF Flags : %s%s\r\n"
	"%sN%s) Mob Kit   : %s%s\r\n"
	"%sR%s) Mob Race  : %s%s\r\n"
	"%sT%s) Mob Moves : %s%d\r\n"		
	"%sU%s) Mob Clan  : %s%s\r\n"
	"%sV%s) Mob Rank  : %s%d\r\n"
	"%sW%s) Mob Aggros: %s%s\r\n"
	"%sX%s) Mob Class : %s%s\r\n"

#if defined(OASIS_MPROG)
	"%sP%s) Mob Progs : %s%s\r\n"
#endif
          
	"%sS%s) Script    : %s%s\r\n"
	"%sQ%s) Quit\r\n"
	"Enter choice : ",

		grn, nrm, yel, position_types[(int)GET_POS(mob)],
		grn, nrm, yel, position_types[(int)GET_DEFAULT_POS(mob)],
		grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
		grn, nrm, cyn, buf1,
		grn, nrm, cyn, buf2,
		grn, nrm, cyn, mob->mob_specials.primary_kit ? mob->mob_specials.primary_kit->name : "Not set.", /* kits */
		grn, nrm, cyn, pc_race_types[(int) GET_RACE(mob)],
		grn, nrm, cyn, GET_MAX_MOVE(mob),
		grn, nrm, cyn, mob->clans ? clan_list[mob->clans->clan].name : "NONE",
		grn, nrm, cyn, mob->clans ? mob->clans->rank : 0,
		grn, nrm, cyn, aggro_list ? aggro_list : "NONE",
		grn, nrm, cyn, class_types[(int) GET_CLASS(mob)],

#if defined(OASIS_MPROG)
		grn, nrm, cyn, (OLC_MPROGL(d) ? "Set." : "Not Set."),
#endif
		grn, nrm, cyn, mob->proto_script?"Set.":"Not Set.",
		grn, nrm
	);
  
	send_to_char(buf, d->character);
//	sprintf(aggro_list, "");
	*aggro_list = '\0';

	OLC_MODE(d) = MEDIT_MAIN_MENU;
}

/************************************************************************
 *			The GARGANTAUN event handler			*
 ************************************************************************/

void medit_parse(struct descriptor_data *d, char *arg)
{
	int i;
	player_clan_data *cl;

	if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE)
	{
		if (!*arg || (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1])))))
		{
			send_to_char("Field must be numerical, try again : ", d->character);
			return;
		}
	}
  
	switch (OLC_MODE(d))
	{
/*-------------------------------------------------------------------*/
	case MEDIT_CONFIRM_SAVESTRING:
		/*
		 * Ensure mob has MOB_ISNPC set or things will go pair shaped.
		 */
    
		SET_BIT_AR(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
		switch (*arg)
		{
	
		case 'y':
		case 'Y':
			/*
			 * Save the mob in memory and to disk.
			 */
      
			send_to_char("Saving mobile to memory.\r\n", d->character);
			medit_save_internally(d);
			sprintf(buf, "OLC: %s edits mob %d", GET_NAME(d->character), OLC_NUM(d));
			mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
			/* FALL THROUGH */

		case 'n':
		case 'N':
			cleanup_olc(d, CLEANUP_ALL);
			return;

		default:
			send_to_char("Invalid choice!\r\n", d->character);
			send_to_char("Do you wish to save the mobile? : ", d->character);
			return;
		}

		break;

/*-------------------------------------------------------------------*/
	case MEDIT_MAIN_MENU:
		i = 0;
    
		switch (*arg) {
			case 'q':
			case 'Q':
				if (OLC_VAL(d)) {	/* Anything been changed? */
					send_to_char("Do you wish to save the changes to the mobile? (y/n) : ", d->character);
					OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
				} 
				
				else
					cleanup_olc(d, CLEANUP_ALL);
      
				return;
    
			case '1':
				OLC_MODE(d) = MEDIT_SEX;
				medit_disp_sex(d);
				return;
    
			case '2':
				OLC_MODE(d) = MEDIT_ALIAS;
				i--;
				break;
    
			case '3':
				OLC_MODE(d) = MEDIT_S_DESC;
				i--;
				break;
    
			case '4':
				OLC_MODE(d) = MEDIT_L_DESC;
				i--;
				break;
    
			case '5':
				OLC_MODE(d) = MEDIT_D_DESC;
				SEND_TO_Q("Enter mob description: (/s saves /h for help)\r\n\r\n", d);
				d->backstr = nullptr;
      
				if (OLC_MOB(d)->player.description) {
					SEND_TO_Q(OLC_MOB(d)->player.description, d);
					d->backstr = str_dup(OLC_MOB(d)->player.description);
				}
      
				d->str = &OLC_MOB(d)->player.description;
				d->max_str = MAX_MOB_DESC;
				d->mail_to = 0;
				OLC_VAL(d) = 1;
				return;
   
			case '6':
				OLC_MODE(d) = MEDIT_LEVEL;
				i++;
				break;
    
			case '7':
				OLC_MODE(d) = MEDIT_OB;
				i++;
				break;
    
			case '8':
				OLC_MODE(d) = MEDIT_DB;
				i++;
				break;
	
			case '9':
				OLC_MODE(d) = MEDIT_PB;
				i++;
				break;
    
			case 'a':
			case 'A':
				OLC_MODE(d) = MEDIT_NDD;
				i++;
				break;
    
			case 'b':
			case 'B':
				OLC_MODE(d) = MEDIT_SDD;
				i++;
				break;
			
			case 'c':
			case 'C':
				OLC_MODE(d) = MEDIT_NUM_HP_DICE;
				i++;
				break;
    
			case 'd':
			case 'D':
				OLC_MODE(d) = MEDIT_SIZE_HP_DICE;
				i++;
				break;
    
			case 'e':
			case 'E':
				OLC_MODE(d) = MEDIT_ADD_HP;
				i++;
				break;
    
			case 'f':
			case 'F':
				OLC_MODE(d) = MEDIT_DAMROLL;
				i++;
				break;
    
			case 'g':
			case 'G':
				OLC_MODE(d) = MEDIT_EXP;
				i++;
				break;
    
			case 'h':
			case 'H':
				OLC_MODE(d) = MEDIT_GOLD;
				i++;
				break;
    
			case 'i':
			case 'I':
				OLC_MODE(d) = MEDIT_POS;
				medit_disp_positions(d);
				return;
    
			case 'j':
			case 'J':
				OLC_MODE(d) = MEDIT_DEFAULT_POS;
				medit_disp_positions(d);
				return;
    
			case 'k':
			case 'K':
				OLC_MODE(d) = MEDIT_ATTACK;
				medit_disp_attack_types(d);
				return;
    
			case 'l':
			case 'L':
				OLC_MODE(d) = MEDIT_NPC_FLAGS;
				medit_disp_mob_flags(d);
				return;
    
			case 'm':
			case 'M':
				OLC_MODE(d) = MEDIT_AFF_FLAGS;
				medit_disp_aff_flags(d);
				return;
	
			case 'n':
			case 'N':
				OLC_MODE(d) = MEDIT_KITS;
				send_to_char("Enter the number of a kit for this mob (0 for nothing) : ", d->character);
				return;

			case 'r':
			case 'R':
				OLC_MODE(d) = MEDIT_RACE;
				medit_race(d);
				return;
	
			case 't':
			case 'T':
				OLC_MODE(d) = MEDIT_MOVES;
				i++;
				break;
	
			case 'u':
			case 'U':
				OLC_MODE(d) = MEDIT_CLANS;
				medit_clan(d);
				return;
	
			case 'v':
			case 'V':
				OLC_MODE(d) = MEDIT_RANK;
				i++;
				break;

			case 'w':
			case 'W':
				OLC_MODE(d) = MEDIT_AGGROS;
				medit_display_aggros(d);
				return;

			case 'x':
			case 'X':
				OLC_MODE(d) = MEDIT_CLASS;
				medit_class(d);
				return;

#if defined(OASIS_MPROG)
			
			case 'p':
			case 'P':
				OLC_MODE(d) = MEDIT_MPROG;
				medit_disp_mprog(d);
				return;
#endif
    
			case 's':
			case 'S':
				OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
				dg_script_menu(d);
				return;
    
			default:
				medit_disp_menu(d);
				return;
		}
    
		if (i != 0)
		{
			send_to_char(i == 1 ? "\r\nEnter new value : " :
			i == -1 ? "\r\nEnter new text :\r\n] " :
			"\r\nOops...:\r\n", d->character);
      
			return;
		}
  
		break;

/*-------------------------------------------------------------------*/
			case OLC_SCRIPT_EDIT:
				if (dg_script_edit_parse(d, arg)) return;
				break;
/*-------------------------------------------------------------------*/
			case MEDIT_ALIAS:
				if (GET_ALIAS(OLC_MOB(d)))
					delete(GET_ALIAS(OLC_MOB(d)));
				
				GET_ALIAS(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
				break;
/*-------------------------------------------------------------------*/
			case MEDIT_S_DESC:
				if (GET_SDESC(OLC_MOB(d)))
					delete[] (GET_SDESC(OLC_MOB(d)));
					GET_SDESC(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
					break;
/*-------------------------------------------------------------------*/
			case MEDIT_L_DESC:
				if (GET_LDESC(OLC_MOB(d)))
					delete[] (GET_LDESC(OLC_MOB(d)));
				
				if (arg && *arg)
				{
					strcpy(buf, arg);
					strcat(buf, "\r\n");
					GET_LDESC(OLC_MOB(d)) = str_dup(buf);
				} 
				
				else
					GET_LDESC(OLC_MOB(d)) = str_dup("undefined");

				break;
/*-------------------------------------------------------------------*/
			case MEDIT_D_DESC:
    /*
     * We should never get here.
     */
				cleanup_olc(d, CLEANUP_ALL);
				mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!",
				BRF, LVL_BUILDER, TRUE);
				send_to_char("Oops...\r\n", d->character);
				break;
/*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
			case MEDIT_MPROG_COMLIST:
    /*
     * We should never get here, but if we do, bail out.
     */
				cleanup_olc(d, CLEANUP_ALL);
				mudlog("SYSERR: OLC: medit_parse(): Reached MPROG_COMLIST case!",
				BRF, LVL_BUILDER, TRUE);
				break;
#endif
/*-------------------------------------------------------------------*/
			case MEDIT_NPC_FLAGS:
				if ((i = atoi(arg)) == 0)
					break;
    
				else if (!((i < 0) || (i > NUM_MOB_FLAGS)))
					TOGGLE_BIT_AR(MOB_FLAGS(OLC_MOB(d)), (i - 1));
				
				medit_disp_mob_flags(d);
				return;
/*-------------------------------------------------------------------*/
			case MEDIT_AFF_FLAGS:
				if ((i = atoi(arg)) == 0)
					break;
    
				else if (!((i < 0) || (i > NUM_AFF_FLAGS)))
					TOGGLE_BIT_AR(AFF_FLAGS(OLC_MOB(d)), (i));
    
				medit_disp_aff_flags(d);
				return;

			case MEDIT_KITS:
				if ( (i = atoi(arg)) == 0)
				{
					OLC_MOB(d)->mob_specials.primary_kit = nullptr;
					break;
				}
				else
				{
					if ( (OLC_MOB(d)->mob_specials.primary_kit = real_kit(i)) == nullptr)
						send_to_char("That kit does not exist!  What kit? (0 for nothing) : ", d->character);

					else
						break;
				}
				return;

			case MEDIT_AGGROS:

				i = atoi(arg);

				if(i <= 0 || i > NUM_AGGROS)
					break;

				else
				{
					i--;

					if(GET_AGGRO(OLC_MOB(d), i))
						REMOVE_AGGRO(OLC_MOB(d), i);

					else
						SET_AGGRO(OLC_MOB(d), i);
					
					medit_display_aggros(d);
					return;
				}

				break;


			case MEDIT_CLANS:
				i = atoi(arg);

				if (i <= 0 || i > NUM_CLANS)
				{
					write_to_output(d, "That is not a valid option.\r\n");
					break;
				}

				else
				{

					if(OLC_MOB(d)->clans)
					{
						OLC_MOB(d)->clans->clan = i;
					}
					else
					{
						CREATE(OLC_MOB(d)->clans, player_clan_data, 1);
						OLC_MOB(d)->clans->clan = i;

						/* Mobs should NOT have their clans->next allocated */
						OLC_MOB(d)->clans->next = nullptr;
					}
					break;
				}
	  


/*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
			
			case MEDIT_MPROG:
				if ((i = atoi(arg)) == 0)
					medit_disp_menu(d);
    
				else if (i == OLC_MTOTAL(d))
				{
					struct mob_prog_data *temp;
					CREATE(temp, struct mob_prog_data, 1);
					temp->next = OLC_MPROGL(d);
					temp->type = -1;
					temp->arglist = nullptr;
					temp->comlist = nullptr;
					OLC_MPROG(d) = temp;
					OLC_MPROGL(d) = temp;
					OLC_MODE(d) = MEDIT_CHANGE_MPROG;
					medit_change_mprog (d);
				} 
				
				else if (i < OLC_MTOTAL(d))
				{
					struct mob_prog_data *temp;
					int x = 1;
					
					for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
						x++;

					OLC_MPROG(d) = temp;
					OLC_MODE(d) = MEDIT_CHANGE_MPROG;
					medit_change_mprog (d);
				} 

				else if (i == OLC_MTOTAL(d) + 1)
				{
					send_to_char("Which mob prog do you want to purge? ", d->character);
					OLC_MODE(d) = MEDIT_PURGE_MPROG;
				} 
				
				else
					medit_disp_menu(d);
    
				return;
  
			case MEDIT_PURGE_MPROG:
				if ((i = atoi(arg)) > 0 && i < OLC_MTOTAL(d))
				{
					struct mob_prog_data *temp;
					int x = 1;

      
					for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
						x++;
      
					OLC_MPROG(d) = temp;
					REMOVE_FROM_LIST(OLC_MPROG(d), OLC_MPROGL(d), next);
					delete[] (OLC_MPROG(d)->arglist);
					delete[] (OLC_MPROG(d)->comlist);
					delete[] (OLC_MPROG(d));
					OLC_MPROG(d) = nullptr;
					OLC_VAL(d) = 1;
				}
    
				medit_disp_mprog(d);
				return;

			case MEDIT_CHANGE_MPROG:
				{
				
				if ((i = atoi(arg)) == 1)
					medit_disp_mprog_types(d);
    
				else if (i == 2)
				{
					send_to_char ("Enter new arg list: ", d->character);
					OLC_MODE(d) = MEDIT_MPROG_ARGS;
				} 
				
				else if (i == 3)
				{
					send_to_char("Enter new mob prog commands:\r\n", d->character);
      
				   /*
					* Pass control to modify.c for typing.
					*/
      
					OLC_MODE(d) = MEDIT_MPROG_COMLIST;
					d->backstr = nullptr;
					
					if (OLC_MPROG(d)->comlist) {
						SEND_TO_Q(OLC_MPROG(d)->comlist, d);
						d->backstr = str_dup(OLC_MPROG(d)->comlist);
					}

					d->str = &OLC_MPROG(d)->comlist;
					d->max_str = MAX_STRING_LENGTH;
					d->mail_to = 0;
					OLC_VAL(d) = 1;
				} 
				
				else
					medit_disp_mprog(d);
    
				return;
#endif

/*-------------------------------------------------------------------*/

/*
 * Numerical responses.
 */

#if defined(OASIS_MPROG)
	/*
	David Klasinc suggests for MEDIT_MPROG_TYPE:
	switch (atoi(arg)) {
		case 0: OLC_MPROG(d)->type = 0; break;
		case 1: OLC_MPROG(d)->type = 1; break;
		case 2: OLC_MPROG(d)->type = 2; break;
		case 3: OLC_MPROG(d)->type = 4; break;
		case 4: OLC_MPROG(d)->type = 8; break;
		case 5: OLC_MPROG(d)->type = 16; break;
		case 6: OLC_MPROG(d)->type = 32; break;
		case 7: OLC_MPROG(d)->type = 64; break;
		case 8: OLC_MPROG(d)->type = 128; break;
		case 9: OLC_MPROG(d)->type = 256; break;
		case 10: OLC_MPROG(d)->type = 512; break;
		case 11: OLC_MPROG(d)->type = 1024; break;
		default: OLC_MPROG(d)->type = -1; break;
		}
	*/

			case MEDIT_MPROG_TYPE:
				OLC_MPROG(d)->type = (1 << MAX(0, MIN(atoi(arg), NUM_PROGS - 1)));
				OLC_VAL(d) = 1;
				medit_change_mprog(d);
				return;

  
			case MEDIT_MPROG_ARGS:
				OLC_MPROG(d)->arglist = str_dup(arg);
				OLC_VAL(d) = 1;
				medit_change_mprog(d);
				return;
#endif

			case MEDIT_SEX:
				GET_SEX(OLC_MOB(d)) = MAX(0, MIN(NUM_GENDERS - 1, atoi(arg)));
				break;

			case MEDIT_OB:
				GET_OB(OLC_MOB(d)) = MAX(-200, MIN(10000, atoi(arg)));
				break;
	
			case MEDIT_DB:
				GET_DB(OLC_MOB(d)) = MAX(-200, MIN(10000, atoi(arg)));
				break;

			case MEDIT_DAMROLL:
				GET_DAMROLL(OLC_MOB(d)) = MAX(0, MIN(50, atoi(arg)));
				break;

			case MEDIT_NDD:
				GET_NDD(OLC_MOB(d)) = MAX(0, MIN(30, atoi(arg)));
				break;

			case MEDIT_SDD:
				GET_SDD(OLC_MOB(d)) = MAX(0, MIN(127, atoi(arg)));
				break;

			case MEDIT_NUM_HP_DICE:
				GET_HIT(OLC_MOB(d)) = MAX(0, MIN(30, atoi(arg)));
				break;

			case MEDIT_SIZE_HP_DICE:
				GET_MANA(OLC_MOB(d)) = MAX(0, MIN(1000, atoi(arg)));
				break;

			case MEDIT_ADD_HP:
				GET_MOVE(OLC_MOB(d)) = MAX(0, MIN(30000, atoi(arg)));
				break;

			case MEDIT_PB:
				GET_PB(OLC_MOB(d)) = MAX(-200, MIN(200, atoi(arg)));
				break;
  
			case MEDIT_MOVES:
				GET_MAX_MOVE(OLC_MOB(d)) = MAX(0, MIN(3000, atoi(arg)));
				break;


			case MEDIT_EXP:
				GET_EXP(OLC_MOB(d)) = MAX(0, atoi(arg));
				break;

			case MEDIT_GOLD:
				GET_GOLD(OLC_MOB(d)) = MAX(0, atoi(arg));
				break;

			case MEDIT_POS:
				GET_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
				break;

			case MEDIT_DEFAULT_POS:
				GET_DEFAULT_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
				break;

			case MEDIT_ATTACK:
				GET_ATTACK(OLC_MOB(d)) = MAX(0, MIN(NUM_ATTACK_TYPES - 1, atoi(arg)));
				break;

			case MEDIT_LEVEL:
				GET_LEVEL(OLC_MOB(d)) = MAX(1, MIN(100, atoi(arg)));
				break;
  
			case MEDIT_RACE:
				GET_RACE(OLC_MOB(d)) = MAX(0, MIN(NUM_RACES, atoi(arg)));
				break;

			case MEDIT_RANK:
				//if(OLC_MOB(d)->clans)
				//	OLC_MOB(d)->rank = MAX(0, MIN(10, atoi(arg)));
				break;

			case MEDIT_CLASS:
				GET_CLASS(OLC_MOB(d)) = MAX(0, MIN(NUM_CLASSES, atoi(arg)));
				break;
/*-------------------------------------------------------------------*/
			default:
			   /*
				* We should never get here.
				*/
				cleanup_olc(d, CLEANUP_ALL);
				mudlog("SYSERR: OLC: medit_parse(): Reached default case!", BRF, LVL_BUILDER, TRUE);
				send_to_char("Oops...\r\n", d->character);
				break;
		}
/*-------------------------------------------------------------------*/

	   /*
		* END OF CASE 
		* If we get here, we have probably changed something, and now want to
		* return to main menu.  Use OLC_VAL as a 'has changed' flag  
		*/

		OLC_VAL(d) = 1;
		medit_disp_menu(d);
}
/*
 * End of medit_parse(), thank god.
 */
