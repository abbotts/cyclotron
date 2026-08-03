CXX := CC
CXXFLAGS := -O -g
LDFLAGS := 

EXE := synchrotron

$(EXE): main.o Synchrotron.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cc Synchrotron.h

Synchrotron.o: Synchrotron.cc Synchrotron.h

clean:
	rm -f *.o $(EXE)
