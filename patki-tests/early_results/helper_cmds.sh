cat baseline_low.dat | grep ELAPSE | cut -d "=" -f 2 > low_sched_time
cat baseline_high.dat | grep ELAPSE | cut -d "=" -f 2 > high_sched_time
cat varaware_results.dat | grep ELAPSE | cut -d "=" -f 2 > varaware_sched_time
cat baseline_low.dat | grep RESOURCES | cut -d "=" -f 2 > low_alloc_vs_reserve
cat baseline_high.dat | grep RESOURCES | cut -d "=" -f 2 > high_alloc_vs_reserve
cat varaware_results.dat | grep RESOURCES | cut -d "=" -f 2 > varaware_alloc_vs_reserve

# The following did not show any difference for this test: same JOB IDs were allocated/reserved.
# 62 jobs were allocated, rest were reserved for future time.
diff --side-by-side varaware_alloc_vs_reserve low_alloc_vs_reserve

cd ..
cat cmd_inp_quartz | cut -d 'b' -f 4 | cut -d '_' -f 2 | cut -d '.' -f 1 > jobIDs
