/*
	Joshua Roys - Serai of 'Kinslayer MUD' and 'Wheel of Time MUD' 2004
	www.kinslayer.mine.nu
	www.wotmud.org
	www.freemud.mine.nu/Serai

04/09/04 22:14 - Started kedit.cpp
04/10/04 17:30 - Can create new kits
04/13/04 22:24 - New faster computer :)
                 Kits save to disk
04/14/04 01:26 - Kits load from disk
04/14/04 03:07 - Appears to be done.  Bugtesting.
07/10/04 02:35 - Added multiple kits and percent loads.  Hopefully.

*/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "olc.h"
#include "dg_olc.h"
#include "constants.h"
#include "interpreter.h"

ACMD(do_klist);

#if 0
#define KEDIT_DEBUG 1
#endif

/* externs */
extern struct obj_data *obj_proto;
extern char_data *mob_proto;
extern int top_of_mobt;
extern struct kit_data *kit_index, *kit_end;

/* local functions */
void kedit_disp_menu(struct descriptor_data *);
void kedit_parse(struct descriptor_data *, char *);
void kedit_save_internally(struct descriptor_data *);
void kedit_save_to_disk(struct descriptor_data *);
void kedit_setup_existing(struct descriptor_data *, struct kit_data *);
void kedit_setup_new(struct descriptor_data *);
struct kit_data *real_kit(int);

ACMD(do_klist)
{
	struct kit_data *k;
	int lo = 0, hi = 99, found = 0;

	two_arguments(argument, buf, buf1);
	strcpy(buf2, "");

	if (*buf)
	{
		if (is_number(buf))
		{
			lo = atoi(buf);

			if (!*buf1)
			{
				hi = lo + 99;
			}
			else
			{
				hi = atoi(buf1);
			}
		}
		else if (!strn_cmp(buf, "all", strlen(buf)))
		{
			hi = 99999;
		}
		else
		{
			send_to_char("Usage: klist [{\"all\" | <start kit #>} [<end kit #>]]\r\n", ch);
			return;
		}
	}

	if (lo < 0 || hi < 0 || lo > 99999 || hi > 99999 || lo > hi)
	{
		send_to_char("Usage: klist [{\"all\" | <start kit #>} [<end kit #>]]\r\n", ch);
		return;
	}

	for(k = kit_index; k; k = k->next)
	{
		if (k->vnum >= lo && k->vnum <= hi)
			sprintf(buf2 + strlen(buf2), "%5d. [%5d] %s\r\n", ++found, k->vnum, k->name);
	}

	if (!found)
		send_to_char("No kits were found in those parameters.\r\n", ch);

	else if (ch->desc)
		page_string(ch->desc, buf2, 1);
}

void kedit_disp_menu(struct descriptor_data *d)
{
	struct kit_data *kit;

	kit = OLC_KIT(d);

	get_char_cols(d->character);
	sprintf(buf,
#if defined(CLEAR_SCREEN)
		"\x1B[2J"
#endif
		"--- Kit Number:  [%s%d%s]  (%d%s kit)\r\n"
		" %sN%s) Name : %s%s\r\n",
		cyn, OLC_KIT(d)->vnum, nrm, OLC_KNUM(d) + 1,
		(OLC_KNUM(d) + 1 > 3 ? "th" : (OLC_KNUM(d) + 1 == 3 ? "rd" : (OLC_KNUM(d) + 1 == 2 ? "nd" : "st"))),
		grn, nrm, yel, kit->name
	);

	for(int i = 0; i < NUM_WEARS; i++)
	{
		sprintf(buf1,
			"%s%2d%s) %s : [%s%3d%%%s] %s%s\r\n",
			grn, i, nrm, where[i],
			cyn, kit->percent[OLC_KNUM(d)][i], nrm,
			yel, (kit->equipment[OLC_KNUM(d)][i] >= 0 ? obj_proto[real_object(kit->equipment[OLC_KNUM(d)][i])].short_description : "Nothing.")
		);
		strcat(buf, buf1);
	}

	sprintf(buf2,
		" %sA%s) Previous kit on mob\r\n"
		" %sD%s) Next kit on mob\r\n"
//		" %sV%s) Vnum an object\r\n",
		" %sQ%s) Quit\r\n"
		"Enter your choice : ",
		grn, nrm, grn, nrm, grn, nrm
	);

	strcat(buf, buf2);

	send_to_char(buf, d->character);

	OLC_MODE(d) = KEDIT_MAIN_MENU;

	return;
}

