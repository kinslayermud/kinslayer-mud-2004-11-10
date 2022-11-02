/* ************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __COMM_C__

#include "screen.h"

#include "conf.h"
#include "sysdep.h"

#ifdef CIRCLE_MACINTOSH /* Includes for the Macintosh */
# define SIGPIPE 13
# define SIGALRM 14
 /* GUSI headers */
# include <sys/ioctl.h>
 /* Codewarrior dependant */
# include <SIOUX.h>
# include <console.h>
#endif

#ifdef CIRCLE_WINDOWS
# include <mmsystem.h>
#endif

#ifdef CIRCLE_AMIGA /* Includes for the Amiga */
# include <sys/ioctl.h>
# include <clib/socket_protos.h>
#endif /* CIRCLE_AMIGA */

#ifdef CIRCLE_ACORN /* Includes for the Acorn (RiscOS) */
# include <socklib.h>
# include <inetlib.h>
# include <sys/ioctl.h>
#endif

/*
* Note, most includes for all platforms are in sysdep.h.  The list of
* files that is included is controlled by conf.h for that platform.
*/

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "olc.h"
#include "dg_scripts.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/* externs */
extern player_index_element *player_table;
extern struct legend_data legend[8];
extern struct clan_data clan_list[50];
extern struct ban_list_element *ban_list;
extern int num_invalid;
extern char *startup;
extern const char circlemud_version[];
extern int circle_restrict;
extern int mini_mud;
extern int no_rent_check;
extern int DFLT_PORT;
extern const char *DFLT_DIR;
extern const char *DFLT_IP;
extern const char *LOGNAME;
extern const char *randhost[];
extern int MAX_PLAYERS;
extern ush_int buffer_opt;
extern struct obj_data *object_list;
extern char_data *mob_proto;
extern struct obj_data *obj_proto;
extern char_data *character_list;
extern struct trig_data *trigger_list;
extern struct index_data **trig_index;
extern int top_of_trigt;
extern int top_of_world;
extern int top_of_socialt;
extern int idle_void;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct social_messg *soc_mess_list;

extern sh_int r_human_start_room;
extern sh_int r_trolloc_start_room;
extern sh_int r_immort_start_room;
extern sh_int r_frozen_start_room;

extern char *credits;
extern char *news;
extern char *motd;
extern char *imotd;
extern char *help;
extern char *info;
extern char *immlist;
extern char *background;
extern char *handbook;
extern char *policies;
extern char *startup;

char mudname[256];  // Serai - 06/18/04 - for copyover
int GLOBAL_RESET = 0;
int ID_NUMBER = 1;

extern struct room_data *world; /* In db.c */
extern int top_of_world; /* In db.c */
extern struct time_info_data time_info; /* In db.c */

extern int top_of_zone_table;
extern struct zone_data *zone_table;
extern const char *save_info_msg[];   /* In olc.c */

/* local globals */
int mother_desc;  // Serai - 06/18/04 - needed to be global for copyover
struct gate_data *first_gate = nullptr, *last_gate = nullptr;  /* Serai - 07/22/04 - gate list! */
struct descriptor_data *descriptor_list = nullptr; /* master desc list */
int buf_largecount = 0; /* # of large buffers which exist */
int buf_overflows = 0; /* # of overflows of output */
int buf_switches = 0; /* # of switches from small to large buf */
int circle_shutdown = 0; /* clean shutdown */
int circle_reboot = 0; /* reboot the game after a shutdown */
int no_specials = 0; /* Suppress ass. of special routines */
int max_players = 0; /* max descriptors available */
int tics = 0; /* for extern checkpointing */
int scheck = 0; /* for syntax checking mode */
int dg_act_check;               /* toggle for act_trigger */
int countdown = 60 * 24;
int seconds = 0;
int global_minutes = 0;
void check_fighting(void);
void reset_skills(char_data *ch);
void import_host(struct descriptor_data *d);
void check_wait_state(void);
unsigned long dg_global_pulse = 0; /* number of pulses since game start */
extern int nameserver_is_slow; /* see config.c */
extern int auto_save; /* see config.c */
extern int autosave_time; /* see config.c */
struct timeval null_time; /* zero-valued time structure */
FILE *logfile = nullptr; /* Where to send the log messages. */

/* functions in this file */
RETSIGTYPE reread_wizlists(int sig);
RETSIGTYPE unrestrict_game(int sig);
RETSIGTYPE reap(int sig);
RETSIGTYPE checkpointing(int sig);
RETSIGTYPE hupsig(int sig);

ssize_t perform_socket_read(socket_t desc, char *read_point,size_t space_left);
ssize_t perform_socket_write(socket_t desc, const char *txt,size_t length);

int get_from_q(struct txt_q *queue, char *dest, int *aliased);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
int perform_alias(struct descriptor_data *d, char *orig);
int parse_ip(const char *addr, struct in_addr *inaddr);
int set_sendbuf(socket_t s);
int init_socket(int port);
int new_descriptor(int s);
int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void check_idle_passwords(void);
void heartbeat(int pulse);
void check_timers(void);
void update_second(void);
void update_minute(void);
void record_usage(void);
void flush_queues(struct descriptor_data *d);
void nonblock(socket_t s);
void init_game(int port);
void signal_setup(void);
void game_loop(int mother_desc);
void echo_off(struct descriptor_data *d);
void echo_on(struct descriptor_data *d);
void sanity_check(void);
void perform_flee(char_data *ch);
void log_output(struct descriptor_data *d, char *buffer);
void send_tm(int till);
void write_aliases(char_data *ch);
void Crash_rentsave(char_data *ch, int cost);
char *make_prompt(struct descriptor_data *point);

void gate_update();

struct copyover *char_to_copyover(char_data *info);
char_data *copyover_to_char(struct copyover *copy);
int prep_execl();

struct memory_data *mlist = nullptr;
struct timeval *timediff(struct timeval a, struct timeval b);
struct timeval *timeadd(struct timeval a, struct timeval b);
struct in_addr *get_bind_addr(void);
#if defined(POSIX)
sigfunc *my_signal(int signo, sigfunc * func);
#endif

char *PERS(char_data *ch, char_data *vict);

/* extern fcnts */
extern char *race_name(char_data *ch);
extern void close_mud(void);
//extern void reboot_wizlists(void);  // Serai - function does not exist!!!!
extern void boot_world(void);
extern void affect_update(void); /* In spells.c */
extern void mobile_activity(void);
extern void string_add(struct descriptor_data *d, char *str);
extern void perform_violence(void);
extern void show_string(struct descriptor_data *d, char *input);
extern int isbanned(char *hostname);
extern void weather_and_time(int mode);
extern void redit_save_to_disk(int zone_num);
extern void oedit_save_to_disk(int zone_num);
extern void medit_save_to_disk(int zone_num);
extern void sedit_save_to_disk(int zone_num);
extern void zedit_save_to_disk(int zone_num);
extern void char_checkup(void);
extern void reboot_countdown(void);
extern int offense_find(char_data *ch);
extern int parry_find(char_data *ch);
extern int dodge_find(char_data *ch);
extern int abs_find(char_data *ch);
extern int real_zone(int number);
extern void trig_data_free(trig_data *this_data);

extern const char *pc_class_types[];

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif

extern FILE *player_fl;


/***********************************************************************
*  main game loop and related stuff                                    *
***********************************************************************/


#if defined(CIRCLE_WINDOWS) || defined(CIRCLE_MACINTOSH)

/* Windows doesn't have gettimeofday, so we'll simulate it. */
/* The Mac doesn't have gettimeofday either. */
void gettimeofday(struct timeval *t, struct timezone *dummy)
{

#if defined(CIRCLE_WINDOWS)
	DWORD millisec = GetTickCount();

#elif defined(CIRCLE_MACINTOSH)
	
	unsigned long int millisec;
	millisec = (int)((float)TickCount() * 1000.0 / 60.0);

#endif

	t->tv_sec = (int) (millisec / 1000);
	t->tv_usec = (millisec % 1000) * 1000;
}

#endif /* CIRCLE_WINDOWS || CIRCLE_MACINTOSH */




