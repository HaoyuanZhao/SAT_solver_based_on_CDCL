CXX = g++
MPICXX = mpic++
PTHREADS = -pthread
CXXFLAGS = -std=c++14 -O3

COMMON = src/cxxopts.h src/boolean_tool.h src/implication_graph.h
HEADERS = src/CDCLSolver.h
TOOL = inputs_generator
SOLVER = sat_solver
THREADS = sat_solver_threads
OBJECTS = src/CDCLSolver.o
MPI = sat_solver_MPI
ALL = $(TOOL) $(SOLVER) $(THREADS) $(MPI) $(OBJECTS)

all: $(ALL)

$(TOOL): %: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(SOLVER): %: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(THREADS): %: %.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(PTHREADS) -o $@ $^

$(MPI): %: %.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $<

$(OBJECTS): $(COMMON) $(HEADERS)

clean: 
	rm -f *.in *.out $(ALL)

.PHONY: inputs_generator sat_solver sat_solver_serial sat_solver_threads sat_solver_MPI
