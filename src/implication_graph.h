#ifndef IMPLICATION_GRAPH
#define IMPLICATION_GRAPH


#include <vector>
#include <iostream>

class Bool_map{
  public:
    unsigned int index;
    std::vector<Bool_map*> parent;
    std::vector<Bool_map*> child;

  Bool_map(unsigned int index): index{index} {}

  void addChild(Bool_map* tar){
    bool repeat_flag = false;
    for(int i = 0; i < child.size(); ++i){
      if(child[i]->index == tar->index){
        repeat_flag = true;
        break;
      }
    }
    if(!repeat_flag){
      child.push_back(tar);
    }

    repeat_flag = false;
    for(int i = 0; i < tar->parent.size(); ++i){
      if(tar->parent[i]->index == index){
        repeat_flag = true;
        break;
      }
    }
    if(!repeat_flag){
      tar->parent.push_back(this);
    }
  }

  void removeChild(Bool_map* tar){
    for(int i = 0; i < child.size(); ++i){
      if(child[i]->index == tar->index){
        child.erase(child.begin() + i);
        break;
      }
    }
    for(int i = 0; i < tar->parent.size(); ++i){
      if(tar->parent[i]->index == index){
        tar->parent.erase(tar->parent.begin() + i);
        break;
      }
    }
  }

  std::vector<unsigned int> myChild(){
    std::vector<unsigned int> child_index;
    for(auto& i : child){
      child_index.push_back(i->index);
    }
    return child_index;
  }

  std::vector<unsigned int> myParents(){
    std::vector<unsigned int> parent_index;
    for(auto& i : parent){
      parent_index.push_back(i->index);
    }
    return parent_index;
  }

  void clean(){
    while(parent.size() != 0){
      parent[0]->removeChild(this);
    }
    while(child.size() != 0){
      this->removeChild(child[0]);
    }
  }

  void show(){
    if(child.size() == 0){
      return;
    }
    printf("%d -> [", index);
    for(int i = 0; i < child.size() - 1; ++i){
      printf("%d, ", child[i]->index);
    }
    printf("%d]\n", child[child.size() - 1]->index);

    for(int i = 0; i < child.size(); ++i){
      child[i]->show();
    }
  }
};

class timeLine{
  public:
    unsigned int index;
    unsigned int i;
    unsigned int j;
  
  timeLine(unsigned int index, unsigned int i, unsigned int j): index{index}, i{i}, j{j} {}
};

#endif