/********** INT MAIN() *************/
#include <filesystem>
#include <iostream>
int main(int argc, char **argv)
{


	int port;
	int pos = 1;
	char *dir;

	/* Initialize these to check for overruns later. */
	buf[MAX_STRING_LENGTH - 1] = buf1[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;
	buf2[MAX_STRING_LENGTH - 1] = arg[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;

#ifdef CIRCLE_MACINTOSH

	/*
	 * ccommand() calls the command line/io redirection dialog box from
	 * Codewarriors's SIOUX library
	 */

	argc = ccommand(&argv);
	/* Initialize the GUSI library calls.  */

	GUSIDefaultSetup();
#endif

	port = DFLT_PORT;
	dir = (char *) DFLT_DIR;

	/*
	 * It would be nice to make this a command line option but the parser uses
	 * the log() function, maybe later. -gg
	 */

	if (LOGNAME == nullptr || *LOGNAME == '\0')
		logfile = fdopen(STDERR_FILENO, "w");

	else
		logfile = freopen(LOGNAME, "w", stderr);


	if (logfile == nullptr)
	{
		printf("error opening log file %s: %s\n",
			LOGNAME ? LOGNAME : "stderr", strerror(errno));

		exit(1);
	}

	log(circlemud_version);
	init_buffers();
	log(DG_SCRIPT_VERSION);

	log("Argc: %d. Argv: %s", argc, *argv);

	// Serai - 06/18/04 - copyover
	strncpy(mudname, argv[0], sizeof(mudname));

	while ((pos < argc) && (*(argv[pos]) == '-'))
	{

		switch (*(argv[pos] + 1))
		{

		case 'v':
			if (*(argv[pos] + 2))
				buffer_opt = atoi(argv[pos] + 2);
     
			else if (++pos < argc)
				buffer_opt = atoi(argv[pos]);

			else
			{
		
				log("SYSERR: Number expected after option -v.");
				exit(1);
			}

			break;

		case 'd':
			if (*(argv[pos] + 2))
				dir = argv[pos] + 2;

			else if (++pos < argc)
				dir = argv[pos];

			else
			{
				log("SYSERR: Directory arg expected after option -d.");
				exit(1);
			}

			break;

		case 'm':
			mini_mud = 1;
			no_rent_check = 1;
			log("Running in minimized mode & with no rent check.");
			break;

		case 'c':
			scheck = 1;
			log("Syntax check mode enabled.");
			break;

		case 'q':
			no_rent_check = 1;
			log("Quick boot mode -- rent check supressed.");
			break;

		case 'r':
			circle_restrict = 1;
			log("Restricting game -- no new players allowed.");
			break;
	
		case 's':
			no_specials = 1;
			log("Suppressing assignment of special routines.");
			break;

		default:
			log("SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
			break;
		}

		pos++;

	}

	if (pos < argc)
	{
		if (!isdigit(*argv[pos]))
		{
			log("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [-v val] [port #]\n", argv[0]);
			exit(1);
		}
		else if ((port = atoi(argv[pos])) <= 1024)
		{
			log("SYSERR: Illegal port number %d.", port);
			exit(1);
		}
	}
	if (chdir(dir) < 0)
	{
		perror("SYSERR: Fatal error changing to data directory");
		exit(1);
	}

	log("Using %s as data directory.", dir);

	if (scheck)
	{
		boot_world();
		log("Done.");
	} 
	
	else
	{
		log("Running game on port %d.", port);
		init_game(port);
	}

	exit_buffers();

	
/*
	catch(...)
	{
		printf("We crashed at catch()\r\n");
		exit(1);
	}
*/
	return 0;
}


void print_memory(void)
{

	struct memory_data *memory;
	FILE *file;
	int total = 0, count = 0;

	if(!(file = fopen("Memory File", "a")))
	{
		printf("Error opening Memory File when printing allocated memory.\r\n");
		return;
	}

	for(memory = mlist;memory;memory = memory->next)
	{

		if(!memory->allocated)
			continue;

		fprintf(file, "Allocated %d bytes of memory. Type '%s'.\n", memory->size, memory->type);
		fprintf(file, "File: %s\n", memory->file);
		fprintf(file, "Line: %d\n\n", memory->line);
		total += memory->size;
		count++;
	}

	fprintf(file, "*************************************************\n");
	fprintf(file, "Total memory traced: %f Mega Bytes.\r\nDate: %s. Time: %s.\r\n\n", (float) ((total / 1024) / 1024), __DATE__, __TIME__);
	log("%d items from mlist have been removed. A total of %d data of un-deleted memory.\r\n", count, total);
}

/* Init sockets, run game, and cleanup sockets */
void init_game(int port)
{
	FILE *copyover_file;
	sh_int load_room = 0;
	char_data *info = nullptr;
	struct copyover copy;

	/* We don't want to restart if we crash before we get up. */
	touch(KILLSCRIPT_FILE);

	circle_srandom(time(0));

	log("Finding player limit.");
	max_players = get_max_players();

	if ( (copyover_file = fopen(".copyover", "rb")) == nullptr)
	{
		log("Opening mother connection.");
		mother_desc = init_socket(port);
	}
	else
	{
		log("Recovering mother connection from copyover file.");
		fread(&mother_desc, sizeof(int), 1, copyover_file);
	}

	boot_db();

	#ifdef CIRCLE_UNIX
		log("Signal trapping.");
		signal_setup();
	#endif

	if (copyover_file != nullptr)
	{
		log("Loading copyover file.");

		while(fread(&copy, sizeof(struct copyover), 1, copyover_file))
		{
			info = copyover_to_char(&copy);

			info->desc->next = descriptor_list;
			descriptor_list = info->desc;

			info->next = character_list;
			character_list = info;

			if (STATE(info->desc) == CON_PLAYING)
			{
				if (PLR_FLAGGED(info, PLR_FROZEN))
					load_room = r_frozen_start_room;
	
				else if(GET_LOADROOM(info) >= 0 && real_room(GET_LOADROOM(info)))
					load_room = real_room(GET_LOADROOM(info));
	
				else
				{
					if(IS_HUMAN(info) && GET_LEVEL(info) < LVL_IMMORT)
						load_room = r_human_start_room;
	
					else if(IS_TROLLOC(info) && GET_LEVEL(info) < LVL_IMMORT)
						load_room = r_trolloc_start_room;
				}

				char_to_room(info, load_room);

				send_to_char("\r\nCopyover completed successfully.\r\n\r\n", info);
				look_at_room(info, 0);

				GET_LOADROOM(info) = load_room;
			}
		}

		fclose(copyover_file);

		if (remove(".copyover") != 0)
			log("SYSERR: Could not remove(.copyover)!  Remove it manually!");
	}

	/* If we made it this far, we will be able to restart without problem. */
	remove(KILLSCRIPT_FILE);

	log("Entering game loop.");

	game_loop(mother_desc);

	// Save OLC before freeing everything..
	if (circle_reboot != 2 && olc_save_list)
	{ /* Don't save zones. */
		struct olc_save_info *entry, *next_entry;
			int rznum;

		for (entry = olc_save_list; entry; entry = next_entry)
		{
			next_entry = entry->next;
     
			if (entry->type < 0 || entry->type > 4)
			{
				sprintf(buf, "OLC: Illegal save type %d!", entry->type);
				log(buf);
			} 
		
			else if ((rznum = real_zone(entry->zone * 100)) == -1)
			{
				sprintf(buf, "OLC: Illegal save zone %d!", entry->zone);
				log(buf);
			} 
		
			else if (rznum < 0 || rznum > top_of_zone_table)
			{
				sprintf(buf, "OLC: Invalid real zone number %d!", rznum);
				log(buf);
			} 
		
			else
			{
				sprintf(buf, "OLC: Reboot saving %s for zone %d.",
				save_info_msg[(int)entry->type], zone_table[rznum].number);
				log(buf);
       
				switch (entry->type)
				{
			
				case OLC_SAVE_ROOM: redit_save_to_disk(rznum); break;
				case OLC_SAVE_OBJ:  oedit_save_to_disk(rznum); break;
				case OLC_SAVE_MOB:  medit_save_to_disk(rznum); break;
				case OLC_SAVE_ZONE: zedit_save_to_disk(rznum); break;
				case OLC_SAVE_SHOP: sedit_save_to_disk(rznum); break;
				default:      log("Unexpected olc_save_list->type"); break;
			
				}
			}
		}
	}

	// Serai - lets free as much allocated memory as we possibly can.  06/11/04

	log("Closing all sockets.");

	while (descriptor_list)
	{
		if(descriptor_list->character)
		{
			char_data *ch = descriptor_list->character;

			ch->save();
			write_aliases(ch);
			Crash_rentsave(ch, 0);
			close_socket(descriptor_list);
		}
	}

	CLOSE_SOCKET(mother_desc);

	log("Freeing PCs and NPCs...");
	while(character_list)
		extract_char(character_list);

	log("Freeing items...");
	while(object_list)
		extract_obj(object_list);

	delete[] (mob_proto);
	delete[] (obj_proto);

	delete[] (mob_index);
	delete[] (obj_index);

	log("Freeing triggers...");
	for(struct trig_data *temp_trig = trigger_list; trigger_list; temp_trig = trigger_list)
	{
		trigger_list = trigger_list->next_in_world;
		trig_data_free(temp_trig);
	}

	for(int i = 0; i < top_of_trigt; i++)
	{
		trig_data_free(trig_index[i]->proto);
	}

	delete[] (trig_index);

	log("Freeing the world...");
	for(int i2 = 0; i2 < top_of_world; i2++)
	{
		delete[] (world[i2].name);
		delete[] (world[i2].description);

		for(int j = 0; j < NUM_OF_DIRS; j++)
		{
			if (world[i2].dir_option[j])
			{
				if (world[i2].dir_option[j]->keyword)
					delete[] (world[i2].dir_option[j]->keyword);

				if (world[i2].dir_option[j]->general_description)
					delete[] (world[i2].dir_option[j]->general_description);
			
				delete[] (world[i2].dir_option[j]);
			}
		}
	}

	delete[] (world);

	log("Freeing everything else...");
	delete[] (credits);
	delete[] (news);
	delete[] (motd);
	delete[] (imotd);
	delete[] (help);
	delete[] (info);
	delete[] (immlist);
	delete[] (background);
	delete[] (handbook);
	delete[] (policies);
	delete[] (startup);

	for(int i3 = 0; i3 < top_of_zone_table; i3++)
	{
		delete[] (zone_table[i3].name);
		delete[] (zone_table[i3].cmd);
	}

	delete[] (zone_table);

	for(int i4 = 0; i4 < top_of_socialt; i4++)
	{
		delete[] (soc_mess_list[i4].command);
		delete[] (soc_mess_list[i4].sort_as);
		delete[] (soc_mess_list[i4].char_no_arg);
		delete[] (soc_mess_list[i4].others_no_arg);
		delete[] (soc_mess_list[i4].char_found);
		delete[] (soc_mess_list[i4].others_found);
		delete[] (soc_mess_list[i4].vict_found);
		delete[] (soc_mess_list[i4].not_found);
		delete[] (soc_mess_list[i4].char_auto);
		delete[] (soc_mess_list[i4].others_auto);
		delete[] (soc_mess_list[i4].char_body_found);
		delete[] (soc_mess_list[i4].others_body_found);
		delete[] (soc_mess_list[i4].vict_body_found);
		delete[] (soc_mess_list[i4].char_obj_found);
		delete[] (soc_mess_list[i4].others_obj_found);
	}

	delete[] (soc_mess_list);

//	log("Writing all allocated memory to Memory File");
//	print_memory();

//	fclose(player_fl);

	if (circle_reboot)
	{
		log("Rebooting.");
		exit(52); /* what's so great about HHGTTG, anyhow? */
	}
 
	log("Normal termination of game.");
}



/*
* init_socket sets up the mother descriptor - creates the socket, sets
* its options up, binds it, and listens.
*/
int init_socket(int port)
{
	int s, opt;
	struct sockaddr_in sa;

#ifdef CIRCLE_WINDOWS
	{
		WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = MAKEWORD(1, 1);

		if (WSAStartup(wVersionRequested, &wsaData) != 0)
		{
			log("SYSERR: WinSock not available!");
			exit(1);
		}

		if ((wsaData.iMaxSockets - 4) < max_players)
		{
			max_players = wsaData.iMaxSockets - 4;
		}

		log("Max players set to %d", max_players);

		if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			log("SYSERR: Error opening network connection: Winsock error #%d",
			WSAGetLastError());
			exit(1);
		}

	}

#else
 /*
  * Should the first argument to socket() be AF_INET or PF_INET?  I don't
  * know, take your pick.  PF_INET seems to be more widely adopted, and
  * Comer (_Internetworking with TCP/IP_) even makes a point to say that
  * people erroneously use AF_INET with socket() when they should be using
  * PF_INET.  However, the man pages of some systems indicate that AF_INET
  * is correct; some such as ConvexOS even say that you can use either one.
  * All implementations I've seen define AF_INET and PF_INET to be the same
  * number anyway, so the point is (hopefully) moot.
  */

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("SYSERR: Error creating socket");
		exit(1);
	}

#endif /* CIRCLE_WINDOWS */


#if defined(SO_REUSEADDR) && !defined(CIRCLE_MACINTOSH)

	opt = 1;

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0)
	{
		perror("SYSERR: setsockopt REUSEADDR");
		exit(1);
	}


#endif

	set_sendbuf(s);

#if defined(SO_LINGER)
	{
		struct linger ld;

		ld.l_onoff = 0;
		ld.l_linger = 0;
   
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
		{
			perror("SYSERR: setsockopt LINGER");
			exit(1);
		}
	}
#endif

	/* Clear the structure */
	memset((char *)&sa, 0, sizeof(sa));


	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr = *(get_bind_addr());

	if (::bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	{
		perror("SYSERR: bind");
		CLOSE_SOCKET(s);
		exit(1);
	}

	nonblock(s);
	listen(s, 5);
	return s;
}


int get_max_players(void)
{


#ifndef CIRCLE_UNIX
	return MAX_PLAYERS;

#else

	int max_descs = 0;
	const char *method;

/*
* First, we'll try using getrlimit/setrlimit.  This will probably work
* on most systems.  HAS_RLIMIT is defined in sysdep.h.
*/


#ifdef HAS_RLIMIT
	{
   
		struct rlimit limit;

		/* find the limit of file descs */
		method = "rlimit";
   
		if (getrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			perror("SYSERR: calling getrlimit");
			exit(1);
		}

		/* set the current to the maximum */
		limit.rlim_cur = limit.rlim_max;
   
		if (setrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			perror("SYSERR: calling setrlimit");
			exit(1);
		}

#ifdef RLIM_INFINITY
   
		if (limit.rlim_max == RLIM_INFINITY)
			max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
   
		else
			max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);

#else
	max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);

#endif
	}

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif

	method = "OPEN_MAX";
	max_descs = OPEN_MAX; /* Uh oh.. rlimit didn't work, but we have

	* OPEN_MAX */

