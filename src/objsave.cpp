/* ************************************************************************
*   File: objsave.c                                     Part of CircleMUD *
*  Usage: loading/saving player objects for rent and crash-save           *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "buffer.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

/* these factors should be unique integers */
#define RENT_FACTOR 	1
#define CRYO_FACTOR 	4

extern str_app_type str_app[];
extern room_data *world;
extern index_data *mob_index;
extern index_data *obj_index;
extern descriptor_data *descriptor_list;
extern player_index_element *player_table;
extern obj_data *boot_object( obj_data obj);
extern obj_data *object_list;
extern int rent_file_timeout, crash_file_timeout;
extern int free_rent;
extern int min_rent_cost;
extern int max_obj_save;	/* change in config.c */

void write_aliases(char_data *ch);
void add_to_watch(char_data *ch);

void obj_from_char(struct object_data *object);

/* Extern functions */
ACMD(do_action);
ACMD(do_tell);
SPECIAL(receptionist);
SPECIAL(cryogenicist);

/* local functions */
int Crash_offer_rent(char_data * ch, char_data * receptionist, int display, int factor);
int Crash_report_unrentables(char_data * ch, char_data * recep, struct obj_data * obj);
void Crash_report_rent(char_data * ch, char_data * recep, struct obj_data * obj, long *cost, long *nitems, int display, int factor);
struct obj_data *Obj_from_store(struct obj_file_elem object);
int Obj_to_store(struct obj_data * obj, FILE * fl);
void update_obj_file(void);
int Crash_write_rentcode(char_data * ch, FILE * fl, struct rent_info * rent);
int gen_receptionist(char_data * ch, char_data * recep, int cmd, char *arg, int mode);
int Crash_save(struct obj_data * obj, FILE * fp);
void Crash_rent_deadline(char_data * ch, char_data * recep, long cost);
void Crash_restore_weight(struct obj_data * obj);
void Crash_extract_objs(struct obj_data * obj);
int Crash_is_unrentable(struct obj_data * obj);
void Crash_extract_norents(struct obj_data * obj);
void Crash_extract_expensive(struct obj_data * obj);
void Crash_calculate_rent(struct obj_data * obj, int *cost);
void Crash_rentsave(char_data * ch, int cost);
void Crash_cryosave(char_data * ch, int cost);
void remove_from_watch(char_data *ch);
void setup_positions(char_data *ch);
int get_watch_time(char_data *ch);
int get_watch_seconds(int ammount);
int get_watch_minutes(int ammount);
int check_multiplaying(char_data *ch);
int find_eq_pos(char_data *ch, obj_data *obj, char *arg);


struct obj_data *Obj_from_store(struct obj_file_elem object)
{
	struct obj_data *obj = 0;
	int j, taeller;

	if(object.item_number == -1)
	{
		return nullptr;
		//obj = create_obj();
	}
	
	else if(object.item_number >= 0)
	{
		obj = read_object(object.item_number, VIRTUAL);
	}

	else
		return nullptr;

	if(!obj)
		return nullptr;
	
	GET_OBJ_VAL(obj, 0) = object.value[0];
	GET_OBJ_VAL(obj, 1) = object.value[1];
	GET_OBJ_VAL(obj, 2) = object.value[2];
	GET_OBJ_VAL(obj, 3) = object.value[3];


	obj->pos = object.pos;

	if(object.item_number == -1)
	{
		
		if(&object.scalp_store)
			obj->scalp = object.scalp_store;
		
		if(*object.obj_store.name)
			obj->name = str_dup(object.obj_store.name);

		else
			obj->name = nullptr;
		
		if(*object.obj_store.description)
			obj->description = str_dup(object.obj_store.description);

		else
			obj->description = nullptr;
		
		if(*object.obj_store.short_description)
			obj->short_description = str_dup(object.obj_store.short_description);

		else
			obj->short_description = nullptr;

		if(*object.obj_store.creator)
			strcpy(obj->creator, str_dup(object.obj_store.creator));
		
		else
			strcpy(obj->creator, "Unknown creator.");
		
		obj->action_description = nullptr;

		if(!(CAN_WEAR(obj, ITEM_WEAR_TAKE)))
			TOGGLE_BIT_AR(GET_OBJ_WEAR(obj), ITEM_WEAR_TAKE);
	}
  
