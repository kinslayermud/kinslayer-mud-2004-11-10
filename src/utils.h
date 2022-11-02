/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern FILE *logfile;
extern struct memory_data *mlist;

#define log			basic_mud_log

#include "spells.h"


/* public functions in utils.c */
template <class T> 
T* allocate(int slots, char *name, char *file, int line) 
{

	struct memory_data *memory = new memory_data;

	T* arr = new T[slots];

	strcpy(memory->type, name);
	strcpy(memory->file, file);
	memory->size = slots * sizeof(T);
	memory->line = line;

	memory->allocated = arr;
	memory->next = mlist;
	mlist = memory;

	return arr;

}

int		str_cmp(const char *arg1, const char *arg2);
int		strn_cmp(const char *arg1, const char *arg2, int n);
int		number(int low, int high);
int		touch(const char *path);
int		dice(int number, int size);
int		get_line(FILE *fl, char *buf);
int		get_filename(char *orig_name, char *filename, int mode);
int		replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
int		num_pc_in_room(struct room_data *room);
void	basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void	mudlog(const char *str, int type, int level, int file);
void	log_death_trap(char_data *ch);
void	sprintbit(long vektor, const char *names[], char *result);
void	sprinttype(int type, const char *names[], char *result);
void	sprintbitarray(int bitvector[], char *names[], int maxar, char *result);
void	core_dump_real(const char *, ush_int);
void	format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
char	*stripcr(char *dest, const char *src);
char	*str_dup(const char *source);
struct	time_info_data *age(char_data *ch);
#define	core_dump()		core_dump_real(__FILE__, __LINE__)

/* random functions in random.c */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);

/* in magic.c */
bool	circle_follow(char_data *ch, char_data * victim);

/* in act.informative.c */
void	look_at_room(char_data *ch, int mode);

/* in act.movmement.c */
int	do_simple_move(char_data *ch, int dir, int need_specials, int air_ok);
int	perform_move(char_data *ch, int dir, int following);

/* in limits.c */
int	mana_limit(char_data *ch);
int	hit_limit(char_data *ch);
int	move_limit(char_data *ch);
int	mana_gain(char_data *ch);
int	hit_gain(char_data *ch);
int	move_gain(char_data *ch);
void	advance_level(char_data *ch, int show);
void	set_title(char_data *ch, char *title);
void	gain_exp(char_data *ch, int gain);
void	gain_exp_regardless(char_data *ch, int gain);
void	gain_condition(char_data *ch, int condition, int value);
void	check_idling(char_data *ch);
void	point_update(void);
void	update_pos(char_data *victim);


/* various constants *****************************************************/

/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

/* get_filename() */
#define CRASH_FILE		0
#define ETEXT_FILE		1
#define ALIAS_FILE		2
#define IGNORE_FILE		3
#define SAY_FILE		4
#define TELL_FILE		5
#define PVAR_FILE		6

/* breadth-first searching */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

/* Memory Sizes */
#define KB_SIZE			(1024)
#define MB_SIZE			(KB_SIZE * KB_SIZE)

#define KB(memory)		((float) (memory) / (float) KB_SIZE)
#define MB(memory)		((float) (memory) / (float) MB_SIZE)

#define BASIC					1
#define THOROUGH				2

#define SILVER_CUFF				217
#define	IS_SILVER_CUFF(obj)		(GET_OBJ_VNUM(obj) == 217)

#define IS_HORSE(ch)			(MOB_FLAGGED(ch, MOB_MOUNT) || MOB_FLAGGED(ch, MOB_SHADOW_MOUNT))
#define HAS_SOURCE(ch)			(PRF_FLAGGED(ch, PRF_SOURCE))

#define SPACE					" "

/*
 * XXX: These constants should be configurable. See act.informative.c
 *	and utils.c for other places to change.
 */
/* mud-life time */
#define SECS_PER_MUD_HOUR   60
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(30*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH)

// real-life time (remember Real Life?) //
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)

