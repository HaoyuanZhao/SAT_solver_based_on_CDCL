#include "src/cxxopts.h"
#include "src/sat_solver_component.h"

#define DEFAULT_PATH "input.in"
#define DEFAULT_MODE "0"
#define DEFAULT_SEED "0"


int main(int argc, char* argv[])
{
  cxxopts::Options options("SAT problem solver",
                           "Base on algorithm CDCL");
  options.add_options(
      "custom",
      {
	        {"path", "path of the input file",
	         cxxopts::value<std::string>()->default_value(DEFAULT_PATH)},
	        {"mode", "Decide the initialized value for the variable",
	         cxxopts::value<int>()->default_value(DEFAULT_MODE)},
          {"seed", "seed for the random number generator",
          cxxopts::value<unsigned int>()->default_value(DEFAULT_SEED)},
      });
  auto cl_options = options.parse(argc, argv);
  std::string path = cl_options["path"].as<std::string>();
  int mode = cl_options["mode"].as<int>();
  unsigned int seed = cl_options["seed"].as<unsigned int>();

  std::ifstream file(path);
  std::string line;
  if(!file.is_open()){
    printf("Cannot open the input file, quit.\n");
    exit(0);
  }
  getline(file, line);
  file.close();

  SATSolver(line, mode, seed);
  
  return 0;
}
