#include "../conf.h"
#include "../sysdep.h"

#include "../structs.h"
#include "../utils.h"

#define INDEX_FILE  "index"
#define KIT_PREFIX  "world/kit/"
#define NUM_WEARS   19

struct old_kit_data
{
	char *name;
	int equipment[NUM_WEARS]; /* vnums/rnums */

	int vnum; /* unique kit number */

	struct old_kit_data *prev, *next; /* Doubly linked list */
};

char buf[8 * 1024];
char buf1[8 * 1024];
char buf2[8 * 1024];

struct kit_data *kit_index = nullptr;
struct old_kit_data *old_kit_index = nullptr, *old_kit_end = nullptr;

int count_hash_records(FILE * fl);
void discrete_load(FILE * fl, char *filename);
int get_line(FILE * fl, char *buf);
char *fread_string(FILE * fl, char *error);
void index_boot();
void kit_save_to_disk();
void parse_kit(FILE *fl, int virtual_nr);
char *str_dup(const char *source);

int main()
{
	int count = 0;

	index_boot();

	for(struct old_kit_data *temp = old_kit_index; temp; temp = temp->next)
	{
		printf("Kit VNum [%d] Name [%s]\n", temp->vnum, temp->name);
		++count;
	}

	if (!count)
	{
		printf("No kits.\n");
		exit(1);
	}

	CREATE(kit_index, struct kit_data, count);

	for(int i = 0; i < count; i++)
	{
		kit_index[i].name = str_dup(old_kit_index[i].name);
		kit_index[i].vnum = old_kit_index[i].vnum;

		for(int j = 0; j < NUM_WEARS; j++)
		{
			kit_index[i].equipment[0][j] = old_kit_index[i].equipment[j];
			kit_index[i].percent[0][j] = 100;

			for(int k = 1; k < NUM_OF_KITS; k++)
			{
				kit_index[i].equipment[k][j] = NOTHING;
				kit_index[i].percent[k][j] = 100;
			}
		}
	}

	kit_index[0].prev = nullptr;
	kit_index[0].next = &(kit_index[1]);

	for(int i2 = 1; i2 < count - 1; i2++)
	{
		kit_index[i2].next = &(kit_index[i2 + 1]);
		kit_index[i2].prev = &(kit_index[i2 - 1]);
	}

	if (count > 1)
	{
		kit_index[count - 1].prev = &(kit_index[count - 2]);
		kit_index[count - 1].next = nullptr;
	}

	kit_save_to_disk();

	return (0);
}

int count_hash_records(FILE * fl)
{
	char buf[128];
	int count = 0;

	while (fgets(buf, 128, fl))
		if (*buf == '#')
			count++;

	return count;
}

void index_boot()
{
	const char *index_filename, *prefix;
	FILE *index, *db_file;
	int rec_count = 0;

	prefix = KIT_PREFIX;
	index_filename = INDEX_FILE;

	sprintf(buf2, "%s%s", prefix, index_filename);

	if (!(index = fopen(buf2, "r")))
	{
		sprintf(buf1, "SYSERR: opening index file '%s'\n", buf2);
		perror(buf1);
		exit(1);
	}

	/* first, count the number of records in the file so we can malloc */
	fscanf(index, "%s\n", buf1);
  
	while (*buf1 != '$')
	{
		sprintf(buf2, "%s%s", prefix, buf1);
    
		if (!(db_file = fopen(buf2, "r")))
		{
			perror(buf2);
			printf("SYSERR: File '%s' listed in %s/%s not found.\n", buf2, prefix, index_filename);
			fscanf(index, "%s\n", buf1);
			continue;
		} 
		else
		{
			rec_count += count_hash_records(db_file);
		}

		fclose(db_file);
		fscanf(index, "%s\n", buf1);
	}

	if (!rec_count)
	{
		perror("No kits.\n");
		exit(1);
	}

	int i;

	CREATE(old_kit_index, struct old_kit_data, rec_count);
	printf("   %d kits, %d bytes.\n", rec_count, sizeof(struct old_kit_data) * rec_count);

	old_kit_index[0].prev = nullptr;
	old_kit_index[0].next = &(old_kit_index[1]);

	for(i = 1; i < rec_count - 1; i++)
	{
		old_kit_index[i].next = &(old_kit_index[i + 1]);
		old_kit_index[i].prev = &(old_kit_index[i - 1]);
	}

	old_kit_index[rec_count - 1].prev = &(old_kit_index[rec_count - 2]);
	old_kit_index[rec_count - 1].next = nullptr;

	rewind(index);
	fscanf(index, "%s\n", buf1);
  
	while (*buf1 != '$')
	{
		sprintf(buf2, "%s%s", prefix, buf1);
    
		if (!(db_file = fopen(buf2, "r")))
		{
			perror(buf2);
			exit(1);
		}
    
		discrete_load(db_file, buf2);

		fclose(db_file);
		fscanf(index, "%s\n", buf1);
	}

	fclose(index);
}

