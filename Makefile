CC := hipcc
CXX := hipcc
CXXFLAGS := -O -g -I$(MPICH_DIR)/include
LDFLAGS := -L$(MPICH_DIR)/lib -lmpi $(PE_MPICH_GTL_DIR_amd_gfx90a) $(PE_MPICH_GTL_LIBS_amd_gfx90a)

EXE := cyclotron

$(EXE): main.o cyclotron.o

main.o: main.cc cyclotron.h gpu.h

cyclotron.o: cyclotron.cc cyclotron.h gpu.h

clean:
	rm -f *.o $(EXE)
