import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt 
import numpy as np 
import os, math

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# ----------- COLLECTING THE TIME TAKEN -----------
data16 = [[[] for i in range(7)] for i in range(3)]
data36 = [[[] for i in range(7)] for i in range(3)]
data49 = [[[] for i in range(7)] for i in range(3)]
data64 = [[[] for i in range(7)] for i in range(3)]

with open("data.txt") as f:
    lines = f.readlines()
    
i = 2
while(i<len(lines) and len(lines[i].strip()) > 0):
    P = lines[i-2][6:-1]
    N = str(int(math.log2(int(lines[i-1][6:-1]))) - 4)
    time = math.log2(float(lines[i].split(":")[1][:-1]))

    if lines[i].startswith("MPI_Send/Recv:"):    
        s = 'data' + P + '[0]['+ N +'].append(time)'
    elif lines[i].startswith("MPI_Pack/Unpack:"):    
        s = 'data' + P + '[1]['+ N +'].append(time)'
    elif lines[i].startswith("Derived datatype:"):    
        s = 'data' + P + '[2]['+ N +'].append(time)'
    
    eval(s)
    i+=3


# ----------PLOTTING THE GRAPH------------
def graph(dataP, P):
    data = [[] for i in range(3)]
    for i in range(7):
        data[0].append(np.asarray(dataP[0][i]))
        data[1].append(np.asarray(dataP[1][i]))
        data[2].append(np.asarray(dataP[2][i]))

    fig = plt.figure() 
    
    ax = fig.add_axes([0, 0, 1, 1]) 
    ax.grid(color='grey', axis='y', linestyle='-', linewidth=0.25, alpha=0.7)# Set plot title

    ax.set_title('Number of elements vs log2(time) for ' + str(P) + ' processors')
    plt.xlabel('Number of datapoints')
    plt.ylabel('time in log2 scale (seconds)')

    labels = ['16^2','32^2','64^2','128^2','256^2','512^2','1024^2']

    bxp = []
    fillcolor = ['lightblue', 'yellow', 'lightgreen']
    mediancolour = ['blue', 'orange', 'green']
    for i in range(3):
        linex = []
        liney = []
        bp = ax.boxplot(data[i], patch_artist=True, labels=labels) 
        
        for fill in bp['boxes']: 
            fill.set_alpha(0.4)
            fill.set(color = fillcolor[i])
            fill.set(edgecolor = 'black', linewidth = 2) 
        
        for median in bp['medians']: 
            median.set(color = mediancolour[i]) 
            liney.append(median.get_ydata()[0])
            linex.append(median.get_xdata()[0]+0.25)

        bxp.append(bp["boxes"][0])
        plt.plot(linex, liney, color=mediancolour[i])

    ax.legend(bxp, ['MPI_Send/Recv', 'MPI_Pack/Unpack', 'Derived-MPI_Type_vector'], loc='upper left')
    fig.savefig('plot'+ str(P) +'.jpg',bbox_inches='tight')


graph(data16, 16)
graph(data36, 36)
graph(data49, 49)
graph(data64, 64)
