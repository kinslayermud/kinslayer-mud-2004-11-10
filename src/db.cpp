/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on Diku, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __DB_C__

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "dg_scripts.h"
#include "olc.h"
#include "binary.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
**************************************************************************/

long max_id = MOBOBJ_ID_BASE;   /* for unique mob/obj id's       */	

int top_of_world = 0;		/* ref to top element of world	 */
int top_of_trigt = 0;           /* top of trigger index table    */
int top_of_mobt = 0;		/* top of mobile index table	 */
int top_of_objt = 0;		/* top of object index table	 */
int no_mail = 0;		/* mail disabled?		 */
int mini_mud = 0;		/* mini-mud mode?		 */
int no_rent_check = 0;		/* skip rent check on boot?	 */
int circle_restrict = 0;	/* level of game restriction	 */
int top_of_helpt = 0;		/* top of help index table	 */
int top_of_zone_table = 0;	/* top element of zone tab	 */
long top_id = 0;

extern int ID_NUMBER;
extern int idle_void;
extern struct memory_data *mlist;
extern struct kit_data *real_kit(int);

class Ideas *idea_list = nullptr;
class wizlist_data *wizlist = nullptr;
list<NoteData> notes;
list<NoteData> mob_notes;
room_data *world = nullptr;	/* array of rooms		 */
char_data *character_list = nullptr; /* global linked list of chars */
index_data **trig_index; /* index table for triggers      */
battle tg;
clan_data clan_list[50];
legend_data legend[13];
index_data *mob_index;	/* index table for mobile file	 */
char_data *mob_proto;	/* prototypes for mobs		 */
obj_data *object_list = nullptr;	/* global linked list of objs	 */
index_data *obj_index;	/* index table for object file	 */
obj_data *obj_proto;	/* prototypes for objs		 */
zone_data *zone_table;	/* zone table			 */
message_list fight_messages[MAX_MESSAGES];	/* fighting messages	 */
player_index_element *player_table = nullptr;	/* index to plr file	 */
time_info_data time_info;/* the infomation about the time    */
weather_data weather_info;	/* the infomation about the weather */
player_special_data dummy_mob;	/* dummy spec area for mobs	*/
reset_q_type reset_q;	/* queue of zones to be reset	 */
help_index_element *help_table = 0;	/* the help table	 */
time_info_data *mud_time_passed(time_t t2, time_t t1);
obj_data *chest_head = nullptr;
kit_data *kit_index = nullptr, *kit_end = nullptr; /* kit list, first and last pointers */
chest_log *cl_list = nullptr;							/* Chest Log List */

time_t boot_time = 0;		/* time of mud boot		 */

sh_int r_human_start_room;  /* room of human start room      */
sh_int r_trolloc_start_room;/* room of trolloc start room    */
sh_int r_immort_start_room;	/* rnum of immort start room	 */
sh_int r_frozen_start_room;	/* rnum of frozen start room	 */

char *credits = nullptr;		/* game credits			 */
char *news = nullptr;		/* mud news			 */
char *motd = nullptr;		/* message of the day - mortals */
char *imotd = nullptr;		/* message of the day - immorts */
char *help = nullptr;		/* help screen			 */
char *info = nullptr;		/* info page			 */
char *immlist = nullptr;		/* list of peon gods		 */
char *background = nullptr;	/* background story		 */
char *handbook = nullptr;		/* handbook for new immortals	 */
char *policies = nullptr;		/* policies page		 */
char *startup = nullptr;		/* startup screen		 */

extern int GLOBAL_RESET;

/* local functions */
long asciiflag_conv(char *flag);
char *parse_object(FILE * obj_f, int nr);
int count_mobs_zone(int mob_no, int znum);
int file_to_string(const char *name, char *buf);
int file_to_string_alloc(const char *name, char **buf);
int is_empty(int zone_nr);
int count_alias_records(FILE *fl);
int count_hash_records(FILE * fl);
void setup_dir(FILE * fl, int room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode, char *filename);
void parse_trigger(FILE *fl, int virtual_nr);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
void boot_battle(void);
void boot_clans(void);
void boot_legends(void);
void reset_zone(int zone);
void reboot_wizlists(void);
void boot_world(void);
void parse_simple_mob(FILE *mob_f, int i, int nr);
void interpret_espec(const char *keyword, const char *value, int i, int nr);
void parse_espec(char *buf, int i, int nr);
void parse_kit(FILE *fl, int virtual_nr);
void parse_enhanced_mob(FILE *mob_f, int i, int nr);
void get_one_line(FILE *fl, char *buf);
void save_etext(char_data * ch);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(int zone, int cmd_no, const char *message);
void add_follower(char_data * ch, char_data * leader);
void reset_time(void);
void load_notes(void);
void load_ideas(void);
void load_values(void);
void load_wizlist(void);
void remove_entry(char *name, long id, int save);
void tag_argument(char *argument, char *tag);
class player_index_element *getEntryByName(char *name);
ACMD(do_reboot);

/* external functions */
extern void free_alias(struct alias * a);
extern void load_messages(void);
extern void weather_and_time(int mode);
extern void mag_assign_spells(void);
extern void boot_social_messages(void);
extern void create_command_list(void);
extern void update_obj_file(void);	/* In objsave.c */
extern void sort_commands(void);
extern void sort_spells(void);
extern void load_banned(void);
extern void Read_Invalid_List(void);
extern void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
extern void reset_skills(char_data *ch);
extern void save_php_file(void);
extern int find_name(char *name, player_index_element *ptable, int ptable_top);

/* external vars */
extern int no_specials;
extern sh_int human_start_room;
extern sh_int trolloc_start_room;
extern sh_int immort_start_room;
extern sh_int frozen_start_room;
extern struct descriptor_data *descriptor_list;
extern int Obj_to_store(struct obj_data * obj, FILE * fl);
extern struct obj_data *Obj_from_store(struct obj_file_elem object);

#define READ_SIZE 256

FILE *player_fl = nullptr;		/* file desc of player file	 */

/*************************************************************************
*  routines for booting the system                                       *
*************************************************************************/

ACMD(do_reboot)
{
	int i;

	one_argument(argument, arg);

	if (!str_cmp(arg, "all") || *arg == '*')
	{
		file_to_string_alloc(IMMLIST_FILE, &immlist);
		file_to_string_alloc(NEWS_FILE, &news);
		file_to_string_alloc(CREDITS_FILE, &credits);
		file_to_string_alloc(MOTD_FILE, &motd);
		file_to_string_alloc(IMOTD_FILE, &imotd);
		file_to_string_alloc(HELP_PAGE_FILE, &help);
		file_to_string_alloc(INFO_FILE, &info);
		file_to_string_alloc(POLICIES_FILE, &policies);
		file_to_string_alloc(HANDBOOK_FILE, &handbook);
		file_to_string_alloc(BACKGROUND_FILE, &background);
		file_to_string_alloc(STARTUP_FILE, &startup);
		boot_clans();
		boot_legends();
	} 

	else if (!str_cmp(arg, "immlist"))
		file_to_string_alloc(IMMLIST_FILE, &immlist);

	else if (!str_cmp(arg, "news"))
		file_to_string_alloc(NEWS_FILE, &news);

	else if (!str_cmp(arg, "credits"))
		file_to_string_alloc(CREDITS_FILE, &credits);

	else if (!str_cmp(arg, "motd"))
		file_to_string_alloc(MOTD_FILE, &motd);

	else if (!str_cmp(arg, "imotd"))
		file_to_string_alloc(IMOTD_FILE, &imotd);

	else if (!str_cmp(arg, "help"))
		file_to_string_alloc(HELP_PAGE_FILE, &help);

	else if (!str_cmp(arg, "info"))
		file_to_string_alloc(INFO_FILE, &info);

	else if (!str_cmp(arg, "policy"))
		file_to_string_alloc(POLICIES_FILE, &policies);

	else if (!str_cmp(arg, "handbook"))
		file_to_string_alloc(HANDBOOK_FILE, &handbook);

	else if (!str_cmp(arg, "background"))
		file_to_string_alloc(BACKGROUND_FILE, &background);

	else if (!str_cmp(arg, "startup"))
		file_to_string_alloc(STARTUP_FILE, &startup);

	else if (!str_cmp(arg, "xhelp"))
	{

		if (help_table)
		{
			for (i = 0; i <= top_of_helpt; i++)
			{
				if (help_table[i].keywords)
					delete(help_table[i].keywords);

				if (help_table[i].entry)
					delete(help_table[i].entry);
			}

			delete(help_table);
		}

		top_of_helpt = 0;
		index_boot(DB_BOOT_HLP);
	} 

	else
	{
		send_to_char("Unknown reload option.\r\n", ch);
		return;
	}

	send_to_char(OK, ch);
}

