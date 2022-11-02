/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"

cpp_extern const char circlemud_version[] = {
	"CircleMUD, version 3.00 beta patchlevel 14"
};


/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */

/* Slopes and Graph Directions */
const char *loc[10] =
{
	"unknown",
	"north",
	"east",
	"south",
	"west",
	"northeast",
	"northwest",
	"southeast",
	"southwest",
	"nearby",
};

const char *dist[9] =
{
	"",
	"",
	" far",
	" very far",
	" extremely far",
	" extremely very far",
	" extremely ver very far",
	" extremely very very very far",
	" extremely VERY VERY VERY VERY far",
};


/* cardinal directions */
char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

char *aggros[] =
{

	"ALL",
	"HUMANS",
	"TROLLOCS",
	"SEANCHAN",
	"AIEL",
	"ALL_NOT_IN_MY_CLAN",
	"MOBS",
	"\n"
};

char *class_types[] =
{

	"WARRIOR",
	"THIEF",
	"RANGER",
	"CHANNELER",
	"FADE",
	"DREADLORD",
	"BLADEMASTER",
	"GREYMAN",
	"DRAGHKAR",
	"\n"
};

/* ROOM_x */
char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "*",				/* BFS MARK */
  "NO TRAVEL",
  "\n"
};


/* EX_x */
char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "\n"
};


/* SECT_ */
char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "\n"
};


/*
 * SEX_x
 * Not used in sprinttype() so no \n.
 */
char *genders[] =
{
  "Neutral",
  "Male",
  "Female"
};


/* POS_x */
char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};


/* PLR_x */
char *player_bits[] = {
	"DF",
	"THIEF",
	"FROZEN",
	"DONTSET",
	"WRITING",
	"MAILING",
	"CSH",
	"SITEOK",
	"NOSHOUT",
	"NOTITLE",
	"DELETED",
	"LOADRM",
	"!WIZL",
	"!DEL",
	"INVST",
	"CRYO",
	"LOG",
	"ZBAN",
	"\n"
};


/* MOB_x */
char *action_bits[] = {
	"SPEC",					//0
	"SENTINEL",				//1	
	"SCAVENGER",
	"ISNPC",
	"AWARE",  
	"STAY-ZONE",			//5
	"WIMPY",
	"MEMORY",
	"HELPER",
	"!SLEEP",
	"!BLIND",				//10
	"MOUNT",
	"TRACK",
	"BASH",
	"MOB_AWARD",
	"MOB_SHADOW_MOUNT",		//15
	"MOB_NOFIGHT",
	"MOB_INVIS",
	"\n"
};

/* PRF_x */
char *preference_bits[] = {
	"BRIEF",
	"COMPACT",
	"SOURCE",
	"!TELL",
	"D_HP",
	"D_MANA",
	"D_MOVE",
	"AUTOEX",
	"!HASS",
	"QUEST",
	"SUMN",
	"!REP",
	"LIGHT",
	"C1",
	"C2",
	"!WIZ",
	"L1",
	"L2",
	"SPARE1",
	"SPARE2",
	"SPARE3",
	"RMFLG",
	"KIT",
	"STATTED",
	"COUNCIL",
	"INCOG",
	"!TELL",
	"!NARR",
	"!CHAT",
	"!YELL",
	"!SHOUT",
	"\n"
};


/* AFF_x */
char *affected_bits[] =
{
	"\0",
	"BLIND",
	"INVIS",
	"WATWALK",
	"SANCT",
	"GROUP",
	"POISON",
	"SLEEP",
	"INCOGNITO",
	"SNEAK",
	"HIDE",
	"NOTICE",
	"NOQUIT",
	"PARANOIA",
	"NIGHT_VISION",
	"HASTE",
	"SOURCE_SHIELD",
	"EFFUSION",
	"AGILITY",
	"STRENGTH",
	"\n"
};



/* CON_x */
char *connected_types[] = {
  "Playing", /* 0 */
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW", /* 5 */
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu", /* 10 */
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1", /* 15 */
  "Self-Delete 2",
  "Disconnecting",
  "Object editor",
  "Room editor",
  "Zone editor", /* 20 */
  "Mobile editor",
  "Shop editor",
  "Trigger editor",
  "Help editor",
  "Action editor", /* 25 */
  "Text editor",
  "Select race", /* Serai - this was off */
  "Kit editor", /* 28 */
  "\n"
};


char *weapon_types[] = {

	"LONG BLADE",
	"SHORT BLADE",
	"CLUB",
	"STAFF",
	"SPEAR",
	"AXE",
	"CHAIN",
	"BOW",
	"LANCE"
};

/*
 * WEAR_x - for eq list
 * Not use in sprinttype() so no \n.
 */
