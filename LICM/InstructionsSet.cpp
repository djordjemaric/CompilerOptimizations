#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
using namespace llvm;

class InstructionsSet {
public:
    InstructionsSet() : idx(0){}
    void insert(Value *Val) {
        ValueSet.insert(Val);
        SortedSet.insert(std::make_pair(idx,Val));
        idx++;
    }
    bool contains(Value *Val) {
        return ValueSet.find(Val) != ValueSet.end();
    }

    void clear() {
        ValueSet.clear();
        SortedSet.clear();
        Values.clear();
    }

    std::vector<Value *>& getElementsVector() {
        for (auto Val : SortedSet) {
            Values.push_back(Val.second);
        }
        return Values;

    }
private:
    int idx;
    std::set<Value *> ValueSet;
    std::set<std::pair<int,Value*>> SortedSet;
    std::vector<Value *> Values;
};