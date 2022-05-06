#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include "src/cxxopts.h"
#include "src/boolean_tool.h"
#include "src/sat_solver_component.h"

#define DEFAULT_LENGTH_OF_EXPERSSION "10000"
#define DEFAULT_NUMBER_OF_VARIABLE "1000"
#define DEFAULT_PATH "input.in"

int main(int argc, char* argv[]){
  cxxopts::Options options("Random logical operator generator",
                           "Genarating AND OR NOT operators randomly");
  options.add_options(
      "custom",
      {
          {"len", "Length of the expression",         
           cxxopts::value<unsigned int>()->default_value(DEFAULT_LENGTH_OF_EXPERSSION)},
          {"num", "Number of variables",
           cxxopts::value<unsigned int>()->default_value(DEFAULT_NUMBER_OF_VARIABLE)},
	        {"path", "path of the output file",
	         cxxopts::value<std::string>()->default_value(DEFAULT_PATH)},
      });
  auto cl_options = options.parse(argc, argv);
  unsigned int len_of_exp = cl_options["len"].as<unsigned int>();
  unsigned int num_of_var = cl_options["num"].as<unsigned int>();
  std::string path = cl_options["path"].as<std::string>();

  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<int> dist_op(0,1);    //P(AND) = 50%, P(OR) = 50%
  std::uniform_int_distribution<int> dist_not(0,1);   //P(NOT) = 50%
  std::uniform_int_distribution<int> dist_initial_value(0,1);   //initial value for node: True = False = 50%
  std::ofstream file;
  std::vector<int> index_list;    //Using to keep the index of the variable (e.g., 0 - 999 by default) 
  bool value_map[num_of_var] = {0}; //Using to store all variables

  // Make sure every variable appears at least once.
  for(int i = 0; i < num_of_var; ++i){
    index_list.push_back(i);
    if(dist_initial_value(generator) == 1){
      value_map[i] = 1;
    }
  }
  // Fill the rest using random variable
  if(len_of_exp > num_of_var){
    std::uniform_int_distribution<int> dist_var(0,num_of_var - 1);
    for(int i = 0; i < len_of_exp - num_of_var; ++i){
      index_list.push_back(dist_var(generator));
    }
  }

  //shuffle the entire index_list to make it randomly distributed.
  std::shuffle(index_list.begin(), index_list.end(), generator);

  std::vector<BoolNode> node_list;
  std::vector<int> op_list;      // Using 0 to represent OR(+), 1 to represent AND(*)
  int clause_length_control = 0;  // Using it to make sure the length of all clauses are >= 3
  for(int i = 0; i < len_of_exp; ++i){
    BoolNode temp_node(index_list[i], &value_map[index_list[i]], 0);
    if(dist_not(generator) == 1){
      temp_node.not_flag = 1;
    }
    node_list.push_back(temp_node);

    if(dist_op(generator) == 1 && clause_length_control > 2){
      op_list.push_back(1);
      clause_length_control = 0;
    }
    else{
      op_list.push_back(0);
    }
    clause_length_control++;
  }
  op_list.pop_back();   //Discard the last op, since the last var have no further op.
  // printf("The number of node is %ld, number of op is %ld", node_list.size(), op_list.size());

  // Modify the random node_list to make sure it has valid solution
  // Basic logic is to make sure all clause has a True result
  // So if any clause was found as False, I will change the NOT flag for the first variable, 
  // which will makes the result become to True
  unsigned int clause_start = 0;
  for(int i = 0; i < op_list.size(); ++i){
    if(op_list[i] == 0){    // OR operator, not the end of current clause, continue.
      continue;
    }
    // And operator
    bool result = node_list[clause_start].getValue();
    for(int j = clause_start + 1; j <= i; ++j){   // Range for the current clause
      result = result + node_list[j];     // Clause only has OR inside
    }
    if(!result){    // False clause found, which also means all variables in this clause is False
      std::uniform_int_distribution<int> dist_index(clause_start,i);
      int random_index = dist_index(generator);
      node_list[random_index].not_flag = not node_list[random_index].not_flag;    // Flip the not flag for a random variable in the clause
    }
    clause_start = i + 1;
  }

  // However, the above code didn't check the last clause
  bool result = node_list[clause_start].getValue();
  for(int i = clause_start + 1; i < node_list.size(); ++i){   // Range for the last clause
    result = result + node_list[i];     // Clause only has OR inside
  }
  if(!result){    // False clause found, which also means all variables in this clause is False
    node_list[clause_start].not_flag = not node_list[clause_start].not_flag;    // Flip the not flag
  }

  //Write the result into file
  file.open(path);
  for(int i = 0; i < node_list.size(); ++i){
    std::string str_var = "x_" + std::to_string(node_list[i].index);
    if(node_list[i].not_flag){
      str_var = "-" + str_var;
    }
    std::string op = "";
    if(i != node_list.size() - 1){
      if(op_list[i]){
        op = " * ";
      }
      else{
        op = " + ";
      }
    }
    file << str_var + op;
  }
  file.close();

  std::vector<BoolClause> clause_list;
  nodeListToClauses(node_list, op_list, clause_list);

  if(clauseValue(clause_list)){
    printf("Number of variables: %d\nLength of the expression: %d\n", num_of_var, len_of_exp);
    printf("Verification succeeded, this input has a valid solution.\n");
  }
  else{
    printf("!!!Warning: No valid solution found!!!\n");
  }

  file.open("solution_by_generator.out");
  for(int i = 0; i < num_of_var; ++i){
    std::string temp = "[" + std::to_string(i) + ", " + (value_map[i]? "True":"False") + "]";
    file << temp + "\n";
  }
  file.close();

  return 0;
}