void boot_world(void)
{
	log("Loading zone table.");
	index_boot(DB_BOOT_ZON);

	log("Loading triggers and generating index.");
	index_boot(DB_BOOT_TRG);

	log("Loading rooms.");
	index_boot(DB_BOOT_WLD);

	log("Renumbering rooms.");
	renum_world();

	log("Checking start rooms.");
	check_start_rooms();

	log("Loading kits and generating index.");
	index_boot(DB_BOOT_KIT);

	log("Loading mobs and generating index.");
	index_boot(DB_BOOT_MOB);

	log("Loading objs and generating index.");
	index_boot(DB_BOOT_OBJ);

	log("Renumbering zone table.");
	renum_zone_table();

	if (!no_specials)
	{
		log("Loading shops.");
		index_boot(DB_BOOT_SHP);
	}
}
void convert_from_binary();
/* body of the booting system */
void boot_db(void)
{
	int i;

	log("Boot db -- BEGIN.");

	log("Generating player index.");
	build_player_index();

	// 2022.11.01: mmason - disabling player auto deletion
	// log("Performing player file auto delete.");
	// playerAutoDelete();

	log("Booting Ideas:");
	load_ideas();

	log("Booting Clans:");
	boot_clans();

	log("Booting Notes:");
	load_notes();
  
	log("Booting legends list:");
	boot_legends();

	log("Booting the battle list:");
	boot_battle();

	log("Resetting the game time:");
	reset_time();

	load_values();

	log("Reading news, credits, help, bground, info & motds.");
	file_to_string_alloc(NEWS_FILE, &news);
	file_to_string_alloc(CREDITS_FILE, &credits);
	file_to_string_alloc(MOTD_FILE, &motd);
	file_to_string_alloc(IMOTD_FILE, &imotd);
	file_to_string_alloc(HELP_PAGE_FILE, &help);
	file_to_string_alloc(INFO_FILE, &info);
	file_to_string_alloc(IMMLIST_FILE, &immlist);
	file_to_string_alloc(POLICIES_FILE, &policies);
	file_to_string_alloc(HANDBOOK_FILE, &handbook);
	file_to_string_alloc(BACKGROUND_FILE, &background);
	file_to_string_alloc(STARTUP_FILE, &startup);

	boot_world();

	log("Loading help entries.");
	index_boot(DB_BOOT_HLP);

	log("Booting Wizlist:");
	load_wizlist();

	log("Loading fight messages.");
	load_messages();

	log("Loading social messages.");
	boot_social_messages();
	create_command_list(); /* aedit patch -- M. Scott */

	log("Assigning function pointers:");

	if (!no_specials)
	{
		log("   Mobiles.");
		assign_mobiles();
		log("   Shopkeepers.");
		assign_the_shopkeepers();
		log("   Objects.");
		assign_objects();
		log("   Rooms.");
		assign_rooms();
	}
  
	log("   Spells.");
	mag_assign_spells();

	log("Assigning spell and skill levels.");
	init_spell_levels();

	log("Sorting command list and spells.");
	sort_commands();
	sort_spells();

	log("Booting mail system.");
  
	if (!scan_file())
	{
		log("    Mail boot failed -- Mail system disabled");
		no_mail = 1;
	}
  
	log("Reading banned site and invalid-name list.");
	load_banned();
	Read_Invalid_List();

	/* Moved here so the object limit code works. -gg 6/24/98 */
	if (!mini_mud) {
		log("Booting houses.");
		House_boot();
	}

	for (i = 0; i <= top_of_zone_table; i++)
	{
		log("Resetting %s (rooms %d-%d).", zone_table[i].name,
		(i ? (zone_table[i - 1].top + 1) : 0), zone_table[i].top);
		reset_zone(i);
	}

	reset_q.head = reset_q.tail = nullptr;
	boot_time = time(nullptr);
//	convert_from_binary();
	log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{

	#if defined(CIRCLE_MACINTOSH)
		long beginning_of_time = -1561789232;

	#else
		long beginning_of_time = 1074420331;
	
	#endif

	time_info = *mud_time_passed(time(0), beginning_of_time);

	if (time_info.hours <= 4)
		weather_info.sunlight = SUN_DARK;
	
	else if (time_info.hours == 5)
		weather_info.sunlight = SUN_RISE;
	
	else if (time_info.hours <= 20)
		weather_info.sunlight = SUN_LIGHT;
	
	else if (time_info.hours == 21)
		weather_info.sunlight = SUN_SET;
	
	else
		weather_info.sunlight = SUN_DARK;

	log("   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
	time_info.day, time_info.month, time_info.year);

	weather_info.pressure = 960;
  
	if ((time_info.month >= 7) && (time_info.month <= 12))
		weather_info.pressure += dice(1, 50);
  
	else
		weather_info.pressure += dice(1, 80);

	weather_info.change = 0;

	if (weather_info.pressure <= 980)
		weather_info.sky = SKY_LIGHTNING;
	
	else if (weather_info.pressure <= 1000)
		weather_info.sky = SKY_RAINING;
	
	else if (weather_info.pressure <= 1020)
		weather_info.sky = SKY_CLOUDY;
	
	else
		weather_info.sky = SKY_CLOUDLESS;
}

/* generate index table for the player file -Galnor */
void build_player_index()
{
	FILE *index;
	char line[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
	long id;
	player_index_element *entry;

	if(!(index = fopen(PVAR_INDEX, "r")))
	{
		log("Critical Error! Unable to open player index file in build_player_index()!");
		log("Index: %s", PVAR_INDEX);
		exit(1);
	}
	while(fgets(line, MAX_STRING_LENGTH, index))
	{
		if(*line == '~')
		{
			sscanf(line, "~ %ld", &top_id);
			break;
		}

		sscanf(line, "%s %ld", name, &id);
		
		//Don't load in more than one entry per name
		if(getEntryByName(name))
			continue;

		CREATE(entry, player_index_element, 1);
		entry->name = str_dup(name);
		entry->id = id;
		entry->next = player_table;
		player_table = entry;
	}

	fclose(index);
}

/*
 * Thanks to Andrey (andrey@alex-ua.com) for this bit of code, although I
 * did add the 'goto' and changed some "while()" into "do { } while()".
 *	-gg 6/24/98 (technically 6/25/98, but I care not.)
 */
int count_alias_records(FILE *fl)
{
	char key[READ_SIZE], next_key[READ_SIZE];
	char line[READ_SIZE], *scan;
	int total_keywords = 0;

	/* get the first keyword line */
	get_one_line(fl, key);

	while (*key != '$')
	{
		/* skip the text */
		
		do
		{
			get_one_line(fl, line);
      
			if (feof(fl))
				goto ackeof;
		} 
		
		while (*line != '#');

		/* now count keywords */
		scan = key;
    
		do
		{
			scan = one_word(scan, next_key);
			++total_keywords;
		} 
		
		while (*next_key);

		/* get next keyword line (or $) */
		get_one_line(fl, key);

		if (feof(fl))
			goto ackeof;
	}

	return total_keywords;

	/* No, they are not evil. -gg 6/24/98 */
	ackeof:	
	log("SYSERR: Unexpected end of help file.");
	exit(1);	/* Some day we hope to handle these things better... */
}

/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE * fl)
{
	char buf[128];
	int count = 0;

	while (fgets(buf, 128, fl))
		if (*buf == '#')
			count++;

	return count;
}

void index_boot(int mode)
{
	const char *index_filename, *prefix;
	FILE *index, *db_file;
	int rec_count = 0;

	switch (mode)
	{
	
	case DB_BOOT_KIT:
		prefix = KIT_PREFIX;
		break;
  
	case DB_BOOT_TRG:
		prefix = TRG_PREFIX;
		break;
  
	case DB_BOOT_WLD:
		prefix = WLD_PREFIX;
		break;
  
	case DB_BOOT_MOB:
		prefix = MOB_PREFIX;
		break;
  
	case DB_BOOT_OBJ:
		prefix = OBJ_PREFIX;
		break;
  
	case DB_BOOT_ZON:
		prefix = ZON_PREFIX;
		break;
  
	case DB_BOOT_SHP:
		prefix = SHP_PREFIX;
		break;
  
	case DB_BOOT_HLP:
		prefix = HLP_PREFIX;
		break;
  
	default:
		log("SYSERR: Unknown subcommand %d to index_boot!", mode);
		exit(1);
		break;
	}

	if (mini_mud)
		index_filename = MINDEX_FILE;
  
	else
		index_filename = INDEX_FILE;

	sprintf(buf2, "%s%s", prefix, index_filename);

	if (!(index = fopen(buf2, "r")))
	{
		sprintf(buf1, "SYSERR: opening index file '%s'", buf2);
		perror(buf1);
		exit(1);
	}

	/* first, count the number of records in the file so we can malloc */
	fscanf(index, "%s\n", buf1);
  
	while (*buf1 != '$') {
		sprintf(buf2, "%s%s", prefix, buf1);
    
		if (!(db_file = fopen(buf2, "r")))
		{
			perror(buf2);
			log("SYSERR: File '%s' listed in %s/%s not found.", buf2, prefix,
			index_filename);
			fscanf(index, "%s\n", buf1);
			continue;
		} 
		
		else {
			if (mode == DB_BOOT_ZON)
				rec_count++;
			
			else if (mode == DB_BOOT_HLP)
				rec_count += count_alias_records(db_file);
			
			else
				rec_count += count_hash_records(db_file);
		}

		fclose(db_file);
		fscanf(index, "%s\n", buf1);
	}

	/* Exit if 0 records, unless this is shops */
	/* Serai - Or kits. */
	if (!rec_count)
	{
		if (mode == DB_BOOT_SHP || mode == DB_BOOT_KIT)
			return;
		
		log("SYSERR: boot error - 0 records counted in %s/%s.", prefix,
		index_filename);
		exit(1);
	}

	rec_count++;

  /*
   * NOTE: "bytes" does _not_ include strings or other later malloc'd things.
   */
  
	int i;
	switch (mode)
	{
	case DB_BOOT_KIT:
		--rec_count;
		CREATE(kit_index, kit_data, rec_count);
		log("   %d kits, %d bytes.", rec_count, sizeof(kit_data) * rec_count);

		kit_index[0].prev;
		kit_index[0].next = &(kit_index[1]);

		for(i = 1; i < rec_count - 1; i++)
		{
			kit_index[i].next = &(kit_index[i + 1]);
			kit_index[i].prev = &(kit_index[i - 1]);
		}

		kit_index[rec_count - 1].prev = &(kit_index[rec_count - 2]);
		kit_index[rec_count - 1].next = nullptr;

		break;
  
	case DB_BOOT_TRG:
		CREATE(trig_index, struct index_data *, rec_count);
		break;
  
	case DB_BOOT_WLD:
		CREATE(world, struct room_data, rec_count);
		log("   %d rooms, %d bytes.", rec_count, sizeof(room_data) * rec_count);
		break;
  
	case DB_BOOT_MOB:
		CREATE(mob_proto, char_data, rec_count);
		CREATE(mob_index, struct index_data, rec_count);
		log("   %d mobs, %d bytes in index, %d bytes in prototypes.", rec_count, sizeof(index_data) * rec_count, sizeof(char_data) * rec_count);
		break;
  
	case DB_BOOT_OBJ:
		CREATE(obj_proto, struct obj_data, rec_count);
		CREATE(obj_index, struct index_data, rec_count);
		log("   %d objs, %d bytes in index, %d bytes in prototypes.", rec_count, sizeof(index_data) * rec_count, sizeof( obj_data) * rec_count);
		break;
  
	case DB_BOOT_ZON:
		CREATE(zone_table, struct zone_data, rec_count);
		log("   %d zones, %d bytes.", rec_count, sizeof(zone_data) * rec_count);
		break;
  
	case DB_BOOT_HLP:
		CREATE(help_table, struct help_index_element, rec_count);
		log("   %d entries, %d bytes.", rec_count, sizeof(help_index_element) * rec_count);
		break;
	}

	rewind(index);
	fscanf(index, "%s\n", buf1);
  
	while (*buf1 != '$') {
		sprintf(buf2, "%s%s", prefix, buf1);
    
		if (!(db_file = fopen(buf2, "r")))
		{
			perror(buf2);
			exit(1);
		}
    
		switch (mode) {
    
		case DB_BOOT_KIT:
		case DB_BOOT_TRG:
		case DB_BOOT_WLD:
		case DB_BOOT_OBJ:
		case DB_BOOT_MOB:
			discrete_load(db_file, mode, buf2);
			break;

    
		case DB_BOOT_ZON:
			load_zones(db_file, buf2);
			break;
    
		case DB_BOOT_HLP:
			/*
			* If you think about it, we have a race here.  Although, this is the
			* "point-the-gun-at-your-own-foot" type of race.
			*/
			load_help(db_file);
			break;
    
		case DB_BOOT_SHP:
			boot_the_shops(db_file, buf2, rec_count);
			break;
		}

		fclose(db_file);
		fscanf(index, "%s\n", buf1);
	}
	fclose(index);

}

void discrete_load(FILE * fl, int mode, char *filename)
{
	int nr = -1, last = 0;
	char line[256];
	  
	const char *modes[] = {"world", "mob", "obj", "ZON", "SHP", "HLP", "trg", "kit"};
	/* modes positions correspond to DB_BOOT_xxx in db.h */
 
	for (;;)
	{
		/*
		 * we have to do special processing with the obj files because they have
		* no end-of-record marker :(
		*/
    
		if (mode != DB_BOOT_OBJ || nr < 0)
			if (!get_line(fl, line))
			{
				
				if (nr == -1)
				{
					log("SYSERR: %s file %s is empty!", modes[mode], filename);
				}
				
				else {
					log("SYSERR: Format error in %s after %s #%d\n"
					"...expecting a new %s, but file ended!\n"
					"(maybe the file is not terminated with '$'?)", filename,
					modes[mode], nr, modes[mode]);	
				}
	
				exit(1);
			}
    
		if (*line == '$')
			return;

		if (*line == '#')
		{
			last = nr;
      
			if (sscanf(line, "#%d", &nr) != 1)
			{
				log("SYSERR: Format error after %s #%d", modes[mode], last);
				exit(1);
			}
      
			if (nr >= 99999)
				return;
      
			else
	
				switch (mode)
			{
				case DB_BOOT_KIT:
					parse_kit(fl, nr);
					break;
				
				case DB_BOOT_TRG:
					parse_trigger(fl, nr);
					break;
	
				case DB_BOOT_WLD:
					parse_room(fl, nr);
					break;
	
				case DB_BOOT_MOB:
					parse_mobile(fl, nr);
					break;
	
				case DB_BOOT_OBJ:
					strcpy(line, parse_object(fl, nr));
					break;
	
			}  
		} 
		
		else
		{
			log("SYSERR: Format error in %s file %s near %s #%d", modes[mode],
			filename, modes[mode], nr);
			log("...offending line: '%s'", line);
			exit(1);
		}
	}
}


long asciiflag_conv(char *flag)
{
	long flags = 0;
	int is_number = 1;
	char *p;

	for (p = flag; *p; p++)
	{
		if (islower(*p))
			flags |= 1 << (*p - 'a');
    
		else if (isupper(*p))
			flags |= 1 << (26 + (*p - 'A'));

		if (!isdigit(*p))
			is_number = 0;
	}

	if (is_number)
		flags = atol(flag);

	return flags;
}

char fread_letter(FILE *fp)
{
	char c;
  
	do
	{
		c = getc(fp);  
	} 	
	
	while (isspace(c));
	
	return c;
}

//* load the kits - Serai */
void parse_kit(FILE *fl, int virtual_nr)
{
	int eq[NUM_WEARS];
	static int index_nr = 0;

	kit_index[index_nr].vnum = virtual_nr;
	kit_index[index_nr].name = fread_string(fl, buf);

	for(int i = 0; i < NUM_OF_KITS; i++)
	{
		if ( !get_line(fl, buf2) )
		{
			log("SYSERR: Expecting list of equipment for kit #%d but file ended!", virtual_nr);
			exit(1);
		}

		if (sscanf(buf2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
			eq, eq + 1, eq + 2, eq + 3, eq + 4, eq + 5, eq + 6, eq + 7, eq + 8, eq + 9, eq + 10,
			eq + 11, eq + 12, eq + 13, eq + 14, eq + 15, eq + 16, eq + 17, eq + 18) != NUM_WEARS)
		{
			log("SYSERR: Incorrect number of equipment numbers reading from kit #%d.", virtual_nr);
			exit(1);
		}

		for(int j = 0; j < NUM_WEARS; j++)
			kit_index[index_nr].equipment[i][j] = eq[j];

		if ( !get_line(fl, buf2) )
		{
			log("SYSERR: Expecting list of percents for kit #%d but file ended!", virtual_nr);
			exit(1);
		}

		if (sscanf(buf2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
			eq, eq + 1, eq + 2, eq + 3, eq + 4, eq + 5, eq + 6, eq + 7, eq + 8, eq + 9, eq + 10,
			eq + 11, eq + 12, eq + 13, eq + 14, eq + 15, eq + 16, eq + 17, eq + 18) != NUM_WEARS)
		{
			log("SYSERR: Incorrect number of percent numbers reading from kit #%d.", virtual_nr);
			exit(1);
		}

		for(int k = 0; k < NUM_WEARS; k++)
			kit_index[index_nr].percent[i][k] = eq[k];
	}

	kit_end = &(kit_index[index_nr]);
	++index_nr;

	return;
}


/* load the rooms */
void parse_room(FILE * fl, int virtual_nr)
{
	static int room_nr = 0, zone = 0;
	int t[10], i;
	char line[256], flags[128], flags2[128], flags3[128], flags4[128];
	struct extra_descr_data *new_descr;
	char letter;

	sprintf(buf2, "room #%d", virtual_nr);

	if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1))
	{
		log("SYSERR: Room #%d is below zone %d.", virtual_nr, zone);
		exit(1);
	}
  
	while (virtual_nr > zone_table[zone].top)
		if (++zone > top_of_zone_table)
		{
			log("SYSERR: Room %d is outside of any zone.", virtual_nr);
			exit(1);
		}

		world[room_nr].zone = zone;
		world[room_nr].number = virtual_nr;
		world[room_nr].name = fread_string(fl, buf2);
		world[room_nr].description = fread_string(fl, buf2);

	if (!get_line(fl, line))
	{
		log("SYSERR: Expecting roomflags/sector type of room #%d but file ended!",
		virtual_nr);
		exit(1);
	}

	if (sscanf(line, " %d %s %s %s %s %d ", t, flags, flags2, flags3, flags4, t + 2) != 6)
	{
		log("SYSERR: Format error in roomflags/sector type of room #%d",
		virtual_nr);
		exit(1);
	}

	/* t[0] is the zone number; ignored with the zone-file system */
	world[room_nr].room_flags[0] = asciiflag_conv(flags);
	world[room_nr].room_flags[1] = asciiflag_conv(flags2);
	world[room_nr].room_flags[2] = asciiflag_conv(flags3);
	world[room_nr].room_flags[3] = asciiflag_conv(flags4);
	world[room_nr].sector_type = t[2];

	world[room_nr].func = nullptr;
	world[room_nr].contents = nullptr;
	world[room_nr].people = nullptr;
	world[room_nr].tracks = nullptr;
	world[room_nr].light = 0;	/* Zero light sources */
	world[room_nr].proto_script = nullptr;

	// Serai - this fixes the largest single count of errors in valgrind - 06/11/04
	// ==8144== 6059 errors in context 71 of 71:
	// ==8144== Conditional jump or move depends on uninitialised value(s)
	// ==8144==    at 0x80B9D85: reset_wtrigger(room_data*) (dg_triggers.cpp:904)
	// ==8144==    by 0x807E8AC: reset_zone(int) (db.cpp:2726)
	// ==8144==    by 0x807A5BE: boot_db() (db.cpp:436)
	// ==8144==    by 0x804ACBE: init_game(int) (comm.cpp:476)
	SCRIPT(&world[room_nr]) = nullptr;

	for (i = 0; i < NUM_OF_DIRS; i++)
		world[room_nr].dir_option[i] = nullptr;

	world[room_nr].ex_description = nullptr;

	sprintf(buf,"SYSERR: Format error in room #%d (expecting D/E/S)",virtual_nr);

	for (;;) {
		if (!get_line(fl, line))
		{
			log("%s", buf);
			exit(1);
		}
    
	switch (*line)
	{
		case 'D':
			setup_dir(fl, room_nr, atoi(line + 1));
			break;
    
		case 'E':
			CREATE(new_descr, struct extra_descr_data, 1);
			new_descr->keyword = fread_string(fl, buf2);
			new_descr->description = fread_string(fl, buf2);
			new_descr->next = world[room_nr].ex_description;
			world[room_nr].ex_description = new_descr;
			break;
    
		case 'S':			/* end of room */
			/* DG triggers -- script is defined after the end of the room */
			letter = fread_letter(fl);
			ungetc(letter, fl);
				
			while (letter=='T')
			{
					dg_read_trigger(fl, &world[room_nr], WLD_TRIGGER);
					letter = fread_letter(fl);
					ungetc(letter, fl);
			}
			top_of_world = room_nr++;
			return;
	 
		default:
			log(buf);
			exit(1);
		}
	}
}



