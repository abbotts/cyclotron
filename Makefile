CXX := hipcc
CXXFLAGS := -O -g -I${MPICH_DIR}/include
LDFLAGS := -L${MPICH_DIR}/lib -lmpi ${CRAY_XPMEM_POST_LINK_OPTS} -lxpmem ${PE_MPICH_GTL_DIR_amd_gfx90a} ${PE_MPICH_GTL_LIBS_amd_gfx90a}

EXE := cyclotron

$(EXE): main.o Cyclotron.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cc Cyclotron.h gpu.h

Cyclotron.o: Cyclotron.cc Cyclotron.h gpu.h

clean:
	rm -f *.o $(EXE)
