//
// Created by MICHAEL MASON on 11/2/22.
//

#ifndef KINSLAYER_MUD_2004_11_10_BINARY_H
#define KINSLAYER_MUD_2004_11_10_BINARY_H

/**
 *
 * 1288 byte versions of char save data structures, reworked to function properly on 64-bit architectures
 * These were the structures used in the 1288-2003 era of the game.
 *
 */

#define MAX_NAME_LENGTH_1288		(20)
#define MAX_SKILLS_1288				(200)
#define MAX_TONGUE_1288				(3)
#define EXDSCR_LENGTH_1288			(240)
#define MAX_TITLE_LENGTH_1288		(80)
#define MAX_PWD_LENGTH_1288			(10)
#define MAX_AFFECT_1288				(32)
#define HOST_LENGTH_1288			(30)

struct char_special_data_saved_1288 {
	int	alignment;
	int	idnum;
	int act;
	int affected_by;
	sh_int apply_saving_throw[5];
};

struct player_special_data_saved_1288 {
	cbyte skills[MAX_SKILLS_1288+1];	/* array of skills plus skill 0		*/
	cbyte PADDING0;		/* used to be spells_to_learn		*/
	bool talks[MAX_TONGUE_1288];	/* PC s Tongues 0 for NPC		*/
	int	wimp_level;		/* Below this # of hit points, flee!	*/
	cbyte freeze_level;		/* Level of god who froze char, if any	*/
	sh_int invis_level;		/* level of invisibility		*/
	room_vnum load_room;		/* Which room to place char in		*/
	int /*bitvector_t*/	pref;	/* preference flags for PC's.		*/
	ubyte bad_pws;		/* number of bad password attemps	*/
	sbyte conditions[3];         // Drunk, full, thirst

	ubyte spare0;
	ubyte spare1;
	ubyte spare2;
	ubyte spare3;
	ubyte spare4;
	ubyte spare5;
	int spells_to_learn;
	int dodge;
	int parry;
	int absorb;
	int spare10;
	int spare11;
	int spare12;
	int spare13;
	int spare14;
	int spare15;
	int spare16;
	int	spare17;
	int	spare18;
	int	spare19;
	int	spare20;
	int spare21;
};

struct char_ability_data_1288 {
	sbyte str;
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis;
	sbyte dex;
	sbyte con;
	sbyte cha;
};

struct char_point_data_1288 {
	sh_int mana;
	sh_int max_mana;     /* Max mana for PC/NPC			   */
	sh_int hit;
	sh_int max_hit;      /* Max hit for PC/NPC                      */
	sh_int move;
	sh_int max_move;     /* Max move for PC/NPC                     */

	sh_int armor;        /* Internal -100..100, external -10..10 AC */
	int	gold;           /* Money carried                           */
	int	bank_gold;	/* Gold the char has in a bank account	   */
	int	exp;            /* The experience of the player            */

	sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
	sbyte damroll;       /* Any bonus or penalty to the damage roll */
};

struct affected_type_1288 {
	sh_int type;          /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	sbyte modifier;       /* This is added to apropriate ability     */
	cbyte location;        /* Tells which ability to change(APPLY_XXX)*/
	int /*bitvector_t*/	bitvector; /* Tells which bits to set (AFF_XXX) */

	int next;
};

struct char_file_u_1288 {
	//  char_player_data
	char	name[MAX_NAME_LENGTH_1288+1];
	char	description[EXDSCR_LENGTH_1288];
	char	title[MAX_TITLE_LENGTH_1288+1];
	cbyte sex;
	cbyte chclass;
	cbyte level;
	sh_int hometown;
	unsigned int birth;   //  Time of birth of character
	int	played;    //  Number of secs played in total
	ubyte weight;
	ubyte height;

	char	pwd[MAX_PWD_LENGTH_1288+1];    //  character's password

	struct char_special_data_saved_1288 char_specials_saved;
	struct player_special_data_saved_1288 player_specials_saved;
	struct char_ability_data_1288 abilities;
	struct char_point_data_1288 points;
	struct affected_type_1288 affected[MAX_AFFECT_1288];

	unsigned int last_logon;		//  Time (in secs) of last logon
	char host[HOST_LENGTH_1288+1];	//  host of last logon
};


/**
 *
 * 6616 byte versions of char save data structures, reworked to function properly on 64-bit architectures
 * These were the structures used in the 2003-2004 era of the game.
 *
 */

#define AF_ARRAY_MAX_6616		(4)
#define EXDSCR_LENGTH_6616		(240)
#define HOST_LENGTH_6616		(30)
#define MAX_AFFECT_6616			(32)
#define MAX_INPUT_LENGTH_6616	(256)
#define MAX_NAME_LENGTH_6616	(20)
#define MAX_PWD_LENGTH_6616		(10)
#define MAX_SKILLS_6616			(200)
#define MAX_TITLE_LENGTH_6616	(80)
#define MAX_TONGUE_6616			(3)
#define PM_ARRAY_MAX_6616		(4)
#define PR_ARRAY_MAX_6616		(4)

