#!/bin/bash

#./resource-query -G patki-tests/grugs/grug_1.graphml -S CA -P power -e 1 < cmd_inp_simplest
./resource-query -G test/data/grugs/simple_power_node.graphml -S CA -P var_aware -e 1 -f test_new_108.csv < cmd_inp
#./resource-query -G test/data/grugs/simplest_power_node.graphml -S CA -P var_aware -e 1 -f test_new.csv < cmd_inp_simplest

