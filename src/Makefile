.PHONY : all clean

CXXFLAGS := -Wall -std=c++14 $(shell pkg-config --cflags libevent)
LDFLAGS := $(shell pkg-config --libs libevent)
LDFLAGS += -lmagic

DIRS := $(shell find . -type d)
FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c) \
		                         $(wildcard $(dir)/*.cpp))
OBJS := $(addsuffix .o, $(basename $(FILES)))

# smoky pocket gopher
APP := spg

all: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(APP)

clean:
	rm -f $(OBJS) $(APP) $(OBJS:.o=.d)

%.d: %.cpp
	$(CXX) $(CXXFLAGS) -MM -MF $@ $<

-include $(OBJS:.o=.d)
