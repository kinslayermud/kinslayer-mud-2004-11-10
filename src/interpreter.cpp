/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "olc.h"
#include "dg_scripts.h"

extern sh_int r_human_start_room;
extern sh_int r_trolloc_start_room;
extern sh_int r_immort_start_room;
extern sh_int r_frozen_start_room;
extern const char *human_class_menu;
extern const char *other_class_menu;
extern const char *race_menu;
extern char *motd;
extern char *imotd;
extern char *background;
extern const char *MENU;
extern const char *WELC_MESSG;
extern const char *START_MESSG;
extern char_data *character_list;
extern descriptor_data *descriptor_list;
extern player_index_element *player_table;
extern int circle_restrict;
extern int no_specials;
extern int max_bad_pws;
extern int top_of_world;
extern index_data *mob_index;
extern index_data *obj_index;
extern room_data *world;
extern memory_data *mlist;

extern int boot_high;

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(char_data *ch);
int parse_class(char arg);
int special(char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
int	search_block(char *arg, char **listy, int exact);
void oedit_parse(struct descriptor_data *d, char *arg);
void redit_parse(struct descriptor_data *d, char *arg);
void hedit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void trigedit_parse(struct descriptor_data *d, char *arg);
void kedit_parse(struct descriptor_data *d, char *arg);
void aedit_parse(struct descriptor_data *d, char *arg);
void read_aliases(char_data *ch);
void delete_aliases(const char *charname);
void roll_real_abils(char_data *ch);
void roll_taveren(char_data *ch);
int Crash_load(char_data *ch, int show);

/* local functions */
int perform_dupe_check(struct descriptor_data *d);
struct alias *find_alias(struct alias *alias_list, char *str);
void free_alias(struct alias *a);
void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a);
void perform_flee(char_data *ch);
int perform_alias(struct descriptor_data *d, char *orig);
int reserved_word(char *argument);
int find_name(char *name, player_index_element *ptable, int ptable_top);
int _parse_name(char *arg, char *name);

extern FILE *player_fl;

/* prototypes for all do_x functions. */
ACMD(do_action);
ACMD(do_advance);
ACMD(do_alias);
ACMD(do_assist);
ACMD(do_at);
ACMD(do_award);
ACMD(do_awardd);
ACMD(do_awardh);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_bash);
ACMD(do_battle);
ACMD(do_buffer);
ACMD(do_butcher);
ACMD(do_scalp);
ACMD(do_cast);
ACMD(do_charge);
ACMD(do_clan);
ACMD(do_clans);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_consider);
ACMD(do_copy);
ACMD(do_copyover);
ACMD(do_countdown);
ACMD(do_council);
ACMD(do_cqpurge);
ACMD(do_credits);
ACMD(do_diceroll);
ACMD(do_dig);
ACMD(do_disable);
ACMD(do_dismount);
ACMD(do_demote);
ACMD(do_drink);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_diagnose);
ACMD(do_display);
ACMD(do_drop);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_effuse);
ACMD(do_enable);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_extra);
ACMD(do_fade);
ACMD(do_flee);
ACMD(do_flip);
ACMD(do_find);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_ignore);
ACMD(do_incognito);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_ipfind);
ACMD(do_kick);
ACMD(do_kill);
ACMD(do_klist);
ACMD(do_lag);
ACMD(do_last);
ACMD(do_lead);
ACMD(do_leave);
ACMD(do_legends);
ACMD(do_legupdate);
ACMD(do_levels);
// ACMD(do_load);
ACMD(do_look);
ACMD(do_mark);
ACMD(do_mbload);
ACMD(do_memory);
/* ACMD(do_move); -- interpreter.h */
ACMD(do_not_here);
ACMD(do_notice);
ACMD(do_note);
ACMD(do_offer);
ACMD(do_oload);
ACMD(do_olc);
ACMD(do_order);
ACMD(do_overflow);
ACMD(do_pardon);
ACMD(do_page);
ACMD(do_peace);
ACMD(do_pfind);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_practice);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_quit);
ACMD(do_qval);
ACMD(do_rank);
ACMD(do_reboot);
ACMD(do_release);
ACMD(do_remove);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_reroll);
ACMD(do_rescue);
ACMD(do_reset);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_ride);
ACMD(do_rlink);
ACMD(do_say);
ACMD(do_speak);
ACMD(do_save);
ACMD(do_scan);
ACMD(do_score);
ACMD(do_search);
ACMD(do_self_delete);
ACMD(do_send);
ACMD(do_sense);
ACMD(do_set);
ACMD(do_shoot);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_sleep);
ACMD(do_slist);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_source);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_statfind);
ACMD(do_steal);
ACMD(do_swap);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_tedit);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_tell_mute);
ACMD(do_time);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_unban);
ACMD(do_unbond);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_visible);
ACMD(do_view);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_wake);
ACMD(do_warn);
ACMD(do_warrant);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_weaves);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizdelete);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wiznoise);
ACMD(do_wizutil);
ACMD(do_wizview);
ACMD(do_write);
ACMD(do_zap);
ACMD(do_zcmd_find);
ACMD(do_zreset);
ACMD(do_mlist);
//ACMD(do_saveall);

/**************************
 * Our Commands
 *************************/
ACMD(do_saveolc);
ACMD(do_stat);
ACMD(do_whois);
ACMD(do_change);

/* DG Script ACMD's */
ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tlist);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_damage);
ACMD(do_mechoaround);
ACMD(do_mhorse_purge);
ACMD(do_mqval);
ACMD(do_mdamage);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_mteleport);
ACMD(do_mforce);
ACMD(do_mexp);
ACMD(do_mgold);
ACMD(do_mhunt);
ACMD(do_mstage);
ACMD(do_mremember);
ACMD(do_mreset);
ACMD(do_mforget);
ACMD(do_mtransform);
ACMD(do_mzreset);

struct command_info *complete_cmd_info;

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

struct command_info cmd_info[] = {
	{	"RESERVED"	, "RESERVED"	, 0, 0, 0, 0, 0.0 },	/* this must be first -- for specprocs */

	/* directions must come before other commands but after RESERVED */
	{	"north"		, "n"		, POS_STANDING, do_move     , 0, SCMD_NORTH, 0.0},
	{	"east"		, "e"		, POS_STANDING, do_move     , 0, SCMD_EAST, 0.0},
	{	"south"		, "s"		, POS_STANDING, do_move     , 0, SCMD_SOUTH, 0.0 },
	{	"west"		, "w"		, POS_STANDING, do_move     , 0, SCMD_WEST, 0.0 },
	{	"up"		, "u"		, POS_STANDING, do_move     , 0, SCMD_UP, 0.0 },
	{	"down"		, "d"		, POS_STANDING, do_move     , 0, SCMD_DOWN, 0.0 },

	/* now, the main list */
	{	"at"		, "at"		, POS_DEAD    , do_at       , LVL_APPR, 0, 0.0 },
	{	"advance"	, "adv"		, POS_DEAD    , do_advance  , LVL_GRGOD, 0, 0.0 },
	{	"aedit"		, "aed"		, POS_DEAD    , do_olc      , LVL_APPR, SCMD_OLC_AEDIT, 0.0 },
	{	"alias"		, "ali"		, POS_DEAD    , do_alias    , 0, 0, 0.0 },
	{	"assist"	, "ass"		, POS_FIGHTING, do_assist   , 1, 0, 0.0 },
	{	"ask"		, "ask"		, POS_RESTING , do_spec_comm, 0, SCMD_ASK, 0.0 },
	{	"autoexit"	, "autoe"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT, 0.0 },
	{	"award"		, "aw"		, POS_DEAD    , do_award    , 0, 0, 0.0 },
	{	"awardd"	, "awa"		, POS_DEAD    , do_awardd   , 0, 0, 0.0 },
	{	"awardh"	, "awar"	, POS_DEAD    , do_awardh   , 0, 0, 0.0 },

	{	"backstab"	, "bac"		, POS_FIGHTING, do_backstab , 1, 0, 4.5 },
	{	"ban"		, "ban"		, POS_DEAD    , do_ban      , LVL_GRGOD, 0, 0.0},
	{	"balance"	, "bal"		, POS_STANDING, do_not_here , 1, 0, 0.0},
	{	"bash"		, "bas"		, POS_FIGHTING, do_bash     , 1, 0, 4.0 },
	{	"battle"	, "bat"		, POS_DEAD,     do_battle   , 0, 0, 0.0 },
	{	"brief"		, "br"		, POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF, 0.0},
	{	"buildwalk"	, "buildwalk",POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_BUILDWALK, 0.0},
	{	"butcher"	, "bu"		, POS_STANDING, do_butcher  , 0, 0, 0.0},
	{	"buy"		, "b"		, POS_STANDING, do_not_here , 0, 0, 0.0 },
	{	"bug"		, "bug"		, POS_DEAD    , do_gen_write, 0, SCMD_BUG, 0.0},
	{	"buffer"	, "buffe"	, POS_DEAD    , do_buffer   , LVL_IMPL, 0, 0.0},

