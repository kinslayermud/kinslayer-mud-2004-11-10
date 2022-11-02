/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "buffer_opt.h"

/*
 * Intended use of this macro is to allow external packages to work with
 * a variety of CircleMUD versions without modifications.  For instance,
 * an IS_CORPSE() macro was introduced in pl13.  Any future code add-ons
 * could take into account the CircleMUD version and supply their own
 * definition for the macro if used on an older version of CircleMUD.
 * You are supposed to compare this with the macro CIRCLEMUD_VERSION()
 * in utils.h.  See there for usage.
 */
#define _CIRCLEMUD	0x03000D /* Major/Minor/Patchlevel - MMmmPP */

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/

#define SPECIAL(name) \
   int (name)(class char_data *ch, void *me, int cmd, char *argument)

/* misc editor defines **************************************************/

#define TYPE_MOVEMENT	0
#define TYPE_MOUNT		1
#define TYPE_SPEACH		2
#define TYPE_BATTLE		3

#define MAX_TYPE		4
#define MAX_NOISE_DEPTH	3
#define NOISE_SHOW_LEVEL	3

/* format modes for format_text */
#define FORMAT_INDENT		(1 << 0)


/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK			0	/* Dark								*/
#define ROOM_DEATH			1	/* Death trap						*/
#define ROOM_NOMOB			2	/* MOBs not allowed					*/
#define ROOM_INDOORS		3	/* Indoors						*/
#define ROOM_PEACEFUL		4	/* Violence not allowed			*/
#define ROOM_SOUNDPROOF		5	/* Shouts, gossip blocked		*/
#define ROOM_NOTRACK		6	/* Track won't go through		*/
#define ROOM_NOMAGIC		7	/* Magic not allowed			*/
#define ROOM_TUNNEL			8	/* room for only 1 pers				*/
#define ROOM_PRIVATE		9	/* Can't teleport in			*/
#define ROOM_GODROOM		10	/* LVL_GOD+ only allowed		*/
#define ROOM_HOUSE			11	/* (R) Room is a house				*/
#define ROOM_HOUSE_CRASH	12	/* (R) House needs saving		*/
#define ROOM_ATRIUM			13	/* (R) The door to a house			*/
#define ROOM_OLC			14	/* (R) Modifyable/!compress			*/
#define ROOM_BFS_MARK		15	/* (R) breath-first srch mrk	*/
#define ROOM_NOPORT			16	/* (R) breath-first srch mrk	*/


/* Exit info: used in room_data.dir_option.exit_info			*/
#define EX_ISDOOR		(1 << 0)	/* Exit is a door			*/
#define EX_CLOSED		(1 << 1)	/* The door is closed		*/
#define EX_LOCKED		(1 << 2)	/* The door is locked		*/
#define EX_PICKPROOF	(1 << 3)	/* Lock can't be picked		*/


/* Sector types: used in room_data.sector_type					*/
#define SECT_INSIDE          0		   /* Indoors				*/
#define SECT_CITY            1		   /* In a city				*/
#define SECT_FIELD           2		   /* In a field			*/
#define SECT_FOREST          3		   /* In a forest			*/
#define SECT_HILLS           4		   /* In the hills			*/
#define SECT_MOUNTAIN        5		   /* On a mountain			*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater			*/
#define SECT_FLYING			 9		   /* Wheee!				*/


/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED		-1
#define CLASS_WARRIOR		0
#define CLASS_THIEF			1
#define CLASS_RANGER		2
#define CLASS_CHANNELER		3
#define CLASS_FADE			4
#define CLASS_DREADLORD		5
#define CLASS_BLADEMASTER	6
#define CLASS_GREYMAN		7
#define CLASS_DRAGHKAR		8

#define NUM_CLASSES			9  /* This must be the number of classes!! */

#define RACE_UNDEFINED -1
#define RACE_HUMAN      0
#define RACE_TROLLOC    1
#define RACE_SEANCHAN   2
#define RACE_AIEL       3
#define RACE_ANIMAL     4
#define RACE_OGIER		5
#define RACE_OTHER      20

#define NUM_RACES       6

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2


/* Positions */
#define POS_DEAD		0	// dead			//
#define POS_MORTALLYW	1	// mortally wounded	//
#define POS_INCAP		2	// incapacitated	//
#define POS_STUNNED		3	// stunned		//
#define POS_SLEEPING	4	// sleeping		//
#define POS_RESTING		5	// resting		//
#define POS_SITTING		6	// sitting		//
#define POS_FIGHTING	7	// fighting		//
#define POS_STANDING	8	// standing		//


/* Player flags: used by class char_data.char_specials.act					*/
#define PLR_DARKFRIEND	(0)		/* Player is a darkfriend				*/
#define PLR_MURDERER	(1)		/* Player is a player-thief				*/
#define PLR_FROZEN		(2)		/* Player is frozen						*/
#define PLR_DONTSET		(3)		/* Don't EVER set (ISNPC bit)			*/
#define PLR_WRITING		(4)		/* Player writing (board/mail/olc)		*/
#define PLR_MAILING		(5)		/* Player is writing mail				*/
#define PLR_CRASH		(6)		/* Player needs to be crash-saved		*/
#define PLR_SITEOK		(7)		/* Player has been site-cleared			*/
#define PLR_NOSHOUT		(8)		/* Player not allowed to shout/goss		*/
#define PLR_NOTITLE		(9)		/* Player not allowed to set title		*/
#define PLR_DELETED		(10)	/* Player deleted - space reusable		*/
#define PLR_LOADROOM	(11)	/* Player uses nonstandard loadroom		*/
#define PLR_NOWIZLIST	(12)	/* Player shouldn't be on wizlist		*/
#define PLR_NODELETE	(13)	/* Player shouldn't be deleted			*/
#define PLR_INVSTART	(14)	/* Player should enter game wizinvis	*/
#define PLR_CRYO		(15)	/* Player is cryo-saved (purge prog)	*/
#define PLR_LOGGER		(16)	/* Player is hooked up to a logger		*/
#define PLR_ZONE_BAN	(19)	/* Player is banned from leaving a zone	*/