#elif defined (_SC_OPEN_MAX)
 
/*
 * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
 * try the POSIX sysconf() function.  (See Stevens' _Advanced Programming
 * in the UNIX Environment_).
 */

	method = "POSIX sysconf";
	errno = 0;

	if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0)
	{
		if (errno == 0)
			max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;

		else
		{
			perror("SYSERR: Error calling sysconf");
			exit(1);
		}
	}
#else
	/* if everything has failed, we'll just take a guess */
	method = "random guess"
	max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;

#endif

	/* now calculate max _players_ based on max descs */
	max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

	if (max_descs <= 0)
	{
		log("SYSERR: Non-positive max player limit!  (Set at %d using %s).",
		max_descs, method);
		exit(1);
	}

	log("Setting player limit to %d using %s.", max_descs, method);
	return max_descs;

#endif /* CIRCLE_UNIX */

}



/*
* game_loop contains the main loop which drives the entire MUD.  It
* cycles once every 0.10 seconds and is responsible for accepting
* new connections, polling existing connections for input, dequeueing
* output and sending it out to players, and calling "heartbeat" functions
* such as mobile_activity().
*/
void game_loop(int mother_desc)
{
	fd_set input_set, output_set, exc_set, null_set;
	struct timeval last_time, before_sleep, opt_time, process_time, now, timeout;
	char comm[MAX_INPUT_LENGTH];
	struct descriptor_data *d, *next_d;
	int pulse = 0, missed_pulses = 0, maxdesc, aliased;

	/* initialize various time values */
	null_time.tv_sec = 0;
	null_time.tv_usec = 0;
	opt_time.tv_usec = OPT_USEC;
	opt_time.tv_sec = 0;
	FD_ZERO(&null_set);
	gettimeofday(&last_time, (struct timezone *) 0);

	/* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
	while (!circle_shutdown)
	{

	/* Sleep if we don't have any connections */
		if (descriptor_list == nullptr)
		{
			log("No connections.  Going to sleep.");
			FD_ZERO(&input_set);
			FD_SET(mother_desc, &input_set);
			
			if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, nullptr) < 0)
			{
				if (errno == EINTR)
					log("Waking up to process signal.");
				else
					perror("Select coma");
			} 
			
			else
				log("New connection.  Waking up.");

			
			gettimeofday(&last_time, (struct timezone *) 0);
		}

		/* Set up the input, output, and exception sets for select(). */
		FD_ZERO(&input_set);
		FD_ZERO(&output_set);
		FD_ZERO(&exc_set);
		FD_SET(mother_desc, &input_set);

		maxdesc = mother_desc;
		
		for (d = descriptor_list; d; d = d->next)
		{
			#ifndef CIRCLE_WINDOWS
				if (d->descriptor > maxdesc)
					maxdesc = d->descriptor;
			#endif
			
			FD_SET(d->descriptor, &input_set);
			FD_SET(d->descriptor, &output_set);
			FD_SET(d->descriptor, &exc_set);
		}

   /*
    * At this point, we have completed all input, output and heartbeat
    * activity from the previous iteration, so we have to put ourselves
    * to sleep until the next 0.1 second tick.  The first step is to
    * calculate how long we took processing the previous iteration.
    */
   
		gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
		process_time = *timediff(before_sleep, last_time);

   /*
    * If we were asleep for more than one pass, count missed pulses and sleep
    * until we're resynchronized with the next upcoming pulse.
    */
   
		if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC)
			missed_pulses = 0;
		
		else
		{
			missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
			missed_pulses += process_time.tv_usec / OPT_USEC;
			process_time.tv_sec = 0;
			process_time.tv_usec = process_time.tv_usec % OPT_USEC;
		}

   /* Calculate the time we should wake up */
		
		last_time = *timeadd(before_sleep, *timediff(opt_time, process_time));

   /* Now keep sleeping until that time has come */
   
		gettimeofday(&now, (struct timezone *) 0);
		timeout = *timediff(last_time, now);

   /* Go to sleep */
		do
		{
			#ifdef CIRCLE_WINDOWS
				Sleep(timeout.tv_sec * 1000 + timeout.tv_usec / 1000);

			#else
				if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0)
				{
					if (errno != EINTR)
					{
						perror("SYSERR: Select sleep");
						exit(1);
					}
				}

			#endif /* CIRCLE_WINDOWS */

			gettimeofday(&now, (struct timezone *) 0);
			timeout = *timediff(last_time, now);
			
		} while (timeout.tv_usec || timeout.tv_sec); /* End of do while */

   /* Poll (without blocking) for new input, output, and exceptions */
		if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
		{
			perror("Select poll");
			return;
		}
   /* If there are new connections waiting, accept them. */
		if (FD_ISSET(mother_desc, &input_set))
			new_descriptor(mother_desc);

   /* Kick out the freaky folks in the exception set and marked for close */
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
			
			if (FD_ISSET(d->descriptor, &exc_set))
			{
				FD_CLR(d->descriptor, &input_set);
				FD_CLR(d->descriptor, &output_set);
				close_socket(d);
			}
		}

		/* Make sure everything is sane with the characters before we go on */
				char_checkup();

   /* Process descriptors with input pending */
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
			if (FD_ISSET(d->descriptor, &input_set))
				if (process_input(d) < 0)
				close_socket(d);
		}


	/* Get the wait states updates */
		check_wait_state();

   /* Process commands we just read from process_input */
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;

			if (d->wait > 0)
				continue;

			if (!get_from_q(&d->input, comm, &aliased))
				continue;

			if (d->character)
			{
				/* Reset the idle timer & pull char back from void if necessary */
				d->character->char_specials.timer = 0;
				
				if (STATE(d) == CON_PLAYING && GET_WAS_IN(d->character) != NOWHERE)
				{
					if (d->character->in_room != NOWHERE)
						char_from_room(d->character);

					char_to_room(d->character, GET_WAS_IN(d->character));
					GET_WAS_IN(d->character) = NOWHERE;
					act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
				}
			}
			d->wait = 1;
			d->has_prompt = 0;

		   /*
			* I reversed these top 2 if checks so that you can use the
			*/
			if (d->showstr_count) /* Reading something w/ pager */
				show_string(d, comm);

			else if (d->str)
				string_add(d, comm);

			else if (STATE(d) != CON_PLAYING) /* In menus, etc. */
				nanny(d, comm);

			else
			{ /* else: we're playing normally. */
				
				if (aliased) /* To prevent recursive aliases. */
					d->has_prompt = 1; /* To get newline before next cmd output. */
				
				else if (perform_alias(d, comm))    /* Run it through aliasing system */
					get_from_q(&d->input, comm, &aliased);
			
				command_interpreter(d->character, comm); /* Send it to interpreter */
					
				if(d->command_ready == 0)
					d->timer = 0;
			}
		}

   /* Send queued output out to the operating system (ultimately to user) */
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
			
			if (*(d->output) && FD_ISSET(d->descriptor, &output_set))
			{
			/* Output for this player is ready */
				if (process_output(d) < 0)
					close_socket(d);

				else
					d->has_prompt = 1;
			}
		}

		/* Print prompts for other descriptors who had no other output */
		for (d = descriptor_list; d; d = d->next)
		{
			if (!d->has_prompt)
			{
				write_to_descriptor(d->descriptor, make_prompt(d));
				d->has_prompt = 1;
			}
		}

		/* Kick out folks in the CON_CLOSE or CON_DISCONNECT state */
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
			
			if (STATE(d) == CON_CLOSE || STATE(d) == CON_DISCONNECT)
				close_socket(d);
		}

   /*
    * Now, we execute as many pulses as necessary--just one if we haven't
    * missed any pulses, or make up for lost time if we missed a few
    * pulses by sleeping for too long.
    */
		missed_pulses++;

		if (missed_pulses <= 0)
		{
			log("SYSERR: **BAD** MISSED_PULSES NONPOSITIVE (%d), TIME GOING BACKWARDS!!", missed_pulses);
			missed_pulses = 1;
		}

		/* If we missed more than 30 seconds worth of pulses, just do 30 secs */
		if (missed_pulses > (2 * PASSES_PER_SEC))
		{
			log("SYSERR: Missed %d seconds worth of pulses.", missed_pulses / PASSES_PER_SEC);
			missed_pulses = 2 * PASSES_PER_SEC;
		}

		/* Now execute the heartbeat functions */
		while (missed_pulses--)
			heartbeat(++pulse);

		/* Roll pulse over after 10 hours */
		if (pulse >= (600 * 60 * PASSES_PER_SEC))
			pulse = 0;

		#ifdef CIRCLE_UNIX
		/* Update tics for deadlock protection (UNIX only) */
			tics++;
		#endif



	}
}

/*This function updates wait states and performs lagged actions if the lag is over. */
void check_wait_state(void)
{
	char_data *ch;
	struct descriptor_data *d;

	/* Subtract mob lag and player lag */
	for(d = descriptor_list;d;d = d->next)
	{
// Serai - 07/22/04 - Noooooooo!
//		if(ch->desc)
		if (d->character)
		{
			d->wait -= (d->wait > 0);
		}
	}

	for(ch = character_list;ch;ch = ch->next)
	{

		if(IS_NPC(ch))
		{
			GET_MOB_WAIT(ch) -= (GET_MOB_WAIT(ch) > 0);
		}

		// If no wait, proceed to do lagged actions.
		if((ch->desc && ch->desc->wait <= 0) || (IS_NPC(ch) && GET_MOB_WAIT(ch) <= 0))
		{
			if(FLEE_GO(ch) == TRUE)
				perform_flee(ch);

			if(IS_BASHED(ch) && GET_POS(ch) == POS_SITTING)
			{
				if(FIGHTING(ch))
					GET_POS(ch) = POS_FIGHTING;

				else
					GET_POS(ch) = POS_STANDING;
			}

			IS_BASHED(ch) = 0;

		}

	}

}

void send_collective_noise(void)
{

	int room = 0, type = 0;

	for(room = 0;room < top_of_world;room++)
	{

		for(type = 0;type < MAX_TYPE;type++)
		{
			if(world[room].noise_level[type] > 0)
				perform_group_noise(room, world[room].noise_level[type], nullptr, type);

			world[room].noise_level[type] = 0;
		}
	}
}

