CXX=g++
CXX_FLAGS=-Wall -Werror -Wextra -pedantic
LD_FLAGS=-lncurses -lpcap
QUIET=@
APP=isa-top
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

.PHONY: clean

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

clean:
	rm $(OBJS) $(APP)
