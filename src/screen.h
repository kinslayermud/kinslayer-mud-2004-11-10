/* ************************************************************************
*   File: screen.h                                      Part of CircleMUD *
*  Usage: header file with ANSI color codes for online color              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define NORMAL		"\x1B[0m"
#define BOLD		"\x1B[1m"
#define RED		"\x1B[31m"
#define GREEN		"\x1B[32m"
#define YELLOW		"\x1B[33m"
#define BLUE		"\x1B[34m"
#define DARK_RED	"\x1B[35m"
#define CYAN		"\x1B[36m"
#define WHITE		"\x1B[37m"
#define nullptr_COLOR	""



/* conditional color.  pass it a pointer to a char_data and a color level. */
#define CL_OFF			0
#define CL_SPARSE		1
#define CL_NORMAL		2
#define CL_COMPLETE		3
#define _clrlevel(ch) (!IS_NPC(ch) ? (PRF_FLAGGED((ch), PRF_COLOR_1) ? 1 : 0) + \
		       (PRF_FLAGGED((ch), PRF_COLOR_2) ? 2 : 0) : 0)
#define clr(ch,lvl) (_clrlevel(ch) >= (lvl))

#define COLOR_NORMAL(ch,lvl)  (clr((ch),(lvl))?NORMAL:nullptr_COLOR)
#define COLOR_BOLD(ch,lvl)  (clr((ch),(lvl))?BOLD:nullptr_COLOR)
#define COLOR_RED(ch,lvl)  (clr((ch),(lvl))?RED:nullptr_COLOR)
#define COLOR_GREEN(ch,lvl)  (clr((ch),(lvl))?GREEN:nullptr_COLOR)
#define COLOR_YELLOW(ch,lvl)  (clr((ch),(lvl))?YELLOW:nullptr_COLOR)
#define COLOR_BLUE(ch,lvl)  (clr((ch),(lvl))?BLUE:nullptr_COLOR)
#define COLOR_DARK_RED(ch,lvl)  (clr((ch),(lvl))?DARK_RED:nullptr_COLOR)
#define COLOR_CYAN(ch,lvl)  (clr((ch),(lvl))?CYAN:nullptr_COLOR)
#define COLOR_WHITE(ch,lvl)  (clr((ch),(lvl))?WHITE:nullptr_COLOR)

#define COLOR_LEV(ch) (_clrlevel(ch))

#define QNRM CCNRM(ch,C_SPR)
#define QRED CCRED(ch,C_SPR)
#define QGRN CCGRN(ch,C_SPR)
#define QYEL CCYEL(ch,C_SPR)
#define QBLU CCBLU(ch,C_SPR)
#define QMAG CCMAG(ch,C_SPR)
#define QCYN CCCYN(ch,C_SPR)
#define QWHT CCWHT(ch,C_SPR)