void update_status_file(void)
{

	char_data *ch;
	int number = 0;
	FILE *status;

	if(!(status = fopen(STATUS_FILE, "w+")))
	{
		mudlog("Error: Unable to open status file for update.", NRM, LVL_IMMORT, TRUE);
		return;
	}

	//Print the number of playing. //
	for(number = 0, ch = character_list;ch;ch = ch->next)
	{
		if(!IS_NPC(ch))
			number++;
	}

	fprintf(status, "%d\n", number);
	
	//Printf the number of linkless. //
	for(number = 0, ch = character_list;ch;ch = ch->next)
	{
		if(!ch->desc && !IS_NPC(ch))
			number++;
	}

	fprintf(status, "%d\n", number);

	//Print the current time//
	fprintf(status, "%d\n", time(0));

	fclose(status);


}

void heartbeat(int pulse)
{
	static int mins_since_crashsave = 0;
	void process_events(void);

	dg_global_pulse++;

	process_events();

	if (!(pulse % PULSE_DG_SCRIPT))
		script_trigger_check();

	if (!(pulse % (30 * PASSES_PER_SEC)))
		sanity_check();

	/* Clear out all the global buffers now in case someone forgot. */
	if (!(pulse % PULSE_BUFFER))
		release_all_buffers(pulse);
	
	if (!(pulse % PULSE_ZONE))
		zone_update();
  
	if (!(pulse % (PASSES_PER_SEC / 2)))
	{
		check_timers();
		char_checkup();
	}

	if (!(pulse % (PASSES_PER_SEC)))
	{
		seconds++;
		check_fighting();
	}

	if(!((pulse + (10 * PASSES_PER_SEC)) % (PASSES_PER_SEC * SECS_PER_MUD_HOUR)))
	{
		send_tm(10);
	}

	if (!(pulse % (180 * PASSES_PER_SEC))) /* 3 minuits */
		check_idle_passwords();

	if (!(pulse % PULSE_MOBILE))
		mobile_activity();

	if (!(pulse % PULSE_VIOLENCE))
	{
		gate_update();
		perform_violence();
		//send_collective_noise();
	}

	if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC)))
	{
		update_minute();
		weather_and_time(1);
		affect_update();
		point_update();
		fflush(player_fl);
		reboot_countdown();
	}

	if(!(pulse % (PASSES_PER_SEC * 10)))
		update_status_file();

	if (auto_save && !(pulse % (60 * PASSES_PER_SEC)))
	{ /* 1 minute */
		if (++mins_since_crashsave >= autosave_time)
		{
			mins_since_crashsave = 0;
			Crash_save_all();
			House_save_all();
		}
	}
 
	if (!(pulse % (5 * 60 * PASSES_PER_SEC))) /* 5 minutes */
		record_usage();
}

void gate_update()
{
	struct room_data *room;

	for(struct gate_data *temp = first_gate, *next; temp; temp = next)
	{
		next = temp->next_in_world;

		if (!(temp->alive))
			continue;

		if (temp->creator && GET_MANA(temp->creator) >= 2 && GET_MOVE(temp->creator) >= 2)
		{
			if (GET_LEVEL(temp->creator) >= LVL_GRGOD)
				continue;

			GET_MANA(temp->creator) -= 2;
			GET_MOVE(temp->creator) -= 2;
		}
		else
		{
			if (temp->creator)
				send_to_char("You can't hold open your Gate any longer!\r\n" , temp->creator);

			sprintf(buf, "In a bright flash of light, the Gate collapses.\r\n");
			send_to_room(buf, temp->in_room);
			send_to_room(buf, temp->to_room);

			room = &(world[temp->in_room]);

			REMOVE_NODE(room->first_live_gate, room->last_live_gate, temp, prev, next);
			ADD_FIRST_NODE(room->first_dead_gate, room->last_dead_gate, temp, prev, next);

			temp->alive = false;
		}
	}
}

void update_minute(void)
{
	char_data *ch;
	
	global_minutes++;

	for(ch = character_list;ch;ch = ch->next)
		if(GET_DEATH_WAIT(ch) > 0 && !IS_NPC(ch))
			GET_DEATH_WAIT(ch)--;
}

/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
*  new code to calculate time differences, which works on systems
*  for which tv_usec is unsigned (and thus comparisons for something
*  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
*/

/*
* code to return the time difference between a and b (a-b).
* always returns a nonnegative value (floors at 0).
*
* Fixed the 'aggregate return' warning.  Now it's not thread-safe.
* -gg 6/18/98
*/

struct timeval *timediff(struct timeval a, struct timeval b)
{
	static struct timeval rslt;

	if (a.tv_sec < b.tv_sec)
		return &null_time;
 
	else if (a.tv_sec == b.tv_sec)
	{
		if (a.tv_usec < b.tv_usec)
			return &null_time;
   
		else {
			rslt.tv_sec = 0;
			rslt.tv_usec = a.tv_usec - b.tv_usec;
			return &rslt;
		}
	} 
	
	else
	{ /* a->tv_sec > b->tv_sec */
		rslt.tv_sec = a.tv_sec - b.tv_sec;
		
		if (a.tv_usec < b.tv_usec)
		{
			rslt.tv_usec = a.tv_usec + 1000000 - b.tv_usec;
			rslt.tv_sec--;
		} 
		
		else
			rslt.tv_usec = a.tv_usec - b.tv_usec;
   
		return &rslt;
	}
}

/*
* add 2 timevals
*
* Fixed the 'aggregate return' warning. Not thread-safe now.
* -gg 6/18/98
*/
struct timeval *timeadd(struct timeval a, struct timeval b)
{
	static struct timeval rslt;

	rslt.tv_sec = a.tv_sec + b.tv_sec;
	rslt.tv_usec = a.tv_usec + b.tv_usec;

	while (rslt.tv_usec >= 1000000)
	{
		rslt.tv_usec -= 1000000;
		rslt.tv_sec++;
	}

	return &rslt;
}

void reboot_countdown(void)
{

	struct descriptor_data *d;
	char text[MAX_INPUT_LENGTH];

	strcpy(text, "\r\n\nThe game is rebooting. Come back in a few moments.\r\n\n");

	if(countdown < 0)
		return;

	countdown--;

	if(countdown <= 15)
	{
		sprintf(buf, "REBOOT IN %d MINUTE%s.\r\n", countdown, countdown > 1 ? "S" : "");
		send_to_all(buf);
	}
	
	if(countdown == 0)
	{
		for(d = descriptor_list;d;d = d->next)
			send(d->descriptor, text, strlen(text), 0);
		Crash_save_all();
		House_save_all();
		circle_shutdown = 1;
	}
}


void record_usage(void)
{
	int sockets_connected = 0, sockets_playing = 0;
	struct descriptor_data *d;

	for (d = descriptor_list; d; d = d->next)
	{
		sockets_connected++;
		
		if (STATE(d) == CON_PLAYING)
			sockets_playing++;
	}

	log("nusage: %-3d sockets connected, %-3d sockets playing",
	sockets_connected, sockets_playing);

#ifdef RUSAGE
{
	struct rusage ru;

	getrusage(RUSAGE_SELF, &ru);
	log("rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
	ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
}
#endif

}



/*
* Turn off echoing (specific to telnet client)
*/
void echo_off(struct descriptor_data *d)
{
	char off_string[] =
	{
		(char) IAC,
		(char) WILL,
		(char) TELOPT_ECHO,
		(char) 0,
	};

	SEND_TO_Q(off_string, d);
}


/*
* Turn on echoing (specific to telnet client)
*/
void echo_on(struct descriptor_data *d)
{
	char on_string[] =
	{
		(char) IAC,
		(char) WONT,
		(char) TELOPT_ECHO,
		(char) TELOPT_NAOFFD,
		(char) TELOPT_NAOCRD,
		(char) 0,
	};

	SEND_TO_Q(on_string, d);
}

char *health(int percent)
{

	if(percent >= 100)
		return "Healthy";
	
	else if(percent >= 90)
		return "Scratched";
	
	else if(percent >= 75)
		return "Hurt";
	
	else if(percent >= 50)
		return "Wounded";
	
	else if(percent >= 30)
		return "Battered";
	
	else if(percent >= 15)
		return "Beaten";
	
	else if(percent >= 0)
		return "Critical";
	
	else
		return "Incapacitated";
}

char *mana(int percent)
{

	if(percent >= 100)
		return "Full";
	
	else if(percent >= 90)
		return "Excellent";
	
	else if(percent >= 75)
		return "Strong";
	
	else if(percent >= 50)
		return "Good";
	
	else if(percent >= 30)
		return "Weakening";
	
	else if(percent >= 15)
		return "Flickering";

	else if(percent >= 1)
		return "Flickering";

	else
		return "Wasted";
}

char *moves(int percent)
{

	if(percent >= 100)
		return "Fresh";
	
	else if(percent >= 90)
		return "Excellent";
	
	else if(percent >= 75)
		return "Strong";
	
	else if(percent >= 50)
		return "Good";
	
	else if(percent >= 30)
		return "Draining";
	
	else if(percent >= 15)
		return "Faint";
	
	else
		return "Burning";
}


char *make_prompt(struct descriptor_data *d)
{
	static char prompt[256];
	char_data *victim;
	/* Note, prompt is truncated at MAX_PROMPT_LENGTH chars (structs.h )*/

	/*
	 * These two checks were reversed to allow page_string() to work in the
	 * online editor.
	 */
	
	if (d->showstr_count)
		sprintf(prompt, "\r[ Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d) ]",
		d->showstr_page, d->showstr_count);

	else if (d->str)
	    strcpy(prompt, "] ");

	else if (STATE(d) == CON_PLAYING && !IS_NPC(d->character))  
	{
		*prompt = '\0';

		if (GET_INVIS_LEV(d->character))
			sprintf(prompt, "i%d ", GET_INVIS_LEV(d->character));

		if (PRF_FLAGGED(d->character, PRF_DISPHP))
			sprintf(prompt + strlen(prompt), "HP:%s ", health(GET_MAX_HIT(d->character) > 0 ?
			100 * GET_HIT(d->character) / (GET_MAX_HIT(d->character)) : 1));

		if (PRF_FLAGGED(d->character, PRF_DISPMANA) && (IS_CHANNELER(d->character) || IS_DREADLORD(d->character)))
			sprintf(prompt + strlen(prompt), "SP:%s  ", mana(GET_MAX_MANA(d->character) > 0 ?
			100 * GET_MANA(d->character) / (GET_MAX_MANA(d->character)) : 1));

		if (PRF_FLAGGED(d->character, PRF_DISPMANA) && (IS_DREADLORD(d->character) || IS_FADE(d->character)))
			sprintf(prompt + strlen(prompt), "SHP:%s  ", mana( GET_MAX_SHADOW(d->character) > 0 ?
			100 * GET_SHADOW(d->character) / (GET_MAX_SHADOW(d->character)) : 1));

		if (PRF_FLAGGED(d->character, PRF_DISPMOVE))
			sprintf(prompt + strlen(prompt), "MV:%s ", moves(GET_MAX_MOVE(d->character) > 0 ?
			100 * GET_MOVE(d->character) / (GET_MAX_MOVE(d->character)) : 1));

		/**********Prompt Addition by Jin**********/
		/**********Updated by Tulon on Feb 15, 2004**********/


		victim = FIGHTING(d->character);


		if(victim)
			sprintf(prompt + strlen(prompt), "  - %s: %s", GET_NAME(victim), health(100 * GET_HIT(victim) / (GET_MAX_HIT(victim))));

	
		

		if(victim && FIGHTING(victim) && FIGHTING(victim) != d->character)
			sprintf(prompt + strlen(prompt), " --- %s: %s\
\n\r",
			GET_NAME(FIGHTING(victim)), health(100 * GET_HIT(FIGHTING(victim)) / (GET_MAX_HIT(FIGHTING(victim)))));
                
		strcat(prompt, ">\
 \n\r");
	}	
	
	else if (STATE(d) == CON_PLAYING && IS_NPC(d->character))
		sprintf(prompt, "\n%s> \n", GET_NAME(d->character));

	else
		*prompt = '\0';

	return prompt;
}

void write_to_q(const char *txt, struct txt_q *queue, int aliased)
{
	struct txt_block *newt;

	if (!txt)
	{
		mudlog("SYSERR: txt is nullptr in write_to_q()!", NRM, LVL_GRGOD, TRUE);
		core_dump();
		return;
	}

	CREATE(newt, struct txt_block, 1);
	CREATE(newt->text, char, strlen(txt) + 1);
	strcpy(newt->text, txt);
	newt->aliased = aliased;

	/* queue empty? */
 
	if (!queue->head)
	{
		newt->next = nullptr;
		queue->head = queue->tail = newt;
	} 
	
	else
	{
		queue->tail->next = newt;
		queue->tail = newt;
		newt->next = nullptr;
	}
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
	struct txt_block *tmp;

	/* queue empty? */
	if (!queue->head)
		return 0;

	tmp = queue->head;
	strcpy(dest, queue->head->text);
	*aliased = queue->head->aliased;
	queue->head = queue->head->next;

	delete[] (tmp->text);
	delete[] (tmp);

	return 1;
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
	int dummy;

	if (d->large_outbuf)
	{
		release_buffer(d->large_outbuf);
		d->output = d->small_outbuf; /*neccessary??*/
	}
 
	while (get_from_q(&d->input, buf2, &dummy));
}

/* Add a new string to a player's output queue */
void vwrite_to_output(struct descriptor_data *t, int swap_args, const char *format, va_list args)
{
	int size;
	char txt[MAX_STRING_LENGTH];

	if(swap_args)
		vsnprintf(txt, sizeof(txt), format, args);

	else
		strcpy(txt, format);

	size = strlen(txt);

	/* if we're in the overflow state already, ignore this new output */
	if (t->bufptr < 0)
		return;

	/* if we have enough space, just write to buffer and that's it! */
 
	if (t->bufspace >= size)
	{
		strcpy(t->output + t->bufptr, txt);
		t->bufspace -= size;
		t->bufptr += size;
		return;
	}
 
	/*
	* If the text is too big to fit into even a large buffer, chuck the
	* new text and switch to the overflow state.
	*/
 
	if (size + t->bufptr > LARGE_BUFSIZE - 1)
	{
		t->bufptr = -1;
		 buf_overflows++;
		return;
	}
 
	buf_switches++;

 /*
  * Just request the buffer. Copy the contents of the old, and make it
  * the primary buffer.
  */
	t->large_outbuf = get_buffer(LARGE_BUFSIZE);
	s2b_cpy(t->large_outbuf, t->output);
	t->output = sz(t->large_outbuf);
	strcat(t->output, txt);

 /* calculate how much space is left in the buffer */

	// Serai - 06/29/04 - YOU MISSED SOMETHING!!!  Update bufptr first!
	// The first output larger than 1024 characters causes a switch from a
	// small buffer to a large one for this pulse...  This missing line
	// basically lost the output that caused the switch...  :(
	t->bufptr += size;

	t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;
}

void write_to_output(struct descriptor_data *t, const char *txt, ...)
{

	if(!t || !txt)
		return;

	vwrite_to_output(t, FALSE, txt, va_list());

}

/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */


/*
* get_bind_addr: Return a struct in_addr that should be used in our
* call to bind().  If the user has specified a desired binding
* address, we try to bind to it; otherwise, we bind to INADDR_ANY.
* Note that inet_aton() is preferred over inet_addr() so we use it if
* we can.  If neither is available, we always bind to INADDR_ANY.
*/

struct in_addr *get_bind_addr()
{
	static struct in_addr bind_addr;

	/* Clear the structure */
	memset((char *) &bind_addr, 0, sizeof(bind_addr));

	/* If DLFT_IP is unspecified, use INADDR_ANY */
 
	
	if (DFLT_IP == nullptr)
	{
		bind_addr.s_addr = htonl(INADDR_ANY);
	} 
	
	else
	{
		/* If the parsing fails, use INADDR_ANY */
		if (!parse_ip(DFLT_IP, &bind_addr))
		{
			log("SYSERR: DFLT_IP of %s appears to be an invalid IP address",DFLT_IP);
			bind_addr.s_addr = htonl(INADDR_ANY);
		}
	}

	/* Put the address that we've finally decided on into the logs */
	if (bind_addr.s_addr == htonl(INADDR_ANY))
		log("Binding to all IP interfaces on this host.");
 
	else
		log("Binding only to IP address %s", inet_ntoa(bind_addr));

	return &bind_addr;
}

#ifdef HAVE_INET_ATON

/*
* inet_aton's interface is the same as parse_ip's: 0 on failure, non-0 if
* successful
*/
int parse_ip(const char *addr, struct in_addr *inaddr)
{
	return inet_aton(addr, inaddr);
}

#elif HAVE_INET_ADDR

/* inet_addr has a different interface, so we emulate inet_aton's */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
	long ip;

	if ((ip = inet_addr(addr)) == -1)
	{
		return 0;
	} 
	
	else
	{
		inaddr->s_addr = (unsigned long) ip;
		return 1;
	}
}

#else

/* If you have neither function - sorry, you can't do specific binding. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
	log(	"SYSERR: warning: you're trying to set DFLT_IP but your system has no\n"
			"functions to parse IP addresses (how bizarre!)");
	return 0;
}

#endif /* INET_ATON and INET_ADDR */



/* Sets the kernel's send buffer size for the descriptor */
int set_sendbuf(socket_t s)
{
#if defined(SO_SNDBUF) && !defined(CIRCLE_MACINTOSH)
	int opt = MAX_SOCK_BUF;

	if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0)
	{
		perror("SYSERR: setsockopt SNDBUF");
		return -1;
	}

#if 0
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0)
	{
		perror("SYSERR: setsockopt RCVBUF");
		return -1;
	}