/* Chest log values */
#define TYPE_IN			1
#define TYPE_OUT		2

/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/

#if !defined(__STRING)
#define __STRING(x)	#x
#endif

#if BUFFER_MEMORY

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) debug_calloc ((number), sizeof(type), __STRING(result), __FUNCTION__, __LINE__)))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
	if (!((result) = (type *) debug_realloc ((result), sizeof(type) * (number), __STRING(result), __FUNCTION__, __LINE__)))\
		{ perror("realloc failure"); abort(); } } while(0)

#define free(variable)	debug_free((variable), __FUNCTION__, __LINE__)

#define str_dup(variable)	debug_str_dup((variable), __STRING(variable), __FUNCTION__, __LINE__)

#else

#define CREATE(result, type, number)  do {\
	if (!((result) = new type[(number)]))\
		{ perror("new failure"); abort(); } \
	else \
		memset((result), 0, sizeof(type) * (number)); \
	} while(0)

/*
#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)
*/

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

#endif 

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */

#define NEW(type, slots)	allocate<type>(slots, #type, __FILE__, __LINE__);

/* Serai - 07/22/04 - Doubly-linked list macros */

#define ADD_NODE(first,last,node,prevPtr,nextPtr,prev,next) \
	do \
	{ \
		if ((prev) == nullptr) \
		{ \
			ADD_FIRST_NODE(first,last,node,prevPtr,nextPtr); \
		} \
		else if ((next) == nullptr) \
		{ \
			ADD_END_NODE(first,last,node,prevPtr,nextPtr); \
		} \
		else \
		{ \
			(prev)->nextPtr = (node); \
			(next)->prevPtr = (node); \
			(node)->nextPtr = (next); \
			(node)->prevPtr = (prev); \
		} \
	} while(0)

#define ADD_FIRST_NODE(first,last,node,prevPtr,nextPtr) \
	do \
	{ \
		(node)->prevPtr = nullptr; \
		(node)->nextPtr = (first); \
		if (!(first)) \
			(last) = (node); \
		else \
			(first)->prevPtr = (node); \
		(first) = (node); \
	} while(0)

#define ADD_END_NODE(first,last,node,prevPtr,nextPtr) \
	do \
	{ \
		if (!(first)) \
		{ \
			(first) = (last) = (node); \
			(node)->nextPtr = (node)->prevPtr = nullptr; \
		} \
		else if ((first) == (last)) \
		{ \
			(first)->nextPtr = (node); \
			(node)->prevPtr = (first); \
			(node)->nextPtr = nullptr; \
			(last) = (node); \
		} \
		else \
		{ \
			(last)->nextPtr = (node); \
			(node)->prevPtr = (last); \
			(node)->nextPtr = nullptr; \
			(last) = (node); \
		} \
	} while(0)

#define REMOVE_NODE(first,last,node,prevPtr,nextPtr) \
	do \
	{ \
		if ((first) == (last)) \
			(first) = (last) = nullptr; \
		else if ((first) == (node)) \
		{ \
			(first) = (first)->nextPtr; \
			(first)->prevPtr = nullptr; \
		} \
		else if ((last) == (node)) \
		{ \
			(last) = (last)->prevPtr; \
			(last)->nextPtr = nullptr; \
		} \
		else \
		{ \
			((node)->prevPtr)->nextPtr = (node)->nextPtr; \
			((node)->nextPtr)->prevPtr = (node)->prevPtr; \
		} \
		(node)->prevPtr = (node)->nextPtr = nullptr; \
	} while(0)

/* ... */

#define REMOVE_FROM_LIST(item, head, next)		\
	if ((item) == (head))						\
		head = (item)->next;					\
												\
	else {										\
		temp = head;							\
												\
		while (temp && (temp->next != (item)))	\
			temp = temp->next;					\
												\
			if (temp)							\
				temp->next = (item)->next;		\
   }											\

/* basic bitvector utils *************************************************/

