#!/bin/sh

proof-generator-single-threaded \
    --circuit="template.crct" \
    --assignment-table="template.tbl" \
    --proof="proof.bin" --skip-verification