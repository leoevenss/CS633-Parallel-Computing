#!/bin/sh

make all 

rm -f data.txt

rm -f *.jpg

chmod u+x ~/UGP/allocator/src/allocator.out

~/UGP/allocator/src/allocator.out 64 8

for i in 1 2 3 4 5
do
	for P in 16 36 49 64
	do 
		for N in 256 1024 4096 16384 65536 262144 1048576
		do 
			mpiexec -np $P -hostfile hosts ./src.x $N 50
		done
	done
done


python3 plot.py

