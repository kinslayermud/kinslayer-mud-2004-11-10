/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)


#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_AGILITY				1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT				2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS					3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS				4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING		5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH			6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER		7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD			8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER			9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND			10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC			11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT			12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE			13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON		14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL				15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT			16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON				17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY				18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP					19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH				20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON			21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REGEN					22	/* Spell for regenerating moves	*/
#define SPELL_AIR_SCYTHE			23	/* Spell for air scythe			*/
#define SPELL_NIGHT_VISION			24	/* Spell to see in the dark		*/
#define SPELL_SHIELD				25	/* Spell for shielding someone from the source	*/
#define SPELL_HASTE					26	/* Spell for the haste weave	*/
#define SPELL_LOCATE_LIFE			27	/* Spell for Locate Life		*/
#define SPELL_TORNADO				28	/* Spell for the tornado weave	*/
#define SPELL_RESTORE				29	/* Spell for restore			*/
#define SPELL_BALEFIRE				30	/* Spell for balefire			*/
#define SPELL_GATE					31	/* Spell for the gate weave		*/
#define SPELL_BOND					32	/* Spell to bond a warder		*/

/* Insert new spells here, up to MAX_SPELLS */
#define MAX_SPELLS					130

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              131 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_BASH                  132 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_HIDE                  133 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_KICK                  134 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_PICK_LOCK             135 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_RESCUE                136 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_SNEAK                 137 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_STEAL                 138 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_TRACK				    139 /* Reserved Skill[] DO NOT CHANGE	*/
#define SKILL_ATTACK                140 /* Skill for bow and arrow.			*/
#define SKILL_BOW					141 /* Skill for weapons				*/
#define SKILL_DODGE                 142 /* Skill for dodging bonus			*/
#define SKILL_PARRY                 143 /* Skill for shield parry			*/
#define SKILL_NOTICE                144 /* Skill for notice					*/
#define SKILL_RIDE					145 /* Skill for riding horses/leading	*/
#define SKILL_SEARCH				146 /* Skill for searching exits		*/
#define SKILL_FADE					147	/* Skill for fades to fade away.	*/
#define SKILL_CRAFT					148 /* Skill for upgrading weapons		*/
#define SKILL_EFFUSION				149 /* Skill crafting					*/
#define SKILL_FEAR					150 /* Skill for leaving unseen			*/
#define SKILL_CLUB					151 /* Skill for fear					*/
#define SKILL_AXE					152 /* Skill for clubs					*/
#define SKILL_LONG_BLADE			153 /* Skill for axes					*/
#define SKILL_SHORT_BLADE			154 /* Skill for long blades			*/
#define SKILL_STAFF					155 /* Skill for short blades			*/
#define SKILL_SPEAR					156 /* Skill for staves					*/
#define SKILL_CHAIN					157 /* Skill for chains					*/
#define SKILL_VITALITY				158 /* Skill for butcher and survival	*/
#define SKILL_CHARGE				159	/* Skill for charge					*/
#define SKILL_SKEWER				160	/* Skill for skewer					*/
#define SKILL_LANCE					161	/* Skill for lances					*/


/* New skills may be added here up to MAX_SKILLS (200)						*/


/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
 */

#define SPELL_FIRE_BREATH            201
#define SPELL_GAS_BREATH             202
#define SPELL_FROST_BREATH           203
#define SPELL_ACID_BREATH            204
#define SPELL_LIGHTNING_BREATH       205

#define TOP_SPELL_DEFINE	     299
/* NEW NPC/OBJECT SPELLS can be inserted here up to 299 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     300
#define TYPE_STING                   301
#define TYPE_WHIP                    302
#define TYPE_SLASH                   303
#define TYPE_BITE                    304
#define TYPE_BLUDGEON                305
#define TYPE_CRUSH                   306
#define TYPE_POUND                   307
#define TYPE_CLAW                    308
#define TYPE_MAUL                    309
#define TYPE_THRASH                  310
#define TYPE_PIERCE                  311
#define TYPE_BLAST		     312
#define TYPE_PUNCH		     313
#define TYPE_STAB		     314

/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING		     399



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type {
   cbyte min_position;	/* Position for caster	 */
   int mana_min;	/* Min amount of mana used by a spell (highest lev) */
   int mana_max;	/* Max amount of mana used by a spell (lowest lev) */
   int mana_change;	/* Change in mana used by spell from lev to lev */
   int class_type;
   float timer;

   int min_level;
   int routines;
   cbyte violent;
   int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4

#define TESTY	2


/* Attacktypes with grammar */

struct attack_hit_type {
   const char	*singular;
   const char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, char_data *ch, \
		  char_data *victim, struct obj_data *obj, char *argument, int type)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict, argument, casttype);

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_gate);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_regen);
ASPELL(spell_locate_life);
ASPELL(spell_tornado);
ASPELL(spell_restore);
ASPELL(spell_bond);

/* basic magic calling functions */

int find_distance(int zone_obj, int zone_char);
int find_zone_slope(int zone_obj, int zone_char);
int prac_add(char_data *ch, int percent, int skill);
int int_apply(int intel);
int can_practice(char_data *ch, int skill);

int find_skill_num(char *name);

int mag_damage(int level, char_data *ch, char_data *victim, int spellnum, int savetype);

void mag_affects(int level, char_data *ch, char_data *victim,
  int spellnum, int savetype);

void mag_groups(int level, char_data *ch, int spellnum, int savetype);

void mag_masses(int level, char_data *ch, int spellnum, int savetype);

void mag_areas(int level, char_data *ch, int spellnum, int savetype);

void mag_summons(int level, char_data *ch, struct obj_data *obj,
	int spellnum, int savetype);

void mag_points(int level, char_data *ch, char_data *victim,
	int spellnum, int savetype);

void mag_unaffects(int level, char_data *ch, char_data *victim,
	int spellnum, int type);

void mag_alter_objs(int level, char_data *ch, struct obj_data *obj,
	int spellnum, int type);

void mag_creations(int level, char_data *ch, int spellnum);

int	call_magic(char_data *caster, char_data *cvict,
	struct obj_data *ovict, int spellnum, int level, int casttype, char *argument);

void	mag_objectmagic(char_data *ch, struct obj_data *obj,
	char *argument);

int	cast_spell(char_data *ch, char_data *tch,
	struct obj_data *tobj, int spellnum, char *argument);


/* other prototypes */
void spell_level(int spell, int chclass, int level);
void init_spell_levels(void);
void unbond(char *name);
const char *skill_name(int num);
