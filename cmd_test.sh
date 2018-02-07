#!/bin/bash

#./resource-query -G test/data/grugs/simple_power_node.graphml -S CA -P power < cmd_inp


./resource-query -G test/data/grugs/simplest_power_node.graphml -S CA -P power -e 1 < cmd_inp_simplest

#/Users/patki1/homebrew/brew/bin/gdb ./resource-query -G test/data/grugs/simple_power_node.graphml -S CA -P power < cmd_inp
