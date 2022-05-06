#ifndef BOOLEAN_TOOL
#define BOOLEAN_TOOL

#include <vector>

class BoolNode{
  public:
    unsigned int index;
    bool* value;
    bool not_flag;

  BoolNode(unsigned int index, bool* value, bool not_flag) : index{index}, value{value}, not_flag{not_flag} {}
  bool getValue(){
    if(not_flag){
      return not (*value);
    }
    return (*value);
  }

  void show(){
    std::string temp = not_flag? "-":"";
    temp += (*value)? "True":"False";
    printf("[%d, %s]", index, temp.c_str());
  }

  bool operator+(const BoolNode& rhs){
    bool lhs_value = *value;
    bool rhs_value = *rhs.value;
    if(not_flag){
      lhs_value = not lhs_value;
    }
    if(rhs.not_flag){
      rhs_value = not rhs_value;
    }
    return lhs_value or rhs_value;
  }

  bool operator*(const BoolNode& rhs){
    bool lhs_value = *value;
    bool rhs_value = *rhs.value;
    if(not_flag){
      lhs_value = not lhs_value;
    }
    if(rhs.not_flag){
      rhs_value = not rhs_value;
    }
    return lhs_value and rhs_value;
  }

  bool operator+(bool rhs){
    bool lhs_value = *value;
    if(not_flag){
      lhs_value = not lhs_value;
    }
    return lhs_value or rhs;
  }

  bool operator*(bool rhs){
    bool lhs_value = *value;
    if(not_flag){
      lhs_value = not lhs_value;
    }
    return lhs_value and rhs;
  }
};

inline bool operator+(bool lhs, const BoolNode& rhs){
  bool rhs_value = *rhs.value;
  if(rhs.not_flag){
    rhs_value = not rhs_value;
  }
  return rhs_value or lhs;
}

inline bool operator*(bool lhs, const BoolNode& rhs){
  bool rhs_value = *rhs.value;
  if(rhs.not_flag){
    rhs_value = not rhs_value;
  }
  return rhs_value and lhs;
}

class BoolClause{
  public:
    std::vector<BoolNode> node_list;
    std::vector<int> op_list;

  BoolClause(std::vector<BoolNode> node_list, std::vector<int> op_list) : 
    node_list{node_list}, op_list{op_list} {}

  bool calValue(){
    bool final_result = node_list[0].getValue();
    for(int i = 1; i < node_list.size(); ++i){
      if(op_list[i-1] == 1){
        final_result = final_result * node_list[i];
      }
      else{
        final_result = final_result + node_list[i];
      }
    }
    return final_result;
  }

  void show(){    // Using to test the correctness
    std::string info;
    for(int i = 0; i < node_list.size(); ++i){
      std::string str_var = "x_" + std::to_string(node_list[i].index);
      if(node_list[i].not_flag){
        str_var = "-" + str_var;
      }
      std::string op = "";
      if(i != node_list.size() - 1){
        if(op_list[i] == 1){
          op = " * ";
        }
        else{
          op = " + ";
        }
      }
      info = info + str_var + op;
    }
    printf("%s\n", info.c_str());
  }

  bool operator<(const BoolClause& rhs){    // Using to sort the clauses
    return node_list.size() < rhs.node_list.size();
  }
};

inline bool clauseValue(std::vector<BoolClause> clause_list){
  bool final_result = clause_list[0].calValue();
  for(int i = 1; i < clause_list.size(); ++i){
    final_result = final_result and clause_list[i].calValue();
  }
  return final_result;
}

#endif
