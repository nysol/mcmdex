
CXX = g++

CXXFLAGS = -O3 -Os -g -Wno-c++11-compat-deprecated-writable-strings -I ./lib  -I ./mod

SSPCOBJS =   cmd/msspc.o mod/kgsspc.o lib/stdlib2.o lib/file2.o lib/trsact.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o 

#SSPC4TRA2GCOBJ =  cmd/msspc4tra2gc.o mod/sspctra2gc.o lib/stdlib2.o lib/file2.o lib/trsact.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o lib/fstar.o 

GRHFILOBJS = cmd/mgrhfil.o mod/kggrhfil.o lib/stdlib2.o lib/file2.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o lib/fstar.o 

MACEOBJS = cmd/mmace.o mod/kgmace.o lib/stdlib2.o lib/file2.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o lib/sgraph.o 

MEDSETOBJS = cmd/mmedset.o mod/kgmedset.o lib/stdlib2.o lib/file2.o lib/queue.o lib/vec.o lib/base.o lib/fstar.o 

LCMOBJ = cmd/mlcm.o mod/kglcm.o lib/stdlib2.o lib/file2.o lib/trsact.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o lib/sgraph.o 

LCMSEQOBJ = cmd/mlcmseq.o mod/kglcm_seq.o lib/stdlib2.o lib/file2.o lib/trsact.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o 

SIMSETOBJS = cmd/msimset.o mod/kgsimset.o mod/kgsspc.o mod/kggrhfil.o mod/kgmace.o mod/kgmedset.o lib/stdlib2.o lib/file2.o lib/trsact.o lib/queue.o lib/vec.o lib/base.o lib/itemset.o lib/fstar.o lib/sgraph.o

#all : msspc mgrhfil mmace mmedset mlcm mlcmseq msspc4tra2gc msimset

all : msspc mgrhfil mlcm mlcmseq mmace mmedset msimset

msimset: $(SIMSETOBJS)
	g++ -o msimset $^


msspc: $(SSPCOBJS)
	g++ -o msspc $^

msspc4tra2gc: $(SSPC4TRA2GCOBJ)
	g++ -o msspc4tra2gc $^


mgrhfil: $(GRHFILOBJS)
	g++ -o mgrhfil $^

mmace: $(MACEOBJS)
	g++ -o mmace $^


mmedset: $(MEDSETOBJS)
	g++ -o mmedset $^

mlcm: $(LCMOBJ)
	g++ -o mlcm $^

mlcmseq: $(LCMSEQOBJ)
	g++ -o mlcmseq $^


lib/%.o : lib/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

mod/%.o : mod/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

cmd/%.o : cmd/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<


clean:
	rm -f *.o
	rm -f lib/*.o
	rm -f mod/*.o
	rm -f cmd/*.o
	rm -f msspc mgrhfil mmedset mmace mlcm mlcmseq msspc4tra2gc msimset