/* Mobile flags: used by class char_data.char_specials.act								*/
#define MOB_SPEC			(0)		/* Mob has a callable spec-proc					*/
#define MOB_SENTINEL		(1)		/* Mob should not move							*/
#define MOB_SCAVENGER		(2)		/* Mob picks up stuff on the ground				*/
#define MOB_ISNPC			(3)		/* (R) Automatically set on all Mobs			*/
#define MOB_AWARE			(4)		/* Mob can't be backstabbed						*/
#define MOB_STAY_ZONE		(5)		/* Mob shouldn't wander out of zone				*/
#define MOB_WIMPY			(6)		/* Mob flees if severely injured				*/
#define MOB_MEMORY			(7)		/* remember attackers if attacked				*/
#define MOB_HELPER			(8)		/* attack PCs fighting other NPCs				*/
#define MOB_NOSLEEP			(9)		/* Mob can't be slept							*/
#define MOB_NOBLIND			(10)	/* Mob can't be blinded							*/
#define MOB_MOUNT			(11)	/* Mob is a mount								*/
#define MOB_TRACK			(12)	/* Currently being tested: Tracking mobs		*/
#define MOB_BASH			(13)	/* Mob can randomly bash while engaged			*/
#define MOB_AWARD			(14)	/* Mob is a master of a clan					*/
#define MOB_SHADOW_MOUNT	(15)	/* Mob is a horse for the Shadow				*/
#define MOB_NOFIGHT			(16)	/* Mob does not fight back						*/
#define MOB_INVIS			(17)	/* Mob can only be seen by immortals			*/

/* Preference flags: used by class char_data.player_specials.pref					*/
#define PRF_BRIEF			(0)		/* Room descs won't normally be shown	*/
#define PRF_COMPACT			(1)		/* No extra CRLF pair before prompts	*/
#define PRF_SOURCE			(2)		/* Is in touch with the True Source		*/
#define PRF_NOTELL			(3)		/* Can't receive tells					*/
#define PRF_DISPHP			(4)		/* Display hit points in prompt			*/
#define PRF_DISPMANA		(5)		/* Display mana points in prompt		*/
#define PRF_DISPMOVE		(6)		/* Display move points in prompt		*/
#define PRF_AUTOEXIT		(7)		/* Display exits in a room				*/
#define PRF_NOHASSLE		(8)		/* Aggr mobs won't attack				*/
#define PRF_QUEST			(9)		/* On quest								*/
#define PRF_SUMMONABLE		(10)	/* Can be summoned						*/
#define PRF_NOREPEAT		(11)	/* No repetition of comm commands		*/
#define PRF_HOLYLIGHT		(12)	/* Can see in dark						*/
#define PRF_COLOR_1			(13)	/* Color (low bit)						*/
#define PRF_COLOR_2			(14)	/* Color (high bit)						*/
#define PRF_NOWIZ			(15)	/* Can't hear wizline					*/
#define PRF_LOG1			(16)	/* On-line System Log (low bit)			*/
#define PRF_LOG2			(17)	/* On-line System Log (high bit)		*/
#define PRF_SPAREONE		(18)	/* Can't hear auction channel			*/
#define PRF_SPARETWO		(19)	/* Can't hear gossip channel			*/
#define PRF_SPARETHREE		(20)	/* Can't hear grats channel				*/
#define PRF_ROOMFLAGS		(21)	/* Can see room flags (ROOM_x)			*/
#define PRF_KIT				(22)	/* Has he had a kit or not.				*/
#define PRF_STATTED			(23)	/* Char has been statted				*/
#define PRF_COUNCIL			(24)	/* Char has Council abilities			*/
#define PRF_INCOG			(25)	/* Char is incognito					*/
#define PRF_TELL_MUTE		(26)	/* Char cannot send tells				*/
#define PRF_SAY_MUTE		(27)	/* Char cannot talk						*/
#define PRF_NONARR			(28)	/* Can't hear narrate channel			*/
#define PRF_NOCHAT			(29)	/* Can't hear chat channel				*/
#define PRF_NOYELL			(30)	/* Can't hear yell channel				*/
#define PRF_NOSHOUT			(31)	/* Can't hear shout channel				*/
#define PRF_BUILDWALK		(32)	/* Build new rooms when walking			*/
#define PRF_SPAM			(33)	/* Player Spam on/off					*/

/* Affect bits: used in class char_data.char_specials.saved.affected_by		*/
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved")	*/
#define AFF_DONOTUSE			0		/* NEVER USE!!!!					*/
#define AFF_BLIND				1		/* (R) Char is blind				*/
#define AFF_INVISIBLE			2		/* Char is invisible				*/
#define AFF_WATERWALK			3		/* Char can walk on water			*/
#define AFF_SANCTUARY			4		/* Char protected by sanct.			*/
#define AFF_GROUP				5		/* (R) Char is grouped				*/
#define AFF_POISON				6		/* (R) Char is poisoned				*/
#define AFF_SLEEP				7		/* (R) Char magically asleep		*/
#define AFF_INCOGNITO			8		/* Room for future expansion		*/
#define AFF_SNEAK				9		/* Char can move quietly			*/
#define AFF_HIDE				10		/* Char is hidden					*/
#define AFF_NOTICE				11		/* Char has notice on				*/
#define AFF_NOQUIT				12		/* Char cannot quit					*/
#define AFF_PARANOIA			13		/* Char is paranoid					*/
#define AFF_NIGHT_VISION		14		/* Char can see in the dark			*/
#define AFF_HASTE				15		/* Char is affected by haste		*/
#define AFF_SHIELD				16		/* Char is blocked from channeling	*/
#define AFF_EFFUSION			17		/* Char leaves unnoticed			*/
#define AFF_AGILITY				18		/* More Dodge Bonus					*/
#define AFF_STRENGTH			19		/* Char has boosted strength		*/

