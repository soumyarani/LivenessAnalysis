#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include <bits/stdc++.h>
#include "llvm/ADT/BitVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Operator.h"

/*
* idx - gen
* idx+1 - kill
* idx+2 - in
* idx+3 - out
*/
using namespace llvm;
namespace {
	class Liveness {
	private:
		
		std::map<BasicBlock *,int> idxToBb;
		std::vector<std::set<Value *>> varList;
		std::deque<BasicBlock *>forwardWorkList;
		void printSet(int);
		
	public:
		void printAllSet(int);
		void run(Function *);
		void findGenKill(BasicBlock *);
		bool whetherUpdateInOut(BasicBlock *);
		void findIndex(Value *, BasicBlock *, bool);
	};
	

	void Liveness::printSet(int idx) {
		for(auto itVal : varList[idx])
			errs() << itVal->getName() << " ";
	}//Liveness::printSet

	void Liveness::printAllSet(int idx) {
		idx *=4;
		errs() <<"Gen  :- ";
		printSet(idx);
		errs()<<"\n";
		errs() <<"Kill :- ";
		printSet(idx+1);
		errs()<<"\n";
		errs() <<"In   :- ";
		printSet(idx+2);
		errs()<<"\n";
		errs() <<"Out  :- ";
		printSet(idx+3);
		errs()<<"\n";
	}//Liveness::printAllSet

	/*
	 * Liveness::findIndex
	 * given an instruction finds the corresponding operand
	 * as well as update the corresponding gen
	 */
	void Liveness::findIndex(Value *ip, BasicBlock *bp, bool whetherKill) {
		std::stack<Value *> st;
		auto idx = idxToBb.find(bp)->second;
		idx *=4;

		st.push(ip);
		do {
			ip = st.top();
			st.pop();
			if(isa<Instruction>(ip)) {
				Instruction *tip = (Instruction *)ip;
				if(isa<AllocaInst>(tip)) { //should be store instead? (can't be)
					if(whetherKill) {
						varList[idx+1].insert(ip);
					} else {
						if(varList[idx+1].find(ip) == varList[idx+1].end())
							varList[idx].insert(ip);
					}
					
				} /*else if(isa<StoreInst>(tip)) { // will never come
					st.push((Value *)tip->getOperand(0));
				}*/
				else if(isa<GetElementPtrInst>(tip)) {
					Value *ti = tip->getOperand(0);
					st.push(ti);
				} else {
					int i=0;
					for(User::op_iterator oi = tip->op_begin(), eo = tip->op_end(); eo != oi; oi++,i++) {
						Value *ti = tip->getOperand(i);						
						if((ip != ti) && !isa<ConstantData>(ti)) {
							st.push(ti);
						}
					}	
				}	
			} else if(auto tip = dyn_cast<GEPOperator>(ip)) {
				Value *ti = tip->getPointerOperand();
				st.push(ti); 
			} else if(isa<GlobalVariable>(ip)) {
				
				if(whetherKill) {
					varList[idx+1].insert(ip);
				} else {
					if(varList[idx+1].find(ip) == varList[idx+1].end())
						varList[idx].insert(ip);
				}
			}
			
		} while(!st.empty());
	}// Liveness::findIndex

	/*
	 * Liveness::whetherUpdateInOut
	 */
	bool Liveness::whetherUpdateInOut(BasicBlock *bp) {
		//find OUT
		auto idx = idxToBb.find(bp)->second;
		idx *=4;
		std::set<Value *> bvTemp,bvT;
		
		bvT = varList[idx+3]; //copy out
		for(BasicBlock *suc : successors(bp)) {
			auto tIdx = idxToBb.find(suc)->second;
			tIdx *=4;
			for(auto itVal : varList[tIdx+2])  //union
				bvT.insert(itVal);
		}

		bvTemp = bvT;
		for(auto itVal : varList[idx+1]) {
			auto index = bvTemp.find(itVal);
			if(index != bvTemp.end())
				bvTemp.erase(index);
		}

		for(auto itVal : bvTemp)
			varList[idx+2].insert(itVal);
		

		for(auto itVal : varList[idx])
			varList[idx+2].insert(itVal);

		
		if(varList[idx+3] != bvT) {
			for(auto itVal : bvT)
				varList[idx+3].insert(itVal);
			return true;

		}
		return false;
		
	}// Liveness::whetherUpdateInOut

	/*
	 * Liveness::findGenKill
	 * ------------find gen and kill set for each basic block-------------
	 */
	void Liveness::findGenKill(BasicBlock *bp) {
		Instruction *ip;
		

		for(BasicBlock::iterator ii = bp->begin(), ei = bp->end(); ei != ii; ii++) {
			ip = &(*ii);
			if(isa<StoreInst>(ip)) {
				
				if(!isa<ConstantData>(ip->getOperand(0))) {
					//Gen
					findIndex(ip->getOperand(0),bp,false);
				}
				// kill
				Value *op = ip->getOperand(1);
				findIndex(ip->getOperand(1),bp,true);
				
					
				
			} else {
				int i=0;
				for(User::op_iterator oi = ip->op_begin(), eo = ip->op_end(); eo != oi; oi++,i++) {
					Value *ti = ip->getOperand(i);
					if(((Value *)ip != ti) && !isa<ConstantData>(ti)) // result of instruction is result itself 
						findIndex(ti,bp,false);
				}
			}
		}
	}// Liveness::findGenKill

	/*
	 * Liveness::run
	 * ------------iterate over basic blocks--------------
	 */
	void Liveness::run(Function *fp){
		BasicBlock *bp;
		bool changed = true;
		
		
		int idx=0;
		errs() <<fp->getName()<<"\n";
		for(Function::iterator bb = fp->begin(), eb = fp->end(); eb != bb; bb++) {
			bp = &(*bb);
			forwardWorkList.push_front(bp);
			std::set<Value *> genSet,killSet,inSet,outSet;

			varList.push_back(genSet); //gen
			varList.push_back(killSet); //kill
			varList.push_back(inSet); //in
			varList.push_back(outSet); //out

			// create a map
			idxToBb.insert(std::pair<BasicBlock *,int>(bp,idx));
			idx++;

			findGenKill(bp);
		
		}


errs() << "----------------------firsts-----------------------------\n"; //for live
		for(auto bp : forwardWorkList) {
			whetherUpdateInOut(bp);
errs() << "----------------------"<<bp->getName()<<"-----------------------------\n";
			auto tIdx = idxToBb.find(bp)->second;
			printAllSet(tIdx);
		}
				
		do {
			changed = false;
errs() << "----------------------iteration-----------------------------\n"; //for live
			for(auto bp : forwardWorkList) {
				if(whetherUpdateInOut(bp))
					changed = true;
				
errs() << "----------------------"<<bp->getName()<<"-----------------------------\n";
				auto tIdx = idxToBb.find(bp)->second;
				tIdx *=4;			
				//in per bb
				errs() <<"In :- ";
				printSet(tIdx+2);
				errs()<<"\n";
				//out per bb
				errs() <<"Out :- ";
				printSet(tIdx+3);
				errs()<<"\n";

			}
			
		}while(changed);
	}// Liveness::run 


	class LivenessPass : public FunctionPass {
		public:
			static char ID;
			LivenessPass() : FunctionPass(ID) {}
			virtual bool runOnFunction(Function &F) override {
				Liveness lvObj;
				lvObj.run(&F);
				return false;
			}
	};
}// end namespace

char LivenessPass::ID = 0;
static RegisterPass<LivenessPass> X("liveness",
				    "Liveness analysis Pass",
				    false,
				    false);