#define Q_FIELD(x) ((int) (x) / 32)
#define Q_BIT(x) (1 << ((x) % 32))
#define IS_SET_AR(var, bit)   ((var)[Q_FIELD(bit)] & Q_BIT(bit))
#define SET_BIT_AR(var, bit)	((var)[Q_FIELD(bit)] |= Q_BIT(bit))
#define REMOVE_BIT_AR(var, bit)  ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
#define TOGGLE_BIT_AR(var, bit)  ((var)[Q_FIELD(bit)] = \
					              (var)[Q_FIELD(bit)] ^ Q_BIT(bit))


									

#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define IS_WARRANTED(ch, clan)	(IS_SET_AR(GET_WARRANTS(ch), clan))

/*
 * Accessing player specific data structures on a mobile is a very bad thing
 * to do.  Consider that changing these variables for a single mob will change
 * it for every other single mob in the game.  If we didn't specifically check
 * for it, 'wimpy' would be an extremely bad thing for a mob to do, as an
 * example.  If you really couldn't care less, change this to a '#if 0'.
 */
#if 0
/* Subtle bug in the '#var', but works well for now. */
#define CHECK_PLAYER_SPECIAL(ch, var) \
	(*(((ch)->player_specials == &dummy_mob) ? (log("SYSERR: Mob using '"#var"' at %s:%d.", __FILE__, __LINE__), &(var)) : &(var)))
#else
#define CHECK_PLAYER_SPECIAL(ch, var)	(var)
#endif

#define MOB_FLAGS(ch)					((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch)					((ch)->char_specials.saved.act)
#define PRF_FLAGS(ch)					CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref))
#define AFF_FLAGS(ch)					((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc)					(world[(loc)].room_flags)

#define IS_NPC(ch)						(IS_SET_AR(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)						(IS_NPC(ch) && ((ch)->nr >-1))

#define MOB_FLAGGED(ch, flag)			(IS_NPC(ch) && IS_SET_AR(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag)			(!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag)			(IS_SET_AR(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag)			(IS_SET_AR(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag)			(IS_SET_AR(ROOM_FLAGS(loc), (flag)))
#define EXIT_FLAGGED(exit, flag)		(IS_SET((exit)->exit_info, (flag)))
#define OBJVAL_FLAGGED(obj, flag)		(IS_SET(GET_OBJ_VAL((obj), 1), (flag)))
#define OBJWEAR_FLAGGED(obj, flag)		(IS_SET_AR((obj)->obj_flags.wear_flags, (flag)))
#define OBJ_FLAGGED(obj, flag)			(IS_SET_AR(GET_OBJ_EXTRA(obj), (flag)))

#define IS_HORSE(ch)					(MOB_FLAGGED(ch, MOB_MOUNT) || MOB_FLAGGED(ch, MOB_SHADOW_MOUNT))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PLR_FLAGS(ch), (flag))) && \
							  (IS_SET_AR(PLR_FLAGS(ch), (flag))))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF_FLAGS(ch), (flag))) && \
							  (IS_SET_AR(PRF_FLAGS(ch), (flag))))


/* room utils ************************************************************/


#define SECT(room)	(world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))

#define GET_ROOM_VNUM(rnum)	((rnum) >= 0 && (rnum) <= top_of_world ? world[(rnum)].number : NOWHERE)
#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : nullptr)

/* char utils ************************************************************/


#define IN_ROOM(ch)	((ch)->in_room)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch)->year)

#define GET_NAME(ch)    (IS_NPC(ch) ? \
			 (ch)->player.short_descr : (ch)->player.name)
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)	((ch)->player.passwd)
#define GET_PFILEPOS(ch)((ch)->pfilepos)

/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */

#define IS_KING(ch)				(!str_cmp(GET_NAME(ch), "Galnor") || !str_cmp(GET_NAME(ch), "Void"))
#define LOCAL_IP(d)				(!str_cmp(d->host, "localhost") || !str_cmp(d->host, "127.0.0.1") || !str_cmp(d->host, "69.140.139.111"))
#define IS_LOCALHOST(str)		(!str_cmp(str, "localhost") || !str_cmp(str, "127.0.0.1") || !str_cmp(str, "69.140.139.111"))

#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_CLASS(ch)			((ch)->player.chclass)
#define GET_RACE(ch)			((ch)->player.race)
#define GET_HOME(ch)			((ch)->player.hometown)
#define GET_HEIGHT(ch)			((ch)->player.height)
#define GET_WEIGHT(ch)			((ch)->player.weight)
#define GET_SEX(ch)				((ch)->player.sex)

#define GET_STR(ch)				((ch)->aff_abils.str)
#define GET_ADD(ch)				((ch)->aff_abils.str_add)
#define GET_DEX(ch)				((ch)->aff_abils.dex)
#define GET_INT(ch)				((ch)->aff_abils.intel)
#define GET_WIS(ch)				((ch)->aff_abils.wis)
#define GET_CON(ch)				((ch)->aff_abils.con)
#define GET_CHA(ch)				((ch)->aff_abils.cha)

/* levels of taintedness, taint added by spell timer */
#define TAINT_MAX      (5000)

// Serai: (1 - average of wis and int over the max of any stat (25)), 90% of that * 100 (* 90.0), * percent tainted.
//#define TAINT_CALC(ch) ((1.0 - (.75*(double)GET_WIS(ch) + .25*(double)GET_INT(ch) + .5) / 25.0) * 90.0 * ((double)GET_TAINT(ch) / (double)TAINT_MAX))
#define TAINT_CALC(ch) (0)

#define GET_EXP(ch)				((ch)->points.exp)
#define GET_HIT(ch)				((ch)->points.hit)
#define GET_MAX_HIT(ch)			((ch)->points.max_hit)
#define GET_MOVE(ch)			((ch)->points.move)
#define GET_MAX_MOVE(ch)		((ch)->points.max_move)
#define GET_MANA(ch)			((ch)->points.mana)
#define GET_MAX_MANA(ch)		((ch)->points.max_mana)
#define GET_SHADOW(ch)			((ch)->points.shadow_points)
#define GET_MAX_SHADOW(ch)		((ch)->points.max_shadow_points)
#define GET_WARRANTS(ch)		((ch)->points.warrants)
#define GET_OB(ch)				((ch)->points.offensive)
#define GET_PB(ch)				((ch)->points.parry)
#define GET_DB(ch)				((ch)->points.dodge)
#define GET_INVIS_LEV(ch)		((ch)->points.invis)
#define GET_ABS(ch)				((ch)->points.absorb)
#define GET_WP(ch)				((ch)->points.weave)
#define GET_TAVEREN(ch)			((ch)->points.taveren)
#define GET_LEGEND(ch)			((ch)->points.legend)
#define GET_GOLD(ch)			((ch)->points.gold)
#define GET_BANK_GOLD(ch)		((ch)->points.bank_gold)
#define GET_DAMROLL(ch)			((ch)->points.damroll)
#define GET_DEATH_WAIT(ch)		((ch)->points.death_wait)
#define GET_WARNINGS(ch)		((ch)->points.warning)
#define GET_SLEW_MESSAGE(ch)	((ch)->points.slew)
#define POOFOUT(ch)				((ch)->points.poofout)
#define POOFIN(ch)				((ch)->points.poofin)
#define GET_HIDDEN_QP(ch)		((ch)->points.hidden_qp)
#define LAST_LOGON(ch)			((ch)->points.last_logon)
#define GET_MASTER_WEAPON(ch)	((ch)->points.master_weapon)
#define IS_BASHED(ch)			((ch)->points.is_bashed)
#define GET_TAINT(ch)			((ch)->points.taint)
#define GET_FORCED(ch)			((ch)->points.forced)
#define GET_WHOIS_EXTRA(ch)		((ch)->points.whois_extra)
#define GET_BOND(ch)			((ch)->points.bond)
#define GET_DP(ch)				((ch)->points.dark_points)
#define GET_SICKNESS(ch)		((ch)->points.sickness)
#define GET_STRAIN(ch)			((ch)->points.strain)