	{	"channel"	, "c"		, POS_SITTING , do_cast     , 1, 0, 0.0},
	{	"call"		, "ca"		, POS_RESTING , do_not_here , 0, 0, 0.0},
	{	"check"		, "ch"		, POS_STANDING, do_not_here , 1, 0, 0.0},
	{	"chat"		, "cha"		, POS_RESTING , do_gen_comm , 0, SCMD_CHAT, 0.0},
	{	"charge"	, "char"	, POS_STANDING,	do_charge	, 0, SCMD_CHARGE, 2.0},
	{	"change"	, "chang"	, POS_DEAD    , do_change   , 0, 0, 0.0 },
	{	"clan"		, "clan"	, POS_DEAD    , do_clan     , 0, 0, 0.0 },
	{	"clans"		, "clans"	, POS_DEAD    , do_clans    , 0, 0, 0.0 },
	{	"clear"		, "cle"		, POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR ,0.0},
	{	"close"		, "clo"		, POS_SITTING , do_gen_door , 0, SCMD_CLOSE, 0.0},
	{	"cls"		, "cls"		, POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR, 0.0},
	{	"consider"	, "con"		, POS_RESTING , do_consider , 0, 0, 0.0 },
	{	"color"		, "col"		, POS_DEAD    , do_color    , 0, 0, 0.0 },
	{	"commands"	, "comm"	, POS_DEAD    , do_commands , 0, SCMD_COMMANDS, 0.0 },
	{	"compact"	, "comp"	, POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT, 0.0 },
	{	"copy"		, "copy"	, POS_DEAD    , do_copy     , LVL_IMMORT, 0, 0.0 },
	{	"copyover"	, "copyo"	, POS_DEAD    , do_copyover , LVL_IMPL, 0, 0.0 },
	{	"cqpurge"	, "cq"		, POS_DEAD    , do_cqpurge  , LVL_APPR, 0, 0.0 },
	{	"credits"	, "cre"		, POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS, 0.0 },
	{	"council"	, "cou"		, POS_DEAD    , do_council  , 0, 0, 0.0  },

	{	"date"		, "date"	, POS_DEAD    , do_date     , LVL_IMMORT, SCMD_DATE, 0.0 },
	{	"dc"		, "dc"		, POS_DEAD    , do_dc       , LVL_GOD, 0,		0.0 },
	{	"demote"	, "dem"		, POS_DEAD    , do_demote   , 0, 0, 0.0 },
	{	"deposit"	, "dep"		, POS_STANDING, do_not_here , 1, 0, 0.0 },
	{	"diagnose"	, "dia"		, POS_RESTING , do_diagnose , 0, 0, 0.0 },
	{	"diceroll"	, "dic"		, POS_SITTING , do_diceroll , 0, 0, 0.0 },
	{	"dig"		, "dig"		, POS_DEAD    , do_dig      , LVL_IMMORT, 0, 0.0},
	{	"disable"	, "disa"	, POS_DEAD    , do_disable  , LVL_GOD, 0, 0.0 },
	{	"dismount"	, "dis"		, POS_FIGHTING, do_dismount , 0, 0, 0.0 },
	{	"display"	, "disp"	, POS_DEAD    , do_display  , 0, 0, 0.0 },
	{	"donate"	, "don"		, POS_RESTING , do_drop     , 0, SCMD_DONATE, 0.0 },
	{	"drink"		, "dr"		, POS_RESTING , do_drink    , 0, SCMD_DRINK, 0.0 },
	{	"drop"		, "dro"		, POS_RESTING , do_drop     , 0, SCMD_DROP, 0.0 },

	{	"eat"		, "ea"		, POS_RESTING , do_eat      , 0, SCMD_EAT, 0.0 },
	{	"echo"		, "echo"	, POS_SLEEPING, do_echo     , LVL_GOD, SCMD_ECHO, 0.0 },
	{	"effuse"	, "ef"		, POS_STANDING, do_effuse   , 0, 0, 0.0 },
	{	"embrace"	, "emb"		, POS_RESTING , do_source   , 0, SCMD_EMBRACE, 0.0 },
	{	"emote"		, "em"		, POS_RESTING , do_echo     , 1, SCMD_EMOTE, 0.0 },
	{	":"		, ":"		, POS_RESTING , do_echo     , 1, SCMD_EMOTE, 0.0 },
	{	"enable"	, "ena"		, POS_DEAD    , do_enable   , LVL_GOD, 0, 0.0 },
	{	"enter"		, "en"		, POS_STANDING, do_enter    , 0, 0, 0.0 },
	{	"equipment"	, "eq"		, POS_SLEEPING, do_equipment, 0, 0, 0.0 },
	{	"exits"		, "exi"		, POS_RESTING , do_exits    , 0, 0, 0.0 },
	{	"examine"	, "exa"		, POS_RESTING , do_examine  , 0, 0, 0.0 },
	{	"extra"		, "ext"		, POS_DEAD    , do_extra    , LVL_GOD, 0, 0.0 },

	{	"flee"		, "f"		, POS_FIGHTING, do_flee     , 1, 0, 0.0 },
	{	"fade"		, "fa"		, POS_STANDING, do_fade     , 0, 0, 0.0 },
	{	"flip"		, "fl"		, POS_SITTING , do_flip     , 0, 0, 0,0 }, 
	{	"find"		, "fi"		, POS_DEAD    , do_find     , LVL_GOD, 0, 0.0 },
	{	"force"		, "for"		, POS_SLEEPING, do_force    , LVL_GOD, 0, 0.0 },
	{	"fill"		, "fil"		, POS_STANDING, do_pour     , 0, SCMD_FILL, 0.0 },
	{	"follow"	, "fol"		, POS_RESTING , do_follow   , 0, 0, 0.0 },
	{	"freeze"	, "free"	, POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE, 0.0 },

	{	"get"		, "g"		, POS_RESTING , do_get      , 0, 0, 0.0 },
	{	"gecho"		, "gech"	, POS_DEAD    , do_gecho    , LVL_GRGOD, 0, 0.0 },
	{	"give"		, "giv"		, POS_STANDING, do_give     , 0, 0, 0.0 },
	{	"goto"		, "got"		, POS_SLEEPING, do_goto     , LVL_IMMORT, 0, 0.0 },
	{	"gold"		, "gol"		, POS_RESTING , do_gold     , 0, 0, 0.0 },
	{	"group"		, "gr"		, POS_RESTING , do_group    , 1, 0, 0.0 },
	{	"grab"		, "gra"		, POS_RESTING , do_grab     , 0, 0, 0.0 },

	{	"help"		, "he"		, POS_DEAD    , do_help     , 0, 0, 0.0 },
	{	"hedit"		, "hedit"	, POS_DEAD    , do_olc	    , LVL_APPR, SCMD_OLC_HEDIT, 0.0 },
	{	"handbook"	, "hand"	, POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_HANDBOOK, 0.0 },
	{	"hcontrol"	, "hcon"	, POS_DEAD    , do_hcontrol , LVL_GRGOD, 0, 0.0 },
	{	"hide"		, "hid"		, POS_STANDING, do_hide     , 0, 0, 0.0 },
	{	"hit"		, "h"		, POS_FIGHTING, do_hit      , 0, SCMD_HIT, 0.0 },
	{	"hold"		, "ho"		, POS_RESTING , do_grab     , 1, 0, 0.0 },
	{	"holylight"	, "holy"	, POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_HOLYLIGHT, 0.0 },
	{	"house"		, "hou"		, POS_RESTING , do_house    , 0, 0, 0.0 },

	{	"inventory"	, "i"		, POS_DEAD    , do_inventory, 0, 0, 0.0 },
	{	"ignore"	, "ig"		, POS_DEAD    , do_ignore   , 0, 0, 0.0 },
	{	"incognito"	, "in"		, POS_DEAD    , do_incognito, 0, 0, 0.0 },
	{	"idea"		, "id"		, POS_DEAD    , do_gen_write, 0, SCMD_IDEA, 0.0 },
	{	"imotd"		, "imo"		, POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_IMOTD, 0.0 },
	{	"immlist"	, "imm"		, POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST, 0.0 },
	{	"info"		, "inf"		, POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO, 0.0 },
	{	"insult"	, "ins"		, POS_RESTING , do_insult   , 0, 0, 0.0 },
	{	"invis"		, "inv"		, POS_DEAD    , do_invis    , LVL_IMMORT, 0, 0.0 },
	{	"ipfind"	, "ip"		, POS_DEAD    , do_ipfind   , LVL_GRGOD, 0, 0.0 },

	{	"kill"		, "k"		, POS_FIGHTING, do_kill     , 0, 0, 0.0 },
	{	"kedit"		, "ke"		, POS_DEAD    , do_olc      , LVL_BLDER, SCMD_OLC_KEDIT, 0.0 },
	{	"kit"		, "ki"		, POS_RESTING , do_not_here , 0, 0, 0.0 },
	{	"kick"		, "kic"		, POS_FIGHTING, do_kick     , 1, 0, 3.0 },
	{	"klist"		, "kl"		, POS_DEAD    , do_klist    , LVL_IMMORT, 0, 0.0 },

	{	"look"		, "l"		, POS_RESTING , do_look     , 0, SCMD_LOOK, 0.0 },
	{	"last"		, "la"		, POS_DEAD    , do_last     , LVL_GOD, 0, 0.0 },
	{	"lag"		, "lag"		, POS_DEAD    , do_lag      , LVL_IMPL, 0, 0.0 },
	{	"lead"		, "lead"	, POS_STANDING, do_lead     , 0, 0, 0.0  },
	{	"leave"		, "lea"		, POS_STANDING, do_leave    , 0, 0, 0.0 },
	{	"legends"	, "leg"		, POS_DEAD    , do_legends  , 0, 0, 0.0 },
	{	"legupdate"	, "legu"	, POS_DEAD    , do_legupdate, 0, 0, 0.0 },
	{	"levels"	, "lev"		, POS_DEAD    , do_levels   , 0, 0, 0.0 },
	{	"list"		, "li"		, POS_STANDING, do_not_here , 0, 0, 0.0 },
	{	"lock"		, "loc"		, POS_SITTING , do_gen_door , 0, SCMD_LOCK, 0.0 },
//	{	"load"		, "loa"		, POS_DEAD    , do_load     , LVL_GOD, 0, 0.0 },

