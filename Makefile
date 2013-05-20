
CC_PATH=$(HOME)/termo/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin

CC=$(CC_PATH)/arm-none-linux-gnueabi-g++
STRIP=$(CC_PATH)/arm-none-linux-gnueabi-strip

VERSION=`git describe`

OUT = ./arm_obj

EXECUTABLE = modbusd

INC_PATH = -I$(HOME)/termo/sqlite-3.6.22 -I./inc/libmodbus
LIB_PATH = -L$(HOME)/termo/sqlite-3.6.22/.libs -L./arm_lib
DEFINES=-D_DAEMON_VERSION_=\"$(VERSION)\"

#for debug. unstripped. file is ab out 1.5 Mb!
#CC_FLAGS = -O0 -g3 -Wall $(INC_PATH) $(DEFINES)

#for Release
CC_FLAGS = -O3 -Wall $(INC_PATH) $(DEFINES)

LD_FLAGS = -lpthread -lsqlite3 -lmodbus $(LIB_PATH) 

CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES = $(patsubst src/%.cpp,$(OUT)/%.o,$(CPP_FILES))

all::dirs $(EXECUTABLE) Makefile

$(EXECUTABLE): $(OBJ_FILES) 
	@echo Linking $@ version $(VERSION)
	@$(CC) $(LD_FLAGS) -o $@ $^
	@$(STRIP) $@

$(OUT)/%.o: src/%.cpp
	@echo Compiling $^
	@$(CC) $(CC_FLAGS) -c -o $@ $^

.PHONY: dirs
dirs:
	mkdir -p $(OUT)
clean::
	rm -rf $(OUT)/*
	rm -rf $(EXECUTABLE)
