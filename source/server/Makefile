BUILD_DIR = build

compile_flags := -g -g3 -O0 -D_GNU_SOURCE -DDEBUG

full_program_name := server
source_dirs := . autogen ../common
source_dirs := $(addprefix ../,$(source_dirs))

version := -std=gnu99

cflags:= -Wall -pedantic -Werror
ldflags := `autoopts-config ldflags` -lpcre2-8 -lpcre -lm
test_ldflags := -lcunit

search_wildcards := $(addsuffix /*.c,$(source_dirs))
wildcard_source := $(wildcard $(filter-out ../tests/*.c,$(search_wildcards)))

$(full_program_name): $(notdir $(patsubst %.c,%.o,$(wildcard_source)))
	gcc $(version) $^ -o $@ $(ldflags)

test_client: $(notdir ../tests/*.o)
	gcc $(version) $^ -o $@ $(ldflags)

VPATH := $(source_dirs) 

%.o: %.c
	gcc $(version) -c -MD $(compile_flags) $(cflags) $(addprefix -I,$(source_dirs)) $<

include $(wildcard *.d) 


.PHONY: clean
clean:
	rm -rf  $(BUILD_DIR)