	{	"medit"		, "med"		, POS_DEAD    , do_olc      , LVL_BLDER, SCMD_OLC_MEDIT, 0.0 },
	{	"mark"		, "mar"		, POS_FIGHTING, do_mark     , 0, 0, 0.0 },
	{	"mbload"		, "mbloa"		, POS_DEAD    , do_mbload     , LVL_BLDER, 0, 0.0 },
	{	"memory"	, "mem"		, POS_DEAD    , do_memory   , LVL_BLDER, 0, 0.0	},
	//  { "mlist"	, "mli"		, POS_DEAD    , do_mlist    , LVL_IMMORT, 0, 0.0},
	{	"motd"		, "motd"	, POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD, 0.0 },
	{	"mail"		, "mail"	, POS_STANDING, do_not_here , 1, 0, 0.0},
	{	"mute"		, "mut"		, POS_DEAD    , do_wizutil  , LVL_APPR, SCMD_SQUELCH, 0.0 },
	{	"murder"	, "mur"		, POS_FIGHTING, do_hit      , 0, SCMD_MURDER, 0.0 },

	{	"narrate"	, "nar"		, POS_RESTING , do_gen_comm , 0, SCMD_NARRATE, 0.0 },
	{	"news"		, "new"		, POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS, 0.0 },
	{	"notice"	, "not"		, POS_RESTING , do_notice   , 0, 0, 0.0 },
	{	"note"		, "note"	, POS_DEAD    , do_note     , 0, 0, 0.0 },
	{	"nochats"	, "nogr"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOCHAT, 0.0 },
	{	"nohassle"	, "noh"		, POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOHASSLE, 0.0 },
	{	"nonarrates"	, "noa"		, POS_DEAD    , do_gen_tog  , 0, SCMD_NONARRATE, 0.0 },
	{	"norepeat"	, "nor"		, POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT, 0.0 },
	{	"notell"	, "note"	, POS_DEAD    , do_gen_tog  , LVL_APPR, SCMD_NOTELL, 0.0 },
	{	"notitle"	, "noti"	, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_NOTITLE, 0.0 },
	{	"nowiz"		, "now"		, POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOWIZ, 0.0 },

	{	"order"		, "ord"		, POS_RESTING , do_order    , 1, 0, 0.0 },
	{	"offer"		, "off"		, POS_STANDING, do_not_here , 1, 0, 0.0 },
	{	"open"		, "op"		, POS_SITTING , do_gen_door , 0, SCMD_OPEN, 0.0 },
	{	"oload"		, "oloa"		, POS_DEAD    , do_oload     , LVL_GRGOD, 0, 0.0 },
	{	"olc"		, "olc"		, POS_DEAD    , do_olc      , LVL_IMMORT, SCMD_OLC_SAVEINFO, 0.0 },
	{	"oedit"		, "oed"		, POS_DEAD    , do_olc      , LVL_GRGOD, SCMD_OLC_OEDIT, 0.0},
	{	"overflow"	, "overfl"	, POS_DEAD    , do_overflow , LVL_GRGOD, 0, 0.0},
	{	"put"		, "p"		, POS_RESTING , do_put      , 0, 0, 0.0 },
	{	"practice"	, "pr"		, POS_RESTING , do_practice , 1, 0, 0.0 },
	{	"page"		, "pag"		, POS_DEAD    , do_page     , LVL_APPR, 0, 0.0 },
	{	"pardon"	, "par"		, POS_DEAD    , do_pardon   , 0, 0, 0.0 },
	  { "peace"    , "pea"	, POS_DEAD    , do_peace    , LVL_GRGOD, 0 },
	{	"pfind"		, "pf"		, POS_DEAD    , do_pfind    , LVL_GOD, 0, 0.0 },
	{	"pick"		, "pi"		, POS_STANDING, do_gen_door , 1, SCMD_PICK, 0.0 },
	{	"policy"	, "pol"		, POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES, 0.0 },
	{	"poofin"	, "poofi"	, POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFIN, 0.0 },
	{	"poofout"	, "poofo"	, POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFOUT, 0.0 },
	{	"pour"		, "pour"	, POS_STANDING, do_pour     , 0, SCMD_POUR, 0.0 },
	{	"prompt"	, "promp"	, POS_DEAD    , do_display  , 0, 0, 0.0 },
	{	"purge"		, "pur"		, POS_DEAD    , do_purge    , LVL_BLDER, 0, 0.0 },

	{	"quaff"		, "q"		, POS_RESTING , do_use      , 0, SCMD_QUAFF, 0.0 },
	{	"qui"		, "qui"		, POS_DEAD    , do_quit     , 0, 0, 0.0 },
	{	"quit"		, "quit"	, POS_DEAD    , do_quit     , 0, SCMD_QUIT, 0.0 },
	{	"qval"		, "qv"		, POS_DEAD    , do_qval     , LVL_APPR, 0, 0.0 },

	{	"rank"		, "ran"		, POS_DEAD    , do_rank     , 0, 0, 0.0 },
	{	"rest"		, "res"		, POS_RESTING , do_rest     , 0, 0, 0.0 },
	{	"reply"		, "r"		, POS_SLEEPING, do_reply    , 0, 0, 0.0 },
	{	"read"		, "rea"		, POS_RESTING , do_look     , 0, SCMD_READ, 0.0 },
	{	"reboot"	, "reb"		, POS_DEAD    , do_countdown, LVL_GRGOD, 0, 0.0 },
	{	"reload"	, "rel"		, POS_DEAD    , do_reboot   , LVL_IMPL, 0, 0.0 },
	{	"recite"	, "rec"		, POS_RESTING , do_use      , 0, SCMD_RECITE, 0.0 },
	{	"receive"	, "rece"	, POS_STANDING, do_not_here , 1, 0, 0.0 },
	{	"release"	, "rel"		, POS_RESTING , do_release  , 0, 0, 0.0 },
	{	"remove"	, "rem"		, POS_RESTING , do_remove   , 0, 0, 0.0 },
	{	"rent"		, "ren"		, POS_STANDING, do_not_here , 1, 0, 0.0 },
	{	"reroll"	, "rer"		, POS_DEAD    , do_reroll   , LVL_GRGOD, 0, 0.0 },
	{	"rescue"	, "resc"	, POS_FIGHTING, do_rescue   , 1, 0, 0.0 },
	{	"reset"		, "res"		, POS_DEAD    , do_reset    , LVL_GOD, 0, 0.0 },
	{	"restore"	, "resto"	, POS_DEAD    , do_restore  , LVL_GOD, 0, 0.0 },
	{	"return"	, "ret"		, POS_DEAD    , do_return   , 0, 0, 0.0 },
	{	"redit"		, "redit"	, POS_DEAD    , do_olc      , LVL_IMMORT, SCMD_OLC_REDIT, 0.0},
	{	"ride"		, "ri"		, POS_FIGHTING, do_ride     , 1, 0, 0.0 },
	{	"rlink"		, "rlink"	, POS_DEAD    , do_rlink    , LVL_IMMORT, 0, 0.0 },
	{	"roomflags"	, "roomf"	, POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_ROOMFLAGS, 0.0 },

	{	"say"		, "sa"		, POS_RESTING , do_say      , 0, 0, 0.0 },
	{	"score"		, "sc"		, POS_DEAD    , do_score    , 0, 0, 0.0 },
	{	"'"		, "'"		, POS_RESTING , do_say      , 0, 0, 0.0 },
	{	"scalp"		, "sca"		, POS_STANDING, do_scalp    , 0, 0, 0.0 },
	{	"speak"		, "sp"		, POS_RESTING , do_speak    , LVL_BLDER, 0, 0.0 },
	{	"save"		, "save"	, POS_SLEEPING, do_save     , 0, 0, 0.0 },
	{	"scan"		, "scan"	, POS_STANDING, do_scan     , 0, 0, 0.0 },
	{	"search"	, "sea"		, POS_STANDING, do_search   , 0, 0, 0.0 },
	//  {	"saveall"  , "sall" , POS_DEAD    , do_saveall  , LVL_IMMORT, 0, 0.0},
	//  {	"saveolc"  , "solc" , POS_DEAD    , do_saveolc  , LVL_IMMORT, 0, 0.0},
	{	"seize"		, "sei"		, POS_RESTING , do_source   , 0, SCMD_SEIZE, 0.0 },
	{	"sell"		, "sel"		, POS_STANDING, do_not_here , 0, 0, 0.0 },
	{	"send"		, "sen"		, POS_SLEEPING, do_send     , LVL_GOD, 0, 0.0 },
	{	"sense"		, "sens"	, POS_STANDING, do_sense    , 0, 0, 10.0 },
	{	"set"		, "set"		, POS_DEAD    , do_set      , LVL_GOD, 0, 0.0 },
	{	"sedit"		, "sedi"	, POS_DEAD    , do_olc      , LVL_BLDER, SCMD_OLC_SEDIT, 0.0},
	{	"selfdelete"	,"self"		, POS_DEAD    , do_self_delete, 0, 0, 0.0 },
	{	"shout"		, "sho"		, POS_RESTING , do_gen_comm , 1, SCMD_SHOUT, 0.0 },
	{	"shake"		, "sha"		, POS_RESTING , do_action   , 0, 0, 0.0 },
	{	"shiver"	, "shiv"	, POS_RESTING , do_action   , 0, 0, 0.0 },
	{	"shoot"		, "sho"		, POS_STANDING, do_shoot    , 0, 0, 3.5 },
	{	"show"		, "shoo"	, POS_DEAD    , do_show     , LVL_IMMORT, 0, 0.0 },
	{	"shutdow"	, "shutdow"	, POS_DEAD    , do_shutdown , LVL_GRGOD, 0, 0.0},
	{	"shutdown"	, "shutdown"	, POS_DEAD    , do_shutdown , LVL_IMPL, SCMD_SHUTDOWN, 0.0 },
	{	"sip"		, "sip"		, POS_RESTING , do_drink    , 0, SCMD_SIP, 0.0 },
	{	"sit"		, "sit"		, POS_RESTING , do_sit      , 0, 0, 0.0 },
	{	"skewer"	, "ske"		, POS_STANDING, do_charge	, 0, SCMD_SKEWER, 2.0 },
	{	"skillset"	, "skillse"	, POS_SLEEPING, do_skillset , LVL_GRGOD, 0, 0.0 },
	{	"sleep"		, "sl"		, POS_SLEEPING, do_sleep    , 0, 0, 0.0 },
	{	"slist"		, "sli"		, POS_DEAD	  ,	do_slist	, LVL_BLDER, 0, 0.0	},
	{	"slowns"	, "slown"	, POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_SLOWNS, 0.0 },
	{	"sneak"		, "snea"	, POS_STANDING, do_sneak    , 1, 0, 0.0 },
	{	"snoop"		, "sno"		, POS_DEAD    , do_snoop    , LVL_APPR, 0, 0.0 },
	{	"socials"	, "soc"		, POS_DEAD    , do_commands , 0, SCMD_SOCIALS, 0.0 },
	{	"split"		, "spl"		, POS_SITTING , do_split    , 1, 0, 0.0 },
	{	"stand"		, "st"		, POS_RESTING , do_stand    , 0, 0, 0.0 },
	{	"stats"		, "stat"	, POS_DEAD    , do_stat     , 1, 0, 0.0 },
	{	"statfind"	, "statf"	, POS_DEAD    , do_statfind , LVL_GOD, 0, 0.0 },
	{	"steal"		, "stea"	, POS_STANDING, do_steal    , 1, 0, 3.0 },
	{	"swap"		, "swa"		, POS_DEAD    , do_swap     , LVL_GOD, 0, 0.0 },
	{	"switch"	, "sw"		, POS_DEAD    , do_switch   , LVL_APPR, 0, 0.0 },
	{	"syslog"	, "sys"		, POS_DEAD    , do_syslog   , LVL_APPR, 0, 0.0 },

