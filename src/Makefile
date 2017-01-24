.PHONY : all clean

CPPFLAGS := -Wall -std=c++11
DIRS := $(shell find . -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c) \
		                         $(wildcard $(dir)/*.cpp))
OBJS := $(addsuffix .o, $(basename $(FILES)))

# smoky pocket gopher
APP := spg

all: $(OBJS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) $(OBJS) -o $(APP)

clean:
	rm -f $(OBJS) $(APP) $(OBJS:.o=.d)

%.d: %.cpp
	$(CXX) $(CPPFLAGS) -MM -MF $@ $<

-include $(OBJS:.o=.d)
