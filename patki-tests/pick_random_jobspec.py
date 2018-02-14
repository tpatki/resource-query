import os
import random

dir = './jobspecs'
filename = random.choice(os.listdir(dir))
path = os.path.join(dir, filename)