	{	"tell"		, "t"		, POS_RESTING , do_tell     , 0, 0, 0.0 },
	{	"tedit"		, "ted"		, POS_DEAD    , do_tedit    , LVL_IMPL, 0, 0.0 },
	{	"tellmute"	, "tellm"	, POS_DEAD    , do_tell_mute, LVL_APPR, 0, 0.0 },
	{	"take"		, "ta"		, POS_RESTING , do_get      , 0, 0, 0.0 },
	{	"taste"		, "tas"		, POS_RESTING , do_eat      , 0, SCMD_TASTE, 0.0 },
	{	"teleport"	, "tel"		, POS_DEAD    , do_teleport , LVL_APPR, 0, 0.0 },
	{	"thaw"		, "thaw"	, POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW, 0.0 },
	{	"time"		, "tim"		, POS_DEAD    , do_time     , 0, 0, 0.0 },
	{	"toggle"	, "tog"		, POS_DEAD    , do_toggle   , 0, 0, 0.0 },
	{	"track"		, "tr"		, POS_STANDING, do_track    , 0, 0, 1.0 },
	{	"transfer"	, "tran"	, POS_SLEEPING, do_trans    , LVL_APPR, 0, 0.0 },
	{	"trigedit"	, "trig"	, POS_DEAD    , do_olc      , LVL_BLDER, SCMD_OLC_TRIGEDIT, 0.0},
	{	"typo"		, "typ"		, POS_DEAD    , do_gen_write, 0, SCMD_TYPO, 0.0 },

	{	"unlock"	, "unl"		, POS_SITTING , do_gen_door , 0, SCMD_UNLOCK, 0.0 },
	{	"ungroup"	, "ung"		, POS_DEAD    , do_ungroup  , 0, 0, 0.0 },
	{	"unban"		, "unb"		, POS_DEAD    , do_unban    , LVL_GRGOD, 0, 0.0 },
	{	"unbond"	, "unbo"	, POS_DEAD	  ,	do_unbond	, LVL_GRGOD, 0, 0.0 },
	{	"unaffect"	, "una"		, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_UNAFFECT, 0.0 },
	{	"uptime"	, "upt"		, POS_DEAD    , do_date     , LVL_IMMORT, SCMD_UPTIME, 0.0 },
	{	"use"		, "us"		, POS_SITTING , do_use      , 1, SCMD_USE, 0.0 },
	{	"users"		, "user"	, POS_DEAD    , do_users    , LVL_GRGOD, 0, 0.0 },

	{	"value"		, "val"		, POS_STANDING, do_not_here , 0, 0, 0.0 },
	{	"version"	, "ver"		, POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION, 0.0 },
	{	"visible"	, "vis"		, POS_RESTING , do_visible  , 1, 0, 0.0 },
	{	"view"		, "vie"		, POS_DEAD    , do_view     , 0, 0, 0.0 },
	{	"vnum"		, "vnum"	, POS_DEAD    , do_vnum     , LVL_IMMORT, 0, 0.0 },
	{	"vstat"		, "vsta"	, POS_DEAD    , do_vstat    , LVL_GRGOD, 0, 0.0 },

	{	"wake"		, "wak"		, POS_SLEEPING, do_wake     , 0, 0, 0.0 },
	{	"warn"		, "warn"	, POS_DEAD    , do_warn     , LVL_APPR, 0, 0.0 },
	{	"warrant"	, "war"		, POS_DEAD    , do_warrant  , 0, 0, 0.0 },
	{	"wear"		, "wea"		, POS_RESTING , do_wear     , 0, 0, 0.0 },
	{	"weather"	, "weat"	, POS_RESTING , do_weather  , 0, 0, 0.0 },
	{	"weaves"	, "weav"	, POS_RESTING , do_weaves   , 0, 0, 0.0 },
	{	"who"		, "who"		, POS_DEAD    , do_who      , 0, 0, 0.0 },
	{	"whois"		, "whoi"	, POS_DEAD    , do_whois    , 0, 0, 0.0},
	{	"where"		, "whe"		, POS_RESTING , do_where    , 1, 0, 0.0 },
	{	"whisper"	, "whis"	, POS_RESTING , do_spec_comm, 0, SCMD_WHISPER, 0.0 },
	{	"wield"		, "wie"		, POS_RESTING , do_wield    , 0, 0, 0.0 },
	{	"wimpy"		, "wim"		, POS_DEAD    , do_wimpy    , 0, 0, 0.0 },
	{	"withdraw"	, "with"	, POS_STANDING, do_not_here , 1, 0, 0.0 },
	{	"wiznet"	, "wiz"		, POS_DEAD    , do_wiznet   , LVL_IMMORT, 0, 0.0 },
	{	";"		, ";"		, POS_DEAD    , do_wiznet   , LVL_IMMORT, 0, 0.0 },
	{	"wiznoise"	, "wizno"	, POS_DEAD    , do_wiznoise , LVL_IMMORT, 0, 0.0 },
	{	"wizdelete"	, "wizd"	, POS_DEAD    , do_wizdelete, LVL_BLDER, 0, 0.0 },
	{	"wizhelp"	, "wizh"	, POS_SLEEPING, do_commands , LVL_IMMORT, SCMD_WIZHELP, 0.0 },
	{	"wizlist"	, "wizl"	, POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST, 0.0 },
	{	"wizlist"	, "wizl"	, POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST, 0.0 },
	{	"wizview"	, "wizv"	, POS_DEAD    , do_wizview  , LVL_BLDER, 0, 0.0 },
	{	"wizlock"	, "wizlo"	, POS_DEAD    , do_wizlock  , LVL_IMPL, 0, 0.0 },
	{	"write"		, "wr"		, POS_STANDING, do_write    , 1, 0, 0.0 },

	{	"yell"		, "yell"	, POS_RESTING , do_gen_comm , 0, SCMD_YELL, 0.0 },

	{	"zap"		, "z"		, POS_DEAD    , do_zap      , LVL_GOD, 0, 0.0 },
	{	"zcmdfind"	, "zc"		, POS_DEAD	  , do_zcmd_find, LVL_GRGOD, 0, 0.0},
	{	"zedit"		, "zed"		, POS_DEAD    , do_olc      , LVL_BLDER, SCMD_OLC_ZEDIT, 0.0},
	{	"zreset"	, "zre"		, POS_DEAD    , do_zreset   , LVL_BLDER, 0, 0.0 },
	{	"zoneban"	, "zon"		, POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_ZONE_BAN, 0.0},