/* Modes of connectedness: used by descriptor_data.state					*/
#define CON_PLAYING			0		/* Playing - Nominal state				*/
#define CON_CLOSE			1		/* Disconnecting						*/
#define CON_GET_NAME		2		/* By what name ..?						*/
#define CON_NAME_CNFRM		3		/* Did I get that right, x?				*/
#define CON_PASSWORD		4		/* Password:							*/
#define CON_NEWPASSWD		5		/* Give me a password for x				*/
#define CON_CNFPASSWD		6		/* Please retype password:				*/
#define CON_QSEX			7		/* Sex?									*/
#define CON_QCLASS			8		/* Class?								*/
#define CON_RMOTD			9		/* PRESS RETURN after MOTD				*/
#define CON_MENU			10		/* Your choice: (main menu)				*/
#define CON_EXDESC			11		/* Enter a new description:				*/
#define CON_CHPWD_GETOLD	12		/* Changing passwd: get old				*/
#define CON_CHPWD_GETNEW	13		/* Changing passwd: get new				*/
#define CON_CHPWD_VRFY		14		/* Verify new password					*/
#define CON_DELCNF1			15		/* Delete confirmation 1				*/
#define CON_DELCNF2			16		/* Delete confirmation 2				*/
#define CON_DISCONNECT		17		/* In-game disconnection				*/
#define CON_OEDIT			18		/*. OLC mode - object edit				*/
#define CON_REDIT			19		/*. OLC mode - room edit       			*/
#define CON_ZEDIT			20		/*. OLC mode - zone info edit  			*/
#define CON_MEDIT			21		/*. OLC mode - mobile edit     			*/
#define CON_SEDIT			22		/*. OLC mode - shop edit				*/
#define CON_TRIGEDIT		23		/*. OLC mode - trigger edit    			*/
#define CON_HEDIT			24		/*. OLC mode - help editor     			*/
#define CON_AEDIT			25		/*. OLC mode - action editor   			*/
#define CON_TEXTED			26		/*. OLC mode - text editor     			*/
#define CON_QRACE			27		/* Race?								*/
#define CON_KEDIT			28		/*. OLC mode - kit editor				*/

/* Character equipment positions: used as index for class char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_HOLD		1
#define WEAR_NECK_1     2
#define WEAR_NECK_2     3
#define WEAR_HEAD       4
#define WEAR_ABOUT		5
#define WEAR_BACK		6
#define WEAR_BODY       7
#define WEAR_ARMS		8
#define WEAR_WAIST		9
#define WEAR_WRIST_R	10
#define WEAR_WRIST_L	11
#define WEAR_HANDS      12
#define WEAR_FINGER_R   13
#define WEAR_FINGER_L   14
#define WEAR_WIELD		15
#define WEAR_SHIELD		16
#define WEAR_LEGS		17
#define WEAR_FEET       18

#define NUM_WEARS		19	/* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT		1		/* Item is a light source		*/
#define ITEM_SCROLL		2		/* Item is a scroll				*/
#define ITEM_WAND		3		/* Item is a wand				*/
#define ITEM_STAFF		4		/* Item is a staff				*/
#define ITEM_WEAPON		5		/* Item is a weapon				*/
#define ITEM_FIREWEAPON	6		/* Unimplemented				*/
#define ITEM_MISSILE	7		/* Unimplemented				*/
#define ITEM_TREASURE	8		/* Item is a treasure, not gold	*/
#define ITEM_ARMOR		9		/* Item is armor				*/
#define ITEM_POTION		10 		/* Item is a potion				*/
#define ITEM_WORN		11		/* Unimplemented				*/
#define ITEM_OTHER		12		/* Misc object					*/
#define ITEM_TRASH		13		/* Trash - shopkeeps won't buy	*/
#define ITEM_TRAP		14		/* Unimplemented				*/
#define ITEM_CONTAINER	15		/* Item is a container			*/
#define ITEM_NOTE		16		/* Item is note 				*/
#define ITEM_DRINKCON	17		/* Item is a drink container	*/
#define ITEM_KEY		18		/* Item is a key				*/
#define ITEM_FOOD		19		/* Item is food					*/
#define ITEM_MONEY		20		/* Item is money (gold)			*/
#define ITEM_PEN		21		/* Item is a pen				*/
#define ITEM_BOAT		22		/* Item is a boat				*/
#define ITEM_FOUNTAIN	23		/* Item is a fountain			*/
#define ITEM_ANGREAL	24		/* Item is an angreal			*/


/* Take/Wear flags: used by obj_data.obj_flags.wear_flags		*/
#define ITEM_WEAR_TAKE		(0)  /* Item can be takes			*/
#define ITEM_WEAR_FINGER	(1)  /* Can be worn on finger		*/
#define ITEM_WEAR_NECK		(2)  /* Can be worn around neck 	*/
#define ITEM_WEAR_BODY		(3)  /* Can be worn on body 		*/
#define ITEM_WEAR_HEAD		(4)  /* Can be worn on head 		*/
#define ITEM_WEAR_LEGS		(5)  /* Can be worn on legs			*/
#define ITEM_WEAR_FEET		(6)  /* Can be worn on feet			*/
#define ITEM_WEAR_HANDS		(7)  /* Can be worn on hands		*/
#define ITEM_WEAR_ARMS		(8)  /* Can be worn on arms			*/
#define ITEM_WEAR_SHIELD	(9)  /* Can be used as a shield		*/
#define ITEM_WEAR_ABOUT		(10) /* Can be worn about body 		*/
#define ITEM_WEAR_WAIST 	(11) /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		(12) /* Can be worn on wrist 		*/
#define ITEM_WEAR_WIELD		(13) /* Can be wielded				*/
#define ITEM_WEAR_HOLD		(14) /* Can be held					*/
#define ITEM_WEAR_BACK		(15) /* Can be worn on back			*/



