#!/bin/sh

make all 


rm -f *.jpg


for i in 1 2 3 4 5 6 7 8 9 10
do
	
	for P in 4 16
	do 
		for PPN in 1 8
		
		do
		       
			V=$((P/4))
			
			python script.py 4 $V $PPN
			
			for D in 2048 32768 262144
			do 
				mpiexec -np $((P*PPN)) -f hostfile ./src.x $D 4 $PPN
			done
		done

	done
done


python3 plot.py


