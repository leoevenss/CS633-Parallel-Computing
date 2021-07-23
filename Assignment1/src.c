#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"


int main( int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int myrank, size;

	MPI_Comm_rank(MPI_COMM_WORLD, &myrank) ;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	MPI_Request request;    
	    
	double sTime, eTime, time, maxtime;
	int datapoints = atoi (argv[1]);
	int iterations = atoi (argv[2]);
	int N = (int)sqrt(datapoints);
	int P = (int)sqrt(size);
	int count;        
	double recvbuf[N];
	double sendbuftop[N],sendbufleft[N],sendbufright[N],sendbufdown[N];
	double recvbuftop[N],recvbufleft[N],recvbufright[N],recvbufdown[N];
	double recvarr[N];
	int position=0;
  	count=sizeof(double)*N*4;
	int iter = 0;
	
	FILE *filePointer;
	filePointer = fopen("data.txt", "a");

	double** buf;
	double* temp;

	buf = malloc(N * sizeof(*buf));
	temp = malloc(N * N * sizeof(buf[0]));
	for (int i = 0; i < N; i++) {
  		buf[i] = temp + (i * N);
	}


    // Multiple MPI_Sends, each MPI_Send transmits only 1 element (1 double in this case).

    // initialize data
    for (int i=0; i<N; i++)
   		for (int j=0; j<N; j++)
    			buf[i][j] = rand();


    // iter will contain iteration of 1 to 50 times, will be used as tag
    sTime = MPI_Wtime();
	for(iter =0 ; iter<iterations;iter++){
        // all sending to top
        if(myrank>=P) {
            for (int i=0; i<N; i++) {
                MPI_Send (&buf[0][i], 1, MPI_DOUBLE, myrank-P, iter, MPI_COMM_WORLD);
            }
        }

        // all sending to left
        if(myrank%P) { 
        for (int i=0; i<N; i++) {
                MPI_Send (&buf[i][0], 1, MPI_DOUBLE, myrank-1, iter, MPI_COMM_WORLD);
            }
        }

        // all sending to right
        if((myrank+1)%P) { 
            for (int i=0; i<N; i++) {
                MPI_Send (&buf[i][N-1], 1, MPI_DOUBLE, myrank+1, iter, MPI_COMM_WORLD);
            }
        }

        // all sending to bottom
        if(myrank+P < size) {
            for (int i=0; i<N; i++) {
                MPI_Send (&buf[N-1][i], 1, MPI_DOUBLE, myrank+P, iter, MPI_COMM_WORLD);
            }
        }

        // receiving from top
        if(myrank>=P) {
            for (int i=0; i<N; i++) {
                MPI_Recv (recvbuf, 1, MPI_DOUBLE, myrank-P, iter, MPI_COMM_WORLD, &status);
                buf[0][i] += recvbuf[0];
            }
        }

        // receiving from left
        if(myrank%P) { 
        for (int i=0; i<N; i++) {
                MPI_Recv (recvbuf, 1, MPI_DOUBLE, myrank-1, iter, MPI_COMM_WORLD, &status);
                buf[i][0] += recvbuf[0];
            }
        }

        // receiving from right
        if((myrank+1)%P) { 
            for (int i=0; i<N; i++) {
                MPI_Recv (recvbuf, 1, MPI_DOUBLE, myrank+1, iter, MPI_COMM_WORLD, &status);
                buf[i][N-1] += recvbuf[0];
            }
        }

        // receiving from bottom
        if(myrank+P < size) {
            for (int i=0; i<N; i++) {
                MPI_Recv (recvbuf, 1, MPI_DOUBLE, myrank+P, iter, MPI_COMM_WORLD, &status);
                buf[N-1][i] += recvbuf[0];
            }
        }
	}
    eTime = MPI_Wtime();
    time = eTime - sTime;

    //obtain max time
    MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (!myrank) {
    	printf ("MPI_Send/Recv:%lf\n", maxtime);
        fprintf(filePointer, "For P=%d\n", size);
        fprintf(filePointer, "For N=%d\n", N);
        fprintf(filePointer, "MPI_Send/Recv:%lf\n", maxtime);
    }


    // MPI_Send/Recv using MPI Pack/Unpack
	
  	for (int i=0; i<N; i++)
   		for (int j=0; j<N; j++)
    			buf[i][j] = rand();
	
	sTime = MPI_Wtime();
	for(iter=0;iter<iterations;iter++){
		position=0;
	
        if(myrank>=P)//all sending to top
        {		
            for (int i=0; i<N; i++) {
                MPI_Pack (&buf[0][i],1, MPI_DOUBLE,sendbuftop,count, &position, MPI_COMM_WORLD);   
            }
            MPI_Send (sendbuftop, position, MPI_PACKED,myrank-P, iter, MPI_COMM_WORLD);
        }

        if((myrank%P)!=0)//all sending to left 
        {
            for (int i=0; i<N; i++) {
                MPI_Pack (&buf[i][0],1, MPI_DOUBLE, sendbufleft,count, &position, MPI_COMM_WORLD);
                }
                MPI_Send (sendbufleft,position, MPI_PACKED, myrank-1, iter, MPI_COMM_WORLD);   
        }

        if(((myrank+1)%P)!=0)//all sending to right 
        {  
            for (int i=0; i<N; i++) {
                    MPI_Pack (&buf[i][N-1],1, MPI_DOUBLE, sendbufright,count, &position, MPI_COMM_WORLD);
            }    
            MPI_Send (sendbufright, position, MPI_PACKED, myrank+1, iter, MPI_COMM_WORLD);
        }

        if((myrank+P)<size)//all sending to bottom 
        { 
            for (int i=0; i<N; i++) {
                MPI_Pack (&buf[N-1][i],1, MPI_DOUBLE,sendbufdown,count, &position, MPI_COMM_WORLD);
            }
            MPI_Send (sendbufdown,position, MPI_PACKED, myrank+P, iter, MPI_COMM_WORLD);
        }
        

        if(myrank>=P)//all receiving from top 
        {
            MPI_Recv (recvbuftop, count, MPI_PACKED, myrank-P, iter, MPI_COMM_WORLD, &status);
            for (int i=0;i<N;i++){
                MPI_Unpack(recvbuftop,count,&position,&recvarr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
                        buf[0][i] += recvarr[i];
            }
        }
    
        if((myrank%P)!=0) //receiving from left
        {
            MPI_Recv (recvbufleft, count, MPI_PACKED, myrank-1, iter, MPI_COMM_WORLD, &status);
            for (int i =0;i<N;i++){
                MPI_Unpack(recvbufleft,count,&position,&recvarr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
                buf[i][0] += recvarr[i];
            }
        }
    
        if(((myrank+1)%P)!=0) //receiving from right
        {
            MPI_Recv (recvbufright, count, MPI_PACKED, myrank+1, iter, MPI_COMM_WORLD, &status);
            for (int i=1;i<N;i++){
                MPI_Unpack(recvbufright,count,&position,&recvarr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
                buf[i][N-1] += recvarr[i];
            }
        }

        if((myrank+P)<size) 
        {
            MPI_Recv (recvbufdown, count, MPI_PACKED, myrank+P, iter, MPI_COMM_WORLD, &status);
            for (int i=1;i<N;i++){
                MPI_Unpack(recvbufdown,count,&position,&recvarr[i],1,MPI_DOUBLE,MPI_COMM_WORLD);
                buf[N-1][i] += recvarr[i];
            }
    	}
	}
	eTime = MPI_Wtime();
  	time = eTime - sTime;

	MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
   	if(!myrank){          
        printf ("MPI_Pack/Unpack:%lf\n", maxtime);
        fprintf(filePointer, "For P=%d\n", size);
        fprintf(filePointer, "For N=%d\n", N);
        fprintf(filePointer, "MPI_Pack/Unpack:%lf\n", maxtime);		
    }


    // MPI_Send/Recv using MPI derived datatypes

    // initialize data
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            buf[i][j] = rand();
        }
    }

    //iter will contain iteration of 1 to 50 times, will be used as tag
    sTime = MPI_Wtime();

    for(int iter=0; iter<iterations; iter++) {
        // all sending to top
        if(myrank>=P) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Send (&buf[0][0], 1, newvec, myrank-P, iter, MPI_COMM_WORLD);
            MPI_Type_free(&newvec);
        }

        // all sending to left
        if(myrank%P) {
            MPI_Datatype newvec;
            MPI_Type_vector(N, 1, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Send (&buf[0][0], 1, newvec, myrank-1, iter, MPI_COMM_WORLD);
            MPI_Type_free(&newvec);
        }

        // all sending to right
        if((myrank+1)%P) {
            MPI_Datatype newvec;
            MPI_Type_vector(N, 1, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Send (&buf[0][N-1], 1, newvec, myrank+1, iter, MPI_COMM_WORLD);
            MPI_Type_free(&newvec);
        }

        // all sending to bottom
        if(myrank+P < size) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Send (&buf[N-1][0], 1, newvec, myrank+P, iter, MPI_COMM_WORLD);
            MPI_Type_free(&newvec);
        }

        // receiving from top
        if(myrank>=P) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Recv (recvbuf, 1, newvec, myrank-P, iter, MPI_COMM_WORLD, &status);
            for (int i=0; i<N; i++) {
                buf[0][i] += recvbuf[i];
            }
            MPI_Type_free(&newvec);
        }

        // receiving from left
        if(myrank%P) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Recv (recvbuf, 1, newvec, myrank-1, iter, MPI_COMM_WORLD, &status);
            for (int i=0; i<N; i++) {
                buf[i][0] += recvbuf[i];
            }
            MPI_Type_free(&newvec);
        }

        // receiving from right
        if((myrank+1)%P) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Recv (recvbuf, 1, newvec, myrank+1, iter, MPI_COMM_WORLD, &status);
            for (int i=0; i<N; i++) {
                buf[i][N-1]  += recvbuf[i];
            }
            MPI_Type_free(&newvec);
        }

        // receiving from bottom
        if(myrank+P < size) {
            MPI_Datatype newvec;
            MPI_Type_vector(1, N, N, MPI_DOUBLE, &newvec);
            MPI_Type_commit(&newvec);
            MPI_Recv (recvbuf, 1, newvec, myrank+P, iter, MPI_COMM_WORLD, &status);
            for (int i=0; i<N; i++) {
                buf[N-1][i]  += recvbuf[i];
            }
            MPI_Type_free(&newvec);
        }
    }
    eTime = MPI_Wtime();
    time = eTime - sTime;

    //obtain max time
    MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (!myrank) {
        printf ("Derived datatype:%lf\n", maxtime);
        fprintf(filePointer, "For P=%d\n", size);
        fprintf(filePointer, "For N=%d\n", N);
        fprintf(filePointer, "Derived datatype:%lf\n", maxtime);
    }
    
    fclose(filePointer);

    MPI_Finalize();
    return 0;
}
