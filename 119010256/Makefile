CXX = g++
CXXFLAGS = -g
EXEC = simulator
OBJS = assembler.o simulator.o # List of object files needed to
 # build the executable.
SRC = ./src
INC = ./include
${EXEC}: ${OBJS}
	${CXX} ${CXXFLAGS} -o ${EXEC} ${OBJS}
assembler.o: ${SRC}/assembler.cpp ${INC}/assembler.h
	${CXX} ${CXXFLAGS} -c ${SRC}/assembler.cpp
simulator.o: ${SRC}/simulator.cpp ${INC}/assembler.h
	${CXX} ${CXXFLAGS} -c ${SRC}/simulator.cpp
clean:
	rm -f ${EXEC} ${OBJS}
