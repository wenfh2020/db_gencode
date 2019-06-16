CXXFLAGS=-I. -g -DDEBUG -Wall -std=c++0x
OBJS= common.o auto_table.o file.o main.o
CXX=g++

EXE=db_gencode

all:$(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(EXE)