/* Extra object flags: used by obj_data.obj_flags.extra_flags				*/
#define ITEM_GLOW				(0)		/* Item is glowing					*/
#define ITEM_HUM				(1)		/* Item is humming					*/
#define ITEM_NORENT				(2)		/* Item cannot be rented			*/
#define ITEM_NODONATE			(3)		/* Item cannot be donated			*/
#define ITEM_NOINVIS			(4)		/* Item cannot be made invis		*/
#define ITEM_INVISIBLE			(5)		/* Item is invisible				*/
#define ITEM_MAGIC				(6)		/* Item is magical					*/
#define ITEM_NODROP				(7)		/* Item is cursed: can't drop		*/
#define ITEM_BLESS				(8)		/* Item is blessed					*/
#define ITEM_ANTI_GOOD			(9)		/* Not usable by good people		*/
#define ITEM_ANTI_EVIL			(10)	/* Not usable by evil people		*/
#define ITEM_ANTI_NEUTRAL		(11)	/* Not usable by neutral people		*/
#define ITEM_ANTI_MAGIC_USER	(12)	/* Not usable by mages				*/
#define ITEM_ANTI_CLERIC		(13)	/* Not usable by clerics			*/
#define ITEM_ANTI_THIEF			(14)	/* Not usable by thieves			*/
#define ITEM_ANTI_WARRIOR		(15)	/* Not usable by warriors			*/
#define ITEM_NOSELL				(16)	/* Shopkeepers won't touch it		*/
#define ITEM_CHAIN				(17)	/* Item hits anyone fighting you	*/
#define ITEM_POISON				(18)	/* Item randomly poisons			*/
#define ITEM_CHEST				(19)	/* Item randomly poisons			*/
#define ITEM_TWO_HANDED			(20)	/* Item can be worn with two hands	*/


/* Modifier constants used with obj affects ('A' fields)			*/
#define APPLY_NONE              0	/* No effect					*/
#define APPLY_STR               1	/* Apply to strength			*/
#define APPLY_DEX               2	/* Apply to dexterity			*/
#define APPLY_INT               3	/* Apply to constitution		*/
#define APPLY_WIS               4	/* Apply to wisdom				*/
#define APPLY_CON               5	/* Apply to constitution		*/
#define APPLY_CHA		6	/* Apply to charisma					*/
#define APPLY_CLASS             7	/* Reserved						*/
#define APPLY_LEVEL             8	/* Reserved			            */
#define APPLY_AGE               9	/* Apply to age			        */
#define APPLY_CHAR_WEIGHT      10	/* Apply to weight	        	*/
#define APPLY_CHAR_HEIGHT      11	/* Apply to height	        	*/
#define APPLY_MANA             12	/* Apply to max mana		    */
#define APPLY_HIT              13	/* Apply to max hit points	    */
#define APPLY_MOVE             14	/* Apply to max move points	    */
#define APPLY_GOLD             15	/* Reserved		            	*/
#define APPLY_EXP              16	/* Reserved			            */
#define APPLY_DB               17	/* Apply to Armor Class		    */
#define APPLY_HITROLL          18	/* Apply to hitroll	         	*/
#define APPLY_DAMROLL          19	/* Apply to damage roll		    */
#define APPLY_SAVING_PARA      20	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       21	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     22	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    23	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     24	/* Apply to save throw: spells	*/
#define APPLY_RACE             25   /* Apply to race                */

/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)	/* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)	/* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)	/* Container is closed		*/
#define CONT_LOCKED         (1 << 3)	/* Container is locked		*/


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15

/* Weapon Types ****************/

#define WEAPON_SWORD	0
#define WEAPON_DAGGER	1
#define WEAPON_CLUB		2
#define WEAPON_STAFF	3
#define WEAPON_SPEAR	4
#define WEAPON_AXE		5
#define WEAPON_CHAIN	6
#define WEAPON_BOW		7
#define WEAPON_LANCE	8

#define MAX_WEAPON_TYPE	9

/***************/


/* other miscellaneous defines *******************************************/

#define RF_ARRAY_MAX 4
#define PM_ARRAY_MAX 4
#define PR_ARRAY_MAX 5
#define AF_ARRAY_MAX 4
#define TW_ARRAY_MAX 4
#define EF_ARRAY_MAX 4

/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2


/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3

/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* Defense Values			*/

/* Offensive Bonuses		*/
#define OB_MAX_BASE				30
#define OB_RIDING_BONUS			7
#define OB_MASTER_NORMAL		8
#define	OB_NORMAL				5
#define OB_MASTER_BRAVE			15
#define	OB_BRAVE				10
#define OB_MASTER_BERSERK		22
#define OB_BERSERK				17

/* Dodging Bonuses			*/
#define DB_MAX_BASE				30
#define DB_RIDING_MALUS			5
#define DB_BRAVE_MALUS			2
#define DB_BERSERK_MALUS		8
#define DB_SITTING_MALUS		.20

/* Parrying Bonuses			*/
#define PB_MASTER_WIMPY			19
#define PB_WIMPY				15
#define PB_MASTER_NORMAL		11
#define PB_NORMAL				7
#define PB_BERSERK				-7
#define PB_MASTER_BERSERK		-5


/* other #defined constants **********************************************/

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 *
 * LVL_IMPL should always be the HIGHEST possible immortal level, and
 * LVL_IMMORT should always be the LOWEST immortal level.  The number of
 * mortal levels will always be LVL_IMMORT - 1.
 */
