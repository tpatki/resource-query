# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

# Generates a specific type of grug for testing the performance-class based scheduling. Uses cab system as a reference.
# Usage:
# Input:
 
# Notes: 
# Specifically, it reads the cab_like_cluster.graphml GRUG and changes the number of racks there, 
# going from 1 to 720 for scaling tests.
# Cab has 72 racks. Each rack as 18 nodes, each node has 16 cores (8 cores per socket). 
# CPU power per socket is 115W. Memory is 32 GB per node.   
# Right now, this is very basic for initial tests, r/w is done line by line, and is not optimal. 
# This can be modified in the future to generate other kinds of GRUGs, 
# where user can input binary values to indicate whether a particular component exists in the GRUG, and then input things like 
# number of racks, nodes per rack, number of sockets, number of cores, number of gpus, amount of memory etc (to be added in future). 

def changeNumRacks (num): 

    # Rack string to match
    strRack = '<edge id="cluster2rack" source="cluster" target="rack">' 
    # Open the file with read only permit
    f = open('cab_like_cluster.graphml', 'r')
    contents = f.read()
    file_as_list = contents.splitlines() # Do this to avoid the newline mess.
    
    i = num;

    # Open the output file    
    outfile = 'grug_' + str(i) + '.graphml' 
    f_out = open(outfile, 'w')
   
   # start with setting flag as 0. flag is 1 when the next line is to be edited.
    flag = 0
    for line in file_as_list:
        if flag == 0:
            f_out.write(line + '\n')
        else: 
            # Flag is 1, we modify the number of racks and reset flag to 0.
            updatedline =  '\t\t<data key="multi_scale">' + str(i) + '</data>'
            f_out.write(updatedline + '\n')
            flag = 0

        # Detect that the next line needs to be modified.     
        if line.strip() == strRack:
            flag = 1

    # Done
    f.close()
    f_out.close()

def main():
    for k in xrange(1,3):
        changeNumRacks(k)


if __name__ == "__main__":
    main()




