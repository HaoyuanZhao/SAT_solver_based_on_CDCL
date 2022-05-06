#ifndef SAT_SOLVER_COMPONENT
#define SAT_SOLVER_COMPONENT

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h>
#include <random>
#include <stdlib.h>
#include "boolean_tool.h"
#include "implication_graph.h"
#include "get_time.h"

#define TIME_PRECISION 5

// This function will return the largest digits found in the string
unsigned int sizeCounter(std::string tar){
  int num = 0, res = 0;
  for (int i = 0; i < tar.length(); i++){
    if(tar[i] >= '0' && tar[i] <= '9'){
      num = num * 10 + (tar[i] - '0');
    }
    else{
      res = std::max(res, num);
      num = 0;
    }
  }
  return std::max(res, num);
}

void stringToBoolNode(std::string tar, bool* value_map, 
  std::vector<BoolNode>& node_list, std::vector<int>& op_list){
  unsigned long long start = 0; 
  while(start < tar.length()){
    unsigned long long end = tar.find(" ", start);
    if(end == std::string::npos){   // No space found, ready to end
      end = tar.length();
    }
    std::string sub_str = tar.substr(start, end - start);
    if(sub_str.length() > 1){   // Is a variable
      unsigned int index = atoi(sub_str.substr(sub_str.find("_") + 1, std::string::npos).c_str());
      BoolNode temp_node(index, (value_map + index), 0);
      if(sub_str.find("-") == 0){
        temp_node.not_flag = 1;
      }
      node_list.push_back(temp_node);
    }
    else{   // Is a operator
      sub_str == "+"? op_list.push_back(0) : op_list.push_back(1);
    }
    start = end + 1;
  }
}

void nodeListToClauses(const std::vector<BoolNode>& node_list, const std::vector<int>& op_list,
  std::vector<BoolClause>& clause_list){
  unsigned int clause_start = 0;
  for(int i = 0; i < op_list.size(); ++i){
    if(op_list[i] == 0){
      continue;
    }
    std::vector<BoolNode> sub_node_list;
    std::vector<int> sub_op_list;
    for(int j = clause_start; j < i; ++j){
      sub_node_list.push_back(node_list[j]);
      sub_op_list.push_back(op_list[j]);
    }
    sub_node_list.push_back(node_list[i]);    // Each clause have x+1 variables and x op
    BoolClause temp_clause(sub_node_list, sub_op_list);
    clause_list.push_back(temp_clause);

    clause_start = i + 1;
  }

  // Add the last clause
  std::vector<BoolNode> sub_node_list;
  std::vector<int> sub_op_list;
  for(int i = clause_start; i < node_list.size(); ++i){
    sub_node_list.push_back(node_list[i]);
    sub_op_list.push_back(0);
  }
  sub_op_list.pop_back();
  BoolClause temp_clause(sub_node_list, sub_op_list);
  clause_list.push_back(temp_clause);
}

long conflictCheck(bool* status_list, std::vector<BoolClause>& clause_list){
  for(int i = 0; i < clause_list.size(); ++i){
    bool all_assigned = true;
    for(int j = 0; j < clause_list[i].node_list.size(); ++j){
      if(*(status_list + clause_list[i].node_list[j].index) == 0){
        all_assigned = false;
        break;
      }
    }
    // Only check the value when all variable had assigned, otherwise 
    // no conflict are possible
    if(all_assigned){   
      if(!clause_list[i].calValue()){   // conflict found !!!
        return i;
      }
    }
  }
  return -1;
}

void addConflictInMap(unsigned int tar_index, unsigned int tar_clause, std::vector<Bool_map>& impli_map, 
    std::vector<BoolClause>& clause_list){
  for(int i = 0; i < clause_list[tar_clause].node_list.size(); ++i){
    unsigned int curr_index = clause_list[tar_clause].node_list[i].index;
    if(curr_index != tar_index){
      impli_map[curr_index].addChild(&impli_map[tar_index]);
    }
  }
}