	for(taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
		obj->obj_flags.extra_flags[taeller] = object.extra_flags[taeller];
    
	GET_OBJ_TIMER(obj) = object.timer;
    
	for(taeller = 0; taeller < AF_ARRAY_MAX; taeller++)    
		obj->obj_flags.bitvector[taeller] = object.bitvector[taeller];

    
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
		obj->affected[j] = object.affected[j];
	
	return obj;

}


int Obj_to_store(struct obj_data * obj, FILE * fl)
{
	int j, taeller;
	struct obj_file_elem object;

	object.item_number = GET_OBJ_VNUM(obj);
	object.value[0] = GET_OBJ_VAL(obj, 0);
	object.value[1] = GET_OBJ_VAL(obj, 1);
	object.value[2] = GET_OBJ_VAL(obj, 2);
	object.value[3] = GET_OBJ_VAL(obj, 3);
	object.pos = obj->pos < 0 ? -1 : obj->pos;


	if(GET_OBJ_VNUM(obj) == -1)
	{

		
		if(&obj->scalp)
			object.scalp_store = obj->scalp;
		
		if(obj->name)
			strcpy(object.obj_store.name, obj->name);
		
		if(obj->description)
			strcpy(object.obj_store.description, obj->description);
		
		if(obj->short_description)
			strcpy(object.obj_store.short_description, obj->short_description);

		if(obj->action_description)
			strcpy(object.obj_store.action_description, obj->action_description);

		if(*obj->creator)
			strcpy(object.obj_store.creator, obj->creator);
	
	  
	}


	for(taeller = 0; taeller < EF_ARRAY_MAX; taeller++)
		object.extra_flags[taeller] = obj->obj_flags.extra_flags[taeller];

	object.timer = GET_OBJ_TIMER(obj);
  
	for(taeller = 0; taeller < AF_ARRAY_MAX; taeller ++)
		object.bitvector[taeller] = obj->obj_flags.bitvector[taeller];
  
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
		object.affected[j] = obj->affected[j];

	if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1)
	{
		perror("SYSERR: error writing object in Obj_to_store");
		return 0;
	}
  
	return 1;
}


int Crash_delete_file(char *name)
{
	char filename[50];
	FILE *fl;

	if (!get_filename(name, filename, CRASH_FILE))
		return 0;
	
	if (!(fl = fopen(filename, "rb"))) {
		if (errno != ENOENT) 
		{	/* if it fails but NOT because of no file */
			sprintf(buf1, "SYSERR: deleting crash file %s (1)", filename);
			perror(buf1);
		}
    
		return 0;
	}
  
	fclose(fl);

	if (unlink(filename) < 0)
	{
		if (errno != ENOENT)
		{	/* if it fails, NOT because of no file */
			sprintf(buf1, "SYSERR: deleting crash file %s (2)", filename);
			perror(buf1);
		}
	}
  
	return (1);
}


int Crash_delete_crashfile(char_data * ch)
{
	char fname[MAX_INPUT_LENGTH];
	struct rent_info rent;
	FILE *fl;

	if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
		return 0;
  
	if (!(fl = fopen(fname, "rb")))
	{
    
		if (errno != ENOENT)
		{	/* if it fails, NOT because of no file */
			sprintf(buf1, "SYSERR: checking for crash file %s (3)", fname);
			perror(buf1);
		}
    
		return 0;
	}
  
	if (!feof(fl))
		fread(&rent, sizeof(struct rent_info), 1, fl);
  
	fclose(fl);

	if (rent.rentcode == RENT_CRASH)
		Crash_delete_file(GET_NAME(ch));

	return 1;
}


