prog?=shell

all_sources=$(wildcard *.c *.cc)
filtered_sources=$(filter-out main.c wrapper.c, $(all_sources))
objs+=$(addsuffix .o,$(basename $(filtered_sources)))

defines+=-D_GNU_SOURCE
ccflags+=-g -Wall -MMD $(defines)
ldflags+=-g -lreadline -lncurses

.SUFFIXES:

%.o: %.c  ; gcc -o $@ -c $< $(ccflags)
%.i: %.c  ; gcc -o $@ -E $< $(defines)
%.s: %.c  ; gcc -o $@ -S $< $(defines)
%.o: %.cc ; g++ -o $@ -c $< $(ccflags)
%.i: %.cc ; g++ -o $@ -E $< $(defines)
%.s: %.cc ; g++ -o $@ -S $< $(defines)

ld?=gcc

$(prog): $(objs) ; $(ld) -o $@ $^ $(ldflags)

.PHONY: clean run valgrind test

clean:: ; rm -f $(prog) *.o *.d *.i try

run:      $(prog) ; ./$< $(args)
valgrind: $(prog) ; $@ --leak-check=full --show-leak-kinds=all ./$< $(args)
test:     $(prog) ; bash Test/run $<

sinclude *.d
