# GNU Make 3.81; MacOSX gcc 4.2.1; MacOSX MinGW 4.3.0

PROJ  := List
VA    := 1
VB    := 0

# dirs
SDIR  := src
TDIR  := test
GDIR  := build
BDIR  := bin
BACK  := backup
DDIR  := doc
PREFIX:= /usr/local

# files in bdir
INST  := $(PROJ)-$(VA)_$(VB)

# extra stuff we should back up
EXTRA := $(PROJ).xcodeproj

# John Graham-Cumming:
# rwildcard is a recursive wildcard
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

# select all automatically
SRCS  := $(call rwildcard, $(SDIR), *.c) # or *.java
TEST  := $(call rwildcard, $(TDIR), *.c)
H_SRC := $(call rwildcard, $(SDIR), *.h)
H     := $(call rwildcard, $(SDIR), *.h) $(call rwildcard, $(TDIR), *.h)
OBJS  := $(patsubst $(SDIR)/%.c, $(GDIR)/%.o, $(SRCS)) # or *.class
TOBJS := $(patsubst $(TDIR)/%.c, $(GDIR)/$(TDIR)/%.o, $(TEST))
DOCS  := $(patsubst $(SDIR)/%.h, $(DDIR)/%.html, $(H_SRC))

CC   := clang #gcc
CF   := -Wall -Wextra -Wno-format-y2k -Wstrict-prototypes \
-Wmissing-prototypes -Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings \
-Wswitch -Wshadow -Wcast-align -Wbad-function-cast -Wchar-subscripts -Winline \
-Wnested-externs -Wredundant-decls -Wfatal-errors -O3 -ffast-math -funroll-loops -pedantic -std=c89 #-ansi # or -std=c99 -mwindows
OF   := # -framework OpenGL -framework GLUT or -lglut -lGLEW
CDOC := cdoc

# props Jakob Borg and Eldar Abusalimov
# $(ARGS) is all the extra arguments
# $(BRGS) is_all_the_extra_arguments
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
ifeq (backup, $(firstword $(MAKECMDGOALS)))
  ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  BRGS := $(subst $(SPACE),_,$(ARGS))
  ifneq (,$(BRGS))
    BRGS := -$(BRGS)
  endif
  $(eval $(ARGS):;@:)
endif

######
# compiles the programme by default

default: $(BDIR)/$(PROJ) $(DOCS)
	# . . . success; executable is in $(BDIR)/$(PROJ)

# linking
$(BDIR)/$(PROJ): $(OBJS) $(TOBJS)
	# linking rule
	@mkdir -p $(BDIR)
	$(CC) $(CF) $(OF) $^ -o $@

# compiling
$(OBJS): $(GDIR)/%.o: $(SDIR)/%.c $(H)
	# objs rule
	@mkdir -p $(GDIR)
	$(CC) $(CF) -c $(SDIR)/$*.c -o $@

$(TOBJS): $(GDIR)/$(TDIR)/%.o: $(TDIR)/%.c $(H)
	# tobjs rule
	@mkdir -p $(GDIR)
	@mkdir -p $(GDIR)/$(TDIR)
	$(CC) $(CF) -c $(TDIR)/$*.c -o $@

# $(SDIR)/%.c
$(DOCS): $(DDIR)/%.html: $(SDIR)/%.h
	# docs rule
	@mkdir -p $(DDIR)
	-cat $^ | $(CDOC) > $@
	-cat $^ | $(CDOC) text > $(DDIR)/$*.txt

######
# phoney targets

.PHONY: setup clean backup icon install uninstall test

clean:
	-rm -f $(OBJS) $(TOBJS) #$(DOCS)
	-rm -rf $(BDIR)/$(TDIR)

backup:
	@mkdir -p $(BACK)
	zip $(BACK)/$(INST)-`date +%Y-%m-%dT%H%M%S`$(BRGS).zip readme.txt gpl.txt copying.txt Makefile $(SRCS) $(TEST) $(H) $(SDIR)/$(ICON) $(EXTRA)
	#git commit -am "$(ARGS)"

icon: default
	# . . . setting icon on a Mac.
	cp $(MDIR)/$(ICON) $(BDIR)/$(ICON)
	-sips --addIcon $(BDIR)/$(ICON)
	-DeRez -only icns $(BDIR)/$(ICON) > $(BDIR)/$(RSRC)
	-Rez -append $(BDIR)/$(RSRC) -o $(BDIR)/$(PROJ)
	-SetFile -a C $(BDIR)/$(PROJ)

setup: default icon
	@mkdir -p $(BDIR)/$(INST)
	cp $(BDIR)/$(PROJ) readme.txt gpl.txt copying.txt $(BDIR)/$(INST)
	rm -f $(BDIR)/$(INST)-MacOSX.dmg
	# or rm -f $(BDIR)/$(INST)-Win32.zip
	hdiutil create $(BDIR)/$(INST)-MacOSX.dmg -volname "$(PROJ) $(VA).$(VB)" -srcfolder $(BDIR)/$(INST)
	# or zip $(BDIR)/$(INST)-Win32.zip -r $(BDIR)/$(INST)
	rm -R $(BDIR)/$(INST)

install: default
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(BDIR)/$(PROJ) $(DESTDIR)$(PREFIX)/bin/$(PROJ)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PROJ)