/* read direction data */
void setup_dir(FILE * fl, int room, int dir)
{
	int t[5];
	char line[256];

	sprintf(buf2, "room #%d, direction D%d", GET_ROOM_VNUM(room), dir);

	CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
	world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
	world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

	if (!get_line(fl, line))
	{
		log("SYSERR: Format error, %s", buf2);
		exit(1);
	}
  
	if (sscanf(line, " %d %d %d %d", t, t + 1, t + 2, t + 3) != 4)
	{
		log("SYSERR: Format error, %s", buf2);
		exit(1);
	}

	if (t[0] == 1)
		world[room].dir_option[dir]->exit_info = EX_ISDOOR;  
  
	else if (t[0] == 2)
		world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;

	else
		world[room].dir_option[dir]->exit_info = 0;
		
	world[room].dir_option[dir]->key = t[1];
	world[room].dir_option[dir]->to_room = t[2];
	world[room].dir_option[dir]->hidden = t[3];

}


/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
	if ((r_human_start_room = real_room(human_start_room)) < 0)
	{
		log("SYSERR:  Human start room does not exist.  Change in config.c.");
		exit(1);
	}
  
	if ((r_trolloc_start_room = real_room(trolloc_start_room)) < 0)
	{
		log("SYSERR:  Trolloc start room does not exist.  Change in config.c.");
		exit(1);
	}

	if ((r_immort_start_room = real_room(immort_start_room)) < 0)
	{
		if (!mini_mud)
			log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
			r_immort_start_room = r_frozen_start_room;
	}

	if ((r_frozen_start_room = real_room(frozen_start_room)) < 0)
	{
		if (!mini_mud)
			log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
		
		r_frozen_start_room = r_immort_start_room;
	}
}


/* resolve all vnums into rnums in the world */
void renum_world()
{
	int room, door;

	for (room = 0; room <= top_of_world; room++)
		for (door = 0; door < NUM_OF_DIRS; door++)
			if (world[room].dir_option[door])
				if (world[room].dir_option[door]->to_room != NOWHERE)
					world[room].dir_option[door]->to_room =
	real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table()
{
	int zone, cmd_no, a, b, c, olda, oldb, oldc;
	char buf[128];

	for (zone = 0; zone <= top_of_zone_table; zone++)
		for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
		{
			a = b = c = 0;
			olda = ZCMD.arg1;
			oldb = ZCMD.arg2;
			oldc = ZCMD.arg3;
      
			switch (ZCMD.command)
			{
				case 'M':
					a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
					c = ZCMD.arg3 = real_room(ZCMD.arg3);
					break;
      
				case 'O':
					a = ZCMD.arg1 = real_object(ZCMD.arg1);
					if (ZCMD.arg3 != NOWHERE)
						c = ZCMD.arg3 = real_room(ZCMD.arg3);
					
					break;
	
				case 'G':
					a = ZCMD.arg1 = real_object(ZCMD.arg1);
					break;
      
				case 'E':
					a = ZCMD.arg1 = real_object(ZCMD.arg1);
					break;
      
				case 'P':
					a = ZCMD.arg1 = real_object(ZCMD.arg1);
					c = ZCMD.arg3 = real_object(ZCMD.arg3);
					break;
      
				case 'D':
					a = ZCMD.arg1 = real_room(ZCMD.arg1);
					break;
      
				case 'R': /* rem obj from room */
					a = ZCMD.arg1 = real_room(ZCMD.arg1);
					b = ZCMD.arg2 = real_object(ZCMD.arg2);
					break;
			}
      
			if (a < 0 || b < 0 || c < 0)
			{
				if (!mini_mud) {
					sprintf(buf,  "Invalid vnum %d, cmd disabled",
					(a < 0) ? olda : ((b < 0) ? oldb : oldc));
					log_zone_error(zone, cmd_no, buf);
				}
			ZCMD.command = '*';
			}
      
			else if (ZCMD.arg3 < 0)
				ZCMD.arg3 = 0;
		}
}



void parse_simple_mob(FILE *mob_f, int i, int nr)
{
	player_clan_data *cl;
	int j, t[15];
	char line[256];

	mob_proto[i].real_abils.str = 20;
	mob_proto[i].real_abils.intel = 20;
	mob_proto[i].real_abils.wis = 20;
	mob_proto[i].real_abils.dex = 20;
	mob_proto[i].real_abils.con = 20;
	mob_proto[i].real_abils.cha = 20;

	if (!get_line(mob_f, line))
	{
		log("SYSERR: Format error in mob #%d, file ended after S flag!", nr);
		exit(1);
	}

	if (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
	t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9)
	{
		log("SYSERR: Format error in mob #%d, first line after S flag\n"
		"...expecting line of form '# # # #d#+# #d#+#'", nr);
		exit(1);
	}

	GET_LEVEL(mob_proto + i) = t[0];

	/* max hit = 0 is a flag that H, M, V is xdy+z */
	mob_proto[i].points.max_hit = 0;
	mob_proto[i].points.hit = t[3];
	mob_proto[i].points.mana = t[4];
	mob_proto[i].points.move = t[5];

	mob_proto[i].points.max_mana = 10;
	mob_proto[i].points.max_move = 50;

	mob_proto[i].mob_specials.damnodice = t[6];
	mob_proto[i].mob_specials.damsizedice = t[7];
	mob_proto[i].points.damroll = t[8];

	if (!get_line(mob_f, line))
	{
		log("SYSERR: Format error in mob #%d, second line after S flag\n"
		"...expecting line of form '# #', but file ended!", nr);
		exit(1);
	}

	if (sscanf(line, " %d %d ", t, t + 1) != 2)
	{
		log("SYSERR: Format error in mob #%d, second line after S flag\n"
		"...expecting line of form '# #'", nr);
		exit(1);
	}

	GET_GOLD(mob_proto + i) = t[0];
	GET_EXP(mob_proto + i) = t[1];

	if (!get_line(mob_f, line)) {
		log("SYSERR: Format error in last line of mob #%d\n"
		"...expecting line of form '# # #', but file ended!", nr);
		exit(1);
	}

	if (sscanf(line, " %d %d %d %d %d %d %d %d %d %d %d %d",
	t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8, t + 9, t + 10, t + 11) != 12)
	{
		log("SYSERR: Format error in last line of mob #%d\n"
		"...expecting line of form '# # #'", nr);
		exit(1);
	}

	mob_proto[i].char_specials.position = t[0];
	mob_proto[i].mob_specials.default_pos = t[1];
	mob_proto[i].player.sex = t[2];
	GET_RACE(mob_proto + i) = t[3];
	GET_OB(mob_proto + i) = t[4];
	GET_DB(mob_proto + i) = t[5];
	GET_PB(mob_proto + i) = t[6];
	GET_MAX_MOVE(mob_proto + i) = t[7];

	/******* Add mob's clan ********/
	CREATE(cl, player_clan_data, 1);
	cl->clan = t[8];
	cl->rank = 9;
	cl->clan_time = 0;
	cl->quest_points = 0;
	cl->rank_time = 0;
	cl->next = (mob_proto + i)->clans;
	(mob_proto + i)->clans = cl;
	/*******************************/

	GET_CLASS(mob_proto + i) = t[10];
	mob_proto[i].mob_specials.primary_kit = real_kit(t[11]);
 
	mob_proto[i].player.weight = 200;
	mob_proto[i].player.height = 198;

  /*
   * These are player specials! -gg
   */

	if(!get_line(mob_f, line))
	{
		log(	"SYSERR: Format error reading mob #%d\n"
				"Expecting mob aggressive list.", nr);
		exit(1);
	}

	if(sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		&GET_AGGRO(mob_proto + i, 0), &GET_AGGRO(mob_proto + i, 1), &GET_AGGRO(mob_proto + i, 2), &GET_AGGRO(mob_proto + i, 3),
		&GET_AGGRO(mob_proto + i, 4), &GET_AGGRO(mob_proto + i, 5), &GET_AGGRO(mob_proto + i, 6), &GET_AGGRO(mob_proto + i, 7),
		&GET_AGGRO(mob_proto + i, 8), &GET_AGGRO(mob_proto + i, 9), &GET_AGGRO(mob_proto + i, 10), &GET_AGGRO(mob_proto + i, 11),
		&GET_AGGRO(mob_proto + i, 12), &GET_AGGRO(mob_proto + i, 13), &GET_AGGRO(mob_proto + i, 14), &GET_AGGRO(mob_proto + i, 15),
		&GET_AGGRO(mob_proto + i, 16), &GET_AGGRO(mob_proto + i, 17), &GET_AGGRO(mob_proto + i, 18), &GET_AGGRO(mob_proto + i, 19)) != 20)
	{
		log("Format error loading mob #%d. Loaded wrong number of aggro types", nr);
		exit(1);
	}

#if 0
	for (j = 0; j < 3; j++)
		GET_COND(mob_proto + i, j) = -1;
#endif

  /*
   * these are now save applies; base save numbers for MOBs are now from
   * the warrior save table.
   */
	for (j = 0; j < 5; j++)
		GET_SAVE(mob_proto + i, j) = 0;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void interpret_espec(const char *keyword, const char *value, int i, int nr)
{
	int num_arg, matched = 0;

	num_arg = atoi(value);

	CASE("BareHandAttack")
	{
		RANGE(0, 99);
		mob_proto[i].mob_specials.attack_type = num_arg;
	}

	CASE("Str")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.str = num_arg;
	}

	CASE("StrAdd")
	{
		RANGE(0, 100);
		mob_proto[i].real_abils.str_add = num_arg;    
	}

	CASE("Int")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.intel = num_arg;
	}

	CASE("Wis")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.wis = num_arg;
	}

	CASE("Dex")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.dex = num_arg;
	}

	CASE("Con")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.con = num_arg;
	}

	CASE("Cha")
	{
		RANGE(3, 25);
		mob_proto[i].real_abils.cha = num_arg;
	}

	if (!matched)
	{
		log("SYSERR: Warning: unrecognized espec keyword %s in mob #%d",
		keyword, nr);
	}    
}

#undef CASE
#undef RANGE

void parse_espec(char *buf, int i, int nr)
{
	char *ptr;

	if ((ptr = strchr(buf, ':')) != nullptr)
	{
		*(ptr++) = '\0';
		while (isspace(*ptr))
			ptr++;
#if 0	/* Need to evaluate interpret_espec()'s nullptr handling. */
	}
#else
	} 

	else
		ptr = "";
#endif
	interpret_espec(buf, ptr, i, nr);
}


void parse_enhanced_mob(FILE *mob_f, int i, int nr)
{
	char line[256];

	parse_simple_mob(mob_f, i, nr);

	while (get_line(mob_f, line))
	{
		if (!strcmp(line, "E") || !strcmp(line, "E\r"))	/* end of the ehanced section */
			return;
		
		else if (*line == '#')
		{	/* we've hit the next mob, maybe? */
			log("SYSERR: Unterminated E section in mob #%d", nr);
			exit(1);
		} 
		
		else
			parse_espec(line, i, nr);
	}

	log("SYESRR: Unexpected end of file reached after mob #%d", nr);
	exit(1);
}

/* Serai - added for the 'G' ZCMD in loading obj's to mobs */
int count_objects_inv(int number, char_data *target)
{
	struct obj_data *obj;
	int counter = 0;

	for(obj = target->carrying; obj; obj = obj->next_content)
	{
		if (GET_OBJ_RNUM(obj) == number)
			++counter; /* the prefix ++ is 'faster' than the suffix ++ :p */
	}

	return (counter);
}

/* Added by Galnor - Count objects inside of a room */
int count_objects_room(int number, int room)
{
	struct obj_data *obj;
	int counter = 0;

	for(obj = world[room].contents; obj;obj = obj->next_content)
	{
		if(GET_OBJ_RNUM(obj) == number) 
			counter++;
	}

	return counter;
}

/* Added by Galnor - Count objects inside of a zone */
int count_objects_zone(int number, int znum)
{
	int rrn, bottom, top, counter = 0;
	struct obj_data *obj;

	if (znum != NOWHERE)
	{
		bottom = zone_table[znum].bot;
		top    = zone_table[znum].top;
	}
	else
		return (0);
  
	for (obj = object_list; obj; obj = obj->next)
	{
		rrn = GET_ROOM_VNUM(obj->in_room);
		if (GET_OBJ_RNUM(obj) == number) 
			if(rrn >= bottom && rrn <= top)
				counter++;
	}
	return counter;
}

/* Added by Galnor - Count objects inside of the entire MUD. */
int count_mobs_mud(int mob_no)
{
	char_data *mob;
	int counter = 0;

	for(mob = character_list; mob;mob = mob->next)
	{
		if(GET_MOB_RNUM(mob) == mob_no) 
			counter++;
	}

	return counter;
}

/* Added by Galnor - Count mobs inside of a room */
int count_mobs_room(int mob_no, int room)
{
	char_data *mob;
	int counter = 0;

	for(mob = world[room].people; mob;mob = mob->next_in_room)
	{
		if(GET_MOB_RNUM(mob) == mob_no) 
			counter++;
	}

	return counter;
}

/* Added by Galnor - Count mobs inside of a zone */
int count_mobs_zone(int mob_no, int znum)
{
	int rrn, bottom, top, counter = 0;
	char_data *ch;

	if (znum != NOWHERE)
	{
		bottom = zone_table[znum].bot;
		top    = zone_table[znum].top;
	}
	else
		return (0);
  
	for (ch = character_list; ch; ch = ch->next)
	{
		rrn = GET_ROOM_VNUM(ch->in_room);
		if (GET_MOB_RNUM(ch) == mob_no) 
			if(rrn >= bottom && rrn <= top)
				counter++;
	}
	return counter;
}

void parse_mobile(FILE * mob_f, int nr)
{
	static int i = 0;
	int j, t[10];
	char line[256], *tmpptr, letter;
	char f1[128], f2[128], f3[128], f4[128], f5[128], f6[128], f7[128], f8[128];

	mob_index[i].vnum = nr;
	mob_index[i].number = 0;
	mob_index[i].func = nullptr;

	clear_char(mob_proto + i);

	/*
	 * Mobiles should NEVER use anything in the 'player_specials' structure.
	 * The only reason we have every mob in the game share this copy of the
	 * structure is to save newbie coders from themselves. -gg 2/25/98
	 */
	
	mob_proto[i].player_specials = &dummy_mob;
	sprintf(buf2, "mob vnum %d", nr);

	/***** String data *****/
	mob_proto[i].player.name = fread_string(mob_f, buf2);
	tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  
	if (tmpptr && *tmpptr)
		if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
		!str_cmp(fname(tmpptr), "the"))
			*tmpptr = LOWER(*tmpptr);
	
	mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
	mob_proto[i].player.description = fread_string(mob_f, buf2);
	mob_proto[i].player.title = nullptr;

	/* *** Numeric data *** */
	if (!get_line(mob_f, line))
	{
		log("SYSERR: Format error after string section of mob #%d\n"
		"...expecting line of form '# # # {S | E}', but file ended!", nr);
		exit(1);
	}

