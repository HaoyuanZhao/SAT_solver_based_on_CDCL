#include "src/cxxopts.h"
#include "src/sat_solver_MPI.h"

#define DEFAULT_PATH "input.in"
#define DEFAULT_SEED "0"
#define DEFAULT_TIMER "300"

int main(int argc, char* argv[])
{
  cxxopts::Options options("SAT problem solver",
                           "Base on algorithm CDCL");
  options.add_options(
      "custom",
      {
	        {"path", "path of the input file",
	         cxxopts::value<std::string>()->default_value(DEFAULT_PATH)},
          {"seed", "seed for the random number generator",
	        cxxopts::value<unsigned int>()->default_value(DEFAULT_SEED)},
          {"timer", "Set timer for the SAT solver (in seconds)",
	        cxxopts::value<unsigned long long>()->default_value(DEFAULT_TIMER)},
      });
  auto cl_options = options.parse(argc, argv);
  std::string path = cl_options["path"].as<std::string>();
  unsigned int seed = cl_options["seed"].as<unsigned int>();
  unsigned long long timer = cl_options["timer"].as<unsigned long long>();
  
  MPI_Init(NULL, NULL);
  int world_rank;
  int world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  
  std::ifstream file(path);
  std::string line;
  if(!file.is_open()){
    if(world_rank == 0){
      printf("Cannot open the input file, quit.\n");
    }
    exit(0);
  }
  getline(file, line);
  file.close();

  SATSolverMPI(world_rank, world_size, line, seed, timer);

  MPI_Finalize();

  return 0;
}