#endif

#endif

 return 0;
}

int new_descriptor(int s)
{
	socket_t desc;
	int sockets_connected = 0;
	socklen_t i;
	int error = 0;
	static int last_desc = 0; /* last descriptor number */
	struct descriptor_data *newd;
	struct sockaddr_in peer;
	struct hostent *from;

	/* accept the new connection */
	i = sizeof(peer);
	
	if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET)
	{
		perror("accept");
		return -1;
	}
	
	/* keep it from blocking */
	nonblock(desc);

	/* set the send buffer size */
	if (set_sendbuf(desc) < 0)
	{
		CLOSE_SOCKET(desc);
		error++;
		return 0;
	}

	/* make sure we have room for it */
	for (newd = descriptor_list; newd; newd = newd->next)
		sockets_connected++;

	if (sockets_connected >= max_players)
	{
		write_to_descriptor(desc, "Sorry, Kinslayer MUD is full right now... please try again later!\r\n");
		CLOSE_SOCKET(desc);
		error++;
		return 0;
	}
 
	/* create a new descriptor */
	CREATE(newd, struct descriptor_data, 1);

	/* find the sitename */
	if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
		sizeof(peer.sin_addr), AF_INET)))
	{

		/* resolution failed */
		if (!nameserver_is_slow)
			perror("gethostbyaddr");

		 /* find the numeric site address */
		strncpy(newd->host, inet_ntoa(peer.sin_addr), HOST_LENGTH);
		*(newd->host + HOST_LENGTH) = '\0';
	}
	
	else
	{
		strncpy(newd->host, from->h_name, HOST_LENGTH);
		*(newd->host + HOST_LENGTH) = '\0';
	}

	if(LOCAL_IP(newd))
		error++;

	if(error)
		import_host(newd);

	/* determine if the site is banned */
	if (isbanned(newd->host) == BAN_ALL)
	{
		CLOSE_SOCKET(desc);
		sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
		mudlog(buf2, CMP, LVL_GOD, TRUE);
		delete[] (newd);
		return 0;
	}

	#if 0
		/* Log new connections - probably unnecessary, but you may want it */
		sprintf(buf2, "New connection from [%s]", newd->host);
		mudlog(buf2, CMP, LVL_GOD, FALSE);
	#endif

	/* initialize descriptor data */
	newd->descriptor = desc;
	newd->connected = CON_GET_NAME;
	newd->idle_tics = 0;
// Serai - 07/22/04 - Nooooooooooo!
// If we set new descriptors wait to 1, they don't have a char_data yet
//   and its never decremented, so they lag out at the login screen...
//	newd->wait = 1;
	newd->wait = 0;
	newd->output = newd->small_outbuf;
	newd->bufspace = SMALL_BUFSIZE - 1;
	newd->login_time = time(0);
	*newd->output = '\0';
	newd->bufptr = 0;
	newd->has_prompt = 1;  /* prompt is part of greetings */
	newd->history_pos = 0;

	/*
	* This isn't exactly optimal but allows us to make a design choice.
	* Do we embed the history in descriptor_data or keep it dynamically
	* allocated and allow a user defined history size?
	*/
	CREATE(newd->history, char *, HISTORY_SIZE);

	if (++last_desc == 1000)
		last_desc = 1;
	
	newd->desc_num = last_desc;

	/* prepend to list */
	newd->next = descriptor_list;
	descriptor_list = newd;

	SEND_TO_Q(startup, newd);

	SEND_TO_Q("By what name do you wish to be known? ", newd);

	return 0;
}


/*
* Send all of the output that we've accumulated for a player out to
* the player's descriptor.
* FIXME - This will be rewritten before 3.1, this code is dumb.
*/
int process_output(struct descriptor_data *t)
{
	char i[MAX_SOCK_BUF];
	int result;

	/* we may need this \r\n for later -- see below */
	strcpy(i, "\r\n");

	/* now, append the 'real' output */
	strcpy(i + 2, t->output);

	/* if we're in the overflow state, notify the user */
	if (t->bufptr < 0)
		strcat(i, "**OVERFLOW**\r\n");

	/* add the extra CRLF if the person isn't in compact mode */
	if (STATE(t) == CON_PLAYING && t->character && !PRF_FLAGGED(t->character, PRF_COMPACT))
		strcat(i + 2, "\r\n");

	/* add a prompt */
	strncat(i + 2, make_prompt(t), MAX_PROMPT_LENGTH);

	/*
	now, send the output.  If this is an 'interruption', use the prepended
	CRLF, otherwise send the straight output sans CRLF.
	*/
 
	if (t->has_prompt) /* && !t->connected) */
		result = write_to_descriptor(t->descriptor, i);
 
	else
		result = write_to_descriptor(t->descriptor, i + 2);

	/* handle snooping: prepend "% " and send to snooper */
	if (t->snoop_by) {
		SEND_TO_Q("% ", t->snoop_by);
		SEND_TO_Q(t->output, t->snoop_by);
	}
	/*
	if we were using a large buffer, put the large buffer on the buffer pool
	and switch back to the small one
	*/
 
	if (t->large_outbuf) {
		release_buffer(t->large_outbuf);
		t->output = t->small_outbuf;
	}
 
	/* reset total bufspace back to that of a small buffer */
	t->bufspace = SMALL_BUFSIZE - 1;
	t->bufptr = 0;
	*(t->output) = '\0';

	return result;
}


