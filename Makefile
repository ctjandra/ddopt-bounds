# --- USER COMPILATION SETTINGS --

# Debug mode
DEBUG_MODE = 0

# Use SCIP; used almost everywhere
USE_SCIP = 1

# Set SCIP directory as follows:
SCIPDIR = ./scip-5.0.1

# Set Boost directory as follows:
BOOSTDIR = ./boost

# Use GMP library; used to compute the center of a decision diagram (not necessary for reproducing experiments)
USE_GMP = 0

# Use ConicBundle; used in Lagrangian relaxation
USE_CONICBUNDLE = 1
CONICBUNDLEDIR = ConicBundle


# --- SYSTEM ---

SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic


# --- OPTIMIZATION FLAGS ---

# These are named USERCFLAGS and USERLDFLAGS because SCIP defines its own CFLAGS and LDFLAGS
USERCFLAGS = -ansi -DIL_STD -fPIC -fstrict-aliasing -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter -fexceptions \
	-ffloat-store -std=c++11 -isystem$(BOOSTDIR) $(DEBUG_OPT) -c
USERLDFLAGS = -lm -pthread -no-pie
USEROFLAGS = -MMD

ifeq ($(USE_GMP),1)
USERCFLAGS += -DUSE_GMP
USERLDFLAGS += -lgmpxx -lgmp
endif

ifeq ($(USE_CONICBUNDLE),1)
USERCFLAGS += -DUSE_CONICBUNDLE -I$(CONICBUNDLEDIR)/include
USERLDFLAGS += -L$(CONICBUNDLEDIR)/lib -lcb
endif


# --- SOLVER-DEPENDENT SETUP ---

override LPS=cpx
override ZIMPL=false
override SHARED=false
override GMP=false
override READLINE=false
override ZLIB=false
ifeq ($(DEBUG_MODE),1)
override OPT=dbg
else
override OPT=opt
endif

include $(SCIPDIR)/make/make.project

USERCFLAGS += -DSOLVER_SCIP
USERLDFLAGS += $(LINKCXXSCIPALL)

# Files that require SCIP
SCIP_FILES = $(shell find src/ -name '*_scip.cpp')
ifneq ($(USE_SCIP),1)
EXCLUDE = $(SCIP_FILES)
endif


# --- COMPILER ---

CCC = g++


# --- DEBUG FLAGS ---

ifeq ($(DEBUG_MODE),1)
DEBUG_OPT = -g -O0
else
DEBUG_OPT = -DNDEBUG -O3
endif


# --- DIRECTORIES  ---

SRC_DIR   := src
OBJ_DIR   := obj

SRC_DIRS  := $(shell find $(SRC_DIR) -type d)
OBJ_DIRS  := $(addprefix $(OBJ_DIR)/,$(SRC_DIRS))

SOURCES   := $(shell find $(SRC_DIR) -name '*.cpp')
SOURCES   := $(filter-out $(EXCLUDE), $(SOURCES))
OBJ_FILES := $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))

DEP_FILES := $(OBJ_FILES:%.o=%.d)

vpath %.cpp $(SRC_DIRS)


# --- TARGETS ---

EXECUTABLE=ddopt

all: $(EXECUTABLE)
	cp $(EXECUTABLE) ../experiments/bin

$(EXECUTABLE): testscipdir makedir $(SOURCES) $(OBJ_FILES)
	$(CCC) $(OBJ_FILES) $(USERLDFLAGS) -o $@

-include $(DEP_FILES)

$(OBJ_DIR)/%.o: %.cpp
	$(CCC) $(FLAGS) $(OFLAGS) $(USEROFLAGS) $(BINOFLAGS) $(CXXFLAGS) $(USERCFLAGS) $< -o $@

makedir: $(OBJ_DIRS)

testscipdir:
	@if [ "$(USE_SCIP)" = "1" -a "$(SCIPDIR)" = "" ]; then echo "*** Error: SCIP directory is not set; modify Makefile"; exit 1; fi

$(OBJ_DIRS):
	@mkdir -p $@

clean:
	@rm -rf obj 
	@rm -f $(EXECUTABLE)
