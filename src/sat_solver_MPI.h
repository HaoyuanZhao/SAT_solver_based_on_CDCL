#ifndef SAT_SOLVER_MPI
#define SAT_SOLVER_MPI

#include "sat_solver_component.h"
#include <mpi.h>

void SATSolverMPI(int world_rank, int world_size, std::string line, unsigned int seed, unsigned long long time_limit){
  timer t_cal_finish, t_total, time_counter;
  t_total.start(); t_cal_finish.start(); time_counter.start();
  unsigned int var_size = sizeCounter(line) + 1;
  bool value_map[var_size] = {0};     // An array to keep the value of each variable
  std::fill_n(value_map, var_size, false);
  std::vector<BoolNode> node_list;    // A list to indicate the order of variable in expression
  std::vector<int> op_list;           // A list to indicate the order of operator in expression
  std::vector<timeLine> time_line_recorder;   
  // Using to record the order of assigned variable 
  // Will be used in back jump process
  std::vector<BoolClause> clause_list;
  bool status_list[var_size] = {0};   // Using to record the initialized status
  unsigned int rank_seed = seed + world_rank; // Using to generate the ramdon initialize value
  for(int i = 0; i < var_size; ++i){
    value_map[i] = (bool)get_random_value(&rank_seed); 
  }
  std::vector<Bool_map> impli_map;    // Using to record the implication map
  for(int i = 0; i < var_size; ++i){  // Initialized the map      
    Bool_map temp(i);
    impli_map.push_back(temp);
  }
  
  stringToBoolNode(line, value_map, node_list, op_list);
  nodeListToClauses(node_list, op_list, clause_list);
  node_list.clear(); op_list.clear();   // Save memory
  std::sort(clause_list.begin(), clause_list.end());    // Sort the clause according to their variable size
  unsigned long input_clause_num = clause_list.size();

  bool fail_to_finish = false;    // Using to record the finish status of current process
  for(int i = 0; i < clause_list.size(); ++i){
    for(int j = 0; j < clause_list[i].node_list.size(); ++j){
      BoolNode& curr_node = clause_list[i].node_list[j];
      unsigned int index = curr_node.index;
      if(!status_list[index]){    // uninitialized
        status_list[index] = 1;
        // Record the order and posotion (e.g., when did it got initialized, and 
        // where it is)
        timeLine curr_order(index, i, j);
        time_line_recorder.push_back(curr_order);
        timeLine jump_back_pos(0, 0, 0);

        // Collision test and this function will also fix the conflict if found
        if(!conflictSolver(index, status_list, value_map, impli_map, time_line_recorder, clause_list, jump_back_pos)){
          // Fixed failed, need to jump back
          // Adjuest the index i and j to jump back
          i = jump_back_pos.i;
          j = jump_back_pos.j - 1;
        }
      }
      // If it is initialized, nothing to do, go to the next variable.
    }
    // Check if any other processes finished the job
    int flag;
    MPI_Iprobe(MPI_ANY_SOURCE , 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    if(flag){
      // Just quit when any thread done the job
      /* printf("%d: Task done by other processes, quit.\n", world_rank); */
      fail_to_finish = true;
      break;
    }

    // Check if overrun the time
    if(time_counter.stop() >= time_limit){
      // I found just start a new solver are not make sense (Too time cousuming)
      // The new solver has lots of work repetition

      /* // Start a new solver */
      /* SAT_SOLVER_MPI(world_rank, world_size, line, seed + world_size, time_limit) */
      
      // Temporary solution: just end the solver
      if(world_rank == 0){
        printf("Cannot finish the task in time, terminate...\n");
      }
      return;
    }
  }

  double time_taken = t_cal_finish.stop();

  if(!fail_to_finish){    // This processes finished task, send notification to other processes
    int done = 1;
    MPI_Request request;
    for(int i = 0; i < world_size; ++i){
      if(i != world_rank){
        MPI_Isend(&done, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
      }
    }
  }

  // Decide who will be the leader, it is possible more than one processes
  // finished the task
  int leader_index = -1;  // Loser cannot be the leader
  int result;
  if(!fail_to_finish){    // If it success, change the index to its rank number 
    leader_index = world_rank;
  }

  // Collect the result from all processes, and the successful processes with
  // largers rank number will be the leader.
  MPI_Allreduce(&leader_index, &result, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  if(result == world_rank){     // Leader will collect the data and print the report 
    unsigned long curr_clause_size[world_size] = {0};
    for(int i = 0; i < world_size; ++i){
      if(world_rank == i){
        curr_clause_size[i] = clause_list.size();
        continue;
      }
      unsigned long temp = 0;
      MPI_Recv(&temp, 1, MPI_UNSIGNED_LONG, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      curr_clause_size[i] = temp;
    }

    double total_time_taken = t_total.stop();

    printf("Number of Threads: %d\nProgram seed: %d\nNumber_clauses before solving: %ld\n\n"
      , world_size, seed, input_clause_num);
    printf("Rank#, time_taken, number_of_learnt_clauses\n");
    for(int i = 0; i < world_size; ++i){
      if(i == world_rank){
        printf("%d, %.*f, %ld\n", i, TIME_PRECISION, time_taken, curr_clause_size[i] - input_clause_num);
      }
      else{
        printf("%d, N/A, %ld\n", i, curr_clause_size[i] - input_clause_num);
      }
    }
    printf("Total program time: %.*f\n", TIME_PRECISION, total_time_taken);
    printf("\nThe solution written to solution_by_MPI.out\n");
    std::ofstream solution_file("solution_by_MPI.out");
    for(int i = 0; i < var_size; ++i){
      std::string temp = "[" + std::to_string(i) + ", " + (value_map[i]? "True":"False") + "]";
      solution_file << temp + "\n";
    }
    solution_file.close();
  }
  else{     // Other processes send the data
    unsigned long send_data = clause_list.size();
    MPI_Send(&send_data, 1, MPI_UNSIGNED_LONG, result, 2, MPI_COMM_WORLD);
  }
}

#endif
