# LivenessAnalysis
Liveness analysis is a program analysis mechanism - data flow analysis. Checks for the liveness of a variable at a particular program point. Variables are live, if they have any further use in the successors ahead. It is a backward and may analysis. Leads to compiler optimizations like dead code elimination, register allocation. The pass is implemented in LLVM infrastructure.

This is an out of tree pass - no fancy RAM specs required!

How to run:
1. one needs to install llvm library
2. change the path of llvm library in test and Cmake files
3. run test file