#define GET_POS(ch)				((ch)->char_specials.position)
#define GET_LAST_POS(ch)		((ch)->char_specials.last_pos)
#define GET_IDNUM(ch)			((ch)->char_specials.saved.idnum)
#define GET_ID(x)				((x)->id)
#define IS_CARRYING_W(ch)		((ch)->char_specials.carry_weight)
#define FLEE_LAG(ch)			((ch)->char_specials.flee_lag)
#define FLEE_GO(ch)				((ch)->char_specials.flee_go)
#define GET_DIRECTION(ch)		((ch)->char_specials.direction)
#define IS_CARRYING_N(ch)		((ch)->char_specials.carry_items)
#define FIGHTING(ch)			((ch)->char_specials.fighting)
#define HUNTING(ch)				((ch)->char_specials.hunting)
#define MOUNT(ch)				((ch)->char_specials.mount)
#define RIDDEN_BY(ch)			((ch)->char_specials.ridden_by)
#define GET_TARGET(ch)			((ch)->char_specials.target)
#define GET_MARKED(ch)			((ch)->char_specials.marked)
#define GET_SAVE(ch, i)			((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch)		((ch)->char_specials.saved.alignment)
#define GET_COMMAND(ch)			((ch)->char_specials.command)
#define GET_STAGE(ch)			((ch)->char_specials.stage)


#define GET_COND(ch, i)			CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.conditions[(i)]))
#define GET_LOADROOM(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.load_room))
#define GET_PRACTICES(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.spells_to_learn))
#define GET_SPRACTICES(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.spellpracs))
#define GET_WIMP_LEV(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.wimp_level))
#define GET_FREEZE_LEV(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.freeze_level))
#define GET_BAD_PWS(ch)			CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bad_pws))
#define GET_TALK(ch, i)			CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.talks[i]))
#define GET_MOOD(ch)			CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.mood))

#define CLAN_GAIDIN				1
#define CLAN_ANDORAN			2
#define CLAN_SHIENARAN			3
#define CLAN_MURANDIAN			4
#define CLAN_COMPANION			5
#define CLAN_DEFENDER			6
#define CLAN_KOBOL				7
#define CLAN_DHAVOL				8
#define CLAN_ALGHOL				9
#define CLAN_KHOMON				10
#define CLAN_GHOBHLIN			11
#define CLAN_CHOSEN				12
#define CLAN_WOLFBROTHER		13
#define CLAN_BLADEMASTERS		14
#define CLAN_BLACK_TOWER		15
#define CLAN_WHITE_TOWER		16
#define CLAN_GRAY_AJAH			17
#define CLAN_RED_AJAH			18
#define CLAN_YELLOW_AJAH		19
#define CLAN_BROWN_AJAH			20
#define CLAN_WHITE_AJAH			21
#define CLAN_GREEN_AJAH			22
#define CLAN_BLUE_AJAH			23
#define CLAN_BLACK_AJAH			24
#define CLAN_SOULLESS			25

#define IS_BONDED(ch)			(!str_cmp(GET_BOND(ch), "") ? 0 : 1)
#define IS_WARDER(ch)			((IS_BONDED(ch)) && (ch->getClan(CLAN_GAIDIN)))

/* Weapon Values */
#define WEAPON_LONG_BLADE		0
#define WEAPON_SHORT_BLADE		1
#define WEAPON_CLUB				2
#define WEAPON_STAFF			3
#define WEAPON_SPEAR			4
#define WEAPON_AXE				5

#define GET_HOUR				time_info.hours

#define GET_LAST_OLC_TARG(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_targ))
#define GET_LAST_OLC_MODE(ch)	CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_mode))
#define GET_ALIASES(ch)			CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->aliases))
#define GET_LAST_TELL(ch)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_tell))

