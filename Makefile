optimize  := -ffast-math
warnbasic := -Wall -pedantic -ansi # -std=c99
warnclang := -Wextra -Weverything \
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
# Some stuff is really new, comment out the warnclang?
warn := $(warnbasic) $(warnclang)
CC   := clang # gcc
CF   := $(optimize) $(warn)
OF   :=
ifeq (release, $(firstword $(MAKECMDGOALS)))
	CF += -funroll-loops -Ofast -D NDEBUG # -O3
	OF += -Ofast
else
	CF += -g
endif

projects := $(patsubst test/test_%.c, bin/%, $(wildcard test/test_*.c))
docs := $(patsubst test/test_%.c, doc/%.md, $(wildcard test/test_*.c))

default: $(projects) $(docs)
	# . . . success making tests in bin/

bin/%: build/test_%.o
	# linking project $@
	@mkdir -p bin
	@mkdir -p graph
	$(CC) $(OF) -o $@ $^

build/test_%.o: test/test_%.c src/%.h
	# compile main $@
	@mkdir -p build
	$(CC) $(CF) -c -o $@ $<

build/%.o: test/%.c test/%.h
	# compile helper $@
	@mkdir -p build
	$(CC) $(CF) -c -o $@ $<

build/%.c: test/%.re.c
	# https://re2c.org/ $@
	@mkdir -p build
	re2c -W --tags -o $@ $<

doc/%.md: src/%.h
	# documentation update
	cdoc -o $@ $<

# additional dependencies
$(projects): build/orcish.o # except bmp, meh
bin/table: build/orcish.o build/lex_dict.c

.PHONY: clean docs release test

test: $(projects)
	@for project in $(projects); do \
		echo "\033[1;36m\033[1m*** Testing $$project ***\033[0m" ; \
		$$project ; \
	done

clean:
	-rm -rf bin/ build/ graph/

docs: $(docs)