#define LVL_IMPL	105
#define LVL_GRGOD	104
#define LVL_GOD		103
#define LVL_APPR    102
#define LVL_BLDER   101
#define LVL_IMMORT	100

/* Level of the 'freeze' command */
#define LVL_FREEZE	LVL_GRGOD

/* Different moods				*/

#define MOOD_WIMPY			0
#define MOOD_NORMAL			1
#define MOOD_BRAVE			2
#define MOOD_BERSERK		3

#define NUM_OF_DIRS	6	/* number of directions in a room (nsewud) */
#define MAGIC_NUMBER	(0x06)	/* Arbitrary number that won't be in a string */

#define OPT_USEC	100000	/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (7 RL_SEC)
#define PULSE_VIOLENCE  (4 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       150          /* Max length of prompt        */
#define GARBAGE_SPACE		32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE		1024        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE	   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

/*
 * --- WARNING ---
 * If you are using a BSD-derived UNIX with MD5 passwords, you _must_
 * make MAX_PWD_LENGTH larger.  A length of 20 should be good. If
 * you leave it at the default value of 10, then any character with
 * a name longer than about 5 characters will be able to log in with
 * _any_ password.  This has not (yet) been changed to ensure pfile
 * compatibility for those unaffected.
 */
#define HISTORY_SIZE		5	/* Keep last 5 commands. */
#define MAX_STRING_LENGTH	20480
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		60
#define MAX_NAME_LENGTH		20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE		3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS		200 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT		6 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_CLAN_QUESTS		20


/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef signed long	      sh_long;
typedef unsigned long		ush_long;
#if !defined(__cplusplus)	/* Anyone know a portable method? */
typedef char			bool;
#endif

#ifndef CIRCLE_WINDOWS
typedef char			cbyte;
#endif

typedef sh_int	room_vnum;	/* A room's vnum type */
typedef sh_int	obj_vnum;	/* An object's vnum type */
typedef sh_int	mob_vnum;	/* A mob's vnum type */

typedef sh_int	room_rnum;	/* A room's real (internal) number type */
typedef sh_int	obj_rnum;	/* An object's real (internal) num type */
typedef sh_int	mob_rnum;	/* A mobile's real (internal) num type */


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/

struct chest_log
{
	char *str;
	int type;
	int vnum;

	struct chest_log *next;
};

/* object flags; used in obj_data										*/
struct obj_flag_data {
	int	value[4];	/* Values of the item (see list)					*/
	cbyte type_flag;	/* Type of item										*/
	int	wear_flags[TW_ARRAY_MAX];	/* Where you can wear it			*/
	int	extra_flags[EF_ARRAY_MAX];	/* If it hums, glows, etc.		    */
	int	weight;		/* Weigt what else									*/
	int	cost;		/* Value when sold (gp.)						    */
	int	cost_per_day;	/* Cost to keep pr. real day				    */
	int	timer;		/* Timer for object								    */
	long	bitvector[AF_ARRAY_MAX];	/* To set chars bits                */
	int  offensive;					/* OB addition                      */
	int  parry;                      /* PB addition						*/
	int  dodge;                      /* DB addition						*/
	int  absorb;                     /* ABS addition                     */
	int  weapon_type;
};

struct scalp_data {
	char *name;
	int is_scalp;
	int race;
	int level;
	int scalped;
	int warrants[50];
};

/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
	cbyte location;      /* Which ability to change (APPLY_XXX) */
	sbyte modifier;     /* How much it changes by              */
};

struct weave_data
{
	class char_data *target;
	struct affected_type *spell;
	struct weave_data *next;
	int tied;
};

/* ================== Memory Structure for Objects ================== */
struct obj_data
{
   obj_vnum item_number;					/* Where in data-base				*/
   room_rnum in_room;						/* In what room -1 when conta/carr	*/

   struct obj_flag_data obj_flags;			/* Object information				*/
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects				*/

   char	*name;								/* Title of object :get etc.        */
   char	*description;						/* When in room                     */
   char	*short_description;					/* when worn/carry/in cont.         */
   char	*action_description;				/* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions				*/
   class char_data *carried_by;			/* Carried by :nullptr in room/conta   */
   class char_data *worn_by;				/* Worn by?							*/
   sh_int worn_on;							/* Worn where?						*/

   struct obj_data *in_obj;					/* In what object nullptr when none	*/
   struct obj_data *contains;				/* Contains objects					*/
   struct obj_data *next_chest;				/* Next chest on the list			*/

   long id;									/* used by DG triggers				*/
   struct trig_proto_list *proto_script;	/* list of default triggers			*/
   struct script_data *script;				/* script info for the object       */

   struct obj_data *next_content;			/* For 'contains' lists				*/
   struct obj_data *next;					/* For the object list				*/
   class char_data *corpse_data;			/* Corpse/Scalp information			*/
   struct scalp_data scalp;

   char creator[MAX_INPUT_LENGTH];

   int food_unit;
   int pos;
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */


struct obj_store_data {
	char	name[MAX_INPUT_LENGTH];						/*	Title of object :get etc.			*/
	char	description[MAX_INPUT_LENGTH];				/*	When in room						*/
	char	short_description[MAX_INPUT_LENGTH];		/*	when worn/carry/in cont.			*/
	char	action_description[MAX_INPUT_LENGTH];		/*	What to write when used			*/
	char	creator[MAX_INPUT_LENGTH];
	char	spares2[MAX_INPUT_LENGTH];
	char	spares3[MAX_INPUT_LENGTH];
	char	spares4[MAX_INPUT_LENGTH];

	int spare1;
	int spare2;
	int spare3;
	int spare4;
	int spare5;
	int spare6;

	struct obj_data obj;
};

struct obj_file_elem {


	obj_vnum item_number;

	int	value[4];
	int	extra_flags[EF_ARRAY_MAX];
	int	weight;
	int	timer;
	long	bitvector[AF_ARRAY_MAX];
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
	int pos;
	int spare;
	int spare2;
	int spare3;
	int spare4;
	int spare5;