int Crash_clean_file(char *name)
{
	char fname[MAX_STRING_LENGTH], filetype[20];
	struct rent_info rent;
	FILE *fl;

	if (!get_filename(name, fname, CRASH_FILE))
		return 0;
  /*
   * open for write so that permission problems will be flagged now, at boot
   * time.
   */
  
	if (!(fl = fopen(fname, "r+b")))
	{
		if (errno != ENOENT)
		{	/* if it fails, NOT because of no file */
			sprintf(buf1, "SYSERR: OPENING OBJECT FILE %s (4)", fname);
			perror(buf1);
		}
    
		return 0;
	}
  
	if (!feof(fl))
		fread(&rent, sizeof(struct rent_info), 1, fl);
  
	fclose(fl);

	if ((rent.rentcode == RENT_CRASH) ||
	(rent.rentcode == RENT_FORCED) || (rent.rentcode == RENT_TIMEDOUT))
	{
    
		if (rent.time < time(0) - (crash_file_timeout * SECS_PER_REAL_DAY))
		{
			//if(GET_LEVEL(ch) <= 5) {
      
			Crash_delete_file(name);
      
			switch (rent.rentcode)
			{
			
			case RENT_CRASH:
				strcpy(filetype, "crash");
				break;
      
			case RENT_FORCED:
				strcpy(filetype, "forced rent");
				break;
      
			case RENT_TIMEDOUT:
				strcpy(filetype, "idlesave");
				break;
      
			default:
				strcpy(filetype, "UNKNOWN!");
				break;
			}
      
		log("    Deleting %s's %s file.", name, filetype);
		return 1;
		
		}
    
		/* Must retrieve rented items w/in 30 days */

	} 
	
	else if (rent.rentcode == RENT_RENTED)
		if (rent.time < time(0) - (rent_file_timeout * SECS_PER_REAL_DAY))
		{
			Crash_delete_file(name);
			log("    Deleting %s's rent file.", name);
			return 1;
		}
  
		return (0);
}



void update_obj_file(void)
{
	player_index_element *entry;

	for (entry = player_table;entry;entry = entry->next)
	{
		Crash_clean_file(entry->name);
	}
	return;
}



void Crash_listrent(char_data * ch, char *name)
{
	FILE *fl;
	char fname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
	struct obj_file_elem object;
	struct obj_data *obj;
	struct rent_info rent;
	int offset = 0;

	if (!get_filename(name, fname, CRASH_FILE))
		return;
  
	if (!(fl = fopen(fname, "rb")))
	{
		message(ch, "%s has no rent file.\r\n", name);
		return;
	}
  
	sprintf(buf, "%s\r\n", fname);
  
	if (!feof(fl))
		fread(&rent, sizeof(struct rent_info), 1, fl);
  
	switch (rent.rentcode)
	{
  
	case RENT_RENTED:
		strcat(buf, "Rent\r\n");
		break;
  
	case RENT_CRASH:
		strcat(buf, "Crash\r\n");
		break;
  
	case RENT_CRYO:
		strcat(buf, "Cryo\r\n");
		break;
  
	case RENT_TIMEDOUT:
	case RENT_FORCED:
		strcat(buf, "TimedOut\r\n");
		break;
  
	default:
		strcat(buf, "Undef\r\n");
		break;
	}
  
	while(!feof(fl))
	{
		fread(&object, sizeof(struct obj_file_elem), 1, fl);

		if(ferror(fl))
			break;

		if(!feof(fl))
		{
			obj = Obj_from_store(object);

			if(!obj || !obj->name)
				continue;
			
			offset += sprintf(buf + offset, " [%5d] (%5dau) %-20s\r\n",
			GET_OBJ_VNUM(obj), GET_OBJ_RENT(obj),
			obj->short_description);
			extract_obj(obj);
				
			if (offset > MAX_STRING_LENGTH - 80)
			{
				strcat(buf, "** Excessive rent listing. **\r\n");
				break;
			}
		}
	}
  
	send_to_char(buf, ch);
	fclose(fl);
}



int Crash_write_rentcode(char_data * ch, FILE * fl, struct rent_info * rent)
{
	if (fwrite(rent, sizeof(struct rent_info), 1, fl) < 1)
	{
		perror("SYSERR: writing rent code");
		return 0;
	}
  
	return 1;
}