char *where[] = {
	"<used as light>             ",
	"<held>                      ",
	"<worn around neck>          ",
	"<worn around neck>          ",
	"<worn on head>              ",
	"<worn about body>           ",
	"<worn on back>              ",
	"<worn on body>              ",
	"<worn on arms>              ",
	"<worn about waist>          ",
	"<worn around wrist>         ",
	"<worn around wrist>         ",
	"<worn on hands>             ",
	"<worn on finger>            ",
	"<worn on finger>            ",
	"<wielded>                   ",
	"<worn as shield>            ",
	"<worn on legs>              ",
	"<worn on feet>              ",
};
/* WEAR_x - for stat */
char *equipment_types[] = {
	"Used as light",
	"Held",
	"First worn around Neck",
	"Second worn around Neck",
	"Worn on head",
	"Worn about body",
	"Worn on back",
	"Worn on body",
	"Worn on arms",
	"Worn around waist",
	"Worn around right wrist",
	"Worn around left wrist",
	"Worn on hands",
	"Worn on right finger",
	"Worn on left finger",
	"Wielded",
	"Worn as shield",
	"Worn on legs",
	"Worn on feet",
	"\n"
};

/* ITEM_x (ordinal object types) */
char *item_types[] = {
	"UNDEFINED",
	"LIGHT",
	"SCROLL",
	"WAND",
	"STAFF",
	"WEAPON",
	"FIRE WEAPON",
	"MISSILE",
	"TREASURE",
	"ARMOR",
	"POTION",
	"WORN",
	"OTHER",
	"TRASH",
	"TRAP",
	"CONTAINER",
	"NOTE",
	"LIQ CONTAINER",
	"KEY",
	"FOOD",
	"MONEY",
	"PEN",
	"BOAT",
	"FOUNTAIN",
	"ANGREAL",
	"\n"
};


/* ITEM_WEAR_ (wear bitvector) */
char *wear_bits[] = {
	"TAKE",
	"FINGER",
	"NECK",
	"BODY",
	"HEAD",
	"LEGS",
	"FEET",
	"HANDS",
	"ARMS",
	"SHIELD",
	"ABOUT",
	"WAIST",
	"WRIST",
	"WIELD",
	"HOLD",
	"BACK",
	"\n"
};


/* ITEM_x (extra bits) */
char *extra_bits[] = {
	"GLOW",
	"HUM",
	"!RENT",
	"!DONATE",
	"!INVIS",
	"INVISIBLE",
	"MAGIC",
	"!DROP",
	"BLESS",
	"!GOOD",
	"!EVIL",
	"!NEUTRAL",
	"!MAGE",
	"!CLERIC",
	"!THIEF",
	"!WARRIOR",
	"!SELL",
	"CHAIN",
	"POISON",
	"CHEST",
	"TWO_HANDED",
	"\n"
};

/* APPLY_x */
char *apply_types[] = {
	"NONE",
	"STR",
	"DEX",
	"INT",
	"WIS",
	"CON",
	"CHA",
	"CLASS",
	"LEVEL",
	"AGE",
	"CHAR_WEIGHT",
	"CHAR_HEIGHT",
	"MAXMANA",
	"MAXHIT",
	"MAXMOVE",
	"GOLD",
	"EXP",
	"ARMOR",
	"HITROLL",
	"DAMROLL",
	"SAVING_PARA",
	"SAVING_ROD",
	"SAVING_PETRI",
	"SAVING_BREATH",
	"SAVING_SPELL",
	"RACE",
	"\n"
};


/* CONT_x */
char *container_bits[] = {
	"CLOSEABLE",
	"PICKPROOF",
	"CLOSED",
	"LOCKED",
	"\n",
};


/* LIQ_x */
char *drinks[] =
{
	"water",
	"beer",
	"wine",
	"ale",
	"dark ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local speciality",
	"slime mold juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt water",
	"clear water",
	"\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
char *drinknames[] =
{
	"water",
	"beer",
	"wine",
	"ale",
	"ale",
	"whisky",
	"lemonade",
	"firebreather",
	"local",
	"juice",
	"milk",
	"tea",
	"coffee",
	"blood",
	"salt",
	"water",
	"\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};


/* color of the various drinks */
char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear"
  "\n"
};


/*
 * level of fullness for drink containers
 * Not used in sprinttype() so no \n.
 */
char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
cpp_extern const struct str_app_type str_app[] = {
  {-5, -4, 0, 0},	/* str = 0 */
  {-5, -4, 3, 1},	/* str = 1 */
  {-3, -2, 3, 2},
  {-3, -1, 10, 3},
  {-2, -1, 25, 4},
  {-2, -1, 55, 5},	/* str = 5 */
  {-1, 0, 80, 6},
  {-1, 0, 90, 7},
  {0, 0, 100, 8},
  {0, 0, 100, 9},
  {0, 0, 115, 10},	/* str = 10 */
  {0, 0, 115, 11},
  {0, 0, 140, 12},
  {0, 0, 140, 13},
  {0, 0, 170, 14},
  {0, 0, 170, 15},	/* str = 15 */
  {0, 1, 195, 16},
  {1, 1, 220, 18},
  {1, 2, 255, 20},	/* str = 18 */
  {3, 7, 640, 40},
  {3, 8, 700, 40},	/* str = 20 */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},	/* str = 25 */
  {1, 3, 280, 22},	/* str = 18/0 - 18-50 */
  {2, 3, 305, 24},	/* str = 18/51 - 18-75 */
  {2, 4, 330, 26},	/* str = 18/76 - 18-90 */
  {2, 5, 380, 28},	/* str = 18/91 - 18-99 */
  {3, 6, 480, 30}	/* str = 18/100 */
};



