# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
# Creates multiple job specs based on the quartz_queue.out file using the base.yaml jobspec file.

def create_quartz_job (nodes, duration, filecnt): 
    # Node string to match
    strn = 'count: tbd'
    strd = 'duration: tbd'
    # Open the file with read only permit
    f = open('base.yaml', 'r')
    contents = f.read()
    file_as_list = contents.splitlines() # Do this to avoid the newline mess.
    # Open the output file    
    outfile = 'quartz_job_' + str(filecnt) + '.yaml' 
    f_out = open(outfile, 'w')
   
   # start with setting flag as 0. flag is 1 when the next line is to be edited.
    for line in file_as_list:
        if line.strip() == strn:
            updatedNodes = '    count: ' + str(nodes) + '\n'
            f_out.write(updatedNodes)
        elif line.strip() == strd:
            updatedDuration = '    duration: ' + str(duration) + '\n'
            f_out.write(updatedDuration)
        else:
            f_out.write(line + '\n')
    # Done
    f.close()
    f_out.close()


def convert_to_sec (d):
    if '-' in d:
        day,hms = d.split('-')
        h,m,s = hms.split(':')
        ds = (int(day) * 86400) + (int(h) * 3600) + (int(m) * 60) + int(s)
    else:
        ds = 0
        for p in d.split(':'):
            ds = ds * 60 + int (p)
    return ds


def main():
        jq = open('quartz_queue.out', 'r')
        jqc = jq.read()
        jq_list = jqc.splitlines()
        r = 1
        for jq_row in jq_list:
            dr,n = jq_row.split()
            duration = convert_to_sec(dr)
            print n,duration,r
            create_quartz_job(n, duration, r)
            r = r + 1
        jq.close()


if __name__ == "__main__":
    main()