/*
* perform_socket_write: takes a descriptor, a pointer to text, and a
* text length, and tries once to send that text to the OS.  This is
* where we stuff all the platform-dependent stuff that used to be
* ugly #ifdef's in write_to_descriptor().
*
* This function must return:
*
* -1  If a fatal error was encountered in writing to the descriptor.
*  0  If a transient failure was encountered (e.g. socket buffer full).
* >0  To indicate the number of bytes successfully written, possibly
*     fewer than the number the caller requested be written.
*
* Right now there are two versions of this function: one for Windows,
* and one for all other platforms.
*/

#if defined(CIRCLE_WINDOWS)

ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
	ssize_t result;

	result = send(desc, txt, length, 0);

	if (result > 0)
	{
		/* Write was sucessful */
		return result;
	}

	if (result == 0)
	{
		/* This should never happen! */
		log("SYSERR: Huh??  write() returned 0???  Please report this!");
		return -1;
	}

	/* result < 0: An error was encountered. */

	/* Transient error? */
	if (WSAGetLastError() == WSAEWOULDBLOCK)
		return 0;

	/* Must be a fatal error. */
	return -1;
}

#else

#if defined(CIRCLE_ACORN)
#define write socketwrite
#endif

/* perform_socket_write for all Non-Windows platforms */
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
	ssize_t result;

	result = write(desc, txt, length);

	if (result > 0)
	{
		/* Write was successful. */
		return result;
	}

	if (result == 0)
	{
		/* This should never happen! */
		log("SYSERR: Huh??  write() returned 0???  Please report this!");
		return -1;
	}

	/*
	result < 0, so an error was encountered - is it transient?
	Unfortunately, different systems use different constants to
	indicate this.
	*/

#ifdef EAGAIN /* POSIX */
	if (errno == EAGAIN)
		return 0;
#endif

#ifdef EWOULDBLOCK /* BSD */
	if (errno == EWOULDBLOCK)
		return 0;
#endif

#ifdef EDEADLK /* Macintosh */
	if (errno == EDEADLK)
		return 0;
#endif

	/* Looks like the error was fatal.  Too bad. */
	return -1;
}

#endif /* CIRCLE_WINDOWS */

   
/*
* write_to_descriptor takes a descriptor, and text to write to the
* descriptor.  It keeps calling the system-level write() until all
* the text has been delivered to the OS, or until an error is
* encountered.
*
* Returns:
*  0  If all is well and good,
* -1  If an error was encountered, so that the player should be cut off
*/
int write_to_descriptor(socket_t desc, const char *txt)
{
	size_t total;
	ssize_t bytes_written;

	total = strlen(txt);

	/* Log the naughty boys */
/*	for(struct descriptor_data *d = descriptor_list;d;d = d->next)
	{

		if(d->descriptor == desc)
			log_output(d, (char *) txt);
	}
*/
	while (total > 0)
	{
		bytes_written = perform_socket_write(desc, txt, total);

		if (bytes_written < 0)
		{
			/* Fatal error.  Disconnect the player. */
			perror("Write to socket");
			return -1;
		} 
	
		else if (bytes_written == 0)
		{
			/*
			
			Temporary failure -- socket buffer full.  For now we'll just
			cut off the player, but eventually we'll stuff the unsent
			text into a buffer and retry the write later.  JE 30 June 98.
			
			*/
			log("process_output: socket write would block, about to close");
			return -1;
		} 
	
		else {
			txt += bytes_written;
			total -= bytes_written;
		}
	}

	return 0;
}


/*
* Same information about perform_socket_write applies here. I like
* standards, there are so many of them. -gg 6/30/98
*/
ssize_t perform_socket_read(socket_t desc, char *read_point, size_t space_left)
{
	ssize_t ret;

#if defined(CIRCLE_ACORN)
	ret = recv(desc, read_point, space_left, MSG_DONTWAIT);

#elif defined(CIRCLE_WINDOWS)
	ret = recv(desc, read_point, space_left, 0);

#else
	ret = read(desc, read_point, space_left);

#endif

	/* Read was successful. */
	if (ret > 0)
		return ret;

	/* read() returned 0, meaning we got an EOF. */
	if (ret == 0) {
		log("EOF on socket read (connection broken by peer)");
		return -1;
	}

	/*
	read returned a value < 0: there was an error
	*/

#if defined(CIRCLE_WINDOWS) /* Windows */
	if (WSAGetLastError() == WSAEWOULDBLOCK)
		return 0;
#else

#ifdef EINTR /* Interrupted system call - various platforms */
	if (errno == EINTR)
		return 0;
#endif

#ifdef EAGAIN /* POSIX */
	if (errno == EAGAIN)
		return 0;
#endif

#ifdef EWOULDBLOCK /* BSD */
	if (errno == EWOULDBLOCK)
		return 0;
#endif /* EWOULDBLOCK */

#ifdef EDEADLK /* Macintosh */
	if (errno == EDEADLK)
		return 0;
#endif

#endif /* CIRCLE_WINDOWS */

	/* We don't know what happened, cut them off. */
	perror("process_input: about to lose connection");
	return -1;
}

/*
* ASSUMPTION: There will be no newlines in the raw input buffer when this
* function is called.  We must maintain that before returning.
*/

/* If the player is set to log, then LOG! :) Tulon - 3-31-2004 */
void log_output(struct descriptor_data *d, char *buffer)
{


	FILE *logger;
	char file_name[MAX_INPUT_LENGTH];
	char str[MAX_STRING_LENGTH];

	for(int i = 0;*buffer;buffer++)
	{

		if(*buffer == '\r')
			continue;


		
		str[i] = *buffer;
		i++;
	}

	strcpy(str + strlen(str), "\0");

	if(d->character && PLR_FLAGGED(d->character, PLR_LOGGER))
	{
	
		sprintf(file_name, "%s%s", LIB_LOGS, GET_NAME(d->character));

		if(!(logger = fopen(file_name, "a+")))
		{
			sprintf(buf, "Error opening log file for %s.", GET_NAME(d->character));
			mudlog(buf, NRM, LVL_BLDER, TRUE);
			return;
		}

		fprintf(logger, "%s", str);
		fclose(logger);
	}
}

int process_input(struct descriptor_data *t)
{
	int buf_length, failed_subst;
	ssize_t bytes_read;
	size_t space_left;
	char *ptr, *read_point, *write_point, *nl_pos = nullptr;
	char tmp[MAX_INPUT_LENGTH + 8];

	/* first, find the point where we left off reading data */
	buf_length = strlen(t->inbuf);
	read_point = t->inbuf + buf_length;
	space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

	do
	{
		if (space_left <= 0)
		{
			log("process_input: about to close connection: input overflow");
			return -1;
		}

		bytes_read = perform_socket_read(t->descriptor, read_point, space_left);

		if (bytes_read < 0) /* Error, disconnect them. */
			return -1;

		else if (bytes_read == 0) /* Just blocking, no problems. */
			return 0;

		/* at this point, we know we got some data from the read */

		*(read_point + bytes_read) = '\0'; /* terminate the string */

		/* search for a newline in the data we just read */

		for (ptr = read_point; *ptr && !nl_pos; ptr++)
			if (ISNEWL(*ptr))
				nl_pos = ptr;

		read_point += bytes_read;
		space_left -= bytes_read;

/*
* on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
* causing the MUD to hang when it encounters input not terminated by a
* newline.  This was causing hangs at the Password: prompt, for example.
* I attempt to compensate by always returning after the _first_ read, instead
* of looping forever until a read returns -1.  This simulates non-blocking
* I/O because the result is we never call read unless we know from select()
* that data is ready (process_input is only called if select indicates that
* this descriptor is in the read set).  JE 2/23/95.
*/

#if !defined(POSIX_NONBLOCK_BROKEN)
	} while (nl_pos == nullptr);
#else
	} while (0);

	if (nl_pos == nullptr)
		return 0;
#endif /* POSIX_NONBLOCK_BROKEN */

 /*
  * okay, at this point we have at least one newline in the string; now we
  * can copy the formatted data to a new array for further processing.
  */

	read_point = t->inbuf;

	while (nl_pos != nullptr)
	{
		write_point = tmp;
		space_left = MAX_INPUT_LENGTH - 1;

		for (ptr = read_point; (space_left > 0) && (ptr < nl_pos); ptr++)
		{
			if (*ptr == '\b' || *ptr == 127)
			{ /* handle backspacing or delete key */
				if (write_point > tmp)
				{
					if (*(--write_point) == '$')
					{
						write_point--;
						space_left += 2;
					} 

					else
						space_left++;
				}
			} 

			else if ((isascii(*ptr) && isprint(*ptr)) ||
			        (*ptr == '\x1B' && t->character && GET_LEVEL(t->character) > LVL_GRGOD))
			{
				if ((*(write_point++) = *ptr) == '$')
				{ /* copy one character */
					*(write_point++) = '$'; /* if it's a $, double it */
					space_left -= 2;
				} 

				else
					space_left--;
			}
		}

		*write_point = '\0';

		if ((space_left <= 0) && (ptr < nl_pos))
		{
			char buffer[MAX_INPUT_LENGTH + 64];

			sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);

			if (write_to_descriptor(t->descriptor, buffer) < 0)
				return -1;
		}

		if (t->snoop_by)
		{
			SEND_TO_Q("% ", t->snoop_by);
			SEND_TO_Q(tmp, t->snoop_by);
			SEND_TO_Q("\r\n", t->snoop_by);
		}

		failed_subst = 0;

		if (*tmp == '!' && !(*(tmp + 1))) /* Redo last command. */
			strcpy(tmp, t->last_input);

		else if (*tmp == '!' && *(tmp + 1))
		{
			char *commandln = (tmp + 1);
			int starting_pos = t->history_pos;
			int cnt = (t->history_pos == 0 ? HISTORY_SIZE - 1 : t->history_pos - 1);

			skip_spaces(&commandln);

			for (; cnt != starting_pos; cnt--)
			{
				if (t->history[cnt] && is_abbrev(commandln, t->history[cnt]))
				{
					strcpy(tmp, t->history[cnt]);
					strcpy(t->last_input, tmp);
					break;
				}

				if (cnt == 0) /* At top, loop to bottom. */
					cnt = HISTORY_SIZE;
			}
		} 

		else if (*tmp == '^')
		{
			if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
				strcpy(t->last_input, tmp);
		} 

		else
		{
			strcpy(t->last_input, tmp);

			if (t->history && t->history[t->history_pos])
				delete[] (t->history[t->history_pos]); /* Clear the old line. */

			t->history[t->history_pos] = str_dup(tmp); /* Save the new. */

			if (++t->history_pos >= HISTORY_SIZE) /* Wrap to top. */
				t->history_pos = 0;
		}

		if (!failed_subst)
			write_to_q(tmp, &t->input, 0);

		/* find the end of this line */
		while (ISNEWL(*nl_pos))
			nl_pos++;

		/* see if there's another newline in the input buffer */
		read_point = ptr = nl_pos;

		for (nl_pos = nullptr; *ptr && !nl_pos; ptr++)
			if (ISNEWL(*ptr))
				nl_pos = ptr;
	}

	/* now move the rest of the buffer up to the beginning for the next pass */
	write_point = t->inbuf;

	while (*read_point)
		*(write_point++) = *(read_point++);

	*write_point = '\0';

	return 1;
}