void kedit_parse(struct descriptor_data *d, char *arg)
{
	int pos = -1, rpos = -1;

	// From act.item.cpp
	int wear_bitvectors[] = {
		ITEM_WEAR_TAKE,
		ITEM_WEAR_TAKE,
		ITEM_WEAR_NECK,
		ITEM_WEAR_NECK,
		ITEM_WEAR_HEAD,
		ITEM_WEAR_ABOUT,
		ITEM_WEAR_BACK,
		ITEM_WEAR_BODY,
		ITEM_WEAR_ARMS,
		ITEM_WEAR_WAIST,
		ITEM_WEAR_WRIST,
		ITEM_WEAR_WRIST,
		ITEM_WEAR_HANDS,
		ITEM_WEAR_FINGER,
		ITEM_WEAR_FINGER,
		ITEM_WEAR_WIELD,
		ITEM_WEAR_SHIELD,
		ITEM_WEAR_LEGS,
		ITEM_WEAR_FEET
	};

	switch(OLC_MODE(d))
	{
	case KEDIT_MAIN_MENU:
		switch(*arg)
		{
		case 'Q':
		case 'q':
			if (OLC_VAL(d))
			{
				send_to_char("Do you wish to save this kit? : ", d->character);
				OLC_MODE(d) = KEDIT_CONFIRM_SAVESTRING;
			}
			else
				cleanup_olc(d, CLEANUP_ALL);

			return;

		case 'N':
		case 'n':
			send_to_char("Enter kit name:-\r\n]", d->character);
			OLC_MODE(d) = KEDIT_NAME;
			return;

		case 'A':
		case 'a':
			if (OLC_KNUM(d) == 0)
				OLC_KNUM(d) = NUM_OF_KITS - 1;

			else
				OLC_KNUM(d) -= 1;

			break;

		case 'D':
		case 'd':
			if (OLC_KNUM(d) == NUM_OF_KITS - 1)
				OLC_KNUM(d) = 0;

			else
				OLC_KNUM(d) += 1;

			break;

		default:
			if (is_number(arg))
			{
				pos = atoi(arg);

				if (pos < 0 || pos >= NUM_WEARS)
				{
					send_to_char("Invalid position!\r\n", d->character);
					break;
				}

				sprintf(buf,
					"%s : [%s%3d%%%s] %s%s%s\r\n",
					equipment_types[pos],
					cyn, OLC_KIT(d)->percent[OLC_KNUM(d)][pos], nrm,
					yel, (OLC_KIT(d)->equipment[OLC_KNUM(d)][pos] >= 0 ? obj_proto[real_object(OLC_KIT(d)->equipment[OLC_KNUM(d)][pos])].short_description : "Nothing."), nrm
				);

				send_to_char(buf, d->character);

				send_to_char("Enter object vnum (0 for Nothing) : ", d->character);

// WARNING:  The below is a hack.  It saves (NUM_WEARS * 3) + 3 lines of code at the minimum.
// Well, not + 3 anymore because of these two comments and the one-line hack.  :p  Serai.
				OLC_MODE(d) = pos + 3;

				return;
			}
			// Will break out to the kedit_disp_menu() at the end of kedit_parse()
			send_to_char("Invalid option!\r\n", d->character);
			break;
		}
		break;

	case KEDIT_ITEM_PERCENT:
		if (!is_number(arg))
		{
			send_to_char("Must be a numeric value, try again : ", d->character);
			return;
		}

		pos = atoi(arg);

		if (pos < 0 || pos > 100)
		{
			send_to_char("Must be from 0 to 100 : ", d->character);
			return;
		}

		if (OLC_KIT(d)->percent[OLC_KNUM(d)][OLC_POS(d)] != pos)
		{
			OLC_VAL(d) = 1;
			OLC_KIT(d)->percent[OLC_KNUM(d)][OLC_POS(d)] = pos;
		}

		break;

	case KEDIT_CONFIRM_SAVESTRING:
		switch(UPPER(*arg))
		{
		case 'Y':
		case 'y':
			kedit_save_internally(d);
			kedit_save_to_disk(d);
			/* fall through */

		case 'N':
		case 'n':
			cleanup_olc(d, CLEANUP_ALL);
			return;

		default:
			send_to_char("Invalid option!\r\nDo you wish to save this kit? : ", d->character);
			return;
		}

		break;

	case KEDIT_NAME:
		OLC_VAL(d) = 1;
		if (OLC_KIT(d)->name)
			delete[] (OLC_KIT(d)->name);
		
		if (strlen(arg) > MAX_KIT_NAME)
			arg[MAX_KIT_NAME - 1] = '\0';
		
		OLC_KIT(d)->name = str_dup((arg && *arg) ? arg : "undefined");
		break;

	case KEDIT_WEAR_LIGHT:
	case KEDIT_WEAR_HOLD:
	case KEDIT_WEAR_NECK_1:
	case KEDIT_WEAR_NECK_2:
	case KEDIT_WEAR_HEAD:
	case KEDIT_WEAR_ABOUT:
	case KEDIT_WEAR_BACK:
	case KEDIT_WEAR_BODY:
	case KEDIT_WEAR_ARMS:
	case KEDIT_WEAR_WAIST:
	case KEDIT_WEAR_WRIST_R:
	case KEDIT_WEAR_WRIST_L:
	case KEDIT_WEAR_HANDS:
	case KEDIT_WEAR_FINGER_R:
	case KEDIT_WEAR_FINGER_L:
	case KEDIT_WEAR_WIELD:
	case KEDIT_WEAR_SHIELD:
	case KEDIT_WEAR_LEGS:
	case KEDIT_WEAR_FEET:
		if (!is_number(arg))
		{
			send_to_char("Must be a numeric value, try again : ", d->character);
			return;
		}

		// WARNING:  The below is the same hack, saving lots more work.  Serai.
		OLC_POS(d) = OLC_MODE(d) - 3;

		if ( (pos = atoi(arg)) == 0)
		{
			OLC_VAL(d) = 1;
			OLC_KIT(d)->equipment[OLC_KNUM(d)][OLC_POS(d)] = NOTHING;
			OLC_KIT(d)->percent[OLC_KNUM(d)][OLC_POS(d)] = 100;
			break;
		}

		rpos = pos;

		if ((pos = real_object(pos)) >= 0)
		{
			if (!CAN_WEAR(&(obj_proto[pos]), wear_bitvectors[OLC_POS(d)]))
			{
				send_to_char("That object can't be worn in this position, try again : ", d->character);
				return;
			}

			if (OLC_MODE(d) == KEDIT_WEAR_LIGHT)
			{
				// From act.item.cpp
				if (GET_OBJ_TYPE(&(obj_proto[pos])) != ITEM_LIGHT)
				{
					send_to_char("That object can't be used as a light, try again : ", d->character);
					return;
				}
			}

			if (OLC_MODE(d) == KEDIT_WEAR_HOLD)
			{
				// From act.item.cpp
				if (!CAN_WEAR(&(obj_proto[pos]), ITEM_WEAR_HOLD)
					&& GET_OBJ_TYPE(&(obj_proto[pos])) != ITEM_WAND
					&& GET_OBJ_TYPE(&(obj_proto[pos])) != ITEM_STAFF
					&& GET_OBJ_TYPE(&(obj_proto[pos])) != ITEM_SCROLL
					&& GET_OBJ_TYPE(&(obj_proto[pos])) != ITEM_POTION)
				{
					send_to_char("That object can't be held, try again : ", d->character);
					return;
				}
			}

			// If it changes, mark the kit as changed for save-on-exit prompt.
			if (OLC_KIT(d)->equipment[OLC_KNUM(d)][OLC_POS(d)] != rpos)
			{
				OLC_VAL(d) = 1;
				OLC_KIT(d)->equipment[OLC_KNUM(d)][OLC_POS(d)] = rpos;
			}

			// Lets load items on percent
			OLC_MODE(d) = KEDIT_ITEM_PERCENT;
			send_to_char("Percent chance to load object : ", d->character);

			return;
		}
		else
		{
			send_to_char("That object does not exist, try again : ", d->character);
			return;
		}

		break;

	default:
		cleanup_olc(d, CLEANUP_ALL);
		mudlog("SYSERR: OLC: Reached default case in kedit_parse() ... Add a new eq position maybe?", BRF, LVL_BUILDER, TRUE);
		return;
	}

	kedit_disp_menu(d);

	return;
}