	struct obj_store_data obj_store;
	struct scalp_data	scalp_store;
};

/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
	int	time;
	int	rentcode;
	int	net_cost_per_diem;
	int	gold;
	int	account;
	int	nitems;
	int	spare0;
	int	spare1;
	int	spare2;
	int	spare3;
	int	spare4;
	int	spare5;
	int	spare6;
	int	spare7;
};
/* ======================================================================= */





/* room-related structures ************************************************/


struct room_direction_data {
	char	*general_description;       /* When look DIR.			*/

	char	*keyword;		/* for open/close			*/

	sh_int exit_info;		/* Exit info				*/
	obj_vnum key;		/* Key's number (-1 for no key)		*/
	room_rnum to_room;		/* Where direction leads (NOWHERE)	*/

	int hidden;
};

struct gate_data
{
	room_rnum to_room, in_room;
	class char_data *creator;

	bool alive;
	int time_of_creation;

	struct gate_data *prev, *next, *prev_in_world, *next_in_world;
};

/* ================== Memory Structure for room ======================= */
struct room_data {
	room_vnum number;		/* Rooms number	(vnum)		      */
	sh_int zone;                 /* Room zone (for resetting)          */
	int	sector_type;            /* sector type (move/hide)            */
	char	*name;                  /* Rooms name 'You are ...'           */
	char	*description;           /* Shown when entered                 */
	struct extra_descr_data *ex_description; /* for examine/look       */
	struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
	int room_flags[RF_ARRAY_MAX];		/* DEATH,DARK ... etc                 */

	cbyte light;                  /* Number of lightsources in room     */
	SPECIAL(*func);

	struct trig_proto_list *proto_script; /* list of default triggers  */
	struct script_data *script;  /* script info for the object         */
	struct track_data *tracks;

	struct gate_data *first_live_gate, *last_live_gate, *first_dead_gate, *last_dead_gate;

	struct obj_data *contents;   /* List of items in room              */
	class char_data *people;    /* List of NPC / PC in room           */

	int noise_depth;
	int noise_level[MAX_TYPE];
};
/* ====================================================================== */


/* char-related structures ************************************************/

/* memory structure for characters */
struct memory_rec_struct
{
	long	id;
	struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
	int hours, day, month;
	sh_int year;
};


/* These data contain information about a players time data */
struct time_data
{
	time_t birth;    /* This represents the characters age                */
	time_t logon;    /* Time of the last logon (used to calculate played) */
	int	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data
{
	char	passwd[MAX_PWD_LENGTH+1]; /* character's password		*/
	char	*name;	       /* PC / NPC s name (kill ...  )			*/
	char	*short_descr;  /* for NPC 'actions'						*/
	char	*long_descr;   /* for 'look'							*/
	char	*description;  /* Extra descriptions					*/
	char	*title;        /* PC / NPC's title						*/
	cbyte sex;           /* PC / NPC's sex						*/
	cbyte chclass;       /* PC / NPC's class						*/
	cbyte race;          /* PC / NPC's race						*/
	cbyte level;         /* PC / NPC's level						*/
	int	hometown;      /* PC s Hometown (zone)					*/
	struct time_data time;  /* PC's AGE in days					*/
	ubyte weight;       /* PC / NPC's weight						*/
	ubyte height;       /* PC / NPC's height						*/
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data
{
	sbyte str;
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis;
	sbyte dex;
	sbyte con;
	sbyte cha;
};

struct char_point_data
{
	sh_int mana;
	sh_int max_mana;     /*   Max move for PC/NPC			         */
	sh_int hit;
	sh_int max_hit;      /*   Max hit for PC/NPC                      */
	sh_int move;
	sh_int max_move;     /*   Max move for PC/NPC                     */
	sh_int offensive;    /*   Offencive Bonus                         */
	sh_int parry;        /*   Parry Bonus                             */
	sh_int dodge;        /*   Dodge Bonus                             */
	sh_int absorb;       /*   Absorb Bonus                            */
	sh_int weave;        /*   Weave Points                            */
	sh_int taveren;      /*   Ta'veren Ammount                        */
	sh_int legend;       /*   Number on the list                      */
	sh_int invis;
	sh_int taint;

	int death_wait;
	int warning;
	int shadow_points;
	int max_shadow_points;
	int hidden_qp;
	int master_weapon;
	int is_bashed;
	int stage;
	int dark_points;		/* This value is for darkfriends, whether known or unknown. */
	int sickness;
	int strain;

	char poofin[MAX_INPUT_LENGTH];
	char poofout[MAX_INPUT_LENGTH];

	char host[MAX_INPUT_LENGTH];		/* Character's IP/Hostname					*/
	char slew[MAX_INPUT_LENGTH];		/* Used in legend list. Last kill			*/
	char forced[MAX_INPUT_LENGTH];		/* Last person to force player				*/
	char whois_extra[MAX_INPUT_LENGTH];
	char bond[MAX_INPUT_LENGTH];		/* Character's bonded partner				*/

	time_t last_logon;
	int warrants[4];

	int	gold;           /* Money carried                             */
	int	bank_gold;	    /* Gold the char has in a bank account	     */
	int	exp;            /* The experience of the player              */
	sbyte damroll;       /* Any bonus or penalty to the damage roll   */

};


/*
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   int	alignment;		/* +-1000 for alignments                */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   long	act[PM_ARRAY_MAX];			/* act flag for NPC's; player flag for PC's */

   int	affected_by[AF_ARRAY_MAX];		/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
	class char_data *fighting;	/* Opponent				*/
	class char_data *hunting;	/* Char hunted by this char		*/
	class char_data *mount;
	class char_data *ridden_by;
	class char_data *target;
	class char_data *marked;	/* Graymen bonus for Marking Targets */

	cbyte position;		/* Standing, fighting, sleeping, etc.	*/
	cbyte last_pos;


	char command[MAX_INPUT_LENGTH];

	int  direction;			/* For flee direction		*/
	int  flee_lag;			/* Ammount of flee lag stacked up	*/
	int	 flee_go;			/* Ready to flee?					*/
	int	carry_weight;		/* Carried weight			*/
	cbyte carry_items;		/* Number of items carried		*/
	int	timer;			/* Timer for update			*/

	struct char_special_data_saved saved; /* constants saved in plrfile	*/

};

/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   cbyte skills[MAX_SKILLS+1];	/* array of skills plus skill 0		*/
   cbyte PADDING0;		/* used to be spells_to_learn		*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   cbyte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   room_vnum load_room;		/* Which room to place char in		*/
   int	pref[PR_ARRAY_MAX];			/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[3];         /* Drunk, full, thirsty			*/

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ubyte spare0;
   ubyte spare1;
   ubyte spare2;
   ubyte spare3;
   ubyte spare4;
   ubyte spare5;
   int spells_to_learn;		/* How many can you learn yet this level  */
   int warrant;          /* Room in which the player left the game */
   int spellpracs ;
   int mood;
   int logout;

};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   struct alias *aliases;	/* Character's aliases			*/
   long last_tell;		/* idnum of last tell from		*/
   void *last_olc_targ;		/* olc control				*/
   int last_olc_mode;		/* olc control				*/
};