/* perform substitution for the '^..^' csh-esque syntax orig is the
* orig string, i.e. the one being modified.  subst contains the
* substition string, i.e. "^telm^tell"
*/
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
	char newsub[MAX_INPUT_LENGTH + 5];

	char *first, *second, *strpos;

	/*
	 * first is the position of the beginning of the first string (the one
	 * to be replaced
	 */
 
	first = subst + 1;

	/* now find the second '^' */
	if (!(second = strchr(first, '^')))
	{
		SEND_TO_Q("Invalid substitution.\r\n", t);
		return 1;
	}
 
	/* terminate "first" at the position of the '^' and make 'second' point
	 * to the beginning of the second string */
	*(second++) = '\0';

	/* now, see if the contents of the first string appear in the original */
	if (!(strpos = strstr(orig, first)))
	{
		SEND_TO_Q("Invalid substitution.\r\n", t);
		return 1;
	}
 
	/* now, we construct the new string for output. */

	/* first, everything in the original, up to the string to be replaced */
	strncpy(newsub, orig, (strpos - orig));
	newsub[(strpos - orig)] = '\0';

	/* now, the replacement string */
	strncat(newsub, second, (MAX_INPUT_LENGTH - strlen(newsub) - 1));

	/* now, if there's anything left in the original after the string to
	 * replaced, copy that too. */
	if (((strpos - orig) + strlen(first)) < strlen(orig))
		strncat(newsub, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(newsub) - 1));

	/* terminate the string in case of an overflow from strncat */
	newsub[MAX_INPUT_LENGTH - 1] = '\0';
	strcpy(subst, newsub);

	return 0;
}

void char_checkup()
{
	struct descriptor_data *d;
	char_data *ch;

	for(d = descriptor_list;d;d = d->next)
	{

		ch = d->character;

		if(!ch)
			return;
		else
			if(MOUNT(ch) && MOUNT(ch)->in_room != ch->in_room) {
				RIDDEN_BY(MOUNT(ch)) = nullptr;
				MOUNT(ch) = nullptr;
			}

		if(!CHECK_WAIT(ch) && FIGHTING(ch) && GET_POS(ch) == POS_SITTING)
			change_pos(ch, POS_FIGHTING);
	}

}

void show_timer_to_char(struct descriptor_data *d) {
	int timer = (int) (d->timer);

	if(!((timer % 4)))
		send(d->descriptor, "\r\n * ", 5, 0);

	else if(!((timer + 1) % 4))
		send(d->descriptor, " = ", 3, 0);

	else if(!((timer + 2) % 4))
		send(d->descriptor, " + ", 3, 0);

	else if(!((timer + 3) % 4))
		send(d->descriptor, " - ", 3, 0);
	
	return;
}

void check_timers()
{
	struct descriptor_data *d;
	char cmdline[MAX_INPUT_LENGTH];

	for (d = descriptor_list; d; d = d->next)
	{

		// ignore those with timer at zero
		if ( d->timer <= 0.0)
		{
			continue;
		}

		// ignore those without a command
		if ( d->delayed_command[0] == '\0')
		{
			continue;
		}

		show_timer_to_char(d);


		d->timer -= .5;
		//d->has_prompt = 1;

		if (d->timer <= 0.0 && d->command_ready)
		{
			// timer reached end
			strcpy( cmdline, d->delayed_command);
			d->forced = 1;
			command_interpreter(d->character, cmdline);
			d->forced = 0;

			if(d->timer <= 0.0)
			{
				strcpy( d->delayed_command, "");
				d->command_ready = 0;
				d->timer = 0.0;
				d->delayed_state = 0;
				d->forced = 0;
			}
		}
	}
}


void close_socket(struct descriptor_data *d)
{
	char buf[128];
	struct descriptor_data *temp;

	REMOVE_FROM_LIST(d, descriptor_list, next);
	CLOSE_SOCKET(d->descriptor);
	flush_queues(d);

	/* Forget snooping */
	if (d->snooping)
	d->snooping->snoop_by = nullptr;

	if (d->snoop_by)
	{
		SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
		d->snoop_by->snooping = nullptr;
	}

	/*. Kill any OLC stuff .*/
	switch(d->connected)
	{
		case CON_OEDIT:
		case CON_REDIT:
		case CON_ZEDIT:
		case CON_MEDIT:
		case CON_SEDIT:
		case CON_HEDIT:
		case CON_AEDIT:
		case CON_TRIGEDIT:
			cleanup_olc(d, CLEANUP_ALL);
   
		default:
			break;
	}

	if (d->character)
	{
		/*
		 Plug memory leak, from Eric Green.
		*/
   
		if (PLR_FLAGGED(d->character, PLR_MAILING) && d->str)
		{
			if (*(d->str))
				delete[] (*(d->str));
     
			delete[] (d->str);
		}
   
		if (STATE(d) == CON_PLAYING || STATE(d) == CON_DISCONNECT)
		{
			d->character->save();
			act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
			sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
			d->character->desc = nullptr;
		}
		
		else
		{
			sprintf(buf, "Losing player: %s.",
			GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
			mudlog(buf, CMP, MAX(GET_INVIS_LEV(d->character), LVL_APPR), TRUE);
		}
	} 
	
	else
		mudlog("Losing descriptor without char.", CMP, LVL_IMMORT, TRUE);

	/* JE 2/22/95 -- part of my unending quest to make switch stable */
 
	if (d->original && d->original->desc)
		d->original->desc = nullptr;

	/* Clear the command history. */
	if (d->history)
	{
		for (int cnt = 0; cnt < HISTORY_SIZE; cnt++)
			if (d->history[cnt])
				delete[] (d->history[cnt]);
			
		delete[] (d->history);
	}

	if (d->showstr_head)
		delete[] (d->showstr_head);
 
	if (d->showstr_count)
		delete[] (d->showstr_vector);

	delete[] (d);
}

void check_idle_passwords(void)
{
	struct descriptor_data *d, *next_d;

	for (d = descriptor_list; d; d = next_d)
	{
		next_d = d->next;
   
		if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME)
			continue;
   
		if (!d->idle_tics)
		{
			d->idle_tics++;
			continue;
		} 
		
		else {
			echo_on(d);
			SEND_TO_Q("\r\nTimed out... goodbye.\r\n", d);
			STATE(d) = CON_CLOSE;
		}
	}
}

/*
* I tried to universally convert Circle over to POSIX compliance, but
* alas, some systems are still straggling behind and don't have all the
* appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
* O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
* this and various other NeXT fixes.)
*/

#if defined(CIRCLE_WINDOWS)

void nonblock(socket_t s)
{
	unsigned long val;

	val = 1;
	ioctlsocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_AMIGA)

void nonblock(socket_t s)
{
	long val;

	val = 1;
	IoctlSocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_ACORN)

void nonblock(socket_t s)
{
	int val = 1;

	socket_ioctl(s, FIONBIO, &val);
}

#elif defined(CIRCLE_UNIX) || defined(CIRCLE_OS2) || defined(CIRCLE_MACINTOSH)

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(socket_t s)
{
	int flags;

	flags = fcntl(s, F_GETFL, 0);
	flags |= O_NONBLOCK;
 
	if (fcntl(s, F_SETFL, flags) < 0)
	{
		perror("SYSERR: Fatal error executing nonblock (comm.c)");
		exit(1);
	}
}

#endif  /* CIRCLE_UNIX || CIRCLE_OS2 || CIRCLE_MACINTOSH */


/* ******************************************************************
*  signal-handling functions (formerly signals.c).  UNIX only.      *
****************************************************************** */

#if defined(CIRCLE_UNIX) || defined(CIRCLE_MACINTOSH)

RETSIGTYPE reread_wizlists(int sig)
{
	mudlog("Signal received - rereading wizlists.", CMP, LVL_IMMORT, TRUE);
//	reboot_wizlists();
}


RETSIGTYPE unrestrict_game(int sig)
{
	mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
	BRF, LVL_IMMORT, TRUE);
	ban_list = nullptr;
	circle_restrict = 0;
	num_invalid = 0;
}

#ifdef CIRCLE_UNIX

/* clean up our zombie kids to avoid defunct processes */
RETSIGTYPE reap(int sig)
{
	while (waitpid(-1, nullptr, WNOHANG) > 0);

	my_signal(SIGCHLD, reap);
}

RETSIGTYPE checkpointing(int sig)
{
	if (!tics)
	{
		log("SYSERR: CHECKPOINT shutdown: tics not updated. (Infinite loop suspected)");
		abort();
	}
	
	else
		tics = 0;
}

RETSIGTYPE hupsig(int sig)
{
	log("SYSERR: Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
	exit(1); /* perhaps something more elegant should
	* substituted */
}


/*
* This is an implementation of signal() using sigaction() for portability.
* (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
* Programming in the UNIX Environment_.  We are specifying that all system
* calls _not_ be automatically restarted for uniformity, because BSD systems
* do not restart select(), even if SA_RESTART is used.
*
* Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
* I just define it to be the old signal.  If your system doesn't have
* sigaction either, you can use the same fix.
*
* SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
*/

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc * func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

#ifdef SA_INTERRUPT
	act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;

	return oact.sa_handler;
}
#endif /* POSIX */


void signal_setup(void)
{
#ifndef CIRCLE_MACINTOSH
	struct itimerval itime;
	struct timeval interval;

	/* user signal 1: reread wizlists.  Used by autowiz system. */
	my_signal(SIGUSR1, reread_wizlists);

	/*
	 * user signal 2: unrestrict game.  Used for emergencies if you lock
	 * yourself out of the MUD somehow.  (Duh...)
	 */
	my_signal(SIGUSR2, unrestrict_game);

	/*
	 * set up the deadlock-protection so that the MUD aborts itself if it gets
	 * caught in an infinite loop for more than 3 minutes.
	 */

	interval.tv_sec = 180;
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, nullptr);
	my_signal(SIGVTALRM, checkpointing);

	/* just to be on the safe side: */
	my_signal(SIGHUP, hupsig);
	my_signal(SIGCHLD, reap);

#endif /* CIRCLE_MACINTOSH */
	my_signal(SIGINT, hupsig);
	my_signal(SIGTERM, hupsig);
	my_signal(SIGPIPE, SIG_IGN);
	my_signal(SIGALRM, SIG_IGN);
}

#endif /* CIRCLE_UNIX */
#endif /* CIRCLE_UNIX || CIRCLE_MACINTOSH */

/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */

void message(char_data *ch, const char *messg, ...)
{
	if (ch->desc && messg && *messg)
	{
		va_list args;

		va_start(args, messg);
		vwrite_to_output(ch->desc, TRUE, messg, args);
		va_end(args);
	}
}

void send_to_char(const char *messg, char_data *ch)
{
	if (ch->desc && messg)
		SEND_TO_Q(messg, ch->desc);
}


