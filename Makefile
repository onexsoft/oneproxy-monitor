BUILD = build
BUILD_TEST_BIN = utest

ifeq ($(LANG),) #windows
	INSTALLDIR = install
	LDFLAGS = -static-libgcc -static-libstdc++ -L./lib -L./
	LIBS = ./lib/win/libssl.a ./lib/win/libcrypto.a ./lib/win/libgdi32.a -lwsock32 -lwinmm -lpacket -lwpcap -lws2_32 -lIPHLPAPI ./lib/win/libgmp.a
	INCLUDE = -IC:\openssl-1.0.2e\win64\include -ID:\winlib\winpcap\Include
else #linux
$(shell if [ ! -d $(BUILD)/$(BUILD_TEST_BIN) ]; then mkdir -p $(BUILD)/$(BUILD_TEST_BIN); fi;)
	INSTALLDIR = /usr/local/superoneproxy/
	LDFLAGS = 
	OPENSSL = /usr/local/openssl
	LIBEV = /usr/local/libev
	LIBPCAP = /usr/local/libpcap
	# -lprofiler -lunwind ./libtcmalloc_minimal.a
	LIBS = -pthread ./stats/libsqlite3.a ./lib/libgmp.a $(OPENSSL)/lib/libssl.a $(OPENSSL)/lib/libcrypto.a $(LIBEV)/lib/libev.a $(LIBPCAP)/lib/libpcap.a -ldl
	INCLUDE = -I$(OPENSSL)/include -I$(LIBEV)/include -I$(LIBPCAP)/include
endif

CXXFLAGS = -Wall -Wformat=0 -Wno-strict-aliasing -g

APPSOURCEDIR = ./sql \
			   ./util \
				./conf \
				./stats \
				./monitor \
				./httpserver \
				./iomultiplex \
				./protocol \
				./protocol/fake \
				./protocol/sqlserver \
				./protocol/postgresql \
				./commercial

TESTSOURCEDIR = ./test/ ./unittest/

SOURCEDIR = $(TESTSOURCEDIR) $(APPSOURCEDIR)

VPATH = ./
VPATH += $(foreach tdir, $(SOURCEDIR), :$(tdir))

DIR = -I./.
DIR += $(foreach tdir, $(SOURCEDIR), -I$(tdir))
DIR += $(INCLUDE)

MAIN_SOURCES = main.cpp

SOURCES = $(filter-out $(MAIN_SOURCES), $(wildcard *.cpp))
SOURCES += $(foreach tdir, $(APPSOURCEDIR), $(filter-out $(MAIN_SOURCES), $(wildcard $(tdir)/*.cpp)))
CSOURCES = $(foreach tdir, $(APPSOURCEDIR), $(filter-out $(MAIN_SOURCES), $(wildcard $(tdir)/*.c)))

HEADERS = $(wildcard ./*.h)
HEADERS += $(foreach tdir, $(APPSOURCEDIR), $(wildcard $(tdir)/*.h))

ifeq ($(MAKECMDGOALS), test)
	SOURCES += $(wildcard test/*.cpp) $(wildcard unittest/*.cpp)
	HEADERS += $(wildcard test/*.h) $(wildcard unittest/*.h)
	CXXFLAGS += -Dprivate=public -Dprojected=public -g
else
	SOURCES += $(MAIN_SOURCES)
	CXXFLAGS += -O2	
endif
OBJS =	 $(patsubst %.cpp, $(BUILD)/%.o, $(notdir $(SOURCES)))
COBJS = $(patsubst %.c, $(BUILD)/%.o, $(notdir $(CSOURCES)))

ifeq ($(MAKECMDGOALS), test)
	ifeq ($(LANG),)
	TARGET = $(BUILD)/$(BUILD_TEST_BIN)/unittest_main.exe
	else
	TARGET = $(BUILD)/$(BUILD_TEST_BIN)/unittest_main
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

$(COBJS): $(BUILD)/%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DIR) $< -o $@

$(TARGET): $(OBJS) $(COBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS) $(COBJS) $(LIBS)

.PHONY: test
test: $(TARGET)

install:
	-@mkdir $(INSTALLDIR)
	-@mkdir $(INSTALLDIR)/include/
	-@mkdir $(INSTALLDIR)/include/conf/
	-@echo $(HEADERS)
	-@cp -rf $(HEADERS) $(INSTALLDIR)/include/
	-@cp -rf conf/config.h $(INSTALLDIR)/include/conf/
	-@cp -rf $(TARGET) $(INSTALLDIR)/bin/

clean:
	-@rm $(BUILD)/*.o $(TARGET)
