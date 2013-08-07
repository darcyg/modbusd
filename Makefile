
CC_PATH=$(HOME)/termo/gcc-4.1.2-glibc-2.5-nptl-3/arm-none-linux-gnueabi/bin

#CC=arm-linux-gnueabi-g++
#AS=arm-linux-gnueabi-as
#LD=arm-linux-gnueabi-ld
#STRIP=arm-linux-gnueabi-strip
CC=$(CC_PATH)/arm-none-linux-gnueabi-g++
AS=$(CC_PATH)/arm-none-linux-gnueabi-as
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
CC_FLAGS = -O3 -g3 -Wall -rdynamic -fno-omit-frame-pointer -mapcs-frame $(INC_PATH) $(DEFINES)

LD_FLAGS =  -fno-omit-frame-pointer -mapcs-frame -O3 -g3 -rdynamic -marm -pthread -lsqlite3 -lmodbus -lrt -ldl $(LIB_PATH)  

CPP_FILES := $(wildcard src/gateway/*.cpp src/*.cpp)
AS_FILES := $(wildcard src/*.s)


OBJ_FILES = $(patsubst src/%.cpp,$(OUT)/%.o,$(CPP_FILES))
AS_OBJ_FILES=$(patsubst src/%.s,$(OUT)/%.o,$(AS_FILES))

OBJ_FILES+=$(AS_OBJ_FILES)

all::dirs $(EXECUTABLE) Makefile

$(EXECUTABLE): $(OBJ_FILES) 
	@echo Linking $@ version $(VERSION)
	$(CC) -o $@ $^ $(LD_FLAGS)
	cp $@ $@.unstripped
	@$(STRIP) $@

$(OUT)/%.o: src/%.cpp
	@echo Compiling $^
	@$(CC) $(CC_FLAGS) -c -o $@ $^

$(OUT)/%.o: src/%.s
	@echo Assembling $^
	@$(AS) $(AS_FLAGS) -o $@ $^

.PHONY: dirs
dirs:
	mkdir -p $(OUT)/gateway
clean::
	rm -rf $(OUT)/*
	rm -rf $(EXECUTABLE)
