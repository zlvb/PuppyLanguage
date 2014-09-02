# Makefile
# 
# usage :
#     make
#     make BUILD=release
#	  make clean
# 

TARGETNAME = bin/vm

CXXFLAGS = -g -D_DEBUG -Wall
OBJPATH = debug

TARGET = $(TARGETNAME)_d

ifeq ($(BUILD), release)
	CXXFLAGS = -O3 -Wall
	OBJPATH = release
	TARGET = $(TARGETNAME)
endif


SRCS = $(wildcard *.cpp src/*.cpp test/*.cpp)
OBJS = $(SRCS:%.cpp=$(OBJPATH)/%.o)

$(TARGET) : $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

clean :
	rm -rf release
	rm -rf debug

$(OBJPATH)/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJPATH)/%.d : %.cpp
	@echo $(CXX) -MM -o $@ -c $<
	@rm -f $@
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MM -MT '$@ $(basename $@).o' $< -o $@
	

-include $(SRCS:%.cpp=$(OBJPATH)/%.d)

