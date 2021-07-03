EXE := server

sources := ${wildcard *.cpp}
headfile := ${wildcard *.h}
objects := ${sources:.cpp=.o}

#CC := g++ -std=c++11 -g
CC  := c++ -std=c++11 -g
RM := rm -rf
LIB := -lpthread -lmysqlclient

${EXE}: ${objects}
	${CC} -o $@ $^ ${LIB}

${objects}: %.o: %.cpp ${headfile}
	${CC} -o $@ -c $<

.PHONY: clean cleanall
cleanall:
	${RM} ${EXE} ${objects}
clean:
	${RM} ${objects}
