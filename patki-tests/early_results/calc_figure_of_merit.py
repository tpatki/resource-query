# Calculate the degree of performance heterogeneity
# Based on two input files
# Output perjob difference
import csv 
from collections import defaultdict

# Create a global dictionary once for the distribution file 
d_dict = defaultdict(list)

def create_dict (fdist): 
    with open(fdist) as f:
        reader = csv.reader(f)
        for i in list(reader):
            d_dict[i[0]] = i[1]
#    print dict(d_dict)

# Inputs to this function are a list of nodes corresponding to a job, and the node distribution
def calc_fom_for_job(nr):
    max_pc = 0
    min_px = 0 
    for i in nr:
        perf_cls = int(d_dict.get(i))
        max_pc = max (max_pc, perf_cls)
        min_pc = min (max_pc, perf_cls)
        # print str(i) + " " +  str(perf_cls)
        # print max_pc + " " + min_pc
    diff = max_pc - min_pc
    return max_pc, min_pc, diff
    
# Inputs to this function are the file with resource-query results, and the node distribution
def calc_fom_for_trace (fin, fout):      
    f = open (fin, 'r')
    fo = open (fout, 'w')
    fo.write ('JobID' + ' ' + 'MaxPC' + ' ' + 'MinPC' + ' ' + 'Diff\n')
    contents = f.read()
    strn = 'node'
    strj = 'JOB'
    noderow = []    # Define empty list
    file_as_list = contents.splitlines()
    
    for line in file_as_list:
        if line.find(strn) != -1 :
            tmp = line.split('e')[1]
            noderow.append(tmp.split('[')[0])  #Extract the numeric ID of the node
        if line.find(strj) != -1 :
            jid = line.split('=')[1]    #Extract JobID
            # We have the Node IDs for a particular Job at this point
            # Now, let us calculate the maximum performance difference betweeen nodes
            max_pc, min_pc, fom = calc_fom_for_job(noderow)    
            fo.write (jid + ' ' + str(max_pc) + ' ' + str(min_pc) + ' ' + str(fom) + '\n')
            #Reset the noderow and move to the next job
            noderow = []

    f.close()
    fo.close()

def main():
    create_dict('../quartz-dist.csv')
    calc_fom_for_trace('baseline_low.dat', 'baseline_low_fom.dat')
    calc_fom_for_trace('baseline_high.dat', 'baseline_high_fom.dat')
    calc_fom_for_trace('varaware_results.dat', 'varaware_fom.dat')

if __name__ == "__main__":
    main()
