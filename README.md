## Input Generator

### Configuration
- compile using:
> make inputs_generator <br>
- run with command:
> ./inputs_generator --num 1000 --len 10000 --path <path_to_the_input_file>

### Parameter
- num - (default: 1000) the number of variables. 
- len - (default: 10000) the length of the expression.
- path - (default: “input.in”) the path of the generated file.


## Serial version

### Configuration
- compile using:
> make sat_solver
- run with command:
> ./sat_solver --path <path_to_the_input_file> --mode 0 --seed 0 

### Parameter
- path - (default: “input.in”) the path to the input file
- mode - (default: 0) decide how to generate the initial value for all variables. Mode 0 will set all variables with 0 as initialized. Mode 1 will set all variables with a random value as initialized (e.g., randomly choose 1 and 0).
- seed - (default: 0) seed used to generate random numbers (Only useful under mode 1)

## MPI parallel version

### Configuration
- compile using:
> make sat_solver_MPI
- run with command:
> mpirun -n 4 ./sat_solver_MPI --path <path_to_the_input_file> --seed 0

### Parameter
- path - (default: “input.in”) the path to the input file
- n - number of MPI processes
- seed- (default: 0) seed used to generate random numbers

