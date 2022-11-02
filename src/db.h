/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD	0
#define DB_BOOT_MOB	1
#define DB_BOOT_OBJ	2
#define DB_BOOT_ZON	3
#define DB_BOOT_SHP	4
#define DB_BOOT_HLP	5
#define DB_BOOT_TRG	6
#define DB_BOOT_KIT	7

#if defined(CIRCLE_MACINTOSH)
#define LIB_WORLD	":world:"
#define LIB_TEXT	":text:"
#define LIB_TEXT_HELP	":text:help:"
#define LIB_MISC	":misc:"
#define LIB_ETC		":etc:"
#define LIB_PLRTEXT	":plrtext:"
#define LIB_PLROBJS	":plrobjs:"
#define LIB_HOUSE	":house:"
#define LIB_CHESTS	":chests:"
#define SLASH		":"
#elif defined(CIRCLE_AMIGA) || defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS) || defined(CIRCLE_ACORN)
#define LIB_WORLD	"world/"
#define LIB_TEXT	"text/"
#define LIB_TEXT_HELP	"text/help/"
#define LIB_MISC	"misc/"
#define LIB_ETC		"etc/"
#define LIB_PLRTEXT	"plrtext/"
#define LIB_OBJS	"objs/"
#define LIB_PLROBJS	"plrobjs/"
#define LIB_PLRALIAS	"plralias/"
#define LIB_PLRIGNORE	"plrignore/"
#define LIB_PLRTELL	"plrtell/"
#define LIB_PLRSAY	"plrsay/"
#define LIB_HOUSE	"house/"
#define LIB_PVARS	"pvars/"
#define LIB_CHESTS	"chests/"
#define LIB_SCRIPTS	"scripts/"
#define LIB_LOGS	"logs/"
#define LIB_QUESTS	"Clan Quests/"
#define SLASH		"/"
#endif


#define SUF_OBJS	"objs"
#define SUF_TEXT	"text"
#define SUF_ALIAS	"alias"
#define SUF_IGNORE	"ign"
#define SUF_TELL	"tell"
#define SUF_SAY		"say"
#define SUF_PVAR	"pvar"

#if defined(CIRCLE_AMIGA)
#define FASTBOOT_FILE   "/.fastboot"    /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "/.killscript"  /* autorun: shut mud down       */
#define PAUSE_FILE      "/pause"        /* autorun: don't restart mud   */
#elif defined(CIRCLE_MACINTOSH)
#define FASTBOOT_FILE	"::.fastboot"	/* autorun: boot without sleep	*/
#define KILLSCRIPT_FILE	"::.killscript"	/* autorun: shut mud down	*/
#define PAUSE_FILE	"::pause"	/* autorun: don't restart mud	*/
#else
#define FASTBOOT_FILE   "../.fastboot"  /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "../.killscript"/* autorun: shut mud down       */
#define PAUSE_FILE      "../pause"      /* autorun: don't restart mud   */
#endif

