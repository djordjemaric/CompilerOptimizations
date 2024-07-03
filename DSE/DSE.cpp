#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <llvm/InterfaceStub/IFSStub.h>
#include <unordered_set>
using namespace llvm;

namespace {
struct DSE : public FunctionPass {
    std::unordered_map<Value*, Value*> VariablesMap;

    static char ID;
    DSE() : FunctionPass(ID) {}

    void mapVariables(Function &F) {
        for (BasicBlock &BB: F) {
            for (Instruction &I : BB) {
                if (isa<LoadInst>(&I)) {
                    VariablesMap[&I] = I.getOperand(0);
                }
            }
        }
    }

    bool eliminateDeadStores(Function &F) {

        std::vector<Instruction *> InstructionsToRemove;
        std::unordered_set<Value*> DeadVariables;
        // idemo odozdo na gore
        for (auto BlockItr = F.end(); BlockItr != F.begin();) {
            --BlockItr;
            BasicBlock &BB = *BlockItr;
            for (auto InsItr = BB.rbegin(); InsItr != BB.rend(); ++InsItr) {
                Instruction& I = *InsItr;
                if (isa<LoadInst>(&I)) {
                    continue; // ucitavanje vrednosti ne racunamo kao upotrebu
                }
                if (auto Store = dyn_cast<StoreInst>(&I)) {
                    Value *Operand = Store->getOperand(1);
                    // posto idemo unazad, kada prvi put naidjemo na store u promenljivu,
                    // to je zapravo poslednji store u tu promenljivu u toku programa
                    // taj store ne brisemo, i u tom slucaju kada posmatramo ovu vrednost bice false,
                    // jer je nema u mapi pa ce se pri prvom pozivu init na false
                    // if (VariableDead[Operand]) {
                    //     InstructionsToRemove.push_back(&I);
                    // }
                    if (DeadVariables.find(Operand) != DeadVariables.end()) {
                        InstructionsToRemove.push_back(&I);
                    }
                    DeadVariables.insert(Operand);
                } else {
                    size_t numOperands = I.getNumOperands();
                    for (size_t i = 0; i < numOperands; i++) {
                        Value *Operand = I.getOperand(i);
                        if (VariablesMap.find(Operand) != VariablesMap.end()) {
                            DeadVariables.erase(Operand);
                        }
                    }
                }
            }
        }

        bool removed = !InstructionsToRemove.empty();
        for (Instruction* I : InstructionsToRemove) {
            I->eraseFromParent();
        }
        return removed;
    }


    bool runOnFunction(Function &F) override {
        mapVariables(F);
        return eliminateDeadStores(F);
    }
  };
}

char DSE::ID = 0;
static RegisterPass<DSE> X("simple-dse-pass", "A simple Dead Store Elimination pass.");