void kedit_save_internally(struct descriptor_data *d)
{
	struct kit_data *temp = kit_index, *tempPrev, *tempNext, *tempNew;

#if defined(KEDIT_DEBUG)
	fprintf(stdout, "KIT LIST: kedit_save_internally()\n");
	for(struct kit_data *tt = kit_index; tt; tt = tt->next)
	{
		fprintf(stdout, "  kit: name: [%s] vnum[%d]\n", tt->name, tt->vnum);
	}
#endif

	CREATE(tempNew, struct kit_data, 1);

	// Copy over OLC_KIT to a new safe block of memory.
	// OLC_KIT will get freed in cleanup_olc() ...
	// Don't really want a free'd node in our list.
	tempNew->name = strdup(OLC_KIT(d)->name ? OLC_KIT(d)->name : "(null)");

	for(int j = 0; j < NUM_OF_KITS; j++)
	{
		for(int i = 0; i < NUM_WEARS; i++)
		{
			tempNew->equipment[j][i] = OLC_KIT(d)->equipment[j][i];
			tempNew->percent[j][i] = OLC_KIT(d)->percent[j][i];
		}
	}

	tempNew->vnum = OLC_KIT(d)->vnum;

#if defined(KEDIT_DEBUG)
	fprintf(stdout, "saving vnum: [%d]\n", OLC_KIT(d)->vnum);
#endif

	// New kit at beginning
	if (!kit_index || OLC_KIT(d)->vnum < kit_index->vnum)
	{
#if defined(KEDIT_DEBUG)
		fprintf(stdout, "  added before\n");
#endif

		ADD_FIRST_NODE(kit_index, kit_end, tempNew, prev, next);

		return;
	}

	while(temp)
	{

#if defined(KEDIT_DEBUG)
		fprintf(stdout, "temp vnum: [%d]\n", temp->vnum);
#endif

		// Kit exists, remove old and add new.
		if (OLC_KIT(d)->vnum == temp->vnum)
		{
			tempPrev = temp->prev;
			tempNext = temp->next;

//			REMOVE_NODE(kit_index, kit_end, temp, prev, next);

#if defined(KEDIT_DEBUG)
			fprintf(stdout, "  saving over existing\n");
#endif

			delete[] (temp->name);
			temp->name = str_dup(OLC_KIT(d)->name ? OLC_KIT(d)->name : "(null)");

			for(int k = 0; k < NUM_OF_KITS; k++)
			{
				for(int l = 0; l < NUM_WEARS; l++)
				{
					temp->equipment[k][l] = OLC_KIT(d)->equipment[k][l];
					temp->percent[k][l] = OLC_KIT(d)->percent[k][l];
				}
			}

			delete[] (tempNew->name);
			delete[] (tempNew);

			return;
		}

		// Kit doesn't exist, but it goes in front of the next node.
		if (temp->next && OLC_KIT(d)->vnum < temp->next->vnum)
		{
			tempPrev = temp;
			tempNext = temp->next;

#if defined(KEDIT_DEBUG)
			fprintf(stdout, "  saving before next\n");
#endif

			ADD_NODE(kit_index, kit_end, tempNew, prev, next, tempPrev, tempNext);

			return;
		}

		temp = temp->next;
	}

#if defined(KEDIT_DEBUG)
	fprintf(stdout, "  saving on end\n");
#endif

	// New kit at end
	ADD_END_NODE(kit_index, kit_end, tempNew, prev, next);

	return;
}