#ifdef CIRCLE_ACORN	/* Ugh. */
	if (sscanf(line, "%s %s %d %s", f1, f2, t + 2, &letter) != 4)
	{
#else
	if (sscanf(line, "%s %s %s %s %s %s %s %s %d %c", f1, f2, f3, f4, f5, f6, f7, f8, t + 2, &letter) != 10)
	{
#endif
		log("SYSERR: Format error after string section of mob #%d\n"
		"...expecting line of form '# # # {S | E}'", nr);
		exit(1);
	}
	
	MOB_FLAGS(mob_proto + i)[0] = asciiflag_conv(f1);
	MOB_FLAGS(mob_proto + i)[1] = asciiflag_conv(f2);
	MOB_FLAGS(mob_proto + i)[2] = asciiflag_conv(f3);
	MOB_FLAGS(mob_proto + i)[3] = asciiflag_conv(f4);
	SET_BIT_AR(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
	AFF_FLAGS(mob_proto + i)[0] = asciiflag_conv(f5);
	AFF_FLAGS(mob_proto + i)[1] = asciiflag_conv(f6);
	AFF_FLAGS(mob_proto + i)[2] = asciiflag_conv(f7);
	AFF_FLAGS(mob_proto + i)[3] = asciiflag_conv(f8);
	GET_ALIGNMENT(mob_proto + i) = t[2];

	switch (UPPER(letter))
	{
		case 'S':	/* Simple monsters */
			parse_simple_mob(mob_f, i, nr);
			break;
		
		case 'E':	/* Circle3 Enhanced monsters */
			parse_enhanced_mob(mob_f, i, nr);
			break;
			/* add new mob types here.. */
		default:
			log("SYSERR: Unsupported mob type '%c' in mob #%d", letter, nr);
			exit(1);
	}

	/* DG triggers -- script info follows mob S/E section */
	letter = fread_letter(mob_f);
	ungetc(letter, mob_f);
  
	while (letter=='T')
	{
		dg_read_trigger(mob_f, &mob_proto[i], MOB_TRIGGER);
		letter = fread_letter(mob_f);
		ungetc(letter, mob_f);
	}

	mob_proto[i].aff_abils = mob_proto[i].real_abils;

	for (j = 0; j < NUM_WEARS; j++)
		mob_proto[i].equipment[j] = nullptr;

	mob_proto[i].nr = i;
	mob_proto[i].desc = nullptr;

	top_of_mobt = i++;
}

/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, int nr)
{
	static int i = 0;
	static char line[256];
	int t[10], j, retval;
	char *tmpptr;
	char f1[256], f2[256], f3[256], f4[256];
	char f5[256], f6[256], f7[256], f8[256];
	struct extra_descr_data *new_descr;

	obj_index[i].vnum = nr;
	obj_index[i].number = 0;
	obj_index[i].func = nullptr;

	clear_object(obj_proto + i);
  
	obj_proto[i].in_room = NOWHERE;
	obj_proto[i].item_number = i;

	sprintf(buf2, "object #%d", nr);
 
	/* *** string data *** */
	if ((obj_proto[i].name = fread_string(obj_f, buf2)) == nullptr) {
		log("SYSERR: Null obj name or format error at or near %s", buf2);
		exit(1);
	}

	tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
 
	if (tmpptr && *tmpptr)
		if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
		!str_cmp(fname(tmpptr), "the"))
			*tmpptr = LOWER(*tmpptr);
 

	tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  
	if (tmpptr && *tmpptr)
		*tmpptr = UPPER(*tmpptr);
  
	obj_proto[i].action_description = fread_string(obj_f, buf2);
   

	 /* *** numeric data *** */
	if (!get_line(obj_f, line)) {
		log("SYSERR: Expecting first numeric line of %s, but file ended!", buf2);
		exit(1);
	}

 
	if ((retval = sscanf(line, " %d %s %s %s %s %s %s %s %s", t, f1, f2, f3, f4, f5, f6, f7, f8)) != 9) {
		log("SYSERR: Format error in first numeric line (expecting 9 args, got %d), %s", retval, buf2);
		exit(1);
	}

   
	obj_proto[i].obj_flags.type_flag = t[0];
	obj_proto[i].obj_flags.extra_flags[0] = asciiflag_conv(f1);
	obj_proto[i].obj_flags.extra_flags[1] = asciiflag_conv(f2);
	obj_proto[i].obj_flags.extra_flags[2] = asciiflag_conv(f3);
	obj_proto[i].obj_flags.extra_flags[3] = asciiflag_conv(f4);
	obj_proto[i].obj_flags.wear_flags[0] = asciiflag_conv(f5);
	 obj_proto[i].obj_flags.wear_flags[1] = asciiflag_conv(f6);
	obj_proto[i].obj_flags.wear_flags[2] = asciiflag_conv(f7);
	obj_proto[i].obj_flags.wear_flags[3] = asciiflag_conv(f8);
 
	if (!get_line(obj_f, line))
	{
		log("SYSERR: Expecting second numeric line of %s, but file ended!", buf2);
		exit(1);
	}
	
	if ((retval = sscanf(line, "%d %d %d %d", t, t + 1, t + 2, t + 3)) != 4)
	{
		log("SYSERR: Format error in second numeric line (expecting 4 args, got %d), %s", retval, buf2);
		exit(1);
	}
  
	obj_proto[i].obj_flags.value[0] = t[0];
	obj_proto[i].obj_flags.value[1] = t[1];
	obj_proto[i].obj_flags.value[2] = t[2];
	obj_proto[i].obj_flags.value[3] = t[3];
 
	if (!get_line(obj_f, line)) {
		log("SYSERR: Expecting third numeric line of %s, but file ended!", buf2);
		exit(1);
	}
  
	if ((retval = sscanf(line, "%d %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6)) != 7) {
		log("SYSERR: Format error in third numeric line (expecting 3 args, got %d), %s", retval, buf2);
		exit(1);
	}
   
	obj_proto[i].obj_flags.weight = t[0];
	obj_proto[i].obj_flags.cost = t[1];
	obj_proto[i].obj_flags.cost_per_day = t[2];
	obj_proto[i].obj_flags.offensive = t[3];
	obj_proto[i].obj_flags.parry = t[4];
	obj_proto[i].obj_flags.dodge =     t[5];
	obj_proto[i].obj_flags.absorb =  t[6];

	/* *** extra descriptions and affect fields *** */

	for (j = 0; j < MAX_OBJ_AFFECT; j++) {
		obj_proto[i].affected[j].location = APPLY_NONE;
		obj_proto[i].affected[j].modifier = 0;
	}

	strcat(buf2, ", after numeric constants\n"
	"...expecting 'E', 'A', '$', or next object number");
	j = 0;

	for (;;) {
		if (!get_line(obj_f, line)) {
			log("SYSERR: Format error in %s", buf2);
			exit(1);
		}
    
		switch (*line) {
			case 'E':
				CREATE(new_descr, struct extra_descr_data, 1);
				new_descr->keyword = fread_string(obj_f, buf2);
				new_descr->description = fread_string(obj_f, buf2);
				new_descr->next = obj_proto[i].ex_description;
				obj_proto[i].ex_description = new_descr;
				break;
    
			case 'A':
				if (j >= MAX_OBJ_AFFECT) {
					log("SYSERR: Too many A fields (%d max), %s", MAX_OBJ_AFFECT, buf2);
					exit(1);
				}
      
				if (!get_line(obj_f, line)) {
					log("SYSERR: Format error in 'A' field, %s\n"
					"...expecting 2 numeric constants but file ended!", buf2);
					exit(1);
				}

				if ((retval = sscanf(line, " %d %d ", t, t + 1)) != 2) {
					log("SYSERR: Format error in 'A' field, %s\n"
					"...expecting 2 numeric arguments, got %d\n"
					"...offending line: '%s'", buf2, retval, line);
					exit(1);
				}
				obj_proto[i].affected[j].location = t[0];
				obj_proto[i].affected[j].modifier = t[1];
				j++;
				break;
    
			case 'T':  /* DG triggers */
				dg_obj_trigger(line, &obj_proto[i]);
				break;
    
			case '$':
			case '#':
				top_of_objt = i++;
				return line;
    
			default:
				log("SYSERR: Format error in %s", buf2);
				exit(1);
		}
	}
}




#define Z	zone_table[zone]

/* load the zone table and command tables */
void load_zones(FILE * fl, char *zonename)
{
	static int zone = 0;
	int cmd_no = 0, num_of_cmds = 0, line_num = 0, tmp, error, arg_num;
	char *ptr, buf[256], zname[256];

	strcpy(zname, zonename);

	while (get_line(fl, buf))
		num_of_cmds++;		/* this should be correct within 3 or so */
	
	rewind(fl);

	if (num_of_cmds == 0)
	{
		log("SYSERR: %s is empty!", zname);
		exit(1);
	} 
	
	else
		CREATE(Z.cmd, struct reset_com, num_of_cmds);

	line_num += get_line(fl, buf);

	if (sscanf(buf, "#%d", &Z.number) != 1)
	{
		log("SYSERR: Format error in %s, line %d", zname, line_num);
		exit(1);
	}
  
	sprintf(buf2, "beginning of zone #%d", Z.number);
	line_num += get_line(fl, buf);
  
	if ((ptr = strchr(buf, '~')) != nullptr)	/* take off the '~' if it's there */
		*ptr = '\0';
  
	Z.name = str_dup(buf);
	line_num += get_line(fl, buf);
  
	if ((ptr = strchr(buf, '~')) != nullptr) /* take off the '~' if it's there */
		*ptr = '\0';
	
	Z.builders = str_dup(buf);
	line_num += get_line(fl, buf);
  
	if (sscanf(buf, " %d %d %d %d %d %d", &Z.bot, &Z.top, &Z.lifespan, &Z.reset_mode, &Z.x, &Z.y) != 6)
	{
		log("SYSERR: Format error in 3-constant line of %s", zname);
		exit(1);
	}
  
	cmd_no = 0;

	for (;;)
	{
		
		if ((tmp = get_line(fl, buf)) == 0)
		{
			log("Format error in %s - premature end of file", zname);
			exit(1);
		}
    
		line_num += tmp;
		ptr = buf;
		skip_spaces(&ptr);

		if ((ZCMD.command = *ptr) == '*')
			continue;

		ptr++;

		if (ZCMD.command == 'S' || ZCMD.command == '$')
		{
			ZCMD.command = 'S';
			break;
		}
    
		error = 0;
		
		if (strchr("D", ZCMD.command) != nullptr)
		{ /* ### */
			if (sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1, &ZCMD.arg2,
			&ZCMD.arg3) != 4)
				error = 1;
		}
    
		else if (strchr("R", ZCMD.command) != nullptr)
		{ /* ### */
			
			if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1,
			&ZCMD.arg2) != 3)
				error = 1;
		}
    
		else if (strchr("G", ZCMD.command) != nullptr)
		{ /* ### */
			if ((arg_num = sscanf(ptr, " %d %d %d %d", &tmp, &ZCMD.arg1,
			&ZCMD.arg2, &ZCMD.arg3)) != 4)
			{
			
				if (arg_num != 3)
					error = 1;
        
				else
					ZCMD.arg3 = 0;
			}
		}
    
		else
		{ /* ### */
		
			if ((arg_num = sscanf(ptr, " %d %d %d %d %d %d %d %d", &tmp, &ZCMD.arg1,
			&ZCMD.arg2, &ZCMD.arg3, &ZCMD.arg4, &ZCMD.arg5, &ZCMD.arg6, &ZCMD.arg7)) != 8)
			{
        
				if (arg_num != 4)
					error = 1;
				else
					ZCMD.arg4 = 0;
			}
		}

		ZCMD.if_flag = tmp;

		if (error)
		{
			log("SYSERR: Format error in %s, line %d: '%s'", zname, line_num, buf);
			exit(1);
		}
    
		ZCMD.mob = nullptr;
		ZCMD.line = line_num;
		cmd_no++;
	}

	top_of_zone_table = zone++;
}

#undef Z


void get_one_line(FILE *fl, char *buf)
{
	if (fgets(buf, READ_SIZE, fl) == nullptr)
	{
		log("SYSERR: error reading help file: not terminated with $?");
		exit(1);
	}

	buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}

void load_help(FILE *fl)
{
	char key[READ_SIZE+1], entry[32384];
	char line[READ_SIZE+1];
	struct help_index_element el;

	/* get the keyword line */
	get_one_line(fl, key);
	
	while (*key != '$')
	{
		get_one_line(fl, line);
		*entry = '\0';
    
		while (*line != '#')
		{
			strcat(entry, strcat(line, "\r\n"));
			get_one_line(fl, line);
		}

		el.min_level = 0;
    
		if ((*line == '#') && (*(line + 1) != 0))
			el.min_level = atoi((line + 1));

		el.min_level = MAX(0, MIN(el.min_level, LVL_IMPL));
    
		/* now, add the entry to the index with each keyword on the keyword line */
		el.entry = str_dup(entry);
		el.keywords = str_dup(key);

		help_table[top_of_helpt] = el;
		top_of_helpt++;

		/* get next keyword line (or $) */
		get_one_line(fl, key);
	}
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*************************************************************************/



int vnum_mobile(char *searchname, char_data * ch)
{
	int nr, found = 0;

	for (nr = 0; nr <= top_of_mobt; nr++)
	{
		if (isname(searchname, mob_proto[nr].player.name))
		{
			sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
			mob_index[nr].vnum,
			mob_proto[nr].player.short_descr);
			send_to_char(buf, ch);
		}
	}

	return (found);
}



int vnum_object(char *searchname, char_data * ch)
{
	int nr, found = 0;

	for (nr = 0; nr <= top_of_objt; nr++)
	{
		if (isname(searchname, obj_proto[nr].name))
		{
			sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
			obj_index[nr].vnum,
			obj_proto[nr].short_description);
		send_to_char(buf, ch);
		}
	}
	return (found);
}


/* create a character, and add it to the char list */
char_data *create_char(void)
{
	char_data *ch;

	CREATE(ch, char_data, 1);
	clear_char(ch);
	ch->next = character_list;
	character_list = ch;

	GET_ID(ch) = max_id++;
	return ch;
}


/* create a new mobile from a prototype */
char_data *read_mobile(int nr, int type)
{
	int i, p, ovnum = -1;
	char_data *mob;

	if (type == VIRTUAL)
	{
		if ((i = real_mobile(nr)) < 0)
		{
			sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
			log(buf);
			return nullptr;
		}
	} 
	
	else
		i = nr;

	CREATE(mob, char_data, 1);
	clear_char(mob);
	*mob = mob_proto[i];
	mob->next = character_list;
	character_list = mob;

	if (!mob->points.max_hit)
	{
		mob->points.max_hit = dice(mob->points.hit, mob->points.mana) +
		mob->points.move;
	} 
	
	else
		mob->points.max_hit = number(mob->points.hit, mob->points.mana);

	mob->points.hit = mob->points.max_hit;
	mob->points.mana = mob->points.max_mana;
	mob->points.move = mob->points.max_move;

	mob->player.time.birth = time(0);
	mob->player.time.played = 0;
	mob->player.time.logon = time(0);

	if(MOB_FLAGGED(mob, MOB_INVIS))
	{
		GET_INVIS_LEV(mob) = 100; 
		SET_BIT_AR(AFF_FLAGS(mob), AFF_INVISIBLE);
	}

	for(p = 0;p < MAX_SKILLS;p++)
		GET_SKILL(mob, p) = 100;

	/* Serai - go through kit and equip mobs on creation */
	if (mob_proto[i].mob_specials.primary_kit)
	{
		for(int tt = 0; tt < NUM_WEARS; tt++)
		{
			if (mob_proto[i].mob_specials.primary_kit->equipment[0][tt] != NOTHING)
			{
				for(int kit = 0; kit < NUM_OF_KITS; kit++)
				{
					if (mob_proto[i].mob_specials.primary_kit->equipment[kit][tt] == NOTHING ||
					    number(0, 100) > mob_proto[i].mob_specials.primary_kit->percent[kit][tt])
						continue;

					ovnum = real_object(mob_proto[i].mob_specials.primary_kit->equipment[kit][tt]);

					if (ovnum < 0)
						continue;

					equip_char(mob, read_object(ovnum, REAL), tt);
					break;
				}
			}
		}
	}

	mob_index[i].number++;
	GET_ID(mob) = max_id++;
	assign_triggers(mob, MOB_TRIGGER);

	return mob;
}



