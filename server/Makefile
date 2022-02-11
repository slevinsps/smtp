BUILD_DIR = build

compile_flags := -g -g3 -O0 -D_GNU_SOURCE -DDEBUG

full_program_name := server tests
source_dirs := . autogen
source_dirs := $(addprefix ../,$(source_dirs))

version := -std=gnu99

cflags:= -Wall -pedantic -Werror
ldflags := `autoopts-config ldflags` -lpcre2-8 -lpcre -lm
test_ldflags := -lcunit

search_wildcards := $(addsuffix /*.c,$(source_dirs))
wildcard_source := $(wildcard $(filter-out ../tests/*.c,$(search_wildcards)))

$(full_program_name): $(notdir $(patsubst %.c,%.o,$(wildcard_source)))
	gcc $(version) $^ -o $@ $(ldflags)

autogen_files:
	cd ./autogen && autogen server.def && autogen serveropts.def && cd ../../

SOURCES = state_handlers.c main.c client_info.c maildir.c mail.c server.c autogen/server-fsm.c help_funcs.c
check_tidy:
	clang-tidy $(SOURCES) --

VPATH := $(source_dirs) 

%.o: %.c
	gcc $(version) -c -MD $(compile_flags) $(cflags) $(addprefix -I,$(source_dirs)) $<

include $(wildcard *.d) 

unit_test:
	gcc ../tests/re_parser_test.c  re_parser.o -Wall -o test -lcheck -pthread -lcheck_pic -lrt -lm -lsubunit $(ldflags) && ./test


.PHONY: clean
clean:
	rm -rf  $(BUILD_DIR)