/* [dex] skill apply (thieves only) */
extern const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},	/* dex = 0 */
  {-90, -90, -60, -90, -50},	/* dex = 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},	/* dex = 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	/* dex = 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},		/* dex = 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},		/* dex = 18 */
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		/* dex = 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}		/* dex = 25 */
};

cpp_extern const struct dex_skill_type race_app_skill[NUM_RACES] =
{
	{ 0,  0,  0,  0,  0, }, /* HUMAN     */
	{ 5, -5,  0,  5, 10, }, /* TROLLOC       */
};

/* [dex] apply (all) */
cpp_extern const struct dex_app_type dex_app[] = {
  {-7, -7, 6},		/* dex = 0 */
  {-6, -6, 5},		/* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-1, -1, 2},		/* dex = 5 */
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},		/* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},		/* dex = 15 */
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},		/* dex = 18 */
  {3, 3, -4},
  {3, 3, -4},		/* dex = 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6}		/* dex = 25 */
};



/* [con] apply (all) */
cpp_extern const struct con_app_type con_app[] = {
  {-4, 20},		/* con = 0 */
  {-3, 25},		/* con = 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},		/* con = 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},		/* con = 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},		/* con = 15 */
  {2, 95},
  {2, 97},
  {3, 99},		/* con = 18 */
  {3, 99},
  {4, 99},		/* con = 20 */
  {5, 99},
  {5, 99},
  {5, 99},
  {6, 99},
  {6, 99}		/* con = 25 */
};



/* [int] apply (all) */
cpp_extern const struct int_app_type int_app[] = {
  {3},		/* int = 0 */
  {5},		/* int = 1 */
  {7},
  {8},
  {9},
  {10},		/* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},		/* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},		/* int = 15 */
  {40},
  {45},
  {50},		/* int = 18 */
  {53},
  {55},		/* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60}		/* int = 25 */
};


/* [wis] apply (all) */
cpp_extern const struct wis_app_type wis_app[] = {
  {0},	/* wis = 0 */
  {0},  /* wis = 1 */
  {0},
  {0},
  {0},
  {0},  /* wis = 5 */
  {0},
  {0},
  {0},
  {0},
  {0},  /* wis = 10 */
  {0},
  {2},
  {2},
  {3},
  {3},  /* wis = 15 */
  {3},
  {4},
  {5},	/* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7}  /* wis = 25 */
};



char *spell_wear_off_msg[] = {
  "RESERVED DB.C",		/* 0 */
  "You feel less protected.",	/* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",		/* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",		/* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",		/* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.",/* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",		/* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",		/* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",		/* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",			/* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "!Animate Dead!",		/* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",	/* 50 */
  "Your feet seem less boyant.",
  "!UNUSED!"
};



char *npc_class_types[] = {
  "Normal",
  "Undead",
  "\n"
};



int rev_dir[] =
{
  2,
  3,
  0,
  1,

#if defined(OASIS_MPROG)
/*
 * Definitions necessary for MobProg support in OasisOLC
 */
char *mobprog_types[] = {
  "INFILE",
  "ACT",
  "SPEECH",
  "RAND",
  "FIGHT",
  "DEATH",
  "HITPRCNT",
  "ENTRY",
  "GREET",
  "ALL_GREET",
  "GIVE",
  "BRIBE",
  "\n"
};
#endif
  5,
  4
};


int movement_loss[] =
{
  2,	/* Inside     */
  2,	/* City       */
  2,	/* Field      */
  2,	/* Forest     */
  2,	/* Hills      */
  3,	/* Mountains  */
  3,	/* Swimming   */
  2,	/* Unswimable */
  1,	/* Flying     */
  5     /* Underwater */
};

/* Not used in sprinttype(). */
char *weekdays[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};


/* Not used in sprinttype(). */
char *month_name[] = {
  "First Month",		/* 0 */
  "Second Month",
  "Third Month",
  "Fourth Month",
  "Fifth Month",
  "Sixth Month",
  "Seventh Month",
  "Eigth Month",
  "Nineth Month",
  "Tenth Month",
  "Eleventh Month",
  "Twelth Month",
  "Thirteenth Month",
  "Fourteenth Month",
  "Fifteenth Month",
  "Sixteenth Month",
  "Seventeenth Month"
};
