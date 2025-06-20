# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_LEGACY = NO

ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
TOOLS=d:\tdm32\bin
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -s -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
endif
CFLAGS += -Ider_libs
IFLAGS += -Ider_libs
LiFLAGS += -Ider_libs
CFLAGS += -DSTANDALONE
LFLAGS += -DSTANDALONE

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
LiFLAGS += -dUNICODE -d_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
IFLAGS += -DUNICODE -D_UNICODE
endif

# This is required for *some* versions of makedepend
IFLAGS += -DNOMAKEDEPEND

ifeq ($(USE_LEGACY),YES)
CFLAGS += -DLEGACY_QUALIFY
endif

ifeq ($(USE_LEGACY),YES)
CPPSRC=der_libs\qualify_orig.cpp 
else
CPPSRC=der_libs\qualify.cpp 
endif

CPPSRC += der_libs\common_funcs.cpp \
der_libs\conio_min.cpp 

OBJS = $(CPPSRC:.cpp=.o)

BIN=qualify

ifeq ($(USE_64BIT),NO)
BINX = $(BIN).exe
else
BINX = $(BIN)64.exe
endif

LIBS=-lshlwapi

#  clang-tidy options
CHFLAGS = -header-filter=.*
CHTAIL = --
CHTAIL += -Ider_libs
ifeq ($(USE_64BIT),YES)
CHTAIL += -DUSE_64BIT
endif
ifeq ($(USE_UNICODE),YES)
CHTAIL += -DUNICODE -D_UNICODE
endif

#**************************************************************************
%.o: %.cpp
	$(TOOLS)/g++ $(CFLAGS) -c $< -o $@

all: $(BINX)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f $(BIN).zip
	zip $(BIN).zip $(BIN) Readme.md

wc:
	wc -l $(CPPSRC)

check:
	cmd /C "d:\clang\bin\clang-tidy.exe $(CHFLAGS) $(CPPSRC) $(CHTAIL)"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(IFLAGS) $(CPPSRC)

$(BINX): $(OBJS)
	$(TOOLS)\g++ $(OBJS) $(LFLAGS) -o $(BINX) $(LIBS) 

# DO NOT DELETE

der_libs\qualify.o: der_libs/common.h der_libs/conio_min.h der_libs/qualify.h
der_libs\common_funcs.o: der_libs/common.h
der_libs\conio_min.o: der_libs/common.h der_libs/conio_min.h
