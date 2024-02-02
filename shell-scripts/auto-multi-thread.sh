#!/bin/sh

# ANSI escape code for bold yellow
BOLD_YELLOW="\033[1;33m"
# ANSI escape code to reset text formatting
RESET="\033[0m"

# Function to extract elapsed seconds from time output
extract_seconds() {
    grep 'elapsed' "$1" | awk -F 'elapsed' '{print $1}' | awk '{print $NF}'
}

# Measure time for 'make' command
(time make -C ${ZKLLVM_BUILD:-build} template) 2> time_make.txt
echo -e "${BOLD_YELLOW}Time for make command: $(extract_seconds time_make.txt) seconds${RESET}"
echo # Line break

# Measure time for 'assigner' command
(time assigner -b build/src/template.ll \
         -i src/public-input.json \
         -p src/private-input.json \
         --circuit template.crct \
         --assignment-table template.tbl \
         -e pallas -f dec -s 40000000) 2> time_assigner.txt
echo -e "${BOLD_YELLOW}Time for assigner command: $(extract_seconds time_assigner.txt) seconds${RESET}"
echo # Line break

# Measure time for 'proof-generator-single-threaded' command
(time proof-generator-multi-threaded \
    --circuit="template.crct" \
    --assignment-table="template.tbl" \
    --proof="proof.bin" \
   --skip-verification \
    --shard0-mem-scale=24) 2> time_proof_generator.txt

echo -e "${BOLD_YELLOW}Time for proof generator command: $(extract_seconds time_proof_generator.txt) seconds${RESET}"
echo # Line break