void discrete_load(FILE * fl, char *filename)
{
	int nr = -1, last = 0;
	char line[256];

	for (;;)
	{    
		if (!get_line(fl, line))
		{
			if (nr == -1)
			{
				printf("SYSERR: kit file %s is empty!\n", filename);
			}
			else
			{
				printf("SYSERR: Format error in %s after kit #%d\n"
				"...expecting a new kit, but file ended!\n"
				"(maybe the file is not terminated with '$'?)\n", filename, nr);
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
				printf("SYSERR: Format error after kit #%d\n", last);
				exit(1);
			}

			if (nr >= 99999)
				return;

			else
				parse_kit(fl, nr);
		}
		else
		{
			printf("SYSERR: Format error in kit file %s near kit #%d\n", filename, nr);
			printf("...offending line: '%s'", line);
			exit(1);
		}
	}
}

void parse_kit(FILE *fl, int virtual_nr)
{
	int eq[NUM_WEARS];
	static int index_nr = 0;

	old_kit_index[index_nr].vnum = virtual_nr;
	old_kit_index[index_nr].name = fread_string(fl, buf);

	if ( !get_line(fl, buf2) )
	{
		printf("SYSERR: Expecting list of equipment for kit #%d but file ended!\n", virtual_nr);
		exit(1);
	}

	if (sscanf(buf2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d ",
		eq, eq + 1, eq + 2, eq + 3, eq + 4, eq + 5, eq + 6, eq + 7, eq + 8, eq + 9, eq + 10,
		eq + 11, eq + 12, eq + 13, eq + 14, eq + 15, eq + 16, eq + 17, eq + 18) != NUM_WEARS)
	{
		printf("SYSERR: Incorrect number of equipment numbers reading from kit #%d.\n", virtual_nr);
		exit(1);
	}

	for(int i = 0; i < NUM_WEARS; i++)
		old_kit_index[index_nr].equipment[i] = eq[i];

	old_kit_end = &(old_kit_index[index_nr]);
	++index_nr;

	return;
}

char *fread_string(FILE * fl, char *error)
{
	char buf[8 * 1024], tmp[512], *rslt;
	char *point;
	int done = 0, length = 0, templength = 0;

	*buf = '\0';

	do {
		if (!fgets(tmp, 512, fl))
		{
			printf("SYSERR: fread_string: format error at or near %s\n", error);
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

		if (length + templength >= 8 * 1024)
		{
			printf("SYSERR: fread_string: string too large\n");
			printf(error);
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

int get_line(FILE * fl, char *buf)
{
	char temp[256];
	int lines = 0;

	memset(temp, 0, sizeof(temp));

	do {
		lines++;
		fgets(temp, 256, fl);
    
		if (*temp)
			temp[strlen(temp) - 1] = '\0';
	} 
	
	while (!feof(fl) && (*temp == '*' || !*temp));

	if (feof(fl))
	{
		*buf = '\0';
		return 0;
	} 
	
	else
	{
		strcpy(buf, temp);
		return lines;
	}
}

void kit_save_to_disk()
{
	FILE *fp;

	for(struct kit_data *kit = kit_index, *next; kit; kit = next)
	{
		sprintf(buf, "%s/%d.new", KIT_PREFIX, (kit->vnum / 100));
		printf("Creating '%s'\n", buf);

		if ( !(fp = fopen(buf, "w")) )
		{
			printf("SYSERR: OLC: Cannot open kit file '%s'!", buf);
			return;
		}

		next = nullptr;

		for(struct kit_data *temp = kit; temp; temp = temp->next)
		{
			if (temp->vnum >= (kit->vnum - (kit->vnum % 100)) &&
			    temp->vnum <= (kit->vnum - (kit->vnum % 100) + 99))
			{
				fprintf(fp, "#%d\n" "%s~\n", temp->vnum, temp->name);

				for(int j = 0; j < NUM_OF_KITS; j++)
				{
					for(int i = 0; i < NUM_WEARS; i++)
					{
						fprintf(fp, "%d ", temp->equipment[j][i]);
					}

					fprintf(fp, "\n");

					for(int k = 0; k < NUM_WEARS; k++)
					{
						fprintf(fp, "%d ", temp->percent[j][k]);
					}

					fprintf(fp, "\n");
				}
			}
			else
			{
				next = temp;
				break;
			}
		}

		fprintf(fp, "$~\n");

		fclose(fp);

		sprintf(buf2, "%s/%d.kit", KIT_PREFIX, (kit->vnum / 100));

		remove(buf2);
		rename(buf, buf2);
	}

	return;
}

char *str_dup(const char *source)
{
	char *new_z;

	CREATE(new_z, char, strlen(source) + 1);
	return (strcpy(new_z, source));
}
