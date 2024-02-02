#!/bin/sh

assigner -b build/src/template.ll \
         -i src/public-input.json \
         -p src/private-input.json \
         --circuit template.crct \
         --assignment-table template.tbl \
         -e pallas \
	-f dec