/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
	struct obj_data *obj;

	CREATE(obj, struct obj_data, 1);
	clear_object(obj);
	obj->next = object_list;
	object_list = obj;

	obj->pos = -1;

	GET_ID(obj) = max_id++;
	assign_triggers(obj, OBJ_TRIGGER);

	return obj;
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type)
{
	struct obj_data *obj;
	int i;

	if (nr < 0)
	{
		log("SYSERR: Trying to create obj with negative (%d) num!", nr);
		return nullptr;
	}
  
	if (type == VIRTUAL)
	{
		if ((i = real_object(nr)) < 0)
		{
			log("Object (V) %d does not exist in database.", nr);
			return nullptr;
		}
	} 
	
	else
		i = nr;

	CREATE(obj, struct obj_data, 1);
	clear_object(obj);
	*obj = obj_proto[i];
	obj->next = object_list;
	object_list = obj;
	obj->pos = -1;

	obj_index[i].number++;

	GET_ID(obj) = max_id++;
	assign_triggers(obj, OBJ_TRIGGER);

	return obj;
}

#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
	int i;
	struct reset_q_element *update_u, *temp;
	static int timer = 0;

	/* jelson 10/22/92 */
	
	if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
    
		/* one minute has passed */
		/*
		* NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
		* factor of 60
		*/

		timer = 0;

		/* since one minute has passed, increment zone ages */
		
		for (i = 0; i <= top_of_zone_table; i++) {
			if (zone_table[i].age < zone_table[i].lifespan &&
			zone_table[i].reset_mode)
				(zone_table[i].age)++;

			if (zone_table[i].age >= zone_table[i].lifespan &&
			zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
				
				/* enqueue zone */

				update_u = new struct reset_q_element;

				update_u->zone_to_reset = i;
				update_u->next = 0;

				if (!reset_q.head)
					reset_q.head = reset_q.tail = update_u;
	
				else {
					reset_q.tail->next = update_u;
					reset_q.tail = update_u;
				}

				zone_table[i].age = ZO_DEAD;
			}
		}
	}	/* end - one minute has passed */


	/* dequeue zones (if possible) and reset */
	/* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
	
	for (update_u = reset_q.head; update_u; update_u = update_u->next)
		if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
		::is_empty(update_u->zone_to_reset)) {
      
			reset_zone(update_u->zone_to_reset);
			/* dequeue */
			
			if (update_u == reset_q.head)
				reset_q.head = reset_q.head->next;
      
			else {
				for (temp = reset_q.head; temp->next != update_u; temp = temp->next);
					if (!update_u->next)
					reset_q.tail = temp;

					temp->next = update_u->next;
			}

			delete(update_u);
			break;
		}
}

void log_zone_error(int zone, int cmd_no, const char *message)
{
	char buf[256];

	sprintf(buf, "SYSERR: zone file: %s", message);
	 mudlog(buf, NRM, LVL_GOD, TRUE);

	sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	ZCMD.command, zone_table[zone].number, ZCMD.line);
	mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

void save_chest(struct obj_data *obj, FILE *fl)
{

	int result;

	if (obj)
	{
		save_chest(obj->contains, fl);
		save_chest(obj->next_content, fl);
		result = Obj_to_store(obj, fl);

		if(!result)
			return;
	}
}

void chest_save_setup(struct obj_data *chest, int room)
{

	FILE *fl;
	char filename[MAX_STRING_LENGTH];

	if(!chest || room < 0)
		return;

	sprintf(filename, LIB_CHESTS"%d.chest", GET_ROOM_VNUM(room));
  
	if (!(fl = fopen(filename, "wb")))
	{
		perror("SYSERR: Error saving chest file,");
		return;
	}
  
	save_chest(chest->contains, fl);
  
	fclose(fl);
}

void restore_chest(struct obj_data *chest, int room)
{

	FILE *fl;
	char filename[MAX_STRING_LENGTH];
	struct obj_file_elem object;
	struct obj_data *obj;

	if(room < 0 || !chest)
		return;

	sprintf(filename, LIB_CHESTS"%d.chest", GET_ROOM_VNUM(room));

	if (!(fl = fopen(filename, "r+b")))
		return;

	while(!feof(fl))
	{
		fread(&object, sizeof(struct obj_file_elem), 1, fl);
    
		if (ferror(fl))
		{
			perror("Reading Chest in restore_chest()");
			fclose(fl);
			return;
		}
    
		if (!feof(fl))
		{
			obj = Obj_from_store(object);
			obj_to_obj(obj, chest);
		}
	}

	chest->next_chest = chest_head;
	chest_head = chest;

	fclose(fl);
	return;
}

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
	int cmd_no, last_cmd = 0;
	char_data *mob = nullptr, *fol = nullptr;
	struct obj_data *obj = nullptr, *obj_to;
	int room_vnum, room_rnum;
	int mob_load = FALSE; /* ### */
	int obj_load = FALSE; /* ### */

	for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
	{

		if(obj)
			sprintf(obj->creator, "Loaded by normal reset in zone %d.", zone);

		if (ZCMD.if_flag && !last_cmd && !mob_load && !obj_load)
			continue;

		if (!ZCMD.if_flag)
		{ /* ### */
			mob_load = FALSE;
			obj_load = FALSE;
		}

		switch (ZCMD.command)
		{
    
		case '*':			/* ignore command */
			last_cmd = 0;
			break;

		case 'M':			/* read a mobile */

			if (ZCMD.arg2 >= number(1, 100) &&
				count_mobs_room(ZCMD.arg1, ZCMD.arg3) < ZCMD.arg4 &&
				count_mobs_zone(ZCMD.arg1, world[ZCMD.arg3].zone) < ZCMD.arg5 &&
				count_mobs_mud(ZCMD.arg1) < ZCMD.arg6)
			{
				// Serai - Sets the previously loaded mob to be the leader
				//  And when loading a mob with the defualt arg7 (-1) it kills fol.
				if (ZCMD.arg7 == 1 && fol == nullptr)
					fol = mob;
				else if (ZCMD.arg7 != 1)
					fol = nullptr;

				mob = read_mobile(ZCMD.arg1, REAL);
				char_to_room(mob, ZCMD.arg3);
				load_mtrigger(mob);
				mob->mob_specials.load = &(ZCMD);
				ZCMD.mob = mob;
				last_cmd = 1;
				mob_load = TRUE;

				// It's not possible to get a loop, no need to call circle_follow()
				if (ZCMD.arg7 == 1)
					add_follower(mob, fol);
			} 
			
			else
			{
				if (ZCMD.mob)
					mob = ZCMD.mob;
				else
					last_cmd = 0;
			}
      
			break;


		case 'O':			/* read an object */
			if (ZCMD.arg2 >= number(1, 100) && count_objects_room(ZCMD.arg1, ZCMD.arg3) < ZCMD.arg4 &&
				count_objects_zone(ZCMD.arg1, world[ZCMD.arg3].zone) < ZCMD.arg5 &&
				obj_index[ZCMD.arg1].number < ZCMD.arg6)
			{
				
				if (ZCMD.arg3 >= 0)
				{
					obj = read_object(ZCMD.arg1, REAL);
					obj_to_room(obj, ZCMD.arg3);
					
					if(IS_OBJ_STAT(obj, ITEM_CHEST))
						restore_chest(obj, ZCMD.arg3);
					
					load_otrigger(obj);
					last_cmd = 1;
					obj_load = TRUE;
				} 
				
				else
				{
					obj = read_object(ZCMD.arg1, REAL);
					obj->in_room = NOWHERE;
					last_cmd = 1;
					obj_load = TRUE;
				}
			} 
			
			else
				last_cmd = 0;
				
			break;

		case 'P':			/* object to object */
			
			if (ZCMD.arg2 >= number(1, 100))
			{
				obj = read_object(ZCMD.arg1, REAL);
	
				if (!(obj_to = get_obj_num(ZCMD.arg3)))
				{
					ZONE_ERROR("target obj not found");
					break;
				}
				
				obj_to_obj(obj, obj_to);
				load_otrigger(obj);
				last_cmd = 1;
			} 
			
			else
				last_cmd = 0;
			
			break;

		case 'G':			/* obj_to_char ### */
		
			if (!mob)
			{
				if (ZCMD.if_flag)
					ZONE_ERROR("attempt to give obj to non-existant mob");
/*
 *  FYI: don't set the if-flag on 'G' commands if you want the command to execute even
 *   if the mob wasn't loaded by a preceding 'M' ZCMD.  It can take it as long as the mob
 *   doesn't die, or if it is purged in any way it is reloaded before here.
 *     - Serai
 */
				break;
			}

			/* Serai - This added for more control on loading obj's to mobs */
			if (ZCMD.arg2 >= number(1, 100) && count_objects_inv(ZCMD.arg1, mob) < ZCMD.arg3)
			{
				obj = read_object(ZCMD.arg1, REAL);
				obj_to_char(obj, mob);
				load_otrigger(obj);
				last_cmd = 1;
			}
			else
				last_cmd = 0;

			break;



		case 'E':			/* object to equipment list ### */
		
			if (!mob)
			{
				ZONE_ERROR("trying to equip non-existant mob");
				break;
			}

			if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS)
			{
				ZONE_ERROR("invalid equipment pos number");
			} 
			
			else
			{
				if(number(1, 100) <= ZCMD.arg2)
				{ 
					obj = read_object(ZCMD.arg1, REAL);
					
					if(!GET_EQ(mob, ZCMD.arg3))
					{
						equip_char(mob, obj, ZCMD.arg3);
						load_otrigger(obj);
						last_cmd = 1;
					}
				}
			}
			break;

		case 'R': /* rem obj from room */
      
			if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != nullptr)
			{
				obj_from_room(obj);
				extract_obj(obj);
			}
      
			last_cmd = 1;
			break;


		case 'D':			/* set state of door */
      
			if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
			(world[ZCMD.arg1].dir_option[ZCMD.arg2] == nullptr))
			{
				ZONE_ERROR("door does not exist");
			} 
			
			else
			{
	
				switch (ZCMD.arg3)
				{
	
					case 0:
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_LOCKED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_CLOSED);
						break;
	
					case 1:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_CLOSED);
						REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_LOCKED);
						break;
	
					case 2:
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_LOCKED);
						SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
						EX_CLOSED);
						break;
				}
			}
      
			last_cmd = 1;
			break;

		default:
			ZONE_ERROR("unknown cmd in reset table; cmd disabled");
			ZCMD.command = '*';
			break;
		}
	}

	zone_table[zone].age = 0;

	/* handle reset_wtrigger's */
	room_vnum = zone_table[zone].number * 100;
  
	while (room_vnum <= zone_table[zone].top)
	{
		room_rnum = real_room(room_vnum);
		
		if (room_rnum != NOWHERE) reset_wtrigger(&world[room_rnum]);
		
		room_vnum++;
	}
}


/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
	struct descriptor_data *i;

	for (i = descriptor_list; i; i = i->next)
		if (STATE(i) == CON_PLAYING)
			if (world[i->character->in_room].zone == zone_nr)
				return 0;

	return 1;
}

/*************************************************************************
*  stuff related to the save/load player system				 *
*************************************************************************/

player_index_element *get_entry(char_data *ch)
{
	player_index_element *entry;

	for(entry = player_table;entry;entry = entry->next)
	{
		if(!str_cmp(entry->name, GET_NAME(ch)))
			return entry;
	}

	return nullptr;
}

player_index_element *getEntryByName(char *name)
{
	player_index_element *entry;

	if(!name)
		return nullptr;

	for(entry = player_table;entry;entry = entry->next)
	{
		if(!str_cmp(entry->name, name))
			return entry;
	}

	return nullptr;
}

long get_id_by_name(char *name)
{
	player_index_element *entry;

	one_argument(name, arg);
	
	for(entry = player_table;entry;entry = entry->next)
	{
		if (!strcmp(entry->name, arg))
		{
			return (entry->id);
		}
	}

	return -1;
}

char *get_name_by_id(long id)
{
	player_index_element *entry;

	for(entry = player_table;entry;entry = entry->next)
	{
		if (entry->id == id)
		{
			return (entry->name);
		}
	}

	return nullptr;
}

/* Separate a 4-character id tag from the data it precedes */
void tag_argument(char *argument, char *tag)
{
	char *tmp = argument, *ttag = tag, *wrt = argument;

	for(;*tmp != ':';)
		*(ttag++) = *(tmp++);

	*ttag = '\0';
  
	while(*tmp == ':' || *tmp == ' ')
		tmp++;

	while(*tmp && *tmp != '\r' && *tmp != '\n')
		*(wrt++) = *(tmp++);

	*wrt = '\0';
}

