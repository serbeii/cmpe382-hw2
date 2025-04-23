#!/bin/bash

rm -rf myDir
mkdir myDir
cd myDir

# --- Function to generate numbers for one file efficiently ---
generate_numbers_for_file() {
  local i=$1
  local count=10000000 # Numbers per file
  local filename="file${i}.txt"
  echo "Generating $filename ..."
  # Use awk (often faster)
  awk -v count="$count" -v seed="$$$i" 'BEGIN{srand(seed); for(j=1;j<=count;j++) print int(32768*rand())}' > "$filename"
  # Or shell loop redirected once (closer to original logic, potentially slower)
  # ( for j in $(seq 1 "$count"); do echo $RANDOM; done ) > "$filename"
  echo "Finished $filename"
}
export -f generate_numbers_for_file # Make function available to subshells

# --- Parallel Execution Logic ---
MAX_PROCS=$(nproc) # Get number of CPU cores, or set manually e.g., 8
echo "Running with up to $MAX_PROCS parallel processes."

TOTAL_FILES=10

for i in $(seq 1 $TOTAL_FILES); do
   generate_numbers_for_file "$i" & # Launch in background

   # Limit the number of concurrent jobs
   while [[ $(jobs -p | wc -l) -ge $MAX_PROCS ]]; do
       wait -n # Wait for any single background job to finish
   done
done

# Wait for all remaining background jobs to complete
wait
cd ..

echo "All files generated."
