CXX=g++
CXX_FLAGS=-Wall -Werror -Wextra -pedantic
LD_FLAGS=-lncurses -lpcap
QUIET=@
APP=isa-top
SRC=src
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(SRC)/%.o, $(SRCS))

.PHONY: clean

all: $(APP)

$(APP): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXX_FLAGS) -c $^ -o $@

clean:
	rm $(OBJS) $(APP)