/* names of various files and directories */
#define INDEX_FILE		"index"		/* index of world files		*/
#define MINDEX_FILE		"index.mini"	/* ... and for mini-mud-mode	*/
#define WLD_PREFIX		LIB_WORLD "wld" SLASH	/* room definitions	*/
#define MOB_PREFIX		LIB_WORLD "mob" SLASH	/* monster prototypes	*/
#define OBJ_PREFIX		LIB_WORLD "obj" SLASH	/* object prototypes	*/
#define ZON_PREFIX		LIB_WORLD "zon" SLASH	/* zon defs & command tables */
#define SHP_PREFIX		LIB_WORLD "shp" SLASH	/* shop definitions	*/
#define TRG_PREFIX		LIB_WORLD "trg" SLASH	/* shop definitions	*/
#define HLP_PREFIX		LIB_TEXT "help" SLASH	/* for HELP <keyword>	*/
#define KIT_PREFIX		LIB_WORLD "kit" SLASH /* kit lists */
#define PVAR_INDEX		LIB_PVARS"index"
#define HELP_FILE		"help.hlp"		/* help file		*/
#define STARTUP_FILE	LIB_TEXT"startup"	/* startup screen	*/
#define CREDITS_FILE	LIB_TEXT"credits"/* for the 'credits' command	*/
#define NEWS_FILE		LIB_TEXT"news"	/* for the 'news' command	*/
#define MOTD_FILE		LIB_TEXT"motd"	/* messages of the day / mortal	*/
#define IMOTD_FILE		LIB_TEXT"imotd"	/* messages of the day / immort	*/
#define HELP_PAGE_FILE	LIB_TEXT_HELP"screen" /* for HELP <CR>		*/
#define INFO_FILE		LIB_TEXT"info"		/* for INFO		*/
#define WIZLIST_FILE	LIB_TEXT"wizlist"	/* for WIZLIST		*/
#define IMMLIST_FILE	LIB_TEXT"immlist"	/* for IMMLIST		*/
#define BACKGROUND_FILE	LIB_TEXT"background"/* for the background story	*/
#define POLICIES_FILE	LIB_TEXT"policies" /* player policies/rules	*/
#define HANDBOOK_FILE	LIB_TEXT"handbook" /* handbook for new immorts	*/

#define IDEA_FILE		LIB_MISC"ideas"	/* for the 'idea'-command		*/
#define TYPO_FILE		LIB_MISC"typos"	/*         'typo'				*/
#define BUG_FILE		LIB_MISC"bugs"	/*         'bug'				*/
#define MESS_FILE		LIB_MISC"messages" /* damage messages			*/
#define SOCMESS_FILE	LIB_MISC"socials" /* messgs for social acts		*/
#define XNAME_FILE		LIB_MISC"xnames" /* invalid name substrings		*/
#define NOTE_FILE		LIB_MISC"notes"	/* For note file				*/
#define STDERR			LIB_MISC"mudlog"	/* For mudlog file			*/
#define STATUS_FILE		LIB_MISC"status"	/* For status file			*/
#define PHP_FILE		LIB_MISC"phpinfo"	/* For the php info file	*/
#define CHEST_LOG_FILE	LIB_MISC"chestlog"	/* For the chest logs		*/

#define PLAYER_FILE		LIB_ETC"players" /* the player database			*/
#define PLAYER_FILE_TWO	LIB_ETC"players2" /* the player database		*/
#define MAIL_FILE		LIB_ETC"plrmail" /* for the mudmail system		*/
#define BAN_FILE		LIB_ETC"badsites" /* for the siteban system		*/
#define LEGEND_FILE		LIB_ETC"ta"       /*File for legend list		*/
#define BATTLE_FILE		LIB_ETC"battle"   /*File for Battle Record		*/
#define CLAN_FILE		LIB_ETC"clans"    /*File for clans				*/
#define LOGGER_FILE		LIB_ETC"logger"		/*File for logging commands	*/
#define HCONTROL_FILE	LIB_ETC"hcontrol"	/* for the house system		*/
#define VALUE_FILE		LIB_ETC"values"	/*File for Battle Record		*/

/* public procedures in db.c */
void	boot_db(void);
class player_index_element *create_entry(char_data *ch);
void	zone_update(void);
int		real_room(int vnum);
char	*fread_string(FILE *fl, char *error);
long	get_id_by_name(char *name);
char	*get_name_by_id(long id);

void	quick_save(struct char_file_u *tmp_store, int pos);
void	init_char(char_data *ch);
		char_data* create_char(void);
		char_data *read_mobile(int nr, int type);
