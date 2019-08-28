import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams['font.family'] = 'Arial'
rcParams['font.size'] = 12
rcParams['figure.figsize'] = 7, 5

a = pd.read_csv('data.txt', sep="\t", header = None)
SampleRTT = a.transpose().iloc[0].values
EstimatedRTT = a.transpose().iloc[1].values
SimpleAverage = a.transpose().iloc[2].values
TimeoutInterval = a.transpose().iloc[3].values
x = np.arange(1, 200, 1).tolist()

plt.plot(x, SampleRTT, x, EstimatedRTT, x, SimpleAverage, x, TimeoutInterval)
plt.gca().legend(('SampleRTT', 'EstimatedRTT', 'SimpleAverage', 'TimeoutInterval'))
plt.ylabel('Time (ms)')
plt.show()