#define GET_SKILL(ch, i)		CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skills[i]))
#define SET_SKILL(ch, i, pct)	do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.skills[i]) = pct; } while(0)

#define GET_EQ(ch, i)			((ch)->equipment[i])

#define GET_MOB_SPEC(ch)		(IS_MOB(ch) ? mob_index[(ch)->nr].func : nullptr)
#define GET_MOB_RNUM(mob)		((mob)->nr)
#define GET_MOB_VNUM(mob)		(IS_MOB(mob) ? \
								mob_index[GET_MOB_RNUM(mob)].vnum : -1)

#define GET_MOB_WAIT(ch)		((ch)->mob_specials.wait_state)
#define GET_DEFAULT_POS(ch)		((ch)->mob_specials.default_pos)
#define MEMORY(ch)				((ch)->mob_specials.memory)
#define GET_AGGRO(ch, num)		((ch)->mob_specials.aggro[num])

#define SET_AGGRO(ch, num)		(GET_AGGRO(ch, num) = 1)
#define REMOVE_AGGRO(ch, num)	(GET_AGGRO(ch, num) = 0)


#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define CAN_CARRY_W(ch)			(GET_STR(ch) * 7)
#define CAN_CARRY_N(ch)			(5 + (GET_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))
#define AWAKE(ch)				(GET_POS(ch) > POS_SLEEPING)

#define CAN_SEE_IN_DARK(ch)		(PRF_FLAGGED(ch, PRF_HOLYLIGHT) || IS_TROLLOC(ch) || IS_NPC(ch) || \
								ch->getClan(CLAN_WOLFBROTHER) || AFF_FLAGGED(ch, AFF_NIGHT_VISION))

#define IS_GOOD(ch)			(GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)			(GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))


/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) { \
	if ((ch)->desc) (ch)->desc->wait = (cycle); \
	else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }

#define LAST_COMMAND(ch) ((ch)->desc->delayed_command)
#define CHECK_WAIT(ch)	(((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
#define STATE(d)	((d)->connected)


/* object utils **********************************************************/

#define GET_CORPSE_DATA(obj) ((obj)->corpse_data)


#define GET_OBJ_TYPE(obj)		((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)		((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)		((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)		((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)		((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)		((obj)->obj_flags.weight)
#define GET_OBJ_OB(obj)			((obj)->obj_flags.offensive)
#define GET_OBJ_PB(obj)			((obj)->obj_flags.parry)
#define GET_OBJ_DB(obj)			((obj)->obj_flags.dodge)
#define GET_OBJ_ABS(obj)		((obj)->obj_flags.absorb)
#define GET_OBJ_TIMER(obj)		((obj)->obj_flags.timer)
#define GET_WEAPON_TYPE(obj)	((obj)->obj_flags.weapon_type)
#define GET_OBJ_RNUM(obj)		((obj)->item_number)
#define GET_OBJ_VNUM(obj)		(GET_OBJ_RNUM(obj) >= 0 ? \
								obj_index[GET_OBJ_RNUM(obj)].vnum : -1)

#define IS_OBJ_STAT(obj,stat)	(IS_SET_AR((obj)->obj_flags.extra_flags,stat))

#define IS_CORPSE(obj)			(GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
								GET_OBJ_VAL((obj), 3) == 1)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : nullptr)

#define CAN_WEAR(obj, part) (IS_SET_AR((obj)->obj_flags.wear_flags, (part)))
#define GET_OBJ_EXTRA_AR(obj, i)	((obj)->obj_flags.extra_flags[(i)])


/* compound utilities and other macros **********************************/

/*
 * Used to compute CircleMUD version. To see if the code running is newer
 * than 3.0pl13, you would use: #if _CIRCLEMUD > CIRCLEMUD_VERSION(3,0,13)
 */
