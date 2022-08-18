# Python Code to Measure Latency of Ceph using Librados

import rados
from time import perf_counter

try:
        cluster = rados.Rados(conffile='')
except TypeError as e:
        print ('Argument validation error: ', e)
        raise e

print ("Created cluster handle.")

try:
        cluster.connect()
except Exception as e:
        print ("connection error: ", e)
        raise e
finally:
        print ("Connected to the cluster.")

print ("\n\nI/O Context and Object Operations")
print ("=================================")

print ("\nCreating a context for the 'mypool' pool")
if not cluster.pool_exists('mypool'):
        raise RuntimeError('No data pool exists')
ioctx = cluster.open_ioctx('mypool')

# Data preparation
blockSizeArr = [4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288,  1048576,  2097152, 4194304]
dataArr = []
data = ""
ctr = 0
ctrArr = 0

for size in blockSizeArr:
    for i in range(ctr, size):
        data += "a"
        ctr += 1
    dataArr.append(data)

# Variables
objName = "bench"
numRep = 2000

# Measure Latency
aveWriteLatency = []
aveReadLatency = []
aveDelLatency = []

aveLatencyTotal = 0.0
for i in range(0, len(blockSizeArr)):
    # Write
    for j in range(numRep):
        objNameAppended = objName+str(i)+str(j)
        t_start = perf_counter()
        ioctx.write(objNameAppended, str(dataArr[i]).encode('UTF-8'))
        t_end = perf_counter()
        aveLatencyTotal += (t_end - t_start)
    aveWriteLatency.append(aveLatencyTotal/numRep)
    aveLatencyTotal = 0.0
    print("write done")

    # Read
    for j in range(numRep):
        objNameAppended = objName+str(i)+str(j)
        t_start = perf_counter()
        ioctx.read(objNameAppended)
        t_end = perf_counter()
        aveLatencyTotal += (t_end - t_start)
    aveReadLatency.append(aveLatencyTotal/numRep)
    aveLatencyTotal = 0.0
    print("read done")

    # Delete
    for j in range(numRep):
        objNameAppended = objName+str(i)+str(j)
        t_start = perf_counter()
        ioctx.remove_object(objNameAppended)
        t_end = perf_counter()
        aveLatencyTotal += (t_end - t_start)
    aveDelLatency.append(aveLatencyTotal/numRep)
    aveLatencyTotal = 0.0
    print("delete done")

print("Write Latency")
for i in range(0, len(blockSizeArr)):
    print(str(blockSizeArr[i])+" : "+str(aveWriteLatency[i]))

print("\nRead Latency")
for i in range(0, len(blockSizeArr)):
    print(str(blockSizeArr[i])+" : "+str(aveReadLatency[i]))

print("\nDelete Latency")
for i in range(0, len(blockSizeArr)):
    print(str(blockSizeArr[i])+" : "+str(aveDelLatency[i]))

print ("\nClosing the connection.")
ioctx.close()

print ("Shutting down the handle.")
cluster.shutdown()