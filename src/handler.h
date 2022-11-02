/* ************************************************************************
*   File: handler.h                                     Part of CircleMUD *
*  Usage: header file: prototypes of handling and utility functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* handling the affected-structures */
void	affect_total(char_data *ch);
void	affect_modify(char_data *ch, cbyte loc, sbyte mod, long bitv, bool add);
void	affect_to_char(char_data *ch, struct affected_type *af);
void	affect_remove(char_data *ch, struct affected_type *af);
void	affect_from_char(char_data *ch, int type, int bit);
bool	affected_by_spell(char_data *ch, int type);
void	affect_join(char_data *ch, struct affected_type *af,
	bool add_dur, bool avg_dur, bool add_mod, bool avg_mod);

void	attach_weave(char_data *ch, char_data *victim, struct affected_type *af);
void	remove_weave(char_data *ch, struct weave_data *weave, int loop);
void	add_affection_to_list(char_data *ch, struct affect_type_not_saved *al);
void	remove_affection_list(char_data *ch, struct affect_type_not_saved *al, int loop);

// Races
int race_alias(char_data *ch, char *alias);

/* Weight */
int CAN_GET_OBJ(char_data *ch, struct obj_data *obj);
int CAN_CARRY_OBJ(char_data *ch, struct obj_data *obj);

int object_weight(struct obj_data *obj);
int carrying_weight(char_data *ch);
int wearing_weight(char_data *ch);
int total_carrying_weight(char_data *ch);

/* utility */

void check_boot_high();
char *money_desc(int amount);
struct obj_data *create_money(int amount);
int	isname(const char *str, const char *namelist);
int	is_name(const char *str, const char *namelist);
char	*fname(char *namelist);
int	get_number(char **name);

/* ******** objects *********** */
extern char *group_noise[MAX_TYPE];
int		is_evil_biotch(char_data *ch);
void	perform_group_noise(int room_number, int size, char *message, int type);
void	noise_addup(int room_number, int size, int type);
void	obj_to_char(struct obj_data *object, char_data *ch);
void	obj_from_char(struct obj_data *object);

void	equip_char(char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(char_data *ch, int pos);

struct obj_data *get_obj_in_list(char *name, struct obj_data *listy);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *listy);
struct obj_data *get_obj(char *name);
struct obj_data *get_obj_num(obj_rnum nr);

void	obj_to_room(struct obj_data *object, room_rnum room);
void	obj_from_room(struct obj_data *object);
void	obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void	obj_from_obj(struct obj_data *obj);
void	object_list_new_owner(struct obj_data *listy, char_data *ch);
void	move_obj_random(struct obj_data *obj, int bottom, int top, int inside_allowed);

void	extract_obj(struct obj_data *obj);

/* ******* characters ********* */

char_data *get_char_room(char *name, room_rnum room);
char_data *get_char_num(mob_rnum nr);
char_data *get_char(char *name);
char_data *get_char_by_name(char *name, int npc);

void	char_from_room(char_data *ch);
void	char_to_room(char_data *ch, room_rnum room);
void	extract_char(char_data *ch);
void	move_char_circle(char_data *ch);
void	move_char_random(char_data *ch, int bottom, int top, int inside_allowed);

void change_pos(char_data *ch, int pos);
void interupt_timer(char_data *ch);
/* find if character can see */
char_data *get_char_room_vis(char_data *ch, char *name);
char_data *get_player_vis(char_data *ch, char *name, int inroom);
char_data *get_char_vis(char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(char_data *ch, char *name, 
struct obj_data *listy);
struct obj_data *get_obj_vis(char_data *ch, char *name);
struct obj_data *get_object_in_equip_vis(char_data *ch,
char *arg, struct obj_data *equipment[], int *j);
char *fade_code(int room);
char *port_code(int room);

/* Slopes */
int find_distance(int zone_obj, int zone_char);
int find_zone_slope(int zone_obj, int zone_char);

int search_for_content(struct obj_data *container, int vnum, int type);

/* find all dots */

int	find_all_dots(char *arg);

#define FIND_INDIV	0
#define FIND_ALL	1
#define FIND_ALLDOT	2


void roll_moves(char_data *ch);

/* Generic Find */

int weapon_skill(char_data *ch, int weapon_type);
int generic_find(char *arg, int bitvector, char_data *ch,
char_data **tar_ch, struct obj_data **tar_obj);

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

/* clan functions */
void change_quest_value(char_data *ch, int number, int change);
void remove_player_clan(char_data *ch, int number);
int get_clan_quest_value(char_data *ch, int number, int clan_number);

/* prototypes from crash save system */

int		Crash_get_filename(char *orig_name, char *filename);
int		Crash_delete_file(char *name);
int		Crash_delete_crashfile(char_data *ch);
int		Crash_clean_file(char *name);
void	Crash_listrent(char_data *ch, char *name);
int		Crash_load(char_data *ch, int show);
void	Crash_crashsave(char_data *ch);
void	Crash_idlesave(char_data *ch);
void	Crash_save_all(void);

/* prototypes from fight.c */
void	set_fighting(char_data *ch, char_data *victim);
void	stop_fighting(char_data *ch);
void	stop_follower(char_data *ch);
void	hit(char_data *ch, char_data *victim, int type);
void	forget(char_data *ch, char_data *victim);
void	remember(char_data *ch, char_data *victim);
int		damage(char_data *ch, char_data *victim, int dam, int attacktype);
int		skill_message(int dam, char_data *ch, char_data *vict,
		int attacktype);
