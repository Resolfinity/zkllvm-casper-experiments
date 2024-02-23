#!/bin/bash

# Define variables
local_json_file="private-input.json"
dir_name="$1" # Use the first argument as the directory name
local_cpp_file="./recursive-circuit/${dir_name}/main.cpp" # Use the directory name in the path
zip_file="private-input.zip"
remote_dir="/home/dima/zknew/src"
remote_host="root@noda"

# Step 1: Zip the private-input.json file
zip -r $zip_file $local_json_file

# Step 2: Upload the zipped file via SSH
scp $zip_file $remote_host:$remote_dir

# Step 2b: Upload the main.cpp file
scp $local_cpp_file $remote_host:$remote_dir

# Step 3: Connect via SSH and unzip the file
ssh $remote_host "rm -f /home/dima/zknew/template.* && unzip -o $remote_dir/$zip_file -d $remote_dir && rm $remote_dir/$zip_file"

# Optional: Cleanup by removing the local zip file
rm $zip_file
