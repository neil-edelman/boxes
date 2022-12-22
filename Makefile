cdoc  := cdoc
re2c  := re2c
mkdir := mkdir -p
#cat   := cat
#zip   := zip
#bison := bison
#lemon := lemon
#gperf := gperf

target    :=
optimize  := -ffast-math
warnbasic := -Wall -pedantic -ansi # -std=c99
# Some stuff is really new.
warnclang := -Wextra \
-Weverything \
-Wno-comma \
-Wno-logical-op-parentheses \
-Wno-parentheses \
-Wno-documentation-unknown-command \
-Wno-documentation \
-Wno-shift-op-parentheses \
-Wno-empty-body \
-Wno-padded \
-Wno-switch-enum \
-Wno-missing-noreturn \
-Wno-implicit-fallthrough

# https://stackoverflow.com/a/12099167
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	warnclang += -Wno-poison-system-directories
endif

warn := $(warnbasic) $(warnclang)

CC   := clang # gcc
CF   := $(target) $(optimize) $(warn)
OF   :=

ifeq (release, $(firstword $(MAKECMDGOALS)))
	CF += -funroll-loops -Ofast -D NDEBUG # -O3
	OF += -Ofast
else
	CF += -g
endif

default: bin/array bin/bmp bin/heap bin/list bin/pool bin/table bin/tree bin/trie
	# . . . success making tests in bin/

bin/array:

docs: $(html_docs)




# linking
$(bin)/$(project): $(c_objs) $(c_other_objs) $(test_c_objs)
	# linking rule
	@$(mkdir) $(bin)
	$(CC) $(OF) -o $@ $^

# compiling
#$(lemon)/$(bin)/$(lem): $(lemon)/$(src)/lemon.c
#	# compiling lemon
#	@$(mkdir) $(lemon)/$(bin)
#	$(CC) $(CF) -o $@ $<

$(c_objs): $(build)/%.o: $(src)/%.c $(all_h)
	# c_objs rule
	@$(mkdir) $(build)
	$(CC) $(CF) -c -o $@ $<

$(c_other_objs): $(build)/%.o: $(build)/%.c $(all_h)
	# c_other_objs rule
	$(CC) $(CF) -c -o $@ $<

$(test_c_objs): $(build)/$(test)/%.o: $(test)/%.c $(all_h)
	# test_c_objs rule
	@$(mkdir) $(build)
	@$(mkdir) $(build)/$(test)
	$(CC) $(CF) -c -o $@ $<

# -8 made my file 32767 lines or longer

$(c_re_builds): $(build)/%.c: $(src)/%.re.c
	# *.re.c build rule
	@$(mkdir) $(build)
	$(re2c) -W -T -o $@ $<

$(c_re_test_builds): $(build)/$(test)/%.c: $(test)/%.re.c
	# *.re.c tests rule
	@$(mkdir) $(build)
	@$(mkdir) $(build)/$(test)
	$(re2c) -W -T -o $@ $<

$(c_rec_builds): $(build)/%.c: $(src)/%.re_c.c
	# *.re_c.c (conditions) build rule
	@$(mkdir) $(build)
	$(re2c) -W -T -c -o $@ $<

$(c_rec_test_builds): $(build)/$(test)/%.c: $(test)/%.re_c.c
	# *.re_c.c (conditions) tests rule
	@$(mkdir) $(build)
	@$(mkdir) $(build)/$(test)
	$(re2c) -W -T -c -o $@ $<

$(c_y_builds): $(build)/%.c: $(src)/%.y # $(lemon)/$(bin)/$(lem)
	# .y rule
	@$(mkdir) $(build)
	$(bison) -o $@ $<

$(c_gperf_builds): $(build)/%.c: $(src)/%.gperf.c
	# *.gperf.c build rule
	@$(mkdir) $(build)
	$(gperf) $@ --output-file $<

$(html_docs): $(doc)/%.html: $(src)/%.c $(src)/%.h
	# docs rule
	@$(mkdir) $(doc)
	cat $^ | $(cdoc) > $@

######
# phoney targets

.PHONY: setup clean backup icon install uninstall test docs release

clean:
	-rm -f $(c_objs) $(test_c_objs) $(c_other_objs) $(c_re_builds) \
$(c_rec_builds) $(html_docs)
	-rm -rf $(bin)/$(test)

backup:
	@$(mkdir) $(backup)
	$(zip) $(backup)/$(project)-`date +%Y-%m-%dT%H%M%S`$(BRGS).zip \
readme.txt Makefile $(all_h) $(all_srcs) $(all_tests) $(all_icons)

icon: default
	# . . . setting icon on a Mac.
	cp $(media)/$(icon) $(bin)/$(icon)
	-sips --addIcon $(bin)/$(icon)
	-DeRez -only icns $(bin)/$(icon) > $(bin)/$(RSRC)
	-Rez -append $(bin)/$(RSRC) -o $(bin)/$(project)
	-SetFile -a C $(bin)/$(project)

setup: default icon
	@$(mkdir) $(bin)/$(install)
	cp $(bin)/$(project) readme.txt $(bin)/$(install)
	rm -f $(bin)/$(install)-MacOSX.dmg
	# or rm -f $(BDIR)/$(INST)-Win32.zip
	hdiutil create $(bin)/$(install)-MacOSX.dmg -volname "$(project)" -srcfolder $(bin)/$(install)
	# or zip $(BDIR)/$(INST)-Win32.zip -r $(BDIR)/$(INST)
	rm -R $(bin)/$(install)

# this needs work
release: clean default
	strip $(bin)/$(project)
	# define NDEBUG

install: release
	@$(mkdir) -p $(DESTDIR)$(PREFIX)/bin
	cp $(bin)/$(project) $(DESTDIR)$(PREFIX)/bin/$(project)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(project)

docs: $(html_docs)
