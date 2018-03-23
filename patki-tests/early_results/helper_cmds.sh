cat baseline_results.dat | grep ELAPSE | cut -d "=" -f 2 > baseline_sched_time
cat varaware_results.dat | grep ELAPSE | cut -d "=" -f 2 > varaware_sched_time
cat baseline_results.dat | grep RESOURCES | cut -d "=" -f 2 > baseline_alloc_vs_reserve
cat varaware_results.dat | grep RESOURCES | cut -d "=" -f 2 > varaware_alloc_vs_reserve

# The following did not show any difference for this test: same JOB IDs were allocated/reserved.
# 62 jobs were allocated, rest were reserved for future time.
diff --side-by-side varaware_alloc_vs_reserve baseline_alloc_vs_reserve

cd ..
cat cmd_inp_quartz | cut -d 'b' -f 4 | cut -d '_' -f 2 | cut -d '.' -f 1 > jobIDs
