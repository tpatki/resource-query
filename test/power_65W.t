#!/bin/sh

test_description='Test simple power aware scheduling'

. $(dirname $0)/sharness.sh

cmd_dir="${SHARNESS_TEST_SRCDIR}/data/commands/simple_power"
exp_dir="${SHARNESS_TEST_SRCDIR}/data/expected/simple_power"
grugs="${SHARNESS_TEST_SRCDIR}/data/grugs/simple_power_node.graphml"
query="${SHARNESS_TEST_SRCDIR}/../resource-query"

#
# Selection Policy -- High ID first (-P high)
#     The resource vertex with higher ID is preferred among its kind
#     (e.g., node1 is preferred over node0 if available)
#

cmds004="${cmd_dir}/test02_power.cmds"
test004_desc="match allocate with power on each node (pol=hi)"
test_expect_success "${test004_desc}" '
    ${query} -G ${grugs} -S CA -P high -t 004.R.out < ${cmds004} &&
    test_cmp 004.R.out ${exp_dir}/004.R.out
'

test_done
