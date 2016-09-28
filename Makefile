BUILD = build

ifeq ($(LANG),) #windows
	INSTALLDIR = $(BUILD)
	LDFLAGS = -static-libgcc -static-libstdc++
	LIBS = -lwsock32 -lwinmm
else #linux
	INSTALLDIR = /usr/local/superoneproxy/
	LDFLAGS = 
	LIBS = -pthread ./libtcmalloc_minimal.a ./stats/libsqlite3.a
endif

CXXFLAGS = -O2 -Wall -Wformat=0 -Wno-strict-aliasing

APPSOURCEDIR = ./sql \
			   ./util \
				./conf \
				./stats \
				./httpserver \
				./iomultiplex \
				./protocol \
				./protocol/fake \
				./protocol/sqlserver \
				./protocol/postgresql

TESTSOURCEDIR = ./test/ ./unittest/

SOURCEDIR = $(TESTSOURCEDIR) $(APPSOURCEDIR)

VPATH = ./
VPATH += $(foreach tdir, $(SOURCEDIR), :$(tdir))

DIR = -I./.
DIR += $(foreach tdir, $(SOURCEDIR), -I$(tdir))

MAIN_SOURCES = main.cpp

SOURCES = $(filter-out $(MAIN_SOURCES), $(wildcard *.cpp))
SOURCES += $(foreach tdir, $(APPSOURCEDIR), $(filter-out $(MAIN_SOURCES), $(wildcard $(tdir)/*.cpp)))
HEADERS = $(wildcard ./*.h)
HEADERS += $(foreach tdir, $(APPSOURCEDIR), $(wildcard $(tdir)/*.h))

ifeq ($(MAKECMDGOALS), )
	SOURCES += $(MAIN_SOURCES)
else ifeq ($(MAKECMDGOALS), all)
	SOURCES += $(MAIN_SOURCES)
else ifeq ($(MAKECMDGOALS), test)
	SOURCES += $(wildcard test/*.cpp) $(wildcard unittest/*.cpp)
	HEADERS += $(wildcard test/*.h) $(wildcard unittest/*.h)
endif
OBJS =	 $(patsubst %.cpp, $(BUILD)/%.o, $(notdir $(SOURCES)))

ifeq ($(MAKECMDGOALS), test)
	ifeq ($(LANG),)
	TARGET = $(BUILD)/unittest_main.exe
	else
	TARGET = $(BUILD)/unittest_main
	endif
else
	ifeq ($(LANG),)
	TARGET = $(BUILD)/oneproxy-for-sqlserver.exe
	else
	TARGET = $(BUILD)/oneproxy-for-sqlserver
	endif
endif

.PHONY: all
all: $(TARGET)

$(OBJS): $(BUILD)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DIR) $< -o $@
	
$(TARGET):	$(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 

.PHONY: test
test: $(TARGET)

install:
	@mkdir -p $(INSTALLDIR)/include/
	@mkdir -p $(INSTALLDIR)/include/conf/
	@echo $(HEADERS)
	@cp -rf $(HEADERS) $(INSTALLDIR)/include/
	@cp -rf conf/config.h $(INSTALLDIR)/include/conf/

clean:
	@rm -rf $(BUILD)
	@mkdir $(BUILD)
