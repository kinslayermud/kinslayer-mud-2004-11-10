/* ***********************************************************************
*  File: alias.c				A utility to CircleMUD	 *
* Usage: writing/reading player's aliases.				 *
*									 *
* Code done by Jeremy Hess and Chad Thompson				 *
* Modifed by George Greer for inclusion into CircleMUD bpl15.		 *
*									 *
* Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
* CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.		 *
*********************************************************************** */

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "db.h"

void write_aliases(char_data *ch);
void read_aliases(char_data *ch);
void delete_aliases(const char *charname);
void send_to_all(const char *messg);
extern int free_rent;
extern struct memory_data *mlist;

void write_aliases(char_data *ch)
{
	FILE *file;
	char fn[MAX_STRING_LENGTH];
	struct alias *temp;

	get_filename(GET_NAME(ch), fn, ALIAS_FILE);

	if (GET_ALIASES(ch) == nullptr)
		return;

	if ((file = fopen(fn, "w")) == nullptr)
	{
		log("SYSERR: Couldn't save aliases for %s in '%s'.", GET_NAME(ch), fn);
		perror("SYSERR: write_aliases");
		return;
	}

	for (temp = GET_ALIASES(ch); temp; temp = temp->next)
	{
		fprintf(file, "%s\n", temp->name);
		fprintf(file, "%s\n", temp->replacement);

	}
  
	fclose(file);
}

void read_aliases(char_data *ch)
{   
	FILE *file;
	char fn[MAX_STRING_LENGTH], xbuf[MAX_STRING_LENGTH];
	struct alias *a;
	unsigned int i = 0;

	get_filename(GET_NAME(ch), fn, ALIAS_FILE);

	if (!(file = fopen(fn, "r")))
	{
		if (errno != ENOENT)
		{
			log("SYSERR: Couldn't open alias file '%s' for %s.", fn, GET_NAME(ch));
			perror("SYSERR: read_aliases");
		}
    
		return;
	}

	while(fgets(xbuf, MAX_INPUT_LENGTH, file))
	{
		CREATE(a, struct alias, 1);

		for(i = 0;i < strlen(xbuf);i++)
		{
			if(xbuf[i] == '\n')
				xbuf[i] = '\0';
		}

		strcpy(a->name, xbuf);

		if(!(fgets(xbuf, MAX_INPUT_LENGTH, file)))
		{
			if(a)
			{
				delete[] (a);
				break;
			}
		}

		for(i = 0;i < strlen(xbuf);i++)
		{
			if(xbuf[i] == '\n')
				xbuf[i] = '\0';
		}

		strcpy(a->replacement, xbuf);
		a->next = GET_ALIASES(ch);
		GET_ALIASES(ch) = a;

	}

	fclose(file);
} 

void delete_aliases(char *charname)
{
	char filename[150];

	if (!get_filename(charname, filename, ALIAS_FILE))
		return;

	if (remove(filename) < 0 && errno != ENOENT)
		log("SYSERR: deleting alias file %s: %s", filename, strerror(errno));
}
