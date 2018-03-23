#!/bin/bash

#200 jobs, delete last line
cat cmd_inp_quartz | cut -d 'b' -f 4 | cut -d '_' -f 2 | cut -d '.' -f 1 | head -n 200 > inp_jobIDs
cat inp_jobIDs | xargs -I {} sh -c 'head -n {} jobspecs/genscripts/quartz_queue.out | tail -n 1 | cut -d " " -f 2' > numNodes
rm inp_jobIDs