void send_to_all(const char *messg)
{
	struct descriptor_data *i;

	if (messg == nullptr)
		return;

	for (i = descriptor_list; i; i = i->next)
		if (STATE(i) == CON_PLAYING)
			SEND_TO_Q(messg, i);
}


void send_to_outdoor(const char *messg)
{
	struct descriptor_data *i;

	if (!messg || !*messg)
		return;

	for (i = descriptor_list; i; i = i->next)
	{
		if (STATE(i) != CON_PLAYING || i->character == nullptr)
			continue;
   
		if (!AWAKE(i->character) || !OUTSIDE(i->character))
			continue;
   
		SEND_TO_Q(messg, i);
	}
}

void send_to_room(const char *messg, int room)
{
	char_data *i;

	if (messg == nullptr)
		return;

	for (i = world[room].people; i; i = i->next_in_room)
		if (i->desc)
			SEND_TO_Q(messg, i->desc);
}



const char *ACTnullptr = "<nullptr>";

#define CHECK_nullptr(pointer, expression) \
 if ((pointer) == nullptr) i = ACTnullptr; else i = (expression);


/* higher-level communication: the act() function */
void perform_act(const char *orig, char_data *ch, struct obj_data *obj,
const void *vict_obj, char_data *to)

{

	const char *i = nullptr;
	char lbuf[MAX_STRING_LENGTH], *buf;
	char_data *dg_victim = nullptr;
	struct obj_data *dg_target = nullptr;
	char *dg_arg = nullptr;

	buf = lbuf;

	for (;;)
	{
		if (*orig == '$')
		{
			switch (*(++orig))
			{
    	
			case 'n':
				i = PERS(ch, to);
				break;
     
			case 'N':
				CHECK_nullptr(vict_obj, PERS((char_data *) vict_obj, to));
				dg_victim = (char_data *) vict_obj;
				break;
     
			case 'm':
				i = HMHR(ch);
				break;
			
			case 'M':
				CHECK_nullptr(vict_obj, HMHR((char_data *) vict_obj));
				dg_victim = (char_data *) vict_obj;
				break;
     
			case 's':
				i = HSHR(ch);
				break;
     
			case 'S':
				CHECK_nullptr(vict_obj, HSHR((char_data *) vict_obj));
				dg_victim = (char_data *) vict_obj;
				break;
     
			case 'e':
				i = HSSH(ch);
				break;
     
			case 'E':
				CHECK_nullptr(vict_obj, HSSH((char_data *) vict_obj));
				dg_victim = (char_data *) vict_obj;
				break;
     
			case 'o':
				CHECK_nullptr(obj, OBJN(obj, to));
				break;
     
			case 'O':
				CHECK_nullptr(vict_obj, OBJN((struct obj_data *) vict_obj, to));
				dg_target = (struct obj_data *) vict_obj;
				break;
     
			case 'p':
				CHECK_nullptr(obj, OBJS(obj, to));
				break;
     
			case 'P':
				CHECK_nullptr(vict_obj, OBJS((struct obj_data *) vict_obj, to));
				dg_target = (struct obj_data *) vict_obj;
				break;
     
			case 'a':
				CHECK_nullptr(obj, SANA(obj));
				break;
     
			case 'A':
				CHECK_nullptr(vict_obj, SANA((struct obj_data *) vict_obj));
				dg_target = (struct obj_data *) vict_obj;
				break;
     
			case 'T':
				CHECK_nullptr(vict_obj, (char *) vict_obj);
				dg_arg = (char *) vict_obj;
				break;
     
			case 't':
				CHECK_nullptr(obj, (char *) obj);
				break;
     
			case 'F':
				CHECK_nullptr(vict_obj, fname((char *) vict_obj));
				break;
     
			case '$':
				i = "$";
				break;
     
			default:
				log("SYSERR: Illegal $-code to act(): %c", *orig);
				log("SYSERR: %s", orig);
				break;
			}
     
			while ((*buf = *(i++)))
				buf++;
	
			orig++;
		}
		
		else if (!(*(buf++) = *(orig++)))
			break;
	}

	*(--buf) = '\r';
	*(++buf) = '\n';
	*(++buf) = '\0';

	if (to->desc)
		SEND_TO_Q(CAP(lbuf), to->desc);

	if (IS_NPC(to))
		act_mtrigger(to, lbuf, ch, dg_victim, obj, dg_target, dg_arg);
}


/* moved this to utils.h --- mah */
	#ifndef SENDOK
		#define SENDOK(ch) ((ch)->desc && (to_sleeping || AWAKE(ch)) && \
		!PLR_FLAGGED((ch), PLR_WRITING))
	#endif

void act(const char *str, int hide_invisible, char_data *ch,
struct obj_data *obj, const void *vict_obj, int type)
{

	char_data *to = nullptr;
	int to_sleeping;

	if (!str || !*str)
		return;

	if (!(dg_act_check = !(type & DG_NO_TRIG)))
		type &= ~DG_NO_TRIG;

	/*
	* Warning: the following TO_SLEEP code is a hack.
	* 
	* I wanted to be able to tell act to deliver a message regardless of sleep
	* without adding an additional argument.  TO_SLEEP is 128 (a single bit
	* high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
	* command.  It's not legal to combine TO_x's with each other otherwise.
	* TO_SLEEP only works because its value "happens to be" a single bit;
	* do not change it to something else.  In short, it is a hack.
	*/

	/* check if TO_SLEEP is there, and remove it if it is. */
	if ((to_sleeping = (type & TO_SLEEP)))
		type &= ~TO_SLEEP;

	if (type == TO_CHAR)
	{
		if (ch && (SENDOK(ch) || IS_NPC(ch)))
			perform_act(str, ch, obj, vict_obj, ch);
			return;
		}

	if (type == TO_VICT)
	{
		if ((to = (char_data *) vict_obj) && SENDOK(to))
			perform_act(str, ch, obj, vict_obj, to);
			return;
	}
 
	/* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

	if (ch && ch->in_room != NOWHERE)
		to = world[ch->in_room].people;
	
	else if (obj && obj->in_room != NOWHERE)
		to = world[obj->in_room].people;
 
	else
	{
		log("SYSERR: no valid target to act()!");
		return;
	}


	for (; to; to = to->next_in_room)
	{
		if (!SENDOK(to) || (to == ch))
			continue;
		
		if (hide_invisible && ch && !CAN_SEE(to, ch))
			continue;
		
		if (type != TO_ROOM && to == vict_obj)
			continue;
		
		perform_act(str, ch, obj, vict_obj, to);
	}
}

/*
* This function is called every 30 seconds from heartbeat().  It checks
* the four global buffers in CircleMUD to ensure that no one has written
* past their bounds.  If our check digit is not there (and the position
* doesn't have a NUL which may result from snprintf) then we gripe that
* someone has overwritten our buffer.  This could cause a false positive
* if someone uses the buffer as a non-terminated character array but that
* is not likely. -gg
*/
#define offset (MAX_STRING_LENGTH - 1)

void sanity_check(void)
{
	int ok = TRUE;

	/*
	* If any line is false, 'ok' will become false also.
	*/
	ok &= (buf[offset] == MAGIC_NUMBER || buf[offset] == '\0');
	ok &= (buf1[offset] == MAGIC_NUMBER || buf1[offset] == '\0');
	ok &= (buf2[offset] == MAGIC_NUMBER || buf2[offset] == '\0');
	ok &= (arg[offset] == MAGIC_NUMBER || arg[offset] == '\0');

   /*
	* This isn't exactly the safest thing to do (referencing known bad memory)
	* but we're doomed to crash eventually, might as well try to get something
	* useful before we go down. -gg
	*/
 
	if (!ok)
		log("SYSERR: *** Buffer overflow! ***\n"
		"buf: %s\nbuf1: %s\nbuf2: %s\narg: %s", buf, buf1, buf2, arg);

	#if 0
		log("Statistics: buf=%d buf1=%d buf2=%d arg=%d",
		strlen(buf), strlen(buf1), strlen(buf2), strlen(arg));
	#endif
}

struct copyover *char_to_copyover(char_data *info)
{
	struct copyover *copy;

	CREATE(copy, struct copyover, 1);

	copy->descriptor = info->desc->descriptor;
	copy->bad_pws = info->desc->bad_pws;
	copy->idle_tics = info->desc->idle_tics;
	copy->connected = info->desc->connected;
	copy->wait = info->desc->wait;
	copy->desc_num = info->desc->desc_num;
	copy->login_time = info->desc->login_time;

	info->save();

	copy->was_in_room = GET_WAS_IN(info);

	strncpy(copy->st.host, info->points.host, HOST_LENGTH);
	copy->st.host[HOST_LENGTH] = '\0';

	if (IN_ROOM(info) == NOWHERE || info->char_specials.timer > idle_void)
		copy->st.player_specials_saved.load_room = NOWHERE;    
	else
		copy->st.player_specials_saved.load_room = GET_ROOM_VNUM(IN_ROOM(info));

	return (copy);
}

char_data *copyover_to_char(struct copyover *copy)
{
	char_data *info;

	CREATE(info, char_data, 1);
	CREATE(info->desc, struct descriptor_data, 1);
	CREATE(info->desc->history, char *, HISTORY_SIZE);

	info->char_specials.position = POS_STANDING;

	info->desc->output = info->desc->small_outbuf;
	info->desc->bufspace = SMALL_BUFSIZE - 1;
	*info->desc->output = '\0';
	info->desc->bufptr = 0;
	info->desc->has_prompt = 1;
	info->desc->history_pos = 0;

	info->desc->character = info;
	info->desc->descriptor = copy->descriptor;
	info->desc->bad_pws = copy->bad_pws;
	info->desc->idle_tics = copy->idle_tics;
	info->desc->connected = copy->connected;
	info->desc->wait = copy->wait;
	info->desc->desc_num = copy->desc_num;
	info->desc->login_time = copy->login_time;

	info->load(copy->st.name);

	GET_WAS_IN(info) = copy->was_in_room;

	return (info);
}

// Serai - 06/18/04 - Copyover for Kinslayer.
int prep_execl()
{
	FILE *copyover_file;
	struct copyover *copy;

	if ( (copyover_file = fopen(".copyover", "wb")) == nullptr)
	{
		sprintf(buf, "Copyover failed:  fopen(.copyover) with error code %d.", errno);
		mudlog(buf, NRM, LVL_IMPL, TRUE);
		return (0);
	}

	fwrite(&mother_desc, sizeof(int), 1, copyover_file);

	for(struct descriptor_data *temp = descriptor_list, *next; temp; temp = next)
	{
		next = temp->next;

		process_output(temp);

		copy = char_to_copyover(temp->character);
		fwrite(copy, sizeof(struct copyover), 1, copyover_file);
		delete[] (copy);
	}

	fclose(copyover_file);

	return (1);
}

void import_host(struct descriptor_data *d)
{
	if(!d || !d->host)
		return;

	sprintf(d->host, "%d.%d.%d.%d", number(20, 198), number(18, 39), number(40, 92), number(1, 12));

}

//A test for later use possibly. //
void send_tm(int till)
{
	struct descriptor_data *d;

	for(d = descriptor_list;d;d = d->next)
	{
		if(d->character && PLR_FLAGGED(d->character, PLR_NOWIZLIST))
		{
			STM(d->character, till);
		}
	}
}