int Crash_load(char_data * ch, int show)
/* return values:
	0 - successful load, keep char in rent room.
	1 - load failure or load of crash items -- put char in temple.
	2 - rented equipment lost (no $)
*/
{
	FILE *fl;
	char fname[MAX_STRING_LENGTH];
	struct obj_file_elem object;
	struct obj_data *obj;
	struct rent_info rent;
	int cost, orig_rent_code, num_objs = 0;
	float num_of_days;

	if (!get_filename(GET_NAME(ch), fname, CRASH_FILE))
		return 1;
  
	if (!(fl = fopen(fname, "r+b")))
	{
		if (errno != ENOENT) 
		{	/* if it fails, NOT because of no file */
			sprintf(buf1, "SYSERR: READING OBJECT FILE %s (5)", fname);
			perror(buf1);
			send_to_char(	"\r\n********************* NOTICE *********************\r\n"
							"There was a problem loading your objects from disk.\r\n"
							"Contact an immortal for assistance.\r\n", ch);
		}
	
		if(show)
		{
			sprintf(buf, "%s entering game with no equipment.", GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
		}
		
		return 1;
	
	}
  
	if (!feof(fl))
		fread(&rent, sizeof(struct rent_info), 1, fl);

	if (rent.rentcode == RENT_RENTED || rent.rentcode == RENT_TIMEDOUT)
	{
		num_of_days = (float) (time(0) - rent.time) / SECS_PER_REAL_DAY;
		cost = (int) (rent.net_cost_per_diem * num_of_days);
    
		if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch))
		{
			fclose(fl);
			sprintf(buf, "%s entering game, rented equipment lost (no $).",
			GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			Crash_crashsave(ch);
			return 2;
		} 
		
		else
		{
			GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
			GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
			ch->save();
		}
	}

	orig_rent_code = rent.rentcode;

	if(show)
	{
		switch (orig_rent_code)
		{
  
		case RENT_RENTED:
			sprintf(buf, "%s un-renting and entering game.", GET_NAME(ch));
			mudlog(buf, CMP, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			break;
  
		case RENT_CRASH:
			sprintf(buf, "%s retrieving crash-saved items and entering game.", GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			break;
  
		case RENT_CRYO:
			sprintf(buf, "%s un-cryo'ing and entering game.", GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			break;
  
		case RENT_FORCED:
		case RENT_TIMEDOUT:
			sprintf(buf, "%s retrieving force-saved items and entering game.", GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			break;
  
		default:
			sprintf(buf, "WARNING: %s entering game with undefined rent code.", GET_NAME(ch));
			mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
			break;
		}
	}

	if(get_watch_time(ch))
	{
		sprintf(buf, "%s waited %d minutes and %d seconds since logging off.", GET_NAME(ch),
		get_watch_minutes(get_watch_time(ch)), get_watch_seconds(get_watch_time(ch)));

		mudlog(buf, NRM, MAX(LVL_APPR, GET_INVIS_LEV(ch)), TRUE);
	}

	remove_from_watch(ch);	
	check_multiplaying(ch);

	while (!feof(fl))
	{
		fread(&object, sizeof(struct obj_file_elem), 1, fl);
    
		if (ferror(fl))
		{
			perror("SYSERR: Reading crash file: Crash_load");
			fclose(fl);
			return 1;
		}
    
		if (!feof(fl))
		{
			++num_objs;
			obj = Obj_from_store(object);

			if(!obj)
				continue;
		
			if(obj->pos > -1 && obj->pos < NUM_WEARS)
			{
				if(!GET_EQ(ch, obj->pos))
					equip_char(ch, obj, obj->pos);
				
				else
					obj_to_char(obj, ch);

			}

			else
				obj_to_char(obj, ch);

			obj->pos = -1;
				
		}
	}


	if(show)
	{
		/* Little hoarding check. -gg 3/1/98 */
		sprintf(fname, "%s (level %d) has %d objects (max %d).",
			GET_NAME(ch), GET_LEVEL(ch), num_objs, max_obj_save);
		mudlog(fname, CMP, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
	}

	/* turn this into a crash file by re-writing the control block */
	rent.rentcode = RENT_CRASH;
	rent.time = time(0);
	rewind(fl);
	Crash_write_rentcode(ch, fl, &rent);

	fclose(fl);

	if ((orig_rent_code == RENT_RENTED) || (orig_rent_code == RENT_CRYO))
		return 0;
  
	else
		return 1;
}

void setup_positions(char_data *ch)
{

	struct obj_data *obj, *contains;
	int i = 0;

	for(i = 0;i < NUM_WEARS;i++)
	{

		obj = GET_EQ(ch, i);

		if(obj)
		{

			if(obj)
				obj->pos = i;

			if(obj->contains)
				for(contains = obj->contains;contains;contains = contains->next_content)
					contains->pos = -1;
		}
	}

	for(obj = ch->carrying;obj;obj = obj->next_content)
	{
		obj->pos = -1;

		if(obj->contains)
			for(contains = obj->contains;contains;contains = contains->next_content)
				contains->pos = -1;
	}
}
	
	



int Crash_save(struct obj_data * obj, FILE * fp)
{
	int result;

	if (obj)
	{
		Crash_save(obj->contains, fp);
		Crash_save(obj->next_content, fp);
		result = Obj_to_store(obj, fp);

		if (!result)
			return 0;
	}
  
	return TRUE;
}


void Crash_restore_weight(struct obj_data * obj)
{

}



void Crash_extract_objs(struct obj_data * obj)
{
	if (obj)
	{
		Crash_extract_objs(obj->contains);
		Crash_extract_objs(obj->next_content);
		extract_obj(obj);
	}
}


int Crash_is_unrentable(struct obj_data * obj)
{
	if (!obj)
		return 0;

	if (IS_OBJ_STAT(obj, ITEM_NORENT))
		return 1;

	return 0;
}


void Crash_extract_norents(struct obj_data * obj)
{
	char_data *ch;

	if(obj)
	{
		Crash_extract_norents(obj->contains);
		Crash_extract_norents(obj->next_content);
    
		if (Crash_is_unrentable(obj))
		{


			if(obj->carried_by)
				ch = obj->carried_by;

			if(obj->worn_by)
				ch = obj->worn_by;

			/* Let's have a heart, and save people the loss of master weapons over crashes. Tulon: 6-26-2004 */
			if(GET_OBJ_VNUM(obj) != GET_MASTER_WEAPON(ch))
			{
				extract_obj(obj);
			}
	
		}
	}
}


void Crash_extract_expensive(struct obj_data * obj)
{
	struct obj_data *tobj, *max;

	max = obj;
  
	for (tobj = obj; tobj; tobj = tobj->next_content)
		if (GET_OBJ_RENT(tobj) > GET_OBJ_RENT(max))
			max = tobj;
	extract_obj(max);
}



void Crash_calculate_rent(struct obj_data * obj, int *cost)
{
	if (obj)
	{
		*cost += MAX(0, GET_OBJ_RENT(obj));
		Crash_calculate_rent(obj->contains, cost);
		Crash_calculate_rent(obj->next_content, cost);
	}
}


void Crash_crashsave(char_data * ch)
{
	char buf[MAX_INPUT_LENGTH];
	struct rent_info rent;
	int j;
	FILE *fp;

	if (IS_NPC(ch))
		return;

	if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
		return;
	
	if (!(fp = fopen(buf, "wb")))
		return;

	memset(&rent, 0, sizeof(rent_info));
	rent.rentcode = RENT_CRASH;
	rent.time = time(nullptr);
	setup_positions(ch);
  
	if (!Crash_write_rentcode(ch, fp, &rent))
	{
		fclose(fp);
		return;
	}
	
	if (!Crash_save(ch->carrying, fp))
	{
		fclose(fp);
		return;
	}
  
	Crash_restore_weight(ch->carrying);

	for (j = 0; j < NUM_WEARS; j++) {
		if (GET_EQ(ch, j)) {
			if (!Crash_save(GET_EQ(ch, j), fp)) {
				fclose(fp);
				return;
			}

			Crash_restore_weight(GET_EQ(ch, j));

		}
	}
	
	fclose(fp);
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_CRASH);
}


void Crash_idlesave(char_data * ch)
{
	char buf[MAX_INPUT_LENGTH];
	struct rent_info rent;
	struct obj_data;
	int j;
	int cost;
	FILE *fp;

	if (IS_NPC(ch))
		return;

	if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
		return;
  
	if (!(fp = fopen(buf, "wb")))
		return;

	setup_positions(ch);

	for (j = 0; j < NUM_WEARS; j++)
		if (GET_EQ(ch, j))
		{
			obj_to_char(unequip_char(ch, j), ch);
		}

	Crash_extract_norents(ch->carrying);

	cost = 0;
	Crash_calculate_rent(ch->carrying, &cost);
	cost *= 2;			/* forcerent cost is 2x normal rent */
  
	while ((cost > GET_GOLD(ch) + GET_BANK_GOLD(ch)) && ch->carrying)
	{
		Crash_extract_expensive(ch->carrying);
		cost = 0;
		Crash_calculate_rent(ch->carrying, &cost);
		cost *= 2;
	}

	if (!ch->carrying)
	{
		fclose(fp);
		Crash_delete_file(GET_NAME(ch));
		return;
	}
  
	rent.net_cost_per_diem = cost;

	rent.rentcode = RENT_TIMEDOUT;
	rent.time = time(0);
	rent.gold = GET_GOLD(ch);
	rent.account = GET_BANK_GOLD(ch);
  
	if (!Crash_write_rentcode(ch, fp, &rent))
	{
		fclose(fp);
		return;
	}
  
	if (!Crash_save(ch->carrying, fp))
	{
		fclose(fp);
		return;
	}
  
	fclose(fp);
	Crash_extract_objs(ch->carrying);
}


void Crash_rentsave(char_data * ch, int cost)
{
	char buf[MAX_INPUT_LENGTH];
	struct rent_info rent;
	int j;
	FILE *fp;

	if (IS_NPC(ch))
		return;

	if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
		return;
  
	if (!(fp = fopen(buf, "wb")))
		return;

	setup_positions(ch);

	for (j = 0; j < NUM_WEARS; j++)
	{
		if (GET_EQ(ch, j))
		{
			obj_to_char(unequip_char(ch, j), ch);
		}
	}

	Crash_extract_norents(ch->carrying);

	rent.net_cost_per_diem = cost;
	rent.rentcode = RENT_RENTED;
	rent.time = time(0);
	rent.gold = GET_GOLD(ch);
	rent.account = GET_BANK_GOLD(ch);
  
	if (!Crash_write_rentcode(ch, fp, &rent))
	{
		fclose(fp);
		return;
	}
  
	if (!Crash_save(ch->carrying, fp))
	{
		fclose(fp);
		return;
	}
  
	fclose(fp);
	Crash_extract_objs(ch->carrying);
}


void Crash_cryosave(char_data * ch, int cost)
{
	char buf[MAX_INPUT_LENGTH];
	struct rent_info rent;
	int j;
	FILE *fp;

	if (IS_NPC(ch))
		return;

	if (!get_filename(GET_NAME(ch), buf, CRASH_FILE))
		return;
  
	if (!(fp = fopen(buf, "wb")))
		return;

	setup_positions(ch);

	for (j = 0; j < NUM_WEARS; j++)
		if (GET_EQ(ch, j))
			obj_to_char(unequip_char(ch, j), ch);

	Crash_extract_norents(ch->carrying);

	GET_GOLD(ch) = MAX(0, GET_GOLD(ch) - cost);

	rent.rentcode = RENT_CRYO;
	rent.time = time(0);
	rent.gold = GET_GOLD(ch);
	rent.account = GET_BANK_GOLD(ch);
	rent.net_cost_per_diem = 0;
  
	if (!Crash_write_rentcode(ch, fp, &rent))
	{
		fclose(fp);
		return;
	}
  
	if (!Crash_save(ch->carrying, fp))
	{
		fclose(fp);
		return;
	}
  
	fclose(fp);

	Crash_extract_objs(ch->carrying);
	SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
}


/* ************************************************************************
* Routines used for the receptionist					  *
************************************************************************* */

void Crash_rent_deadline(char_data * ch, char_data * recep,
			      long cost)
{
	long rent_deadline;

	if (!cost)
		return;

	rent_deadline = ((GET_GOLD(ch) + GET_BANK_GOLD(ch)) / cost);
	sprintf(buf,
		"$n tells you, 'You can rent for %ld day%s with the gold you have\r\n"
		"on hand and in the bank.'\r\n",
		rent_deadline, (rent_deadline > 1) ? "s" : "");
	act(buf, FALSE, recep, 0, ch, TO_VICT);
}

int Crash_report_unrentables(char_data * ch, char_data * recep,
			         struct obj_data * obj)
{
	char buf[128];
	int has_norents = 0;

	if (obj)
	{
		if (Crash_is_unrentable(obj))
		{
			has_norents = 1;
			sprintf(buf, "$n tells you, 'You cannot store %s.'", OBJS(obj, ch));
			act(buf, FALSE, recep, 0, ch, TO_VICT);
		}
    
		has_norents += Crash_report_unrentables(ch, recep, obj->contains);
		has_norents += Crash_report_unrentables(ch, recep, obj->next_content);
	}
  
	return (has_norents);
}



void Crash_report_rent(char_data * ch, char_data * recep,
		            struct obj_data * obj, long *cost, long *nitems, int display, int factor)
{
	static char buf[256];

	if (obj)
	{
		if (!Crash_is_unrentable(obj))
		{
			(*nitems)++;
			*cost += MAX(0, (GET_OBJ_RENT(obj) * factor));
      
			if (display)
			{
				sprintf(buf, "$n tells you, '%5d coins for %s..'",
				(GET_OBJ_RENT(obj) * factor), OBJS(obj, ch));
				act(buf, FALSE, recep, 0, ch, TO_VICT);
			}
		}
    
		Crash_report_rent(ch, recep, obj->contains, cost, nitems, display, factor);
		Crash_report_rent(ch, recep, obj->next_content, cost, nitems, display, factor);
	}
}



int Crash_offer_rent(char_data * ch, char_data * receptionist,
		         int display, int factor)
{
	char buf[MAX_INPUT_LENGTH];
	int i;
	long totalcost = 0, numitems = 0, norent = 0;

	norent = Crash_report_unrentables(ch, receptionist, ch->carrying);
  
	for (i = 0; i < NUM_WEARS; i++)
		norent += Crash_report_unrentables(ch, receptionist, GET_EQ(ch, i));

	if (norent)
		return 0;

	totalcost = min_rent_cost * factor;

	Crash_report_rent(ch, receptionist, ch->carrying, &totalcost, &numitems, display, factor);

	for (i = 0; i < NUM_WEARS; i++)
		Crash_report_rent(ch, receptionist, GET_EQ(ch, i), &totalcost, &numitems, display, factor);

	if (!numitems)
	{
		act("$n tells you, 'But you are not carrying anything!  Just quit!'",
		FALSE, receptionist, 0, ch, TO_VICT);
		return (0);
	}
  
	if (numitems > max_obj_save)
	{
		sprintf(buf, "$n tells you, 'Sorry, but I cannot store more than %d items.'",
		max_obj_save);
		act(buf, FALSE, receptionist, 0, ch, TO_VICT);
		return (0);
	}
  
	if (display)
	{
		sprintf(buf, "$n tells you, 'Plus, my %d coin fee..'",
		min_rent_cost * factor);
		act(buf, FALSE, receptionist, 0, ch, TO_VICT);
		sprintf(buf, "$n tells you, 'For a total of %ld coins%s.'",
		totalcost, (factor == RENT_FACTOR ? " per day" : ""));
		act(buf, FALSE, receptionist, 0, ch, TO_VICT);
    
		if (totalcost > GET_GOLD(ch) + GET_BANK_GOLD(ch))
		{
			act("$n tells you, '...which I see you can't afford.'",
			FALSE, receptionist, 0, ch, TO_VICT);
			return (0);
		} 
		
		else if (factor == RENT_FACTOR)
			Crash_rent_deadline(ch, receptionist, totalcost);
	}
  
	return (totalcost);
}



int gen_receptionist(char_data * ch, char_data * recep,
		         int cmd, char *arg, int mode)
{
	sh_int save_room;
	struct descriptor_data *d, *next_d;
	const char *action_table[] = { "smile", "dance", "sigh", "blush", "burp",
		"cough", "fart", "twiddle", "yawn" };

	int i = 0, norents = 0;

	if (!ch->desc || IS_NPC(ch))
		return FALSE;

	if (!cmd && !number(0, 5))
	{
		do_action(recep, nullptr, find_command(action_table[number(0, 8)]), 0);
		return FALSE;
	}
  
	if (!CMD_IS("offer") && !CMD_IS("rent"))
		return FALSE;
  
	if (!AWAKE(recep))
	{
		send_to_char("She is unable to talk to you...\r\n", ch);
		return TRUE;
	}
  
	if (!CAN_SEE(recep, ch))
	{
		act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
		return TRUE;
	}

	norents = Crash_report_unrentables(ch, recep, ch->carrying);

	for(i = 0;i < NUM_WEARS;i++)
	{
		if(GET_EQ(ch, i))
		{
			norents += Crash_report_unrentables(ch, recep, GET_EQ(ch, i));
		}
	}

	if(norents)
		return FALSE;

	if(CMD_IS("rent"))
	{
	
		if (IS_NPC(ch) || !ch->desc)
			return FALSE;
  
		if(AFF_FLAGGED(ch, AFF_NOQUIT)) 
		{
			send_to_char("Can't you feel the speed of your heartbeat?\r\n", ch);
			return FALSE;
		}


		if (GET_POS(ch) == POS_FIGHTING)
		{
			send_to_char("No way!  You're fighting for your life!\r\n", ch);
			return FALSE;
		}
	

		if(GET_RACE(ch) != GET_RACE(recep) && !PLR_FLAGGED(ch, PLR_DARKFRIEND))
			return FALSE;

		if (!GET_INVIS_LEV(ch))
			act("$n is helped into a room.", TRUE, ch, 0, 0, TO_ROOM);
		
		sprintf(buf, "%s has rented.", GET_NAME(ch));
		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
		send_to_char("Goodbye, friend.. Come back soon!\r\nYour equipment has been saved!\r\n", ch);


		/*
		 * kill off all sockets connected to the same player as the one who is
		 * trying to quit.  Helps to maintain sanity as well as prevent duping.
		 */
    
		for (d = descriptor_list; d; d = next_d)
		{
			next_d = d->next;
		
			if (d == ch->desc)
				continue;
      
			if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
				STATE(d) = CON_DISCONNECT;
		}

		write_aliases(ch);
		add_to_watch(ch);
		save_room = ch->in_room;
   
		Crash_rentsave(ch, 0);
    
		extract_char(ch);		/* Char is saved in extract char */
		
	}

	/*
  
	if (CMD_IS("rent"))
	{
    
		if (!(cost = Crash_offer_rent(ch, recep, FALSE, mode)))
			return TRUE;
    
		if (mode == RENT_FACTOR)
			sprintf(buf, "$n tells you, 'Rent will cost you %d gold coins per day.'", cost);
    
		else if (mode == CRYO_FACTOR)
			sprintf(buf, "$n tells you, 'It will cost you %d gold coins to be frozen.'", cost);
			
		act(buf, FALSE, recep, 0, ch, TO_VICT);   
	
		if (cost > GET_GOLD(ch) + GET_BANK_GOLD(ch))
		{
			act("$n tells you, '...which I see you can't afford.'",
			FALSE, recep, 0, ch, TO_VICT);
			return TRUE;
		}
    
		if (cost && (mode == RENT_FACTOR))
			Crash_rent_deadline(ch, recep, cost);

		if (mode == RENT_FACTOR)
		{
			act("$n stores your belongings and helps you into your private chamber.",
			FALSE, recep, 0, ch, TO_VICT);
			Crash_rentsave(ch, cost);
			sprintf(buf, "%s has rented (%d/day, %d tot.)", GET_NAME(ch),
			cost, GET_GOLD(ch) + GET_BANK_GOLD(ch));
		} 
		
		else
		{			
			act(	"$n stores your belongings and helps you into your private chamber.\r\n"
					"A white mist appears in the room, chilling you to the bone...\r\n"
					"You begin to lose consciousness...",
			FALSE, recep, 0, ch, TO_VICT);
			Crash_cryosave(ch, cost);
			sprintf(buf, "%s has cryo-rented.", GET_NAME(ch));
			SET_BIT_AR(PLR_FLAGS(ch), PLR_CRYO);
		}

		mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
		act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);
		save_room = ch->in_room;
		extract_char(ch);
		ch->save();
	} 
	
	else
	{
		Crash_offer_rent(ch, recep, TRUE, mode);
		act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
	}

  */
  
	return TRUE;
}


SPECIAL(receptionist)
{
	return (gen_receptionist(ch, (char_data *)me, cmd, argument, RENT_FACTOR));
}


SPECIAL(cryogenicist)
{
	return (gen_receptionist(ch, (char_data *)me, cmd, argument, CRYO_FACTOR));
}


void Crash_save_all(void)
{
	struct descriptor_data *d;
  
	for (d = descriptor_list; d; d = d->next)
	{
		if ((STATE(d) == CON_PLAYING) && !IS_NPC(d->character))
		{
			if (PLR_FLAGGED(d->character, PLR_CRASH))
			{
				Crash_crashsave(d->character);

				d->character->save();
				REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRASH);
			}
		}
	}
}