	/* DG trigger commands */
	{	"attach"	, "att"		, POS_DEAD    , do_attach   , LVL_BLDER, 0, 0.0 },
	{	"detach"	, "det"		, POS_DEAD    , do_detach   , LVL_BLDER, 0, 0.0 },
	{	"tlist"		, "tli"		, POS_DEAD    , do_tlist    , LVL_BLDER, 0, 0.0 },
	{	"tstat"		, "tst"		, POS_DEAD    , do_tstat    , LVL_GOD, 0, 0.0 },
	{	"masound"	, "masound"	, POS_DEAD    , do_masound  , 0, 0, 0.0 },
	{	"mkill"		, "mkill"	, POS_STANDING, do_mkill    , 0, 0, 0.0 },
	{	"mjunk"		, "mjunk"	, POS_SITTING , do_mjunk    , 0, 0, 0.0 },
	{	"mecho"		, "mecho"	, POS_DEAD    , do_mecho    , 0, 0, 0.0 },
	{	"mechoaround"	,"mechoaround"	, POS_DEAD    , do_mechoaround, 0, 0, 0.0 },
	{	"mhorsepurge"	,"mhor"		, POS_DEAD    , do_mhorse_purge, 0, 0, 0.0 },
	{	"mqval"		, "mqval"	, POS_DEAD    , do_mqval    , 0, 0, 0.0 },
	{	"msend"		, "msend"	, POS_DEAD    , do_msend    , 0, 0, 0.0 },
	{	"mload"		, "mload"	, POS_DEAD    , do_mload    , 0, 0, 0.0 },
	{	"mdamage"	, "mdamage"	, POS_DEAD    , do_mdamage  , 0, 0, 0.0 },
	{	"mpurge"	, "mpurge"	, POS_DEAD    , do_mpurge   , 0, 0, 0.0 },
	{	"mgoto"		, "mgoto"	, POS_DEAD    , do_mgoto    , 0, 0, 0.0 },
	{	"mat"		, "mat"		, POS_DEAD    , do_mat      , 0, 0, 0.0 },
	{	"mteleport"	, "mteleport"	, POS_DEAD    , do_mteleport, 0, 0, 0.0 },
	{	"mforce"	, "mforce"	, POS_DEAD    , do_mforce   , 0, 0, 0.0 },
	{	"mexp"		, "mexp"	, POS_DEAD    , do_mexp     , 0, 0, 0.0 },
	{	"mgold"		, "mgold"	, POS_DEAD    , do_mgold    , 0, 0, 0.0 },
	{	"mhunt"		, "mhunt"	, POS_DEAD    , do_mhunt    , 0, 0, 0.0 },
	{	"mremember"	, "mremember"	, POS_DEAD    , do_mremember, 0, 0, 0.0 },
	{	"mreset"	, "mreset"	, POS_DEAD    , do_mreset   , 0, 0, 0.0 },
	{	"mforget"	, "mforget"	, POS_DEAD    , do_mforget  , 0, 0, 0.0 },
	{	"mtransform"	, "mtransform"	, POS_DEAD    , do_mtransform, 0, 0, 0.0 },
	{	"mzreset"	, "mzreset"	, POS_DEAD    , do_mzreset  , 0, 0, 0.0 },

	{	"\n"		, "zzzzzzz"	, 0, 0, 0, 0, 0.0 } /* this must be last */
};	


const char *filler[] =
{
	"in",
	"from",
	"with",
	"the",
	"on",
	"at",
	"to",
	"\n"
};

