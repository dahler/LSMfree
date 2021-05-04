import random
import pandas as pd
import numpy as np

max = 1000000
#l = list(range(1, max)) 
np.random.seed(42)

l = np.random.randint(low=1, high=max, size=100000)
d = np.random.randint(low=1, high=max, size=100000)

# the cast to list is optional in Python 2
# random.shuffle(l)
# d = list(range(1, max))  # the cast to list is optional in Python 2
# random.shuffle(d)

df = pd.DataFrame([l, d]).T

df.to_csv('generated-data/new100K.random.data', header=False, sep=' ', index=False)

#print (df) 




### generate data with normal distribution

# import numpy as np
# import matplotlib.pyplot as plt
# from scipy import stats
# #%matplotlib notebook
# import matplotlib.pyplot as plt


# mean = np.mean(l)
# print(mean)
# stdev = np.std(l)
# print(stdev)

# # sample data generation
# np.random.seed(42)
# #data = sorted(stats.lognorm.rvs(s=stdev, loc=mean, scale=1000000, size=1000))

# data = np.random.normal(mean, stdev, 1000)

# d = list(map(int, data))
# print(d)

# df = pd.DataFrame([d]).T
# print(df)
# df.to_csv('generated-data/1000.normal.data', header=False, sep=' ', index=False)
# #print(data)
# #print(len(data))

# # fit lognormal distribution
# shape, loc, scale = stats.lognorm.fit(data, loc=0)
# pdf_lognorm = stats.lognorm.pdf(data, shape, loc, scale)

# # fit normal distribution
# mean, std = stats.norm.fit(data, loc=0)
# pdf_norm = stats.norm.pdf(data, mean, std)

# #print(data)

# # fit weibull distribution
# shape, loc, scale = stats.weibull_min.fit(data, loc=0)
# pdf_weibull_min = stats.weibull_min.pdf(data, shape, loc, scale)


# fig, ax = plt.subplots(figsize=(8, 4))
# ax.hist(data, bins='auto', density=True)
# #ax.plot(data, pdf_lognorm, label='lognorm')
# #ax.plot(data, pdf_norm, label='normal')
# #ax.plot(data, pdf_weibull_min, label='Weibull_Min')
# ax.set_xlabel('X values')
# ax.set_ylabel('probability')
# ax.legend()

# plt.show()