void kedit_save_to_disk(struct descriptor_data *d)
{
	FILE *fp;

	sprintf(buf, "%s/%d.new", KIT_PREFIX, (OLC_NUM(d) / 100));

	if ( !(fp = fopen(buf, "w")) )
	{
		mudlog("SYSERR: OLC: Cannot open kit file!", BRF, LVL_BUILDER, TRUE);
		return;
	}

	for(struct kit_data *temp = kit_index; temp; temp = temp->next)
	{
		if (temp->vnum >= (OLC_NUM(d) - (OLC_NUM(d) % 100)) && temp->vnum <= (OLC_NUM(d) - (OLC_NUM(d) % 100) + 99))
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
	}

	fprintf(fp, "$~\n");

	fclose(fp);

	sprintf(buf2, "%s/%d.kit", KIT_PREFIX, (OLC_NUM(d) / 100));

	remove(buf2);
	rename(buf, buf2);

	return;
}

void kedit_setup_existing(struct descriptor_data *d, struct kit_data *kit)
{
	OLC_NUM(d) = kit->vnum;
	OLC_KNUM(d) = 0;
	OLC_VAL(d) = 0;

	CREATE(OLC_KIT(d), struct kit_data, 1);

	OLC_KIT(d)->name = str_dup(kit->name);
	OLC_KIT(d)->vnum = kit->vnum;

	for(int j = 0; j < NUM_OF_KITS; j++)
	{
		for(int i = 0; i < NUM_WEARS; i++)
		{
			OLC_KIT(d)->equipment[j][i] = kit->equipment[j][i];
			OLC_KIT(d)->percent[j][i] = kit->percent[j][i];
		}
	}

	kedit_disp_menu(d);

	return;
}

void kedit_setup_new(struct descriptor_data *d)
{
	OLC_KNUM(d) = 0;
	OLC_VAL(d) = 0;

	CREATE(OLC_KIT(d), struct kit_data, 1);

	OLC_KIT(d)->name = str_dup("an unfinished kit");
	OLC_KIT(d)->vnum = OLC_NUM(d);

	for(int j = 0; j < NUM_OF_KITS; j++)
		for(int i = 0; i < NUM_WEARS; i++)
			OLC_KIT(d)->equipment[j][i] = NOTHING;

	kedit_disp_menu(d);

	return;
}

struct kit_data *real_kit(int vnum)
{
	for(struct kit_data *temp = kit_index; temp; temp = temp->next)
		if (temp->vnum == vnum)
			return (temp);

	return (nullptr);
}

#undef OLC_KNUM