struct ignore_data
{
	char name[MAX_NAME_LENGTH + 1];
	struct ignore_data *next;
};

/* Specials used by NPCs, not PCs */
struct mob_special_data
{
   cbyte last_direction;     /* The last direction the monster went     */
   int	attack_type;        /* The Attack Type Bitvector for NPC's     */
   cbyte default_pos;        /* Default position for NPC                */
   memory_rec *memory;	    /* List of attackers to remember	       */
   cbyte damnodice;          /* The number of damage dice's	       */
   cbyte damsizedice;        /* The size of the damage dice's           */
   int wait_state;	    /* Wait state for bashed mobs	       */
   int aggro[20];
   struct kit_data *primary_kit; /* Primary eq kit */
   struct reset_com *load;  /* ZCMD that loaded the mob */
};

/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type
{
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   cbyte location;        /* Tells which ability to change(APPLY_XXX)*/
   long	bitvector;       /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};

struct affect_type_not_saved
{
	class char_data *caster;
	struct affected_type *affect;
	struct affect_type_not_saved *next;
};

/* Structure used for chars following other chars */
struct follow_type
{
	class char_data *follower;
	struct follow_type *next;
};

/* Structure used for storing time of rank/clanning for a person's clan */
class player_clan_data
{
public:
	time_t rank_time;
	time_t clan_time;
	sh_int clan;
	sh_int rank;
	sh_int quest_points;

	player_clan_data *next;
};

/* ================== Structure for player/non-player ===================== */
class char_data
{

public:
	int pfilepos;			 /* playerfile pos		  */
	sh_int nr;                            /* Mob's rnum			  */
	room_rnum in_room;                    /* Location (real room number)	  */
	room_rnum was_in_room;		 /* location for linkdead people  */

	struct char_player_data player;       /* Normal data                   */
	struct char_ability_data real_abils;	 /* Abilities without modifiers   */
	struct char_ability_data aff_abils;	 /* Abils with spells/stones/etc  */
	struct char_point_data points;        /* Points                        */
	struct char_special_data char_specials;	/* PC/NPC specials	  */
	struct player_special_data *player_specials; /* PC specials		  */
	struct mob_special_data mob_specials;	/* NPC specials		  */
	class player_clan_data *clans;

	struct affected_type *affected;       /* affected by what spells       */
	struct affect_type_not_saved *affection_list;
	struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

	struct obj_data *carrying;            /* Head of list                  */
	struct descriptor_data *desc;         /* nullptr for mobiles              */

	long id;                            /* used by DG triggers             */
	struct trig_proto_list *proto_script; /* list of default triggers      */
	struct script_data *script;         /* script info for the object      */
	struct script_memory *memory;       /* for mob memory triggers         */

	char_data *next_in_room;     /* For room->people - list         */
	char_data *next;             /* For either monster or ppl-list  */
	char_data *next_fighting;    /* For fighting list               */

	struct follow_type *followers;        /* List of chars followers       */
	char_data *master;             /* Who is char following?        */

	struct ignore_data *ignores;
	struct weave_data *weaves;

	int rollMana();
	int rollHealth();
	int save();
	int load(char *name);
	int totalQP();
	int getLegend();

	bool isMaster();
	bool canViewClan(int clannum);
	bool AES_SEDAI();
	bool TOWER_MEMBER();
	bool wantedByPlayer(char_data *ch);

	player_clan_data *getClan(int clan_num);

	void allocate();
	void init();


};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
	/* char_player_data */
	char	name[MAX_NAME_LENGTH+1];
	char	description[EXDSCR_LENGTH];
	char	title[MAX_TITLE_LENGTH+1];
	cbyte sex;
	cbyte chclass;
	cbyte race;
	cbyte level;
	sh_int hometown;
	time_t birth;   /* Time of birth of character     */
	int	played;    /* Number of secs played in total */
	ubyte weight;
	ubyte height;



	char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

	struct char_special_data_saved char_specials_saved;
	struct player_special_data_saved player_specials_saved;
	struct char_ability_data abilities;
	struct char_point_data points;
	struct affected_type affected[MAX_AFFECT];

	time_t last_logon;		/* Time (in secs) of last logon */
	char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */

// Serai - 06/18/04 - Copyover for Kinslayer.
struct copyover
{
	// descriptor_data info
	socket_t	descriptor;  // The critical piece of data, the socket.
	cbyte		bad_pws;
	cbyte		idle_tics;
	int		connected;
	int		wait;
	int		desc_num;
	time_t		login_time;

