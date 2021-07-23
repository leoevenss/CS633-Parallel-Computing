

make clean


make all 

rm -f output.txt
rm -f time.txt
rm -f *.jpg


	
for P in 1 2
do 
	for PPN in 1 2 4
		
	do
		       
			
		python script.py 1 $P $PPN
			
			
	 
		mpiexec -np $((P*PPN)) -f hostfile ./src.x tdata.csv
		
	

	done
done


#python3 plot.py
