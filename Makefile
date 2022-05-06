CXX = g++
MPICXX = mpic++
PTHREADS = -pthread
CXXFLAGS = -std=c++14 -O3

COMMON = src/cxxopts.h src/boolean_tool.h src/implication_graph.h
TOOL = inputs_generator
SOLVER = sat_solver
MPI = sat_solver_MPI
ALL = $(TOOL) $(SOLVER) $(MPI)

all: $(ALL)

$(TOOL): %: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(SOLVER): %: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(MPI): %: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

clean: 
	rm -f *.in *.out $(ALL)

.PHONY: inputs_generator sat_solver sat_solver_MPI
