import os
import random

def random_jobspec ():
    dir = './jobspecs/quartz_jobspecs'
    filename = random.choice(os.listdir(dir))
    path = os.path.join(dir, filename)
    print(path)
    return path

def main(): 
    base = 'match allocate_orelse_reserve '
    num_jobs = 200
    f = open('./cmd_inp_quartz', 'w')

    for i in range(0,num_jobs):
        rj = random_jobspec()
        f.write(base + rj + '\n')

    f.write('quit\n')
    f.close()


if __name__ == "__main__":
        main()
