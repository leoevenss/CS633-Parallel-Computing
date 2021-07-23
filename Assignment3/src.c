#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include<math.h>
#include "mpi.h"

int main(int argc , char *argv[]){
	
	MPI_Init(&argc, &argv);
  
	int P, rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &P);
	MPI_Status status;

	char * line,*header,*record;
	char buffer[1024];
	int row=0,col=0;
    	char *csvfile = argv[1];
	double sTime, eTime, time, maxtime;    
		
	
	FILE *fstream = fopen(csvfile,"r");
	
	// get rows and columns
	header=fgets(buffer,sizeof(buffer),fstream);
	
	while((line=fgets(buffer,sizeof(buffer),fstream))!=NULL)
    	{
		row++;
	}

	record = strtok(header,",");
   
	while(record != NULL)
    	{
        	col++;
        	record = strtok(NULL,",");
	}
	fclose(fstream);  
	
	
	float* data= (float *)malloc(row*col*sizeof(float));
    	char * rline,*rheader,*rrecord;
	
	
	//reading data by root
	if(rank==0) {
	    FILE *istream = fopen(csvfile,"r");	    
	    char *temp;
	    
	    //read data 
	    int i=0;
	    
	    //skip first-header row
	    rline=fgets(buffer,sizeof(buffer),istream);
	    
	while((rline=fgets(buffer,sizeof(buffer),istream))!=NULL)
	{
            rrecord = strtok(rline,",");
            while(rrecord != NULL)
            {
                data[i++]=(float)strtod(rrecord, &temp); //for char* to double  
                rrecord = strtok(NULL,",");
            }
        }
    	}
         

    // READING THE FILE COMPLETED SO TIMING THE CODE STARTS HERE
    
    	sTime = MPI_Wtime();

    	int count=(int)floor(row/P)*col;

    //Distribute data row wise        
    
    	//define contiguous datatype
    	MPI_Datatype newcont;
	MPI_Type_contiguous(count,MPI_FLOAT, &newcont);
	MPI_Type_commit(&newcont);

	//initialising the local yearlymin 
	float *yearlymin=(float*)malloc(col*sizeof(float));
	for(int i =0;i<col;i++){
        	yearlymin[i]=99999.9;
    	}


	if(rank==0){ //send data from rank 1 to p-1     
        for(int i =1;i<P;i++){
            MPI_Send (&data[(i-1)*count], 1, newcont, i, i, MPI_COMM_WORLD);
        }
    	}

	if(rank!=0) {
	 	float *recvbuf=(float*)malloc(count*sizeof(float));       

				
	    
		MPI_Recv (recvbuf,count, MPI_FLOAT, 0, rank, MPI_COMM_WORLD, &status);
    		
    		//calculating local yearly min    
		for(int i=0;i<col;i++){
			for(int j=i;j<count;j+=col){		
				if(yearlymin[i]>recvbuf[j]){
	    			yearlymin[i]=recvbuf[j];
				}
	    	}
		}		
	}

	if (rank==0) { //calculating local yearly min at root
	    for(int i=0;i<col;i++) {
            for(int j=count*(P-1)+i;j<row*col;j+=col) {
                if(yearlymin[i]>data[j]) {
                    yearlymin[i]=data[j];
                }
		    }
	    }
	}

	float globalmin=99999.9;

	float* combine=(float*)malloc(col*sizeof(float));

	for(int i=0;i<col;i++){
    	combine[i]=0.0;	
	}
	
	//using reduce to get yearly min 
	for(int i =0;i<col;i++)
	MPI_Reduce (&yearlymin[i], &combine[i], 1, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD);
	if (rank==0){
	
	//computing global min from yearly min
	for(int i=2;i<col;i++){
		    if(globalmin>combine[i]) {
                globalmin=combine[i];
            }
	    }
	}
    eTime = MPI_Wtime();
    time = eTime - sTime;
    
    //obtain max time and print the 3 output lines
    MPI_Reduce (&time, &maxtime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    	
    	//writing to file
	if(rank==0){
        FILE *fout, *ftime;
        fout = fopen("output.txt", "w");
        ftime = fopen("time.txt", "a");

	    for (int i=2;i<col-1;i++) {
		printf("%.2f,",combine[i]);
            	fprintf(fout,"%.2f,\t",combine[i]);
        }
        printf("%.2f\t\n",combine[col-1]);
	fprintf(fout,"%.2f\n",combine[col-1]);
	    
	 printf("\n%.2f\n",globalmin);
        fprintf(fout, "globalmin:%.2f\n",globalmin);
        
        printf("%lf\n", maxtime);
        fprintf(fout, "maxtime:%lf\n", maxtime);
        fprintf(ftime, "%lf\n", maxtime);
	}

    MPI_Finalize();
    return 0;
}
