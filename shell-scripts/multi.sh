#!/bin/sh

proof-generator-multi-threaded \
    --circuit="template.crct" \
    --assignment-table="template.tbl" \
    --proof="proof.bin" \
   --skip-verification \
    --shard0-mem-scale=24