# LivenessAnalysis
Liveness analysis is a program analysis mechanism - data flow analysis. Checks for the liveness of a variable at a particular program point. Variables are live, if they have any further use in the successors ahead. It is a backward and may analysis. Leads to compiler optimizations like dead code elimination, register allocation. The pass is implemented in LLVM infrastructure.