	// class char_data
	room_rnum was_in_room;

	struct char_file_u st;
};

/* descriptor-related structures ******************************************/


struct txt_block {
	char	*text;
	int aliased;
	struct txt_block *next;
};


struct txt_q {
	struct txt_block *head;
	struct txt_block *tail;
};


struct descriptor_data {
	socket_t	descriptor;	/* file descriptor for socket		*/
	char		host[HOST_LENGTH+1];	/* hostname				*/
	cbyte		bad_pws;		/* number of bad pw attemps this login	*/
	cbyte		idle_tics;		/* tics idle at password prompt		*/
	int		connected;		/* mode of 'connectedness'		*/
	int		wait;			/* wait for how many loops		*/
	int		desc_num;		/* unique num assigned to desc		*/
	time_t	login_time;		/* when the person connected		*/
	char		*showstr_head;		/* for keeping track of an internal str	*/
	char		**showstr_vector;	/* for paging through texts		*/
	int		showstr_count;		/* number of pages to page through	*/
	int		showstr_page;		/* which page are we currently showing?	*/
	char		**str;			/* for the modify-str system		*/
	size_t	max_str;	        /*		-			*/
	char		*backstr;		/* added for handling abort buffers	*/
	long		mail_to;		/* name for mail system			*/
	int		has_prompt;		/* is the user at a prompt?             */
	char		inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
	char		last_input[MAX_INPUT_LENGTH]; /* the last input			*/
	char		small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
	char		*output;		/* ptr to the current output buffer	*/
	char		**history;		/* History of commands, for ! mostly.	*/
	int		history_pos;		/* Circular array position.		*/
	int		bufptr;			/* ptr to end of current output		*/
	int		bufspace;		/* space left in the output buffer	*/
	buffer	*large_outbuf; /* ptr to large buffer, if we need it */
	struct	txt_q input;		/* q of unprocessed input		*/
	class char_data *character;	/* linked to char			*/
	class char_data *original;	/* original char if switched		*/
	struct descriptor_data *snooping; /* Who is this char snooping	*/
	struct descriptor_data *snoop_by; /* And who is snooping this char	*/
	struct descriptor_data *next; /* link to next descriptor		*/
	struct olc_data *olc;	     /*. OLC info - defined in olc.h   .*/

	float	timer;
	int		command_ready;
	int		delayed_state;
	int		forced;
	char		delayed_command[MAX_INPUT_LENGTH];

};


/* other miscellaneous structures ***************************************/

class wizlist_data
{
	public:
	string name;
	int level;
	class wizlist_data *next;
};

class NoteData
{
	public:
	string message;
	string poster;
	int date;
	int npc;
};

class Ideas
{

	public:
	string idea;
	string date;
	string poster;
	int room;
	class Ideas *next;
};


struct battle
{
	int humanwp;
	int trollwp;
};

struct memory_data
{
	int size;
	char type[200];
	char file[200];
	int line;

	void *allocated;
	struct memory_data *next;
};

struct clan_player_data
{
	char name[MAX_NAME_LENGTH];
	int quests[MAX_CLAN_QUESTS];
	struct clan_player_data *next;
};

struct clan_data
{
	int vnum;
	int secret;
	char name[126];
	struct clan_player_data *players;
};

struct legend_data
{
	char person[MAX_INPUT_LENGTH];
	int ammount;
};

#define NUM_OF_KITS 3
struct kit_data
{
	char *name;
	int equipment[NUM_OF_KITS][NUM_WEARS]; /* vnums/rnums */
	int percent[NUM_OF_KITS][NUM_WEARS];

	int vnum; /* unique kit number */

	struct kit_data *prev, *next; /* Doubly linked list */
};

struct track_data
{
	char name[MAX_INPUT_LENGTH];
	int laytime;
	int age;
	int direction;
	int race;
	struct track_data *next_in_room;
	struct track_data *next;
};

struct watch_data
{
	char host[HOST_LENGTH+1];
	char name[200];
	int time;

	struct watch_data *next;
};

struct msg_type
{
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type
{
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   cbyte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   cbyte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int	change;	/* How fast and what way does it change. */
   int	sky;	/* How is the sky. */
   int	sunlight;	/* And how much sun. */
};


struct title_type {
   char	*title_m;
   char	*title_f;
   int	exp;
};


/* element in monster and object index-tables   */
struct index_data {
   int	vnum;		/* virtual number of this mob/obj		*/
   int	number;		/* number of existing units of this mob/obj	*/
   SPECIAL(*func);

   char *farg;         /* string argument for special function     */
   struct trig_data *proto;     /* for triggers... the trigger     */
};

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
  int vnum;                             /* vnum of the trigger   */
  struct trig_proto_list *next;         /* next trigger          */
};

/* used in the socials */
struct social_messg {
  int act_nr;
  char *command;               /* holds copy of activating command */
  char *sort_as;             /* holds a copy of a similar command or
                             * abbreviation to sort by for the parser */
  int hide;                  /* ? */
  int min_victim_position;   /* Position of victim */
  int min_char_position;     /* Position of char */
  int min_level_char;          /* Minimum level of socialing char */

  /* No argument was supplied */
  char *char_no_arg;
  char *others_no_arg;

  /* An argument was there, and a victim was found */
  char *char_found;
  char *others_found;
  char *vict_found;

  /* An argument was there, as well as a body part, and a victim was found */
  char *char_body_found;
  char *others_body_found;
  char *vict_body_found;

  /* An argument was there, but no victim was found */
  char *not_found;

  /* The victim turned out to be the character */
  char *char_auto;
  char *others_auto;

  /* If the char cant be found search the char's inven and do these: */
  char *char_obj_found;
  char *others_obj_found;
};
