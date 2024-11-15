CXX=g++
CXX_FLAGS=-Wall -Werror -Wextra -pedantic -std=c++11
LD_FLAGS=-lncurses -lpcap
QUIET=@
APP=isa-top
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

.PHONY: clean, tar

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(CXX_FLAGS)  $^ -o $@ $(LD_FLAGS)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

tar:
	tar cf xpanek11.tar argument_parser.cpp argument_parser.hpp capturing_utils.cpp capturing_utils.hpp flow_monitor.cpp flow_monitor.hpp flow_table.cpp flow_table.hpp main.cpp ncurses_terminal_view.cpp ncurses_terminal_view.hpp isa-top.1 Makefile manual.pdf

clean:
	rm $(OBJS) $(APP)