const char *reserved[] =
{
	"a",
	"an",
	"self",
	"me",
	"all",
	"room",
	"someone",
	"something",
	"\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */

const char *randomList[] = 
{
	"look", /* 0 */
	"kill",
	"north",
	"south",
	"east",
	"west", /* 5 */
	"up",
	"down",
	"flee",
	"channel",
	"say", /* 10 */
	"shout",
	"narrate",
	"emote",
	"yell", /* 14 */
};

const int numOfRandomCommands = 14; /* plus socials, but don't add that here */

int randomCommand()
{
	static int numOfCommands = find_command("zreset"); /* preferably be last command */
	int cmd = number(0, numOfCommands);

	if (complete_cmd_info[cmd].command_pointer != do_action) /* if not a social */
		cmd = find_command(randomList[number(0, numOfRandomCommands)]);

	if (cmd == -1) /* a precaution for misspelled commands in list */
	{
		mudlog("Index -1 in randomCommand()", BRF, LVL_IMPL, TRUE);
		cmd = find_command("channel");
	}

	return (cmd);
}

void command_interpreter(char_data *ch, char *argument)
{
	int cmd, length;
	char *line;
	bool timer = FALSE;

	if(ch->desc && ch->desc->timer > 0.0 && !ch->desc->forced)
	{
		send_to_char( "Cancelled.\r\n", ch);
		ch->desc->timer = 0.0;
		ch->desc->command_ready = 0;
		ch->desc->delayed_state = 0;
		ch->desc->forced = 0;
		strcat(LAST_COMMAND(ch), "");
	}

	if (ch->desc && ch->desc->timer < -9.0)
	{
		ch->desc->timer = 0.0000;
		timer = TRUE;
	}

  /* just drop to next line for hitting CR */
	skip_spaces(&argument);
	
	if (!*argument)
		return;

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
	if (!isalpha(*argument))
	{
		arg[0] = argument[0];
		arg[1] = '\0';
		line = argument + 1;
	} 
	
	else
		line = any_one_arg(argument, arg);

  /* otherwise, find the command */
	if ((GET_LEVEL(ch) < LVL_IMMORT) &&
	(command_wtrigger(ch, arg, line) ||
	command_mtrigger(ch, arg, line) ||
	command_otrigger(ch, arg, line)))
		return; /* command trigger took over */

	for (length = strlen(arg), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
		if (!strncmp(complete_cmd_info[cmd].command, arg, length))
			if (GET_LEVEL(ch) >= complete_cmd_info[cmd].minimum_level)
				break;

	if (*complete_cmd_info[cmd].command != '\n' && GET_TAINT(ch) && TAINT_CALC(ch) > number(0, 100))
		cmd = randomCommand();

		//Pffft, blame the code //
	if(CMD_NAME == "where" || CMD_NAME == "backstab")  {}

	else
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

	
	if (*complete_cmd_info[cmd].command == '\n')
		send_to_char("What?!?\r\n", ch);
	
	else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL)
		send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
	
	else if (complete_cmd_info[cmd].command_pointer == nullptr)
		send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
	
	else if(complete_cmd_info[cmd].disable == 1)
		send_to_char("That command has been disabled.\r\n", ch);
	
	//else if (IS_NPC(ch) && complete_cmd_info[cmd].minimum_level >= LVL_IMMORT)
	//	send_to_char("You can't use immortal commands while switched.\r\n", ch);
	
	else if (GET_POS(ch) < complete_cmd_info[cmd].minimum_position)
		
		switch (GET_POS(ch))
	{
			case POS_DEAD:
				send_to_char("Lie still; you are DEAD!!!\r\n", ch);
				break;
    
			case POS_INCAP:
			case POS_MORTALLYW:
				send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
				break;
    
			case POS_STUNNED:
				send_to_char("All you can do right now is think about the stars!\r\n", ch);
				break;
    
			case POS_SLEEPING:
				send_to_char("In your dreams, or what?\r\n", ch);
				break;
    
			case POS_RESTING:
				send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
				break;
    
			case POS_SITTING:
				send_to_char("Maybe you should get on your feet first?\r\n", ch);
				break;
    
			case POS_FIGHTING:
				send_to_char("No way!  You're fighting for your life!\r\n", ch);
				break;
		}

	  

	else if (no_specials || !special(ch, cmd, line))
	{

	if ((GET_LEVEL(ch) < LVL_IMMORT) &&
	(command_wtrigger(ch, (char *) CMD_NAME, line) ||
	command_mtrigger(ch, (char *) CMD_NAME, line) ||
	command_otrigger(ch, (char *) CMD_NAME, line)))
		return;   



		if((!str_cmp(CMD_NAME, "channel") || !str_cmp(CMD_NAME, "search")) && ch->desc)
			strcpy(LAST_COMMAND(ch), argument);

		strcpy(GET_COMMAND(ch), CMD_NAME);

		//ONLY PC's using commands with a delay have to do this.
		if(!timer && ch->desc && complete_cmd_info[cmd].timer != 0.0 && !ch->desc->command_ready)
		{
			ch->desc->timer = complete_cmd_info[cmd].timer;
			strcpy(LAST_COMMAND(ch), argument);
		}

		((*complete_cmd_info[cmd].command_pointer) (ch, line, cmd, complete_cmd_info[cmd].subcmd));
	}
}




/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{

	struct alias *alias;

	for(alias = alias_list;alias;alias = alias->next)
	{
		
		if (alias->name && *str == *alias->name)	/* hey, every little bit counts :-) */
			if (!strcmp(str, alias->name))
				return alias;

	}

	return nullptr;
}


void free_alias(struct alias *a)
{
	delete[] (a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
	char *repl;
	struct alias *a, *temp;

	if (IS_NPC(ch))
		return;

	repl = any_one_arg(argument, arg);

	if (!*arg)
	{			/* no argument specified -- list currently defined aliases */
		
		send_to_char("Currently defined aliases:\r\n", ch);
    
		if ((a = GET_ALIASES(ch)) == nullptr)
			send_to_char(" None.\r\n", ch);
    
		else
		{
			while (a)
			{
				message(ch, "%-15s %s\r\n", a->name, a->replacement);
				a = a->next;
			}
		}
	} 
	
	else
	{			/* otherwise, add or remove aliases */
    
		/* is this an alias we've already defined? */
    
		if ((a = find_alias(GET_ALIASES(ch), arg)) != nullptr)
		{
			REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
			free_alias(a);
		}
    
		/* if no replacement string is specified, assume we want to delete */
    
		if (!*repl)
		{
      
			if (a == nullptr)
				send_to_char("No such alias.\r\n", ch);
      
			else
				send_to_char("Alias deleted.\r\n", ch);
		} 
		
		else
		{			/* otherwise, either add or redefine an alias */
      
			if (!str_cmp(arg, "alias"))
			{
				send_to_char("You can't alias 'alias'.\r\n", ch);
				return;
			}
      
			CREATE(a, struct alias, 1);
//			a = new struct alias;
			strcpy(a->name, arg);
			delete_doubledollar(repl);
			strcpy(a->replacement, repl);

			a->next = GET_ALIASES(ch);
			GET_ALIASES(ch) = a;
			send_to_char("Alias added.\r\n", ch);
		}
	}
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
	struct txt_q temp_queue;
	char *tokens[NUM_TOKENS], *temp, *write_point;
	int num_of_tokens = 0, num;

	/* First, parse the original string */
	temp = strtok(strcpy(buf2, orig), " ");
  
	while (temp != nullptr && num_of_tokens < NUM_TOKENS)
	{
		tokens[num_of_tokens++] = temp;
		temp = strtok(nullptr, " ");
	}

	/* initialize */
	write_point = buf;
	temp_queue.head = temp_queue.tail = nullptr;

	/* now parse the alias */
	for (temp = a->replacement; *temp; temp++)
	{
		if (*temp == ALIAS_SEP_CHAR)
		{
			*write_point = '\0';
			buf[MAX_INPUT_LENGTH - 1] = '\0';
			write_to_q(buf, &temp_queue, 1);
			write_point = buf;
		} 
		
		else if (*temp == ALIAS_VAR_CHAR)
		{
			temp++;
      
			if ((num = *temp - '1') < num_of_tokens && num >= 0)
			{
				strcpy(write_point, tokens[num]);
				write_point += strlen(tokens[num]);
			} 
			
			else if (*temp == ALIAS_GLOB_CHAR)
			{
				strcpy(write_point, orig);
				write_point += strlen(orig);
			} 
			
			else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act safety */
				*(write_point++) = '$';
		} 
		
		else
			*(write_point++) = *temp;
	}

	*write_point = '\0';
	buf[MAX_INPUT_LENGTH - 1] = '\0';
	write_to_q(buf, &temp_queue, 1);

	/* push our temp_queue on to the _front_ of the input queue */
	if (input_q->head == nullptr)
		*input_q = temp_queue;
	else
	{
		temp_queue.tail->next = input_q->head;
		input_q->head = temp_queue.head;
	}
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
	char first_arg[MAX_INPUT_LENGTH], *ptr;
	struct alias *a, *tmp;

	/* Mobs don't have alaises. */
	if (IS_NPC(d->character))
		return 0;

	/* bail out immediately if the guy doesn't have any aliases */
	if ((tmp = GET_ALIASES(d->character)) == nullptr)
		return 0;

	/* find the alias we're supposed to match */
	ptr = any_one_arg(orig, first_arg);

	/* bail out if it's null */
	if (!*first_arg)
		return 0;

	/* if the first arg is not an alias, return without doing anything */
	if ((a = find_alias(tmp, first_arg)) == nullptr)
		return 0;

	strcpy(orig, a->replacement);
	return 0;

	
/*	else
	{
		perform_complex_alias(&d->input, ptr, a);
		return 1;
	}
*/
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, const char **listy, int exact)
{
	int i, l;

	/* Make into lower case, and get length of string */
	for (l = 0; *(arg + l); l++)
		*(arg + l) = LOWER(*(arg + l));

	if (exact)
	{
		for (i = 0; **(listy + i) != '\n'; i++)
			if (!strcmp(arg, *(listy + i)))
				return (i);
	} 
	
	else
	{
		if (!l)
			l = 1;			/* Avoid "" to match the first available
				 * string */
		for (i = 0; **(listy + i) != '\n'; i++)
			if (!strncmp(arg, *(listy + i), l))
				return (i);
	}

	return -1;
}


int is_number(const char *str)
{
	while (*str)
		if (!isdigit(*(str++)))
			return 0;

	return 1;
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
	for (; **string && isspace(**string); (*string)++);
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
	char *read, *write;

	/* If the string has no dollar signs, return immediately */
	if ((write = strchr(string, '$')) == nullptr)
		return string;

	/* Start from the location of the first dollar sign */
	read = write;


	while (*read)   /* Until we reach the end of the string... */
		if ((*(write++) = *(read++)) == '$') /* copy one char */
			if (*read == '$')
				read++; /* skip if we saw 2 $'s in a row */

	*write = '\0';

	return string;
}


int fill_word(char *argument)
{
	return (search_block(argument, filler, TRUE) >= 0);
}


int reserved_word(char *argument)
{
	return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
	char *begin = first_arg;

	if (!argument)
	{
		log("SYSERR: one_argument received a nullptr pointer!");
		*first_arg = '\0';
		return nullptr;
	}

	do
	{
		skip_spaces(&argument);

		first_arg = begin;
    
		while (*argument && !isspace(*argument))
		{
			*(first_arg++) = LOWER(*argument);
			argument++;
		}

		*first_arg = '\0';
	}

	while (fill_word(begin));

	return argument;
}



/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
	char *begin = first_arg;

	do
	{
		skip_spaces(&argument);

		first_arg = begin;

		if (*argument == '\"')
		{
			argument++;
			while (*argument && *argument != '\"')
			{
				*(first_arg++) = LOWER(*argument);
				argument++;
			}
      
			argument++;
		} 
		
		else
		{
			while (*argument && !isspace(*argument))
			{
				*(first_arg++) = LOWER(*argument);
				argument++;
			}
		}

		*first_arg = '\0';
	} 
	
	while (fill_word(begin));

	return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
	skip_spaces(&argument);

	while (*argument && !isspace(*argument))
	{
		*(first_arg++) = LOWER(*argument);
		argument++;
	}

	*first_arg = '\0';

	return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
	return one_argument(one_argument(argument, first_arg), second_arg); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(const char *arg1, const char *arg2)
{
	if (!*arg1)
		return 0;

	for (; *arg1 && *arg2; arg1++, arg2++)
		if (LOWER(*arg1) != LOWER(*arg2))
			return 0;

	if (!*arg1)
		return 1;
  
	else
		return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
	char *temp;

	temp = any_one_arg(string, arg1);
	skip_spaces(&temp);

	// Serai - to fix some more valgrind errors - 06/11/04
	memmove(arg2, temp, strlen(temp) + 1);
//	strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(const char *command)
{
	int cmd;

	for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
		if (!strcmp(complete_cmd_info[cmd].command, command))
			return cmd;

	return -1;
}

void read_ignores(char_data *ch)
{
	struct ignore_data *i;
	char filename[MAX_STRING_LENGTH], line[MAX_STRING_LENGTH];
	FILE *file;
	int p = 0;

	if(!ch)
		return;

	get_filename(GET_NAME(ch), filename, IGNORE_FILE);

	if(!(file = fopen(filename, "r")))
		return;

	while(fgets(line, MAX_STRING_LENGTH, file))
	{

		for(p = 0;line[p];p++)
		{
			if(!isalpha(line[p]))
			{
				line[p] = '\0';
				break;
			}
		}

		i = new ignore_data;
		strcpy(i->name, line);
		i->next = ch->ignores;
		ch->ignores = i;
	}

	fclose(file);
}

void save_ignores(char_data *ch)
{
	struct ignore_data *i;
	char filename[MAX_STRING_LENGTH];
	FILE *file;

	if(!ch)
		return;

	get_filename(GET_NAME(ch), filename, IGNORE_FILE);

	if(!(file = fopen(filename, "w+")))
		return;

	for(i = ch->ignores;i;i = i->next)
	{
		fprintf(file, "%s\n", i->name);
	}

	fclose(file);
}

int is_ignoring(char_data *ch, char *person)
{
	struct ignore_data *i;

	if(!ch || !person)
		return 0;

	for(i = ch->ignores;i;i = i->next)
	{
		if(!str_cmp(person, i->name))
		{
			return 1;
		}
	}

	return 0;
}

int special(char_data *ch, int cmd, char *arg)
{
	struct obj_data *i;
	char_data *k;
	int j;

	/* special in room? */
	if (GET_ROOM_SPEC(ch->in_room) != nullptr)
		if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
			return 1;

	/* special in equipment list? */
	for (j = 0; j < NUM_WEARS; j++)
		if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != nullptr)
			if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
				return 1;

	/* special in inventory? */
	for (i = ch->carrying; i; i = i->next_content)
	{
		if (GET_OBJ_SPEC(i) != nullptr)
			if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
				return 1;
	}

	/* special in mobile present? */
	for (k = world[ch->in_room].people; k; k = k->next_in_room)
		if (GET_MOB_SPEC(k) != nullptr)
			if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
				return 1;

	/* special in object present? */
	for (i = world[ch->in_room].contents; i; i = i->next_content)
		if (GET_OBJ_SPEC(i) != nullptr)
			if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
				return 1;

	return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name, player_index_element *ptable, int ptable_top)
{
	int i;

	for (i = 0; i <= ptable_top; i++) 
	{
		if (!str_cmp((ptable + i)->name, name))
			return i;
	}

	return -1;
}


int _parse_name(char *arg, char *name)
{
	int i;

	/* skip whitespaces */
	for (; isspace(*arg); arg++);

	for (i = 0; (*name = *arg); arg++, i++, name++)
		if (!isalpha(*arg))
			return 1;

	if (!i)
		return 1;

	return 0;
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

/*
 * XXX: Make immortals 'return' instead of being disconnected when switched
 *      into person returns.  This function seems a bit over-extended too.
 */
int perform_dupe_check(struct descriptor_data *d)
{
	struct descriptor_data *k, *next_k;
	char_data *target = nullptr, *ch, *next_ch;
	int mode = 0;


	int id = GET_IDNUM(d->character);

	/*
	 * Now that this descriptor has successfully logged in, disconnect all
	 * other descriptors controlling a character with the same ID number.
	 */

	for (k = descriptor_list; k; k = next_k)
	{
		next_k = k->next;

		if (k == d)
			continue;

		if (k->original && (GET_IDNUM(k->original) == id))
		{    /* switched char */
			SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
			STATE(k) = CON_CLOSE;
		
			if (!target)
			{
				target = k->original;
				mode = UNSWITCH;
			}
      
			if (k->character)
				k->character->desc = nullptr;
		
			k->character = nullptr;
			k->original = nullptr;
		}

		else if (k->character && (GET_IDNUM(k->character) == id))
		{
			if (!target && STATE(k) == CON_PLAYING)
			{
				SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
				target = k->character;
				mode = USURP;
			}
      
			k->character->desc = nullptr;
			k->character = nullptr;
			k->original = nullptr;
			SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
			STATE(k) = CON_CLOSE;
		}
	}

	/*
	 * now, go through the character list, deleting all characters that
	 * are not already marked for deletion from the above step (i.e., in the
	 * CON_HANGUP state), and have not already been selected as a target for
	 * switching into.  In addition, if we haven't already found a target,
	 * choose one if one is available (while still deleting the other
	 * duplicates, though theoretically none should be able to exist).
	 */

	for (ch = character_list; ch; ch = next_ch)
	{
		next_ch = ch->next;

		if (IS_NPC(ch))
			continue;
    
		if (GET_IDNUM(ch) != id)
			continue;

		/* ignore chars with descriptors (already handled by above step) */
		if (ch->desc)
			continue;

		/* don't extract the target char we've found one already */
		if (ch == target)
			continue;

		/* we don't already have a target and found a candidate for switching */
		if (!target)
		{
			target = ch;
			mode = RECON;
			continue;
		}

		/* we've found a duplicate - blow him away, dumping his eq in limbo. */
		if (ch->in_room != NOWHERE)
			char_from_room(ch);
    
		char_to_room(ch, 1);
		extract_char(ch);
	}

	/* no target for swicthing into was found - allow login to continue */
	if (!target)
		return 0;

	/* Okay, we've found a target.  Connect d to target. */
	reset_char(d->character);
	free_char(d->character); /* get rid of the old char */
	d->character = target;
	d->character->desc = d;
	d->original = nullptr;
	d->character->char_specials.timer = 0;
	REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	STATE(d) = CON_PLAYING;

	switch (mode)
	{
		
	case RECON:
		SEND_TO_Q("Reconnecting.\r\n", d);
		act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
		sprintf(buf, "%s has reconnected.", GET_NAME(d->character));
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
		break;
  
	case USURP:
		SEND_TO_Q("You take over your own body, already in use!\r\n", d);
		act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
		"$n's body has been taken over by a new spirit!",
		TRUE, d->character, 0, 0, TO_ROOM);
		sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
		GET_NAME(d->character));
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
		break;
  
	case UNSWITCH:
		SEND_TO_Q("Reconnecting to unswitched char.", d);
		sprintf(buf, "%s has reconnected.", GET_NAME(d->character));
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
		break;
	}

	return 1;
}

void fix_name(char *name)
{
	int i = 0, len = 0;;
	char *pt;

	if(!name)
	{
		return;
	}

	len = strlen(name);
	pt = name;

	for(i = 0;i < len;i++, name++)
	{
		if(i)
			*name = tolower(*name);
	}

	name = pt;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
	char buf[128];
	int load_result;
	char tmp_name[MAX_INPUT_LENGTH];
	sh_int load_room = r_frozen_start_room;

	skip_spaces(&arg);

	switch (STATE(d))
	{

	/*. OLC states .*/
	case CON_HEDIT:
		hedit_parse(d, arg);
		break;

	case CON_OEDIT: 
		oedit_parse(d, arg);
		break;

	case CON_REDIT: 
		redit_parse(d, arg);
		break;

	case CON_ZEDIT: 
		zedit_parse(d, arg);
		break;

	case CON_MEDIT: 
		medit_parse(d, arg);
		break;
  
	case CON_SEDIT: 
		sedit_parse(d, arg);
		break;

	case CON_AEDIT:
		aedit_parse(d, arg);
		break;

	case CON_KEDIT:
		kedit_parse(d, arg);
		break;

	case CON_TRIGEDIT:
		trigedit_parse(d, arg);
		break;
		/*. End of OLC states .*/

	case CON_GET_NAME:		/* wait for input of name */

		if (d->character == nullptr)
		{
			CREATE(d->character, char_data, 1);
			clear_char(d->character);
			CREATE(d->character->player_specials, player_special_data, 1);
			d->character->desc = d;
			strcpy(d->character->points.host, d->host);
		}

		if (!*arg)
			STATE(d) = CON_CLOSE;

		else
		{
			if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
			strlen(tmp_name) > MAX_NAME_LENGTH || !Valid_Name(tmp_name) ||
			fill_word(strcpy(buf, tmp_name)) || reserved_word(buf))
			{
				SEND_TO_Q(	"Invalid name, please try another.\r\n"
							"Name: ", d);
				return;
			}

			if (d->character->load(tmp_name))
			{

				if (PLR_FLAGGED(d->character, PLR_DELETED))
				{
					/* We get a false positive from the original deleted character. */
					free_char(d->character);
					d->character = nullptr;
					/* Check for multiple creations... */

					if (!Valid_Name(tmp_name))
					{
						SEND_TO_Q("Invalid name, please try another.\r\nName: ", d);
						return;
					}

					fix_name(tmp_name);
					CREATE(d->character, char_data, 1);
					clear_char(d->character);
					CREATE(d->character->player_specials, struct player_special_data, 1);
					d->character->desc = d;
					GET_NAME(d->character) = str_dup(CAP(tmp_name));
					sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
					SEND_TO_Q(buf, d);
					STATE(d) = CON_NAME_CNFRM;
				} 

				else
				{

					/* undo it just in case they are set */
					REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
					REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
					REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRYO);

					SEND_TO_Q("Password: ", d);
					echo_off(d);
					d->idle_tics = 0;
					STATE(d) = CON_PASSWORD;
				}
			}

			else
			{
				/* player unknown -- make new character */

				/* Check for multiple creations of a character. */
				if (!Valid_Name(tmp_name))
				{
					SEND_TO_Q("Invalid name, please try another.\r\nName: ", d);
					return;
				}

				CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
				strcpy(d->character->player.name, CAP(tmp_name));

				fix_name(tmp_name);
				sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
				SEND_TO_Q(buf, d);
				STATE(d) = CON_NAME_CNFRM;
			}
		}

		break;

	case CON_NAME_CNFRM:		/* wait for conf. of new name    */

		if (UPPER(*arg) == 'Y')
		{
			if (isbanned(d->host) >= BAN_NEW)
			{
				sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
				GET_NAME(d->character), d->host);
				mudlog(buf, NRM, LVL_APPR, TRUE);
				SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
				STATE(d) = CON_CLOSE;
				return;
			}

			if (circle_restrict)
			{
				SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
				sprintf(buf, "Request for new char %s denied from [%s] (wizlock)",
				GET_NAME(d->character), d->host);
				mudlog(buf, NRM, LVL_APPR, TRUE);
				STATE(d) = CON_CLOSE;
				return;
			}

			SEND_TO_Q("New character.\r\n", d);
			sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
			SEND_TO_Q(buf, d);
			echo_off(d);
			STATE(d) = CON_NEWPASSWD;
		}

		else if (*arg == 'n' || *arg == 'N')
		{
			SEND_TO_Q("Okay, what IS it, then? ", d);
			delete[] (d->character->player.name);
			d->character->player.name = nullptr;
			STATE(d) = CON_GET_NAME;
		}

		else
		{
			SEND_TO_Q("Please type Yes or No: ", d);
		}

		break;

	case CON_PASSWORD:		/* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */

		echo_on(d);    /* turn echo back on */

		if (!*arg)
			STATE(d) = CON_CLOSE;

		else
		{
			if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH))
			{
				sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), !LOCAL_IP(d) ? d->host : "");
				mudlog(buf, BRF, LVL_GOD, TRUE);
				GET_BAD_PWS(d->character)++;
				d->character->save();

				if (++(d->bad_pws) >= max_bad_pws)
				{	/* 3 strikes and you're out. */
					SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
					STATE(d) = CON_CLOSE;
				}

				else
				{
					SEND_TO_Q("Wrong password.\r\nPassword: ", d);
					echo_off(d);
				}

				return;
			}

			/* Password was correct. */
			load_result = GET_BAD_PWS(d->character);
			GET_BAD_PWS(d->character) = 0;
			d->bad_pws = 0;

			if (isbanned(d->host) == BAN_SELECT &&
			!PLR_FLAGGED(d->character, PLR_SITEOK))
			{

				SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
				STATE(d) = CON_CLOSE;
				sprintf(buf, "Connection attempt for %s denied from %s",
				GET_NAME(d->character), d->host);
				mudlog(buf, NRM, LVL_GOD, TRUE);
				return;
			}

			if (GET_LEVEL(d->character) < circle_restrict)
			{
				SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
				STATE(d) = CON_CLOSE;
				sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
				GET_NAME(d->character), d->host);
				mudlog(buf, NRM, LVL_GOD, TRUE);
				return;
			}

			/* check and make sure no other copies of this player are logged in */
			if (perform_dupe_check(d))
				return;

			if (GET_LEVEL(d->character) >= LVL_IMMORT)
				SEND_TO_Q(imotd, d);

			else
				SEND_TO_Q(motd, d);

			sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character),
				!(LOCAL_IP(d) && d->character && !IS_KING(d->character)) ? d->host : "");
			mudlog(buf, BRF, MAX(LVL_GRGOD, GET_INVIS_LEV(d->character)), TRUE);

			if (load_result)
			{
				sprintf(buf,	"\r\n\r\n\007\007\007"
								"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
				COLOR_RED(d->character, CL_SPARSE), load_result,
				(load_result > 1) ? "S" : "", COLOR_NORMAL(d->character, CL_SPARSE));

				SEND_TO_Q(buf, d);
				GET_BAD_PWS(d->character) = 0;
			}

			STATE(d) = CON_MENU;
			goto menu;
		}

	case CON_NEWPASSWD:
	case CON_CHPWD_GETNEW:

		if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
		!str_cmp(arg, GET_NAME(d->character)))
		{

			SEND_TO_Q("\r\nIllegal password.\r\n", d);
			SEND_TO_Q("Password: ", d);
			return;
		}

		strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
		*(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

		SEND_TO_Q("\r\nPlease retype password: ", d);

		if (STATE(d) == CON_NEWPASSWD)
			STATE(d) = CON_CNFPASSWD;

		else
			STATE(d) = CON_CHPWD_VRFY;

		break;

	case CON_CNFPASSWD:
	case CON_CHPWD_VRFY:

		if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH))
		{

			SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
			SEND_TO_Q("Password: ", d);

			if (STATE(d) == CON_CNFPASSWD)
				STATE(d) = CON_NEWPASSWD;

			else
				STATE(d) = CON_CHPWD_GETNEW;

			return;
		}

		echo_on(d);

		if (STATE(d) == CON_CNFPASSWD)
		{
			SEND_TO_Q("What is your sex (M/F)? ", d);
			STATE(d) = CON_QSEX;
		}

		else
		{
			d->character->save();
			echo_on(d);
			SEND_TO_Q("\r\nDone.\r\n", d);
			//SEND_TO_Q(MENU, d);
			STATE(d) = CON_MENU;
		}

		break;
	case CON_QSEX:		/* query sex of new user         */
		switch (*arg)
		{

		case 'm':
		case 'M':
			d->character->player.sex = SEX_MALE;
			break;

		case 'f':
		case 'F':
			d->character->player.sex = SEX_FEMALE;
			break;

		default:
			SEND_TO_Q(	"That is not a sex..\r\n"
						"What IS your sex? ", d);
			return;
		}
		SEND_TO_Q(race_menu, d);
		SEND_TO_Q("Race:\r\n", d);
		STATE(d) = CON_QRACE;
		break;

	case CON_QRACE:

		switch(*arg)
		{
		case 'h':
		case 'H':
			GET_RACE(d->character) = RACE_HUMAN;
			break;
		
		case 't':
		case 'T':
			GET_RACE(d->character) = RACE_TROLLOC;
			break;
		
//		case 'a':
//		case 'A':
//			GET_RACE(d->character) = RACE_AIEL;
//			break;

		default:
			SEND_TO_Q("\r\nThat's not a race.\r\nRace: ", d);
			return;
		}

		if(GET_RACE(d->character) == RACE_HUMAN || GET_RACE(d->character) == RACE_AIEL) 
		  SEND_TO_Q(human_class_menu, d);
		
		else
		  SEND_TO_Q(other_class_menu, d);
		
		SEND_TO_Q("Class:\r\n", d);
		STATE(d) = CON_QCLASS;
		break;


	case CON_QCLASS:
		switch(*arg)
		{
		
		case 'w':
		case 'W':
			GET_CLASS(d->character) = CLASS_WARRIOR;
			break;
		
		case 'r':
		case 'R':
			GET_CLASS(d->character) = CLASS_RANGER;
			break;
		
		case 't':
		case 'T':
			GET_CLASS(d->character) = CLASS_THIEF;
			break;

		case 'c':
		case 'C':
			if(GET_RACE(d->character) == RACE_HUMAN || GET_RACE(d->character) == RACE_AIEL)
			{
				GET_CLASS(d->character) = CLASS_CHANNELER;
				break;
			}

			else
			{
				write_to_output(d, "That is not a class!\r\nClass:");
				return;
			}

		default:
			GET_CLASS(d->character) = CLASS_UNDEFINED;
			SEND_TO_Q("That is not a class!\r\nClass:", d);
			return;
		}

		create_entry(d->character);

		d->character->init();
		d->character->save();

		SET_BIT_AR(PRF_FLAGS(d->character), PRF_COLOR_2);
		SET_BIT_AR(PRF_FLAGS(d->character), PRF_DISPHP);
		SET_BIT_AR(PRF_FLAGS(d->character), PRF_DISPMOVE);
		SET_BIT_AR(PRF_FLAGS(d->character), PRF_DISPMANA);
		SET_BIT_AR(PRF_FLAGS(d->character), PRF_AUTOEXIT);
		SEND_TO_Q(motd, d);
		STATE(d) = CON_MENU;

		sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), !LOCAL_IP(d) ? d->host : "");
		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(d->character)), TRUE);

	case CON_MENU:		/* get selection from main menu  */
	
	{
		//Newbie code, I know, but It is a hundred times easier than re-writing the entire menu -- Tulon//
		menu:

		reset_char(d->character);
		read_aliases(d->character);
		read_ignores(d->character);

		LAST_LOGON(d->character) = time(0);
		
		if (PLR_FLAGGED(d->character, PLR_INVSTART))
			GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      
		if ((load_result = Crash_load(d->character, 1)))
			d->character->in_room = NOWHERE;
		
	
		/* with the copyover patch, this next line goes in enter_player_game() */
		GET_ID(d->character) = GET_IDNUM(d->character);
		d->character->save();
		send_to_char(WELC_MESSG, d->character);
		d->character->next = character_list;
		character_list = d->character;
		check_boot_high();

		if (PLR_FLAGGED(d->character, PLR_FROZEN))
			load_room = r_frozen_start_room;

		else if (GET_LEVEL(d->character) >= LVL_IMMORT) 
			load_room = r_immort_start_room;

		else if(GET_LOADROOM(d->character) >= 0 && real_room(GET_LOADROOM(d->character)))
			load_room = real_room(GET_LOADROOM(d->character));

		else
		{
			if(IS_HUMAN(d->character) && GET_LEVEL(d->character) < LVL_IMMORT)
				load_room = r_human_start_room;
	
			else if(IS_TROLLOC(d->character) && GET_LEVEL(d->character) < LVL_IMMORT)
				load_room = r_trolloc_start_room;
		}

		char_to_room(d->character, load_room);
		act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

		sprintf(buf, "%s logging in at room %d.", GET_NAME(d->character), GET_LOADROOM(d->character));
		mudlog(buf, CMP, MAX(GET_INVIS_LEV(d->character), LVL_APPR), TRUE);

		GET_LOADROOM(d->character) = load_room;
		d->character->save();

		STATE(d) = CON_PLAYING;
		
		if (!GET_LEVEL(d->character))
		{
			command_interpreter(d->character, "newbie");
			do_start(d->character);
			roll_real_abils(d->character);
			roll_taveren(d->character);
			send_to_char(START_MESSG, d->character);
		}

		look_at_room(d->character, 0);
      
		if (has_mail(GET_IDNUM(d->character)))
			send_to_char("You have mail waiting.\r\n", d->character);
      
		if (load_result == 2)
		{	/* rented items lost */
			send_to_char("\r\n\007You could not afford your rent!\r\n",
			d->character);
		}
      
		d->has_prompt = 0;
		//break;

		/*case '2':
			
			  if (d->character->player.description) {
				SEND_TO_Q("Current description:\r\n", d);
				SEND_TO_Q(d->character->player.description, d);
	*//*
	 * Don't free this now... so that the old description gets loaded
	 * as the current buffer in the editor.  Do setup the ABORT buffer
	 * here, however.
	 *
	 * free(d->character->player.description);
	 * d->character->player.description = nullptr;
	 */
	/*d->backstr = str_dup(d->character->player.description);
      }
      SEND_TO_Q("Enter the new text you'd like others to see when they look at you.\r\n", d);
      SEND_TO_Q("(/s saves /h for help)\r\n", d);
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;*/

    /*case '3':
      page_string(d, background, 0);
      STATE(d) = CON_RMOTD;
      break;

    case '4':
      SEND_TO_Q("\r\nEnter your old password: ", d);
      echo_off(d);
      STATE(d) = CON_CHPWD_GETOLD;
      break;

    case '5':
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d);
      STATE(d) = CON_DELCNF1;
      break;

    default:
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      SEND_TO_Q(MENU, d);
      break;*/
		}

		break;

	case CON_CHPWD_GETOLD:
		
		if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH))
		{
			echo_on(d);
			SEND_TO_Q("\r\nIncorrect password.\r\n", d);
			SEND_TO_Q(MENU, d);
			STATE(d) = CON_MENU;
		} 
		
		else
		{
			SEND_TO_Q("\r\nEnter a new password: ", d);
			STATE(d) = CON_CHPWD_GETNEW;
		}
    
		return;

	case CON_DELCNF1:
    
		echo_on(d);
    
		if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH))
		{
			SEND_TO_Q("\r\nIncorrect password.\r\n", d);
			SEND_TO_Q(MENU, d);
			STATE(d) = CON_MENU;
		}	
		
		else
		{
			SEND_TO_Q(	"\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
						"ARE YOU ABSOLUTELY SURE?\r\n\r\n"
						"Please type \"yes\" to confirm: ", d);
			STATE(d) = CON_DELCNF2;
		}
		
		break;

	case CON_DELCNF2:
    
		if (!strcmp(arg, "yes") || !strcmp(arg, "YES"))
		{
			if (PLR_FLAGGED(d->character, PLR_FROZEN))
			{
				SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
				SEND_TO_Q("Character not deleted.\r\n\r\n", d);
				STATE(d) = CON_CLOSE;
				return;
			}
      
			if (GET_LEVEL(d->character) < LVL_GRGOD)
				SET_BIT_AR(PLR_FLAGS(d->character), PLR_DELETED);
     
			d->character->save();
			Crash_delete_file(GET_NAME(d->character));
			sprintf(buf,	"Character '%s' deleted!\r\n"
							"Goodbye.\r\n", GET_NAME(d->character));
      
			SEND_TO_Q(buf, d);
			sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
			GET_LEVEL(d->character));
			mudlog(buf, NRM, LVL_GOD, TRUE);
			STATE(d) = CON_CLOSE;
			return;
		} 
		
		else
		{
			SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
			SEND_TO_Q(MENU, d);
			STATE(d) = CON_MENU;
		}
    
		break;

/*	Taken care of in game_loop()
	case CON_CLOSE:
		close_socket(d);
		break;
*/

	default:
		log("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
		STATE(d), d->character ? GET_NAME(d->character) : "<unknown>");
		STATE(d) = CON_DISCONNECT;	/* Safest to do. */
		break;
	}
}
