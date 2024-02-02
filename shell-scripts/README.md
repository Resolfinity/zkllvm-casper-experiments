# Shell scripts to build proofs.

Run this scripts in /zkllvm root.
Circuits and private/public inputs should be placed in src.
Use circuits from final-exps folder, place, run, replace with others.

### Auto scripts with time measure

#### auto-single-thread.sh and auto-multi-thread.sh

consists of make, assigner, and prover commands. output working time reports for each step.

prover works in single-thread or multi-thread mode.

justify --shard0-mem-scale flag in multi script. set value twice of your server cores.

problem - no failures on errors, use if you're sure everything is ok.

### Step-by-step scripts

#### make.sh

run first

#### assigner.sh

can be used with -s 40000000 and more value, in case of default stack size generates error.

#### single.sh and multi.sh

runs prover, also justify --shard0-mem-scale