bool conflictSolver(unsigned int tar_index, bool* status_list, bool* value_map, std::vector<Bool_map>& impli_map, 
    std::vector<timeLine>& time_line_recorder, std::vector<BoolClause>& clause_list, timeLine& jump_back_pos){
  long index_recorder = conflictCheck(status_list, clause_list);
  if(index_recorder != -1){
    // Conflict found, try to fix the conflict by flip the tar_variable
    // Also add them into the implication map
    addConflictInMap(tar_index, index_recorder, impli_map, clause_list);
    *(value_map + tar_index) = not *(value_map + tar_index);
    index_recorder = conflictCheck(status_list, clause_list);

    if(index_recorder != -1){
      // Still have a conflict, we need to jump back to fix it...
      // And if happened, this conflict must from the clause where 
      // tar_index belongs to
      // Also add them into the implication map
      addConflictInMap(tar_index, index_recorder, impli_map, clause_list);
      // Create the new learned clause, and insert it into clause.
      std::vector<unsigned int> parents_index = impli_map[tar_index].myParents();
      std::vector<BoolNode> new_node_list;
      std::vector<int> new_bool_list;
      for(int i = 0; i < parents_index.size(); ++i){
        BoolNode temp(parents_index[i], value_map + parents_index[i], *(value_map + parents_index[i]));
        // The variable has NOT when it is 1, and does not have NOT when it is 0.
        new_node_list.push_back(temp);
        new_bool_list.push_back(0);
      }
      new_bool_list.pop_back();
      BoolClause learned_clause(new_node_list, new_bool_list);
      clause_list.push_back(learned_clause);
      // Then figure out where should we go back
      // Find the lastest assigned variable in the learned clause
      unsigned int last_index;
      unsigned int last_index_pos;
      for(int i = 0; i < time_line_recorder.size(); ++i){
        if(std::find(parents_index.begin(), parents_index.end(), time_line_recorder[i].index) 
            != parents_index.end()){
          last_index = time_line_recorder[i].index;
          last_index_pos = i;
        }
      }
      // Now we will jump back to the time before we assigned LAST_INDEX
      // Erase all reocrds after this variable (also including itself)
      for(int i = last_index_pos; i < time_line_recorder.size(); ++i){
        unsigned int temp_index = time_line_recorder[i].index;
        // Erase the initialize status
        *(status_list + temp_index) = 0;
        // Clean its implication map
        impli_map[i].clean();
      }
      // Clean the time_line_recorder
      jump_back_pos = time_line_recorder[last_index_pos];
      time_line_recorder.erase(time_line_recorder.begin() + last_index_pos, time_line_recorder.end());
      return false;
    }
    // flip fixed this problem... jobs done
  }
  // No conflict found, just reutrn
  return true;
}

int get_random_value(unsigned int* random_seed) {
  return ((int)rand_r(random_seed)) % 2;  // thread-safe random number generator
}

void SATSolver(std::string line, int mode, unsigned int seed){
  timer t1;
  t1.start();
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
  std::vector<Bool_map> impli_map;    // Using to record the implication map
  for(int i = 0; i < var_size; ++i){  // Initialized the map              
    Bool_map temp(i);
    impli_map.push_back(temp);
  }
  unsigned int seed_temp = seed;
  if(mode == 1){    // If user choose the random initialized value
    for(int i = 0; i < var_size; ++i){
      value_map[i] = (bool)get_random_value(&seed_temp);
    }
  }

  stringToBoolNode(line, value_map, node_list, op_list);
  nodeListToClauses(node_list, op_list, clause_list);
  node_list.clear(); op_list.clear();   // Save memory
  std::sort(clause_list.begin(), clause_list.end());    // Sort the clause according to their variable size
  unsigned long input_clause_num = clause_list.size();

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
  }
  
  printf("Number of Threads: 1\nProgram mode: %d\nProgram seed: %d\nNumber_clauses before solving: %ld\n\n"
      , mode, seed, input_clause_num);
  printf("thread_id, time_taken, number_of_learnt_clauses\n0, %.*f, %ld\n", TIME_PRECISION, t1.stop(), clause_list.size() - input_clause_num);
  printf("\nThe solution written to solution_by_serial.out\n");

  std::ofstream solution_file("solution_by_serial.out");
  for(int i = 0; i < var_size; ++i){
    std::string temp = "[" + std::to_string(i) + ", " + (value_map[i]? "True":"False") + "]";
    solution_file << temp + "\n";
  }
  solution_file.close();
}

#endif
