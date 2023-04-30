EXE=ld53
CC_LINUX=cc
LIBS_LINUX=-lm
FLAGS_LINUX=-I ./include/ -I /usr/include/GLFW/ -I /usr/include/AL/
CC_WINDOWS=x86_64-w64-mingw32-gcc
LIBS_WINDOWS=-lm -lgdi32 -lmingwex -lwinmm
FLAGS_WINDOWS=-I ./include/ -I /usr/include/GLFW/ -I /usr/include/AL/ -I /usr/x86_64-w64-mingw32/include -m64 -mwindows
CFILES=$(wildcard src/*.c)
OBJS_LINUX=$(patsubst src/%.c, bin/linux/%.o, $(CFILES))
OBJS_WINDOWS=$(patsubst src/%.c, bin/windows/%.o, $(CFILES))
DEPS_LINUX=$(patsubst src/%.c, bin/linux/%.d, $(CFILES))
DEPS_WINDOWS=$(patsubst src/%.c, bin/windows/%.d, $(CFILES))
OS=LINUX
MODE=DEVELOPMENT

ifeq ($(MODE), DEVELOPMENT)
	FLAGS_LINUX += -Wall -Wextra -Werror -g
	FLAGS_WINDOWS += -Wall -Wextra -Werror -g
else ifeq ($(MODE), RELEASE)
	FLAGS_LINUX += -Ofast
	FLAGS_WINDOWS += -Ofast
else
$(error unknown mode $(RELEASE).)
endif

.PHONY: all clean

all: bin/linux/$(EXE) bin/windows/$(EXE).exe

bin/linux/$(EXE): $(OBJS_LINUX)
	@echo linking object files into elf...
	@$(CC_LINUX) $(FLAGS_LINUX) -o bin/linux/$(EXE) $^ $(LIBS_LINUX)
	@echo updating linux resources...
	@cp -r data/ shaders/ ../linux/
	@cp -r libs/linux/ ../linux/libs/
	@cp bin/linux/$(EXE) ../linux/
	@echo $(EXE) was succesfully builded for linux!

bin/windows/$(EXE).exe: $(OBJS_WINDOWS)
	@echo linking object files into exe...
	@$(CC_WINDOWS) $(FLAGS_WINDOWS) -o bin/windows/$(EXE) $^ $(LIBS_WINDOWS)
	@echo updating windows resources...
	@cp -r data/ shaders/ ../windows/
	@cp -r libs/windows/ ../windows/libs/
	@cp bin/windows/$(EXE).exe ../windows/
	@echo $(EXE) was succesfully builded for windows!

res:
	@cp -r data/ libs/linux/ shaders/ ../linux/
	@cp -r data/ libs/windows/ shaders/ ../windows/
	@echo resources updated


-include $(DEPS_WINDOWS)
-include $(DEPS_LINUX)

bin/linux/%.o: src/%.c
	@echo compiling $< with gcc...
	@$(CC_LINUX) $(FLAGS_LINUX) -MMD -c -DLINUX $< -o $@

bin/windows/%.o: src/%.c
	@echo compiling $< with mingw...
	@$(CC_WINDOWS) $(FLAGS_WINDOWS) -MMD -c -DWINDOWS $< -o $@

clean:
	@rm bin/linux/*
	@rm bin/windows/*
	@echo cleaned up binaries!
	@rm -r ../windows/*
	@rm -r ../linux/*
	@echo cleaned up build!

