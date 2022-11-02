# CircleMUD 3.0 Makefile.in - Makefile template used by 'configure'
#

# C compiler to use
CC = g++

# Path to cxref utility
CXREF = cxref

# Any special flags you want to pass to the compiler
MYFLAGS = \
-O0 -std=c++0x -lstdc++ -Wformat-security -Wformat -Wpointer-arith \
-Wcast-align -Wredundant-decls

#flags for profiling (see hacker.doc for more information)
PROFILE = 

##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

BINDIR = ../bin

CPPFLAGS = -ggdb3 $(MYFLAGS) $(PROFILE)

LIBS = -lcrypt

OBJDIR = obj

OBJFILES = comm.o act.comm.o act.informative.o act.movement.o act.item.o \
	act.offensive.o act.other.o act.social.o act.wizard.o ban.o boards.o \
	class.o config.o constants.o db.o fight.o graph.o handler.o \
	house.o interpreter.o limits.o magic.o mail.o mobact.o modify.o \
	objsave.o olc.o random.o shop.o spec_assign.o spec_procs.o \
	spell_parser.o spells.o utils.o weather.o redit.o oedit.o zedit.o \
	medit.o sedit.o \
	dg_comm.o dg_db_scripts.o dg_event.o dg_handler.o dg_mobcmd.o \
	dg_objcmd.o dg_scripts.o dg_triggers.o dg_wldcmd.o dg_olc.o \
	hedit.o aedit.o kedit.o alias.o buffer.o

OBJFILES := $(patsubst %.o,$(OBJDIR)/%.o,$(OBJFILES))

CXREF_FILES = act.comm.cpp act.informative.cpp act.item.cpp act.movement.cpp \
	act.offensive.cpp act.other.cpp act.social.cpp act.wizard.cpp ban.cpp boards.cpp \
	castle.cpp class.cpp comm.cpp config.cpp constants.cpp db.cpp fight.cpp graph.cpp \
	handler.cpp house.cpp interpreter.cpp limits.cpp magic.cpp mail.cpp mobact.cpp \
	modify.cpp objsave.cpp olc.cpp random.cpp shop.cpp spec_assign.cpp spec_procs.cpp \
	spell_parser.cpp spells.cpp utils.cpp weather.cpp redit.cpp oedit.cpp zedit.cpp \
	medit.cpp sedit.cpp \
	dg_comm.cpp dg_db_scripts.cpp dg_event.cpp dg_handler.cpp dg_mobcmd.cpp \
	dg_objcmd.cpp dg_scripts.cpp dg_triggers.cpp dg_wldcmd.cpp dg_olc.cpp \
	hedit.cpp aedit.cpp kedit.cpp alias.cpp buffer.cpp

$(OBJDIR)/%.d: %.cpp
	@mkdir -p $(@D)
	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $< | \
	sed '\''s,\($*\)\.o[ :]*,$(OBJDIR)\/\1.o $@ : ,g'\'' > $@'

default: all

-include $(OBJFILES:.o=.d)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -c	$<	-o $@

all:
	@mkdir -p ../bin
	$(MAKE) $(BINDIR)/kinslayer
	$(MAKE) utils

utils:
	@mkdir -p ../bin
	(cd util; $(MAKE) all)

kinslayer:
	@mkdir -p ../bin
	$(MAKE) $(BINDIR)/kinslayer

$(BINDIR)/kinslayer : $(OBJFILES)
	$(CC) -o $(BINDIR)/kinslayer $(PROFILE) $(OBJFILES) $(LIBS)

clean:
	rm -f $(OBJFILES) $(OBJDIR)/*.d *~
