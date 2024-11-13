CXX=g++
CXX_FLAGS=-Wall -Werror -Wextra -pedantic -std=c++11
LD_FLAGS=-lncurses -lpcap
QUIET=@
APP=isa-top
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

.PHONY: clean

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(CXX_FLAGS)  $^ -o $@ $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(APP)