#define CIRCLEMUD_VERSION(major, minor, patchlevel) \
	(((major) << 16) + ((minor) << 8) + (patchlevel))

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!AFF_FLAGGED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_TROLLOC(sub) || sub->getClan(CLAN_WOLFBROTHER) || IS_NPC(sub) || \
	AFF_FLAGGED(sub, AFF_NIGHT_VISION)))

#define INVIS_OK(sub, obj) \
	((!AFF_FLAGGED((obj), AFF_HIDE) || ((AFF_FLAGGED(sub, AFF_NOTICE)) && \
	GET_SKILL(sub, SKILL_NOTICE) >= GET_SKILL(obj, SKILL_HIDE) - 15)))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
	(MORT_CAN_SEE(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED(sub, PRF_HOLYLIGHT)))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= (GET_INVIS_LEV(obj))) && \
   IMM_CAN_SEE(sub, obj)) && !(IS_KING(obj) && GET_MAX_HIT(obj) == 4999))

/* End of CAN_SEE */

#define INVIS_OK_OBJ(sub, obj) \
	(!IS_OBJ_STAT((obj), ITEM_INVISIBLE))

/* Is anyone carrying this object and if so, are they visible? */
#define CAN_SEE_OBJ_CARRIER(sub, obj) \
  ((!obj->carried_by || CAN_SEE(sub, obj->carried_by)) &&	\
   (!obj->worn_by || CAN_SEE(sub, obj->worn_by)))

#define MORT_CAN_SEE_OBJ(sub, obj) \
	(LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && CAN_SEE_OBJ_CARRIER(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED((sub), PRF_HOLYLIGHT)))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))


#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])
#define RACE_ABBREV(ch) (IS_NPC(ch) ? "--" : race_abbrevs[(int)GET_RACE(ch)])

#define IS_WARRIOR(ch)		((GET_CLASS(ch) == CLASS_WARRIOR))
#define IS_THIEF(ch)		((GET_CLASS(ch) == CLASS_THIEF))
#define IS_RANGER(ch)		((GET_CLASS(ch) == CLASS_RANGER))
#define IS_CHANNELER(ch)	((GET_CLASS(ch) == CLASS_CHANNELER))
#define IS_FADE(ch)			((GET_CLASS(ch) == CLASS_FADE))
#define IS_DREADLORD(ch)	((GET_CLASS(ch) == CLASS_DREADLORD))
#define IS_BLADEMASTER(ch)	((GET_CLASS(ch) == CLASS_BLADEMASTER))
#define IS_GREYMAN(ch)		((GET_CLASS(ch) == CLASS_GREYMAN))



#define IS_HUMAN(ch)		((GET_RACE(ch) == RACE_HUMAN))
#define IS_TROLLOC(ch)		((GET_RACE(ch) == RACE_TROLLOC))
#define IS_SEANCHAN(ch)		((GET_RACE(ch) == RACE_SEANCHAN))
#define IS_AIEL(ch)			((GET_RACE(ch) == RACE_AIEL))

#define CAN_RIDE(ch)		(!(IS_TROLLOC(ch) && !IS_FADE(ch) && !IS_DREADLORD(ch)))

#define AGGRO_ALL		0
#define AGGRO_HUMAN		1
#define AGGRO_TROLLOC	2
#define AGGRO_SEANCHAN	3
#define AGGRO_AIEL		4
#define AGGRO_NON_CLAN	5
#define AGGRO_MOB		6

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))

#define ATTACK_TYPE(obj, val) (


/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif


#define STM(ch, till)			message(ch, "%s%sTICK IN %d SECONDS.%s\r\n", COLOR_BOLD(ch, CL_COMPLETE), \
COLOR_GREEN(ch, CL_COMPLETE), till, COLOR_NORMAL(ch, CL_COMPLETE))

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#define SENDOK(ch)	((ch)->desc && (to_sleeping || AWAKE(ch)) && \
			!PLR_FLAGGED((ch), PLR_WRITING))
