BUILD = build
INSTALLDIR = /usr/local/superoneproxy/

CXXFLAGS = -O2 -g -Wall -Wformat=0 -Wno-strict-aliasing
APPSOURCEDIR = ./sql/ \
			   ./util/ \
				./conf/ \
				./stats/ \
				./httpserver/ \
				./iomultiplex/ \
				./protocol/ \
				./protocol/fake/ \
				./protocol/sqlserver/ \
				./protocol/postgresql/

TESTSOURCEDIR = ./test/ ./unittest/

SOURCEDIR = $(TESTSOURCEDIR) $(APPSOURCEDIR)

VPATH = ./
VPATH += $(foreach dir, $(SOURCEDIR), :$(dir))

DIR = -I./.
DIR += $(foreach dir, $(SOURCEDIR), -I$(dir))

MAIN_SOURCES = main.cpp

SOURCES = $(filter-out $(MAIN_SOURCES), $(wildcard *.cpp))
SOURCES += $(foreach dir, $(APPSOURCEDIR), $(filter-out $(MAIN_SOURCES), $(wildcard $(dir)/*.cpp)))
HEADERS = $(wildcard ./*.h)
HEADERS += $(foreach dir, $(APPSOURCEDIR), $(wildcard $(dir)/*.h))

ifeq ($(MAKECMDGOALS), )
	SOURCES += $(MAIN_SOURCES)
else ifeq ($(MAKECMDGOALS), all)
	SOURCES += $(MAIN_SOURCES)
else ifeq ($(MAKECMDGOALS), test)
	SOURCES += $(wildcard test/*.cpp) $(wildcard unittest/*.cpp)
	HEADERS += $(wildcard test/*.h) $(wildcard unittest/*.h)
endif
OBJS =	 $(patsubst %.cpp, $(BUILD)/%.o, $(notdir $(SOURCES)))

ifeq ($(LANG),)
LIBS = -lwsock32 -lwinmm
else
LIBS = -pthread ./libtcmalloc_minimal.a ./stats/libsqlite3.a
endif

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
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $(DIR) $< -o $@
	
$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

.PHONY: test
test: $(TARGET)

install:
	@mkdir -p $(INSTALLDIR)/include/
	@mkdir -p $(INSTALLDIR)/include/conf/
	@echo $(HEADERS)
	@cp -rf $(HEADERS) $(INSTALLDIR)/include/
	@cp -rf conf/config.h $(INSTALLDIR)/include/conf/

clean:
	rm -rf $(BUILD)
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS)
