
import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
  
# x axis values  (1 and 2 nodes with 1, 2 and 4 cores per node)
x = ['(1,1)','(1,2)','(1,4)','(2,1)','(2,2)','(2,4)']

# y axis values
y = []
with open("time.txt") as f:
    time = f.readlines();


for i in time:
    if i.strip() != "":
        y.append(float(i.strip()))


# plotting the points 
plt.plot(x, y, color='blue', linewidth = 3,
         marker='o', markerfacecolor='red', markersize=12)

  
plt.xlabel('(Nodes, Cores per node)')
plt.ylabel('Time in seconds')
plt.savefig("plot.jpg")
