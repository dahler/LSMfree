import random
import pandas as pd

max = 100000
l = list(range(1, max))  # the cast to list is optional in Python 2
#random.shuffle(l)
d = list(range(1, max))  # the cast to list is optional in Python 2
#random.shuffle(d)

df = pd.DataFrame([l, d]).T

df.to_csv('generated-data/100K.data', header=False, sep=' ', index=False)

#print (df) 

