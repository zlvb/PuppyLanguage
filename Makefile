# Makefile
# 
# usage :
#     make
#     make BUILD=release
#	  make clean
# 

TARGETNAME = puppy

ifeq ($(BUILD), release)
	CXXFLAGS = -O3 -Wall
	OBJPATH = Release
	TARGET_BIN = bin/$(TARGETNAME)
	TARGET_LIB = lib/lib$(TARGETNAME).a
else
	CXXFLAGS = -g -D_DEBUG -Wall
	OBJPATH = Debug
	TARGET_BIN = bin/$(TARGETNAME)_d
	TARGET_LIB = lib/lib$(TARGETNAME)_d.a
endif


SRCS_BIN = $(wildcard test/*.cpp)
OBJS_BIN = $(SRCS_BIN:%.cpp=$(OBJPATH)/%.o)
SRCS_LIB = $(wildcard *.cpp src/*.cpp)
OBJS_LIB = $(SRCS_LIB:%.cpp=$(OBJPATH)/%.o)

$(TARGET_BIN) : $(OBJS_BIN) $(TARGET_LIB)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ 

$(TARGET_LIB) : $(OBJS_LIB)
	mkdir -p lib
	$(AR) rc $(TARGET_LIB) $(OBJS_LIB)

clean :
	rm -rf Release
	rm -rf Debug
	rm -rf bin

$(OBJPATH)/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJPATH)/%.d : %.cpp
	@echo $(CXX) -MM -o $@ -c $<
	@rm -f $@
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MM -MT '$@ $(basename $@).o' $< -o $@
	

-include $(SRCS:%.cpp=$(OBJPATH)/%.d)

