.PHONY : all clean

CXXFLAGS := -Wall -std=c++14 $(shell pkg-config --cflags libevent)
LDFLAGS := $(shell pkg-config --libs libevent)
LDFLAGS += -lmagic

DIRS := $(shell find . -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c) \
		                         $(wildcard $(dir)/*.cc))
OBJS := $(addsuffix .o, $(basename $(FILES)))

# smoky pocket gopher
APP := goofy

all: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(APP)

clean:
	rm -f $(OBJS) $(APP) $(OBJS:.o=.d)

%.d: %.cc
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

-include $(OBJS:.o=.d)
