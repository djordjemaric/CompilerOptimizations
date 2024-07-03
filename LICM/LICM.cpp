#include "InstructionsSet.cpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"

#include <unordered_set>

using namespace llvm;

namespace {
    struct LICMPass : public LoopPass {
        std::vector<BasicBlock *> LoopBasicBlocks;
        std::unordered_map<Value*, Value*> VariablesMap;
        std::unordered_set<Value*> UsedVariables;
        Instruction* InsertBefore;
        InstructionsSet InvariantInstructions;


        static char ID;
        LICMPass() : LoopPass(ID) {}

        void mapVariables() {
            Function *Parent = LoopBasicBlocks[0]->getParent();
            for (BasicBlock &BB: *Parent) {
                for (Instruction &I : BB) {
                    if (isa<LoadInst>(&I)) {
                        VariablesMap[&I] = I.getOperand(0);
                    }
                }
            }
        }

        // We will handle the following instructions:
        // addition, subtraction, multiplication, division

        bool isInvariant(Value* Val) {
            // check whether it is a variable, const or instruction
            if (isa<Constant>(Val))
                return true;

            // if it is a variable, check if any value is stored in it during the execution of the loop
            if (VariablesMap.find(Val) != VariablesMap.end()) {
                // if we already determined that something is stored in the variable during the loop, end search early
                if (UsedVariables.find(VariablesMap[Val]) != UsedVariables.end())
                    return false;

                for (BasicBlock *BB : LoopBasicBlocks) {
                    for (Instruction &I : *BB) {
                        if (isa<StoreInst>(&I) && I.getOperand(1) == VariablesMap[Val]) {
                            UsedVariables.insert(VariablesMap[Val]);
                            return false;
                        }
                    }
                }
                return true;
            }

            if (isa<MulOperator, AddOperator, SubOperator, SDivOperator>(Val)) {
                auto I = dyn_cast<Instruction>(Val);
                size_t numOperands = I->getNumOperands();
                for (size_t i = 0; i < numOperands; i++) {
                    // check if every operand is invariant
                    if (InvariantInstructions.contains(I->getOperand(i)))
                        continue;
                    if (!isInvariant(I->getOperand(i))) {
                        return false;
                    }
                }
                return true;
            }

            return false;
        }

        bool extractInvariants() {

            std::unordered_map<Value*, Value*> InstructionMap;
            std::vector<Value* > &InstructionsToCopy = InvariantInstructions.getElementsVector();

            Instruction *InstrCopy, *I;
            for (auto Val : InstructionsToCopy){
                I = dyn_cast<Instruction>(Val);
                InstrCopy = I->clone();
                InstrCopy->insertBefore(InsertBefore);
                InstructionMap[I] = InstrCopy;
            }

            for (auto Val : InstructionsToCopy){
                I = dyn_cast<Instruction>(Val);
                InstrCopy = dyn_cast<Instruction>(InstructionMap[I]);
                size_t numOperands = InstrCopy->getNumOperands();

                for (size_t i = 0; i < numOperands; i++) {
                    if (InstructionMap.find(InstrCopy->getOperand(i)) != InstructionMap.end()) {
                        InstrCopy->setOperand(i, InstructionMap[InstrCopy->getOperand(i)]);
                    }
                }
                I->replaceAllUsesWith(InstructionMap[I]);
            }
            bool removed = !InstructionsToCopy.empty();
            for (auto I : InstructionsToCopy) {
                auto Instr = dyn_cast<Instruction>(I);
                Instr->eraseFromParent();
            }

            return removed;

        }

         void findInvariants() {
            for (BasicBlock *BB : LoopBasicBlocks) {
                for (Instruction &I : *BB) {
                    if (isInvariant(&I)) {
                        InvariantInstructions.insert(&I);
                    }
                }
            }
        }

        bool runOnLoop(Loop *L, LPPassManager&) override {
            LoopBasicBlocks = L->getBlocksVector();
            InsertBefore = L->getLoopPreheader()->getTerminator();

            mapVariables();
            findInvariants();
            bool removed = extractInvariants();

            VariablesMap.clear();
            InvariantInstructions.clear();

            return removed;
        }
    };
}

char LICMPass::ID = 0;
static RegisterPass<LICMPass> X("licm-pass", "This is a simple LICM pass!");