struct char_special_data_saved_6616 {
	int alignment;		/* +-1000 for alignments                */
	int idnum;			/* player's idnum; -1 for mobiles	*/
	int act[PM_ARRAY_MAX_6616];			/* act flag for NPC's; player flag for PC's */

	int	affected_by[AF_ARRAY_MAX_6616];		/* Bitvector for spells/skills affected by */
	sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};

struct player_special_data_saved_6616 {
	cbyte skills[MAX_SKILLS_6616+1];	/* array of skills plus skill 0		*/
	cbyte PADDING0;		/* used to be spells_to_learn		*/
	bool talks[MAX_TONGUE_6616];	/* PC s Tongues 0 for NPC		*/
	int	wimp_level;		/* Below this # of hit points, flee!	*/
	cbyte freeze_level;		/* Level of god who froze char, if any	*/
	sh_int invis_level;		/* level of invisibility		*/
	room_vnum load_room;		/* Which room to place char in		*/
	int	pref[PR_ARRAY_MAX_6616];			/* preference flags for PC's.		*/
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
	int qp;
	int rank;
	int warrant;          /* Room in which the player left the game */
	int spellpracs ;
	int clan;
	int mood;
	int logout;

	int spare14;
	int spare15;
	int spare16;
	int spare17;
	int spare18;
	int spare19;
	int spare20;
	int spare21;
};

struct char_ability_data_6616
{
	sbyte str;
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis;
	sbyte dex;
	sbyte con;
	sbyte cha;
};

struct char_point_data_6616
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
	sh_int clan;
	sh_int invis;

	sh_int taint;

	sh_int spare2;
	sh_int spare3;

	int death_wait;
	int warning;
	int spam;
	int shadow_points;
	int max_shadow_points;
	int hidden_qp;
	int master_weapon;
	int is_bashed;
	int stage;

	int spare10;
	int spare11;
	int spare12;
	int spare13;
	int spare14;
	int spare15;
	int spare16;
	int spare17;
	int spare18;
	int warrant[50];

	char poofin[MAX_INPUT_LENGTH_6616];
	char poofout[MAX_INPUT_LENGTH_6616];

	int array2[50];
	int array3[100];
	int array4[100];
	int array5[100];

	char host[MAX_INPUT_LENGTH_6616];		/* Character's IP/Hostname					*/
	char slew[MAX_INPUT_LENGTH_6616];		/* Used in legend list. Last kill			*/
	char forced[MAX_INPUT_LENGTH_6616];		/* Last person to force player				*/
	char whois_extra[MAX_INPUT_LENGTH_6616];

	char string3[MAX_INPUT_LENGTH_6616];
	char string4[MAX_INPUT_LENGTH_6616];
	char string5[MAX_INPUT_LENGTH_6616];
	char string6[MAX_INPUT_LENGTH_6616];
	char string7[MAX_INPUT_LENGTH_6616];
	char string8[MAX_INPUT_LENGTH_6616];
	char string9[MAX_INPUT_LENGTH_6616];
	char string10[MAX_INPUT_LENGTH_6616];

	unsigned int last_logon;
	unsigned int time_spare1;
	unsigned int time_spare2;
	unsigned int time_spare3;

	int	gold;           /* Money carried                             */
	int	bank_gold;	    /* Gold the char has in a bank account	     */
	int	exp;            /* The experience of the player              */
	sbyte damroll;       /* Any bonus or penalty to the damage roll   */
};

struct affected_type_6616
{
	sh_int type;          /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	sbyte modifier;       /* This is added to apropriate ability     */
	cbyte location;        /* Tells which ability to change(APPLY_XXX)*/
	int bitvector;       /* Tells which bits to set (AFF_XXX)       */

	int next; // Consume the space of a 32 bit pointer
};

struct char_file_u_6616 {
	/* char_player_data */
	char	name[MAX_NAME_LENGTH_6616+1];
	char	description[EXDSCR_LENGTH_6616];
	char	title[MAX_TITLE_LENGTH_6616+1];
	cbyte sex;
	cbyte chclass;
	cbyte race;
	cbyte level;
	sh_int hometown;
	unsigned int birth;   /* Time of birth of character     */
	int	played;    /* Number of secs played in total */
	ubyte weight;
	ubyte height;

	char	pwd[MAX_PWD_LENGTH_6616+1];    /* character's password */

	struct char_special_data_saved_6616 char_specials_saved;
	struct player_special_data_saved_6616 player_specials_saved;
	struct char_ability_data_6616 abilities;
	struct char_point_data_6616 points;
	struct affected_type_6616 affected[MAX_AFFECT_6616];

	unsigned int last_logon;		/* Time (in secs) of last logon */
	char host[HOST_LENGTH_6616+1];	/* host of last logon */
};

template <typename T>
std::list<T> load_binary_users()
{
	std::list<T> users;
	FILE *player_file;

	if (!(player_file = fopen(PLAYER_FILE, "r+b")))
	{
		log("Error opening playerfile.");
		return users;
	}

	while(!feof(player_file)) {
		T user{};
		fread(&user, sizeof(T), 1, player_file);
		users.push_back(user);
	}
	return users;
}

#endif //KINSLAYER_MUD_2004_11_10_BINARY_H