int char_data::load(char *name)
{
	char filename[MAX_INPUT_LENGTH], line[MAX_STRING_LENGTH], tag[MAX_STRING_LENGTH], f1[128], f2[128], f3[128], f4[128], f5[128];
	long num = 0, v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
	int qp = 0, rank = 0;
	bool old_clan = false;
	FILE *file;
	player_clan_data *cl;

	if(!name)
		return 0;

	get_filename(name, filename, PVAR_FILE);

	if(!(file = fopen(filename, "r")))
	{
		return 0;
	}

	if(!this->player_specials)
		CREATE(this->player_specials, player_special_data, 1);

	while(fgets(line, MAX_STRING_LENGTH, file))
	{
		tag_argument(line, tag);
		num = atoi(line);
	// Serai 11/08/04 - fread_string() for description .. caused problems.
		if(!str_cmp(tag, "Name"))
			GET_NAME(this) = str_dup(line);
		else if(!str_cmp(tag, "Password"))
			strcpy(GET_PASSWD(this), line);
		else if(!str_cmp(tag, "Description"))
			this->player.description = fread_string(file, buf);
		else if(!str_cmp(tag, "Title"))
			GET_TITLE(this) = str_dup(line);
		else if(!str_cmp(tag, "Pin"))
			strcpy(POOFIN(this), line);
		else if(!str_cmp(tag, "Pout"))
			strcpy(POOFOUT(this), line);
		else if(!str_cmp(tag, "Level"))
			GET_LEVEL(this) = num;
		else if(!str_cmp(tag, "Birth"))
			this->player.time.birth = num;
		else if(!str_cmp(tag, "Class"))
			GET_CLASS(this) = num;
		else if(!str_cmp(tag, "Race"))
			GET_RACE(this) = num;
		else if(!str_cmp(tag, "Home"))
			GET_HOME(this) = num;
		else if(!str_cmp(tag, "Height"))
			GET_HEIGHT(this) = num;
		else if(!str_cmp(tag, "Weight"))
			GET_WEIGHT(this) = num;
		else if(!str_cmp(tag, "Sex"))
			GET_SEX(this) = num;
		else if(!str_cmp(tag, "Str"))
		{
			this->real_abils.str = num;
			GET_STR(this) = num;
		}
		else if(!str_cmp(tag, "Dex"))
		{
			this->real_abils.dex = num;
			GET_DEX(this) = num;
		}
		else if(!str_cmp(tag, "Wis"))
		{
			this->real_abils.wis = num;
			GET_WIS(this) = num;
		}
		else if(!str_cmp(tag, "Int"))
		{
			this->real_abils.intel = num;
			GET_INT(this) = num;
		}
		else if(!str_cmp(tag, "Con"))
		{
			this->real_abils.con = num;
			GET_CON(this) = num;
		}
		else if(!str_cmp(tag, "Exp"))
			GET_EXP(this) = num;
		else if(!str_cmp(tag, "ID Num"))
			GET_IDNUM(this) = num;
		else if(!str_cmp(tag, "Hit"))
			GET_HIT(this) = num;
		else if(!str_cmp(tag, "Max Hit"))
			GET_MAX_HIT(this) = num;
		else if(!str_cmp(tag, "Move"))
			GET_MOVE(this) = num;
		else if(!str_cmp(tag, "Max Move"))
			GET_MAX_MOVE(this) = num;
		else if(!str_cmp(tag, "Mana"))
			GET_MANA(this) = num;
		else if(!str_cmp(tag, "Max Mana"))
			GET_MAX_MANA(this) = num;
		else if(!str_cmp(tag, "Shadow"))
			GET_SHADOW(this) = num;
		else if(!str_cmp(tag, "Max Shadow"))
			GET_MAX_SHADOW(this) = num;
		else if(!str_cmp(tag, "Invis"))
			GET_INVIS_LEV(this) = num;
		else if(!str_cmp(tag, "Weave Points"))
			GET_WP(this) = num;
		else if(!str_cmp(tag, "Taveren"))
			GET_TAVEREN(this) = num;
		else if(!str_cmp(tag, "Gold"))
			GET_GOLD(this) = num;
		else if(!str_cmp(tag, "Bank"))
			GET_BANK_GOLD(this) = num;
		else if(!str_cmp(tag, "Death Wait"))
			GET_DEATH_WAIT(this) = num;
		else if(!str_cmp(tag, "Warnings"))
			GET_WARNINGS(this) = num;
		else if(!str_cmp(tag, "Last Login"))
			LAST_LOGON(this) = num;
		else if(!str_cmp(tag, "Master Weapon"))
			GET_MASTER_WEAPON(this) = num;
		else if(!str_cmp(tag, "Whois Extra"))
			strcpy(GET_WHOIS_EXTRA(this), line);
		else if(!str_cmp(tag, "Bond"))
			strcpy(GET_BOND(this), line);
		else if(!str_cmp(tag, "Slew"))
			strcpy(GET_SLEW_MESSAGE(this), line);
		else if(!str_cmp(tag, "Dark Points"))
			GET_DP(this) = num;
		else if(!str_cmp(tag, "Sickness"))
			GET_SICKNESS(this) = num;
		else if(!str_cmp(tag, "Strain"))
			GET_STRAIN(this) = num;
		else if(!str_cmp(tag, "Loadroom"))
			GET_LOADROOM(this) = num;
		else if(!str_cmp(tag, "Practices"))
			GET_PRACTICES(this) = num;
		else if(!str_cmp(tag, "Spell Practices"))
			GET_SPRACTICES(this) = num;
		else if(!str_cmp(tag, "Freeze"))
			GET_FREEZE_LEV(this) = num;
		else if(!str_cmp(tag, "Bad Passwords"))
			GET_BAD_PWS(this) = num;
		else if(!str_cmp(tag, "Quest Points"))
			qp = num;
		else if(!str_cmp(tag, "Rank"))
			rank = num;
		else if(!str_cmp(tag, "Host"))
			strcpy(this->points.host, line);
		else if(!str_cmp(tag, "Hunger"))
			GET_COND(this, FULL) = num;
		else if(!str_cmp(tag, "Thirst"))
			GET_COND(this, THIRST) = num;
		else if(!str_cmp(tag, "Drunk"))
			GET_COND(this, DRUNK) = num;
		/* "Clan", "Rank" and "Quest Points" need to remain so that we can convert to the new system */
		else if(!str_cmp(tag, "Clan") && num > 0)
		{
			old_clan = true;
			CREATE(cl, player_clan_data, 1);
			cl->clan = num;
			cl->clan_time = time(0);
			cl->rank_time = time(0);
			cl->next = this->clans;
			this->clans = cl;
		}
		else if(!str_cmp(tag, "Clan Data"))
		{
			player_clan_data *cl;
			CREATE(cl, player_clan_data, 1);

			sscanf(line, "%d %d %d %d %d", &cl->clan, &cl->rank, &cl->quest_points, &cl->clan_time, &cl->rank_time);
			cl->next = this->clans;
			this->clans = cl;
		}
		else if(!str_cmp(tag, "Skill"))
		{
			sscanf(line, "%d %d", &v1, &v2);
			GET_SKILL(this, v1) = v2;
		}
		else if(!str_cmp(tag, "Plr"))
		{
			sscanf(line, "%s %s %s %s", &f1, &f2, &f3, &f4);
			PLR_FLAGS(this)[0] = asciiflag_conv(f1);
			PLR_FLAGS(this)[1] = asciiflag_conv(f2);
			PLR_FLAGS(this)[2] = asciiflag_conv(f3);
			PLR_FLAGS(this)[3] = asciiflag_conv(f4);
		}
		else if(!str_cmp(tag, "Prf"))
		{
			int ret = sscanf(line, "%s %s %s %s %s", &f1, &f2, &f3, &f4, &f5);
			PRF_FLAGS(this)[0] = asciiflag_conv(f1);
			PRF_FLAGS(this)[1] = asciiflag_conv(f2);
			PRF_FLAGS(this)[2] = asciiflag_conv(f3);
			PRF_FLAGS(this)[3] = asciiflag_conv(f4);
			if(ret > 4)
				PRF_FLAGS(this)[4] = asciiflag_conv(f5);
		}
		else if(!str_cmp(tag, "Aff"))
		{
			sscanf(line, "%s %s %s %s", &f1, &f2, &f3, &f4);
			AFF_FLAGS(this)[0] = asciiflag_conv(f1);
			AFF_FLAGS(this)[1] = asciiflag_conv(f2);
			AFF_FLAGS(this)[2] = asciiflag_conv(f3);
			AFF_FLAGS(this)[3] = asciiflag_conv(f4);
		}
		else if(!str_cmp(tag, "Warrant"))
		{
			sscanf(line, "%s %s %s %s", &f1, &f2, &f3, &f4);
			GET_WARRANTS(this)[0] = asciiflag_conv(f1);
			GET_WARRANTS(this)[1] = asciiflag_conv(f2);
			GET_WARRANTS(this)[2] = asciiflag_conv(f3);
			GET_WARRANTS(this)[3] = asciiflag_conv(f4);
			
		}
	}

	/**************************************************************************
	Be sure that this->clans is even allocated, and if old_clan is set to true,
	then we are basically converting the old format to the new
	***************************************************************************/
	if(this->clans && old_clan)
	{
		this->clans->quest_points = qp;
		this->clans->rank = rank;
	}

	fclose(file);

	return 1;
}

int char_data::save()
{
	char filename[MAX_STRING_LENGTH];
	FILE *sfile;
	player_clan_data *cl;
	int i = 0;

	if(IS_NPC(this))
		return 0;

	get_filename(GET_NAME(this), filename, PVAR_FILE);

	if(!(sfile = fopen(filename, "w+")))
	{
		sprintf(buf, "Error opening %s's playerfile for save!", GET_NAME(this));
		mudlog(buf, NRM, LVL_BLDER, TRUE);
		return 0;
	}
	if(this->in_room != NOWHERE)
		GET_LOADROOM(this) = GET_ROOM_VNUM(this->in_room);

	if((this->char_specials.timer) > idle_void)
		GET_LOADROOM(this) = NOWHERE;

	fprintf(sfile, "Name: %s\n", GET_NAME(this));
	fprintf(sfile, "Password: %s\n", GET_PASSWD(this));

	if(this->player.description)
		fprintf(sfile, "Description: \n%s~\n", this->player.description);
	if(GET_TITLE(this))
		fprintf(sfile, "Title: %s\n", GET_TITLE(this));	
	if(POOFIN(this))
		fprintf(sfile, "Pin: %s\n", POOFIN(this));
	if(POOFOUT(this))
		fprintf(sfile, "Pout: %s\n", POOFOUT(this));

	fprintf(sfile, "Level: %d\n", GET_LEVEL(this));
	fprintf(sfile, "Birth: %d\n", this->player.time.birth);
	fprintf(sfile, "Class: %d\n", GET_CLASS(this));
	fprintf(sfile, "Race: %d\n", GET_RACE(this));
	fprintf(sfile, "Home: %d\n", GET_HOME(this));
	fprintf(sfile, "Height: %d\n", GET_HEIGHT(this));
	fprintf(sfile, "Weight: %d\n", GET_WEIGHT(this));
	fprintf(sfile, "Sex: %d\n", GET_SEX(this));
	fprintf(sfile, "Str: %d\n", this->real_abils.str);
	fprintf(sfile, "Dex: %d\n", this->real_abils.dex);
	fprintf(sfile, "Int: %d\n", this->real_abils.intel);
	fprintf(sfile, "Wis: %d\n", this->real_abils.wis);
	fprintf(sfile, "Con: %d\n", this->real_abils.con);
	fprintf(sfile, "Exp: %d\n", GET_EXP(this));
	fprintf(sfile, "Hit: %d\n", GET_HIT(this));
	fprintf(sfile, "Max Hit: %d\n", GET_MAX_HIT(this));
	fprintf(sfile, "Move: %d\n", GET_MOVE(this));
	fprintf(sfile, "Max Move: %d\n", GET_MAX_MOVE(this));
	fprintf(sfile, "Mana: %d\n", GET_MANA(this));
	fprintf(sfile, "Max Mana: %d\n", GET_MAX_MANA(this));
	fprintf(sfile, "Shadow: %d\n", GET_SHADOW(this));
	fprintf(sfile, "Max Shadow: %d\n", GET_MAX_SHADOW(this));
	fprintf(sfile, "Invis: %d\n", GET_INVIS_LEV(this));
	fprintf(sfile, "Weave Points: %d\n", GET_WP(this));
	fprintf(sfile, "Taveren: %d\n", GET_TAVEREN(this));
	fprintf(sfile, "Legend: %d\n", GET_LEGEND(this));
	fprintf(sfile, "Gold: %d\n", GET_GOLD(this));
	fprintf(sfile, "Bank: %d\n", GET_BANK_GOLD(this));
	fprintf(sfile, "Death Wait: %d\n", GET_DEATH_WAIT(this));
	fprintf(sfile, "Warnings: %d\n", GET_WARNINGS(this));
	fprintf(sfile, "Last Login: %d\n", LAST_LOGON(this));
	fprintf(sfile, "Master Weapon: %d\n", GET_MASTER_WEAPON(this));
	fprintf(sfile, "ID: %d\n", GET_ID(this));
	fprintf(sfile, "ID Num: %d\n", GET_IDNUM(this));

	if(GET_WHOIS_EXTRA(this))
		fprintf(sfile, "Whois Extra: %s\n", GET_WHOIS_EXTRA(this));
	if(GET_BOND(this))
		fprintf(sfile, "Bond: %s\n", GET_BOND(this));
	if(GET_SLEW_MESSAGE(this))
		fprintf(sfile, "Slew: %s\n", GET_SLEW_MESSAGE(this));

	fprintf(sfile, "Dark Points: %d\n", GET_DP(this));
	fprintf(sfile, "Sickness: %d\n", GET_SICKNESS(this));
	fprintf(sfile, "Strain: %d\n", GET_STRAIN(this));
	fprintf(sfile, "Loadroom: %d\n", GET_LOADROOM(this));
	fprintf(sfile, "Practices: %d\n", GET_PRACTICES(this));
	fprintf(sfile, "Spell Practices: %d\n", GET_SPRACTICES(this));
	fprintf(sfile, "Freeze: %d\n", GET_FREEZE_LEV(this));
	fprintf(sfile, "Bad Passwords: %d\n", GET_BAD_PWS(this));
	
	//Save player's clan data
	for(cl = this->clans;cl;cl = cl->next)
	{
		fprintf(sfile, "Clan Data: %d %d %d %d %d\n", cl->clan, cl->rank, cl->quest_points, cl->clan_time, cl->rank_time);
	}
	
	if(this->desc && this->desc->host)
		fprintf(sfile, "Host: %s\n", this->desc->host);
	else
		fprintf(sfile, "Host: Un-retrievable\n");
	fprintf(sfile, "Hunger: %d\n", GET_COND(this, FULL));
	fprintf(sfile, "Thirst: %d\n", GET_COND(this, THIRST));
	fprintf(sfile, "Drunk: %d\n", GET_COND(this, DRUNK));

	for(i = 0;i <= MAX_SKILLS;i++)
	{
		if(GET_SKILL(this, i))
			fprintf(sfile, "Skill: %d %d\n", i, GET_SKILL(this, i));
	}

	fprintf(sfile, "Prf: %d %d %d %d %d\n",
		PRF_FLAGS(this)[0], PRF_FLAGS(this)[1], PRF_FLAGS(this)[2], PRF_FLAGS(this)[3], PRF_FLAGS(this)[4]);

	fprintf(sfile, "Plr: %d %d %d %d\n",
		PLR_FLAGS(this)[0], PLR_FLAGS(this)[1], PLR_FLAGS(this)[2], PLR_FLAGS(this)[3]);

	fprintf(sfile, "Aff: %d %d %d %d\n",
		AFF_FLAGS(this)[0], AFF_FLAGS(this)[1], AFF_FLAGS(this)[2], AFF_FLAGS(this)[3]);

	fprintf(sfile, "Warrant: %d %d %d %d\n",
		GET_WARRANTS(this)[0], GET_WARRANTS(this)[1], GET_WARRANTS(this)[2], GET_WARRANTS(this)[3]);


	fclose(sfile);
	return 1;

}

int playerExists(char *name)
{

	FILE *file;
	char filename[MAX_INPUT_LENGTH];

	get_filename(name, filename, PVAR_FILE);

	if(!name)
		return 0;

	if(!(file = fopen(filename, "r")))
		return 0;

	else
	{
		fclose(file);
		return 1;
	}
}

/* Load a char, TRUE if loaded, FALSE if not */
/*
int load_char(char *name, struct char_file_u * char_element, FILE *file)
{
	int player_i;

	if(file == player_fl)
		player_i = find_name(name, player_table, top_of_p_table);

	else
		player_i = find_name(name, player_table2, top_of_p_table2);

	if (player_i >= 0)
	{
		fseek(file, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
		fread(char_element, sizeof(struct char_file_u), 1, file);
		return (player_i);
	} 
	
	else
		return (-1);
}
*/


/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, char_data * ch)
{
//	 to save memory, only PC's -- not MOB's -- have player_specials 
	if (ch->player_specials == nullptr)
		CREATE(ch->player_specials, struct player_special_data, 1);

	time_t ll =  time(0) - st->last_logon;

	GET_SEX(ch)   = st->sex;
	GET_CLASS(ch) = st->chclass;
	GET_RACE(ch)  = st->race;
	GET_LEVEL(ch) = st->level;

	ch->player.short_descr = nullptr;
	ch->player.long_descr  = nullptr;
	ch->player.title       = str_dup(st->title);

	ch->player.description = str_dup(st->description);

	ch->player.hometown    = st->hometown;
	ch->player.time.birth  = st->birth;
	ch->player.time.played = st->played;
	ch->player.time.logon  = time(0);

	ch->player.weight = st->weight;
	ch->player.height = st->height;

	ch->real_abils = st->abilities;
	ch->aff_abils = st->abilities;
	ch->points = st->points;
	strcpy(ch->points.host, st->host);
	ch->char_specials.saved = st->char_specials_saved;
	ch->player_specials->saved = st->player_specials_saved;
	GET_LAST_TELL(ch) = NOBODY;

	if (ch->points.max_mana < 100)
		ch->points.max_mana = 100;

	ch->char_specials.carry_weight = 0;
	ch->char_specials.carry_items = 0;
	ch->points.damroll = 0;

	ch->player.name = new char[MAX_NAME_LENGTH];

	strcpy(ch->player.name, st->name);
	strcpy(ch->player.passwd, st->pwd);

  
   // If you're not poisioned and you've been away for more than an hour of
   // real time, we'll set your HMV back to full


	if (!AFF_FLAGGED(ch, AFF_POISON) &&
	(((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR))
	{
		GET_HIT(ch) = GET_MAX_HIT(ch);
		GET_MOVE(ch) = GET_MAX_MOVE(ch);
		GET_MANA(ch) = GET_MAX_MANA(ch);
	}

	GET_DEATH_WAIT(ch) -= ll / 60;

	if(ch->desc)
		strcpy(ch->points.host, ch->desc->host);
//		strcpy(ch->desc->host, ch->points.host);

}				/* store_to_char */

void savePlayerIndex()
{
	player_index_element *entry;
	FILE *index;

	if(!(index = fopen(PVAR_INDEX, "w+")))
	{
		mudlog("Critical Error! Unable to open player index file for printing: savePlayerIndex()!", NRM, LVL_BLDER, TRUE);
		return;
	}
	for(entry = player_table;entry;entry = entry->next)
	{
		fprintf(index, "%s %ld\n", entry->name, entry->id);
	}

	fprintf(index, "~ %ld\n", top_id);
	fclose(index);
}

/* Galnor: 11-2-2004
 * This function will remove an entry from the player_table list. It can be done
 * by either the player name or the player id number. The list saves to the index
 * file after everything is completed.
 */
void remove_entry(char *name, long id, int save)
{
	player_index_element *entry, *temp;

	for(entry = player_table;entry;entry = entry->next)
	{
		if(id && entry->id == id || name && !str_cmp(name, entry->name))
		{
			REMOVE_FROM_LIST(entry, player_table, next);

			if(entry->name)
				delete entry->name;

			delete [] entry;
			break;
		}
	}

	if(save)
		savePlayerIndex();
}

/* create a new entry in the in-memory index table for the player file */
player_index_element *create_entry(char_data *ch)
{
	player_index_element *end, *entry;

	if(!ch)
	{
		log("Error in create_entry(): ch is null.");
		return nullptr;
	}

	// Look for the last entry //
	if(!player_table)
		end = nullptr;

	else
		for(end = player_table;end->next;end = end->next);

	//Galnor: This loop will make sure that no player is in the index twice.
	while((entry = getEntryByName(GET_NAME(ch))))
	{
		//We save the index at the end of this function.
		remove_entry(GET_NAME(ch), 0, FALSE);
	}

	CREATE(entry, player_index_element, 1);
	entry->name = str_dup(GET_NAME(ch));

	// Serai 11/08/04 - Can't forget this ;)
	entry->id = top_id++;

	// Add this guy to the end of the list //
	if(!end)
	{
		entry->next = nullptr;
		player_table = entry;
	}
	else
	{
		end->next = entry;
		entry->next = nullptr;
	}

	savePlayerIndex();

	return entry;
}

/************************************************************************
*  funcs of a (more or less) general utility nature			*
************************************************************************/

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
	char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
	char *point;
	int done = 0, length = 0, templength = 0;

	*buf = '\0';

	do {
		if (!fgets(tmp, 512, fl))
		{
			log("SYSERR: fread_string: format error at or near %s", error);
			exit(1);
		}
    
		/* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
		if ((point = strchr(tmp, '~')) != nullptr)
		{
			*point = '\0';
			done = 1;
		} 
		
		else
		{
			point = tmp + strlen(tmp) - 1;
			*(point++) = '\r';
			*(point++) = '\n';
			*point = '\0';
		}

		templength = strlen(tmp);

		if (length + templength >= MAX_STRING_LENGTH)
		{
			log("SYSERR: fread_string: string too large (db.c)");
			log("%s", error);
			exit(1);
		} 
		
		else
		{
			strcat(buf + length, tmp);
			length += templength;
		}
	} 
	
	while (!done);

	/* allocate space for the new string and copy it */
	if (strlen(buf) > 0)
	{
		CREATE(rslt, char, length + 1);
		strcpy(rslt, buf);
	} 
		
	else
		rslt = nullptr;

	return rslt;
}


/* release memory allocated for a char struct */
void free_char(char_data * ch)
{
	int i;
	struct alias *a;
	struct ignore_data *ig;

	if (ch->player_specials != nullptr && ch->player_specials != &dummy_mob)
	{
		while ((a = GET_ALIASES(ch)) != nullptr)
		{
			GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
			delete[] (a);
		}
    

		delete[] (ch->player_specials);
    
		if (IS_NPC(ch))
			log("SYSERR: Mob %s (#%d) had player_specials allocated!", GET_NAME(ch), GET_MOB_VNUM(ch));
	}

	ig = ch->ignores;
	while(ig)
	{
		ch->ignores = ch->ignores->next;
		delete[] (ig);
		ig = ch->ignores;
	}
  
	if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1))
	{
		/* if this is a player, or a non-prototyped non-player, free all */
    
		if (GET_NAME(ch))
			delete[] (GET_NAME(ch));
    
		if (ch->player.title)
			delete[] (ch->player.title);
    
		if (ch->player.short_descr)
			delete[] (ch->player.short_descr);
    
		if (ch->player.long_descr)
			delete[] (ch->player.long_descr);
    
		if (ch->player.description)
			delete[] (ch->player.description);
	} 
	
	else if ((i = GET_MOB_RNUM(ch)) > -1)
	{
    /* otherwise, free strings only if the string is not pointing at proto */
    
		if (ch->player.name && ch->player.name != mob_proto[i].player.name)
			delete[] (ch->player.name);
    
		if (ch->player.title && ch->player.title != mob_proto[i].player.title)
			delete[] (ch->player.title);
    
		if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
			delete[] (ch->player.short_descr);
    
		if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
			delete[] (ch->player.long_descr);
    
		if (ch->player.description && ch->player.description != mob_proto[i].player.description)
			delete[] (ch->player.description);
	}
	
  while (ch->affected)
	affect_remove(ch, ch->affected);

	if (ch->desc)
		ch->desc->character = nullptr;

	delete[] (ch);
	ch = nullptr;
}



/* release memory allocated for an obj struct */
void free_obj(struct obj_data * obj)
{
	int nr;
	struct extra_descr_data *thisd, *next_one;

	if ((nr = GET_OBJ_RNUM(obj)) == -1)
	{
    
		if (obj->name)
			delete[] (obj->name);
    
		if (obj->description)
			delete[] (obj->description);
    
		if (obj->short_description)
			delete[] (obj->short_description);
    
		if (obj->action_description)
			delete[] (obj->action_description);
    
		if (obj->ex_description)
			for (thisd = obj->ex_description; thisd; thisd = next_one)
			{
	
				next_one = thisd->next;
	
				if (thisd->keyword)
					delete[] (thisd->keyword);
	
				if (thisd->description)
					delete[] (thisd->description);
	
				delete[] (thisd);
			}
	} 
	
	else
	{
    
		if (obj->name && obj->name != obj_proto[nr].name)
			delete[] (obj->name);
    
		if (obj->description && obj->description != obj_proto[nr].description)
			delete[] (obj->description);
    
		if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
			delete[] (obj->short_description);
    
		if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
			delete[] (obj->action_description);
    
		if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
      
			for (thisd = obj->ex_description; thisd; thisd = next_one)
			{
	
				next_one = thisd->next;
	
				if (thisd->keyword)
					delete[] (thisd->keyword);
	
				if (thisd->description)
					delete[] (thisd->description);
	
				delete[] (thisd);
			}
	}

	delete[] (obj);
	obj = nullptr;
}



/* read contets of a text file, alloc space, point buf to it */
int file_to_string_alloc(const char *name, char **buf)
{
	char temp[MAX_STRING_LENGTH];

	if (*buf)
	{
		delete[] (*buf);
		*buf = nullptr;
	}

	if (file_to_string(name, temp) < 0)
	{
		*buf = nullptr;
		return -1;
	} 
	
	else
	{
		*buf = str_dup(temp);
		return 0;
	}
}


/* read contents of a text file, and place in buf */
int file_to_string(const char *name, char *buf)
{
	FILE *fl;
	char tmp[READ_SIZE+3];

	*buf = '\0';

	if (!(fl = fopen(name, "r")))
	{
		sprintf(tmp, "SYSERR: reading %s", name);
		perror(tmp);
		return (-1);
	}
  
	do
	{
		fgets(tmp, READ_SIZE, fl);
		tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
		strcat(tmp, "\r\n");

		if (!feof(fl))
		{
			if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH)
			{
				log("SYSERR: %s: string too big (%d max)", name,
				MAX_STRING_LENGTH);
				*buf = '\0';
				return -1;
			}
      
			strcat(buf, tmp);
		}
	} 
	
	while (!feof(fl));
	fclose(fl);
	return (0);
}



/* clear some of the the working variables of a char */
void reset_char(char_data * ch)
{
	int i;

	for (i = 0; i < NUM_WEARS; i++)
		GET_EQ(ch, i) = nullptr;

	ch->followers = nullptr;
	ch->master = nullptr;
	ch->in_room = NOWHERE;
	ch->carrying = nullptr;
	ch->next = nullptr;
	ch->next_fighting = nullptr;
	ch->next_in_room = nullptr;
	FIGHTING(ch) = nullptr;
	GET_ALIASES(ch) = nullptr;
	ch->ignores = nullptr;
	ch->weaves = nullptr;
	ch->affection_list = nullptr;

	if(ch->desc)
	{
		ch->desc->command_ready = 0;
		ch->desc->delayed_state = 0;
	}

	GET_LAST_POS(ch) = POS_STANDING;
	ch->char_specials.position = POS_STANDING;
	ch->mob_specials.default_pos = POS_STANDING;
	ch->char_specials.carry_weight = 0;
	ch->char_specials.carry_items = 0;

	if (GET_HIT(ch) <= 0)
		GET_HIT(ch) = 1;
	
	if (GET_MOVE(ch) <= 0)
		GET_MOVE(ch) = 1;
	
	if (GET_MANA(ch) <= 0)
		GET_MANA(ch) = 1;


	GET_LAST_TELL(ch) = NOBODY;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void clear_char(char_data * ch)
{
	memset((char *) ch, 0, sizeof(char_data));

	ch->in_room = NOWHERE;
	GET_PFILEPOS(ch) = -1;
	GET_MOB_RNUM(ch) = NOBODY;
	GET_WAS_IN(ch) = NOWHERE;
	GET_LAST_POS(ch) = POS_STANDING;
	change_pos(ch, POS_STANDING);
	ch->clans = nullptr;
	ch->mob_specials.default_pos = POS_STANDING;

	if(ch->desc)
	{
		ch->desc->command_ready = 0;
		ch->desc->delayed_state = 0;
	}

	if (ch->points.max_mana < 100)
		ch->points.max_mana = 100;
}


void clear_object(struct obj_data * obj)
{
	memset((char *) obj, 0, sizeof(struct obj_data));

	obj->item_number = NOTHING;
	obj->in_room = NOWHERE;
	obj->worn_on = NOWHERE;
}




/* initialize a new character only if class is set */
void char_data::init()
{
	int i, taeller;
	player_index_element *entry;

	GET_TITLE(this) = new char[MAX_INPUT_LENGTH];

	/* create a player_special structure */
	if (this->player_specials == nullptr)
		CREATE(this->player_specials, struct player_special_data, 1);
	
	/* *** if this is our first player --- he be God *** */

	/*	If either player_table or player_table->next is nullptr, then we
	 *	know that the only entry is our implementor.
	 */
	if (!player_table || !player_table->next)
	{
		GET_EXP(this) = 7000000;
		GET_LEVEL(this) = LVL_IMPL;

		this->points.max_hit = 500;
		this->points.max_mana = 100;
		this->points.max_move = 82;
	}

	set_title(this, "");

	this->player.short_descr = nullptr;
	this->player.long_descr = nullptr;
	this->player.description = nullptr;

	this->player.hometown = 1;

	this->player.time.birth = time(0);
	this->player.time.played = 0;
	this->player.time.logon = time(0);

	for (i = 0; i < MAX_TONGUE; i++)
		GET_TALK(this, i) = 0;

	/* make favors for sex */
	if (this->player.sex == SEX_MALE)
	{
		this->player.weight = number(120, 180);
		this->player.height = number(160, 200);
	} 
	
	else
	{
		this->player.weight = number(100, 160);
		this->player.height = number(150, 180);
	}

	// Serai 11/08/04 - Need to increment here, too..
	GET_IDNUM(this) = top_id++;

	if(!(entry = get_entry(this)))
	{
		sprintf(buf, "Error: get_entry returning nullptr in char_data::init() for player %s.", GET_NAME(this));
		mudlog(buf, NRM, LVL_BLDER, TRUE);
	}

	else
		entry->id = GET_IDNUM(this);

	this->points.max_mana = 100;
	this->points.mana = GET_MAX_MANA(this);
	this->points.hit = GET_MAX_HIT(this);
	this->points.max_move = 82;
	this->points.move = GET_MAX_MOVE(this);
	FLEE_LAG(this) = 0;
	FLEE_GO(this) = FALSE;

	for (i = 1; i <= MAX_SKILLS; i++)
	{
		if (GET_LEVEL(this) < LVL_IMPL)
			SET_SKILL(this, i, 0);
    
		else
			SET_SKILL(this, i, 100);
	}

	//this->char_specials.saved.affected_by = 0;
	for(taeller=0; taeller < AF_ARRAY_MAX; taeller++)
		this->char_specials.saved.affected_by[taeller] = 0;  


	for (i = 0; i < 5; i++)
		GET_SAVE(this, i) = 0;

	this->real_abils.intel = 25;
	this->real_abils.wis = 25;
	this->real_abils.dex = 25;
	this->real_abils.str = 25;
	this->real_abils.str_add = 100;
	this->real_abils.con = 25;
	this->real_abils.cha = 25;

	this->clans = nullptr;
	GET_MAX_SHADOW(this) = 1;
	GET_WARNINGS(this) = 0;

	for (i = 0; i < 3; i++)
		GET_COND(this, i) = (GET_LEVEL(this) == LVL_IMPL ? -1 : 24);

	SCRIPT(this) = nullptr;
	GET_LOADROOM(this) = NOWHERE;
}

/* returns the real number of the room with given virtual number */
int real_room(int vnum)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_world;

	/* perform binary search on world-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((world + mid)->number == vnum)
			return mid;
		if (bot >= top)
			return NOWHERE;
		if ((world + mid)->number > vnum)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int vnum)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_mobt;

	/* perform binary search on mob-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((mob_index + mid)->vnum == vnum)
			return (mid);
		if (bot >= top)
			return (-1);
		if ((mob_index + mid)->vnum > vnum)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

/* returns the real number of the object with given virtual number */
int real_object(int vnum)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_objt;
	
	/* perform binary search on obj-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((obj_index + mid)->vnum == vnum)
			return (mid);
		if (bot >= top)
			return (-1);
		if ((obj_index + mid)->vnum > vnum)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

void boot_battle(void)
{
	char buff[30];

	FILE *file;
	if(!(file = fopen(BATTLE_FILE, "r")))
	{
		mudlog("BATTLE FILE CANNOT BE OPENED IN BOOT_DB()", CMP, LVL_APPR, FALSE);
		return;
	}
	fgets(buff, 30, file);
	
	if((sscanf(buff, "%d %d", &tg.humanwp, &tg.trollwp)) != 2)
		exit(1);
}

void add_to_clan_list(char_data *ch, int p)
{
	struct clan_player_data *pl;
	char filename[MAX_STRING_LENGTH], str[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
	int i = 0;
	FILE *file;

	if(!ch || p <= 0)
		return;

	CREATE(pl, clan_player_data, 1);
	strcpy(pl->name, GET_NAME(ch));
	pl->next = clan_list[p].players;
	clan_list[p].players = pl;

	/* We do this here to make sure we get rid of the random values. */
	for(i = 0;i < MAX_CLAN_QUESTS;i++)
		pl->quests[i] = 0;

	sprintf(filename, "%s%d", LIB_QUESTS, p);
	
	if(!(file = fopen(filename, "r+")))
	{
		sprintf(buf, "Error opening clan quest file for clan #%d for booting.", p);
		mudlog(buf, BRF, LVL_APPR, TRUE);
		return;
	}

	while(fgets(str, MAX_STRING_LENGTH, file))
	{
		if(!strn_cmp(str, GET_NAME(ch), strlen(GET_NAME(ch))))
		{
			if( (i = sscanf(str, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", name, &pl->quests[0],
			&pl->quests[1], &pl->quests[2], &pl->quests[3], &pl->quests[4], &pl->quests[5], &pl->quests[6],
			&pl->quests[7], &pl->quests[8], &pl->quests[9], &pl->quests[10], &pl->quests[11], &pl->quests[12],
			&pl->quests[13], &pl->quests[14], &pl->quests[15], &pl->quests[16], &pl->quests[17], &pl->quests[18],
			&pl->quests[19])) != 21)
			{
				log("Error parsing Clan Quest Values for clan %d, player %s. Returned %d.", p, GET_NAME(ch), i);
				exit(1);
			}

			

			break;
		}
	}

	fclose(file);
	save_clan_quests(p);

}

void assign_clan_players(void)
{
	char_data *ch;
	player_index_element *entry;
	int i = 0;

	CREATE(ch, char_data, 1);
	clear_char(ch); 
	for(entry = player_table;entry;entry = entry->next)
	{
		for(i = 0;i < NUM_CLANS;++i)
		{
			if(ch->getClan(i))
				add_to_clan_list(ch, i);
		}
	}
	if(ch)
		free_char(ch);
}

/* This function is responsible for saving clan quest values to a text formatted file
 * and removing or adding and entries based on who is currently in the clan. Galnor: 5-9-2004
 */
void save_clan_quests(int clan_number)
{
	char filename[MAX_INPUT_LENGTH];
	struct clan_player_data *p;
	int i = 0;
	FILE *file;

	sprintf(filename, "%s%d", LIB_QUESTS, clan_number);


	if(!(file = fopen(filename, "w+")))
	{
		sprintf(buf, "Unable to open file for clan %s. Aborting...", clan_list[clan_number].name);
		mudlog(buf, BRF, LVL_APPR, TRUE);
		return;
	}

	for(p = clan_list[clan_number].players;p;p = p->next)
	{
		fprintf(file, "%s", p->name);

		for(i = 0;i < MAX_CLAN_QUESTS;i++)
			fprintf(file, " %d", p->quests[i]);

		fprintf(file, "\n");
	}

	fclose(file);
}

void boot_clans()
{
	int i = 0, length = 0, p = 0, num = 0, loop;
	char buff[128];
	FILE *file;
	
	if( !(file = fopen(CLAN_FILE, "r")))
	{
		log("ERROR READING CLAN FILE IN BOOT_CLANS()");
		return;
	}

	for(loop = 0;fgets(buff, 200, file) != nullptr && i <= NUM_CLANS;loop++)
	{
		length = strlen(buff);

		if(i > NUM_CLANS)
		{
			i = 0;
			num = 0;
		}

		for(p = 0;p <= length;p++)
		{
			if(buff[p] == '\r' || buff[p] == '\n')
				buff[p] = '\0';
		}

		if(buff[0] == '*')
			sscanf(buff, "*%d", &clan_list[i].secret);

		else
		{
			i++;
			strcpy(clan_list[i].name, buff);
			num = 0;
		}


	}

	assign_clan_players();
	
	for(i = 1;i < NUM_CLANS;i++)
		save_clan_quests(i);
	
	fclose(file);
}

void boot_legends()
 {
	int i = 0, loop = 0;
	char buff[200];
	FILE *file;

	buff[0] = '\0';

	if( !(file = fopen(LEGEND_FILE, "r")))
	{
		log("ERROR OPENING LEGEND FILE IN BOOT_LEGENDS()! ABORTING!");
		return;
	}

	for(i = 0;i <= 12;i++)
	{
		for(loop = 0;loop < 8;loop++)
		{
			legend[loop].person[0] = '\0';
			legend[loop].ammount = 0;
		}
	}

	for(i = 1;fgets(buff, 200, file) != nullptr && i <= 12;i++)
	{
		if(sscanf(buff, "~%s %d\n", (legend[i].person), &(legend[i].ammount)) != 2)
		{
			sprintf(buf, "ERROR LOADING LEGEND NUMBER %d... Skipping\r\n", i);
			log(buf);
			continue;
		}
	}

	fclose(file);
}

void eatwhite(ifstream &fin)
{

	while(isspace(fin.peek()))
		fin.get();
}

int char_timeout(char_data *ch, int days)
{
	if(GET_LEVEL(ch) <= 5 && days >= 3)
		return true;

	else if(GET_LEVEL(ch) <= 10 && days >= 21)
		return true;

	else if(GET_LEVEL(ch) <= 20 && days >= 42)
		return true;

	return false;

}

void playerAutoDelete()
{
	char_data *ch;
	char filename[MAX_INPUT_LENGTH];
	player_index_element *entry, *next;

	CREATE(ch, char_data, 1);
	clear_char(ch);

	for(entry = player_table;entry;entry = next)
	{

		next = entry->next;

		if(!entry->name)
			continue;

		get_filename(entry->name, filename, PVAR_FILE);

		if(!ch->load(entry->name) || !playerExists(entry->name))
		{
			remove(filename);
			remove_entry(entry->name, 0, FALSE);
			continue;
		}

		if(PLR_FLAGGED(ch, PLR_DELETED) || !str_cmp(GET_NAME(ch), "") || !*GET_NAME(ch))
		{
			remove(filename);
			remove_entry(entry->name, 0, FALSE);
			continue;
		}

		if(char_timeout(ch, ((time(0) - LAST_LOGON(ch)) / SECS_PER_REAL_DAY)))
		{
			remove(filename);
			remove_entry(entry->name, 0, FALSE);
			continue;
		}
	}

	free_char(ch);
	savePlayerIndex();
}

void load_ideas(void) 
{

	ifstream nin(IDEA_FILE);
	string room_string;

	while(!nin.eof())
	{
		Ideas *idea;
//		CREATE(idea, class Ideas, 1);
		idea = new Ideas;
		eatwhite(nin);

		if(!isalnum(nin.peek())) 
		{
			cerr << "Posters name must be alphanumberic." << endl;
			return;
		}

		while(nin.peek() != ':')
			idea->poster += nin.get();

		eatwhite(nin);


		if(nin.get() == ':')
			nin.get();

		else
		{
			cerr << "Error load_ideas(), expected ':', found '" << nin.peek() << "'" << endl;
			return;
		}

		eatwhite(nin);

		if(nin.peek() == '(')
			nin.get();

		else
		{
			cerr << "Error in load_ideas(), expected '(', found '" << nin.peek() << "'" << endl;
			return;
		}

		while(nin.peek() != ')')
			idea->date += nin.get();

			
		if(nin.peek() == ')')
			nin.get();

		else
		{
			cerr << "Error in load_ideas(), expected ')', found '" << nin.peek() << "'" << endl;
			return;
		}

		eatwhite(nin);

		if(nin.peek() == '[')
			nin.get();

		else
		{
			cerr << "Error in load_ideas(), expected '[', found '" << nin.peek() << "'" << endl;
			return;
		}

		eatwhite(nin);

		while(nin.peek() != ']')
			room_string += nin.get();

		idea->room = atoi(room_string.c_str());

		if(nin.peek() == ']')
			nin.get();

		else
		{
			cerr << "Error in load_ideas(), expected ']', found '" << nin.peek() << "'" << endl;
			return;
		}

		eatwhite(nin);

		while(nin.peek() != '~')
			idea->idea += nin.get();

		if(nin.peek() == '~')
			nin.get();

		else
		{
			cerr << "Error in load_ideas(), expected '~', found '" << nin.peek() << "'" << endl;
			return;
		}

		eatwhite(nin);

		idea->next = idea_list;
		idea_list = idea;
		room_string = '\0';
	}
}

//By Galnor on December 24th, 2003 (11:30 if you want to be technical) //
void load_wizlist(void)
{

	char_data *ch;
	player_index_element *entry;
	class wizlist_data *wiz;

	for(entry = player_table;entry;entry = entry->next)
	{
		CREATE(ch, char_data, 1);
		clear_char(ch);

		if(ch->load(entry->name))
		{
			
			if(GET_LEVEL(ch) >= LVL_IMMORT)
			{
				wiz = new wizlist_data;
				wiz->name = GET_NAME(ch);
				wiz->level = GET_LEVEL(ch);
				wiz->next = wizlist;
				wizlist = wiz;
			}
		}
		if(ch)
			delete[] (ch);
	}
}

void load_notes(void)
 {
	ifstream fin(NOTE_FILE);
	string temp;
	int count = 0;

	while(!fin.eof())
	{

		NoteData note;
		eatwhite(fin);
		
		if(!isalnum(fin.peek()))
		{
			cerr << "Posters name must be alphanumeric." << endl;
			return;
		}
		
		while(fin.peek() != ':')
			note.poster += fin.get();
		
		eatwhite(fin);
		
		if(fin.peek() == ':')
			fin.get();
		
		else
		{
			cerr << "Expected ':' found '" << fin.peek() << "'" << endl;
			return;
		}
		
		eatwhite(fin);
		if(fin.peek() == '~')
			fin.get();
		else
		{
			cerr << "Expected '~' found '" << fin.peek() << "'" << endl;
			return;
		}
		
		while(fin.peek() != '~')
			note.message += fin.get();
		
		if(fin.peek() == '~')
			fin.get();
		
		else
		{
			cerr << "Expected '~' found '" << fin.peek() << "'" << endl;
			return;
		}
		
		eatwhite(fin);

		while(isalnum(fin.peek()))
			temp += fin.get();

		note.npc = atoi(temp.c_str());
		temp = ' ';
		
		eatwhite(fin);


		if(!note.npc)
			notes.insert(notes.end(), note);

		else
			mob_notes.insert(mob_notes.end(), note);
		
		count++;
	}

	sprintf(buf, "%d notes have been booted.", count);
	log(buf);
	return;
}

void save_notes(void)
 {
	list<NoteData>::iterator cur;
	fstream f;

	f.open(NOTE_FILE, ios::out);

	for(cur = notes.begin(); cur != notes.end(); cur++) 
		f << cur->poster << ": ~" << cur->message << "~" << " " << cur->npc << endl;

	for(cur = mob_notes.begin();cur != mob_notes.end();cur++)
		f << cur->poster << ": ~" << cur->message << "~" << " " << cur->npc << endl;

	f.close();

}

void save_ideas(void)
{
	class Ideas *idea;

	fstream  fin;

	fin.open(IDEA_FILE, ios::out);

	for(idea = idea_list;idea;idea = idea->next)
		fin << idea->poster << ": (" << idea->date <<") [" << idea->room <<"] "<< idea->idea << "~" << endl;

	fin.close();
}

void load_values(void) 
{
	ifstream vin(VALUE_FILE);
//	int current = 0;
//	int count = 0;
	string line;
//	char *other;

/*	while(!vin.eof()) 
	{
		
		if( (vin.peek()) == '~') 
		{
			count++;
			eatwhite(vin);
		}

		else
			line += vin.get();

		if(count != current) 
		{

			if(count == 1)
			{
				other = (char *) line;
				if( (sscanf(other, "%s~", GLOBAL_RESET) != 1))
					GLOBAL_RESET = 0;
			}
		}
	}
	*/
}

void convert_from_binary()
{
	char_data *ch;
	player_index_element *entry, *next;
	char_file_u st;
	FILE *pfile;
	int i = 1;

	log("Begging transfer...");
	CREATE(ch, char_data, 1);

	if (!(pfile = fopen(PLAYER_FILE, "r+b")))
	{
		log("Error opening playerfile.");
		return;
	}

	for(entry = player_table;entry;entry = next)
	{
		next = entry->next;
		remove_entry(entry->name, 0, TRUE);
	}

	for (; !feof(pfile);)
	{
		fread(&st, sizeof(struct char_file_u), 1, pfile);

		if (!feof(pfile))
		{	/* new record */

			store_to_char(&st, ch);
			create_entry(ch);
			ch->save();
		}
	}

	free_char(ch);
	log("Done converting... Counting up entries.");

	for(i = 0, entry = player_table;entry;entry = entry->next)
		++i;

	log("%d entries... Exiting.", i);
	exit(1);
}