int		real_mobile(int vnum);
int		vnum_mobile(char *searchname, char_data *ch);
int		count_mobs_mud(int mob_no);
int		count_mobs_room(int mob_no, int room);
int		count_mobs_zone(int mob_no, int znum);
int		count_objects_zone(int number, int znum);
int		count_objects_room(int number, int room);
int		count_objects_inv(int number, char_data *target);
int		playerExists(char *name);
void	clear_char(char_data *ch);
void	reset_char(char_data *ch);
void	free_char(char_data *ch);
void	chest_save_setup(struct obj_data *chest, int room);
void	save_chest(struct obj_data *obj, FILE *fl);
void	restore_chest(struct obj_data *chest, int room);
void	reset_zone(int zone);
void	playerAutoDelete(void);
void	save_clan_quests(int clan_number);
void	add_to_clan_list(char_data *ch, int p);

struct	obj_data *create_obj(void);
void	clear_object(struct obj_data *obj);
void	free_obj(struct obj_data *obj);
int		real_object(int vnum);
struct	obj_data *read_object(int nr, int type);
int		vnum_object(char *searchname, char_data *ch);

#define REAL 0
#define VIRTUAL 1

struct reset_com
{
   char	command;   /* current command                      */

   bool if_flag;	/* if TRUE: exe only if preceding exe'd */
   int	arg1;		/*                                      */
   int	arg2;		/* Arguments to the command             */
   int	arg3;		/*                                      */
   int  arg4;
   int	arg5;
   int	arg6;
   int	arg7;
   int	line;		/* line number this command appears on  */

/* whenever a mob is loaded with 'M', this is that mob */
   char_data *mob;

   /* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
   */
};



/* zone definition structure. for the 'zone-table'					*/
struct zone_data
{
	char *name;		    /* name of this zone						*/
	int	lifespan;           /* how long between resets (minutes)	*/
	int	age;                /* current age of this zone (minutes)	*/
	int	x;					/* X-axis for zone location				*/
	int y;					/* Y-axis for zone location				*/

	int bot;           /* starting room number for this zone			*/
	int top;           /* upper limit for rooms in this zone			*/
	int	reset_mode;         /* conditions for reset (see below)		*/
	int	number;		    /* virtual number of this zone				*/
	struct reset_com *cmd;   /* command table for reset				*/

	char *builders;	    /* for OLC.  OBuild like extention,			*
                             * part of OLC+							*/
	/*
	 *  Reset mode:                              *
	 *  0: Don't reset, and don't update age.    *
	 *  1: Reset if no PC's are located in zone. *
	 *  2: Just reset.                           *
	 */
};



/* for queueing zones for update   */
struct reset_q_element
{
	int	zone_to_reset;            /* ref to zone_data */
	struct reset_q_element *next;
};



/* structure for the update queue     */
struct reset_q_type
{
	struct reset_q_element *head;
	struct reset_q_element *tail;
};

/*Class for the player index */
class player_index_element
{

public:
	char *name;
	long id;

	player_index_element *next;
};


struct help_index_element
{
	char *keywords;
	char *entry;
	int min_level;
};


/* don't change these */
#define BAN_NOT 	0
#define BAN_NEW 	1
#define BAN_SELECT	2
#define BAN_ALL		3

#define BANNED_SITE_LENGTH    50
struct ban_list_element
{
	char site[BANNED_SITE_LENGTH+1];
	int	type;
	time_t date;
	char name[MAX_NAME_LENGTH+1];
	struct ban_list_element *next;
};


/* global buffering system */

#ifdef __DB_C__
char	buf[MAX_STRING_LENGTH];
char	buf1[MAX_STRING_LENGTH];
char	buf2[MAX_STRING_LENGTH];
char	arg[MAX_STRING_LENGTH];
#else
extern struct player_special_data dummy_mob;
extern char	buf[MAX_STRING_LENGTH];
extern char	buf1[MAX_STRING_LENGTH];
extern char	buf2[MAX_STRING_LENGTH];
extern char	arg[MAX_STRING_LENGTH];
#endif


extern const char	*OK;
extern const char	*NOPERSON;
extern const char	*NOEFFECT;

