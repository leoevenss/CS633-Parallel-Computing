#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"


int main( int argc, char *argv[])

{

  double sTime, eTime, time, maxtime, comm_time, alltime;
  int rank, P, c;
  int D = atoi(argv[1]);
  int no_groups = atoi(argv[2]);
  int ppn = atoi(argv[3]);
  
  // Setup
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);

  int newrank, newsize, headrank, headsize;
  MPI_Comm newcomm, headcomm;

  FILE *fbcast, *freduce, *fgather, *falltoallv;
  fbcast = fopen("Bcast.txt", "a");
  freduce = fopen("Reduce.txt", "a");
  fgather = fopen("Gather.txt", "a");
  falltoallv = fopen("Alltoallv.txt", "a");



  sTime = MPI_Wtime();
  // Intra-group subcommunicator
  MPI_Comm_split (MPI_COMM_WORLD, rank/(P/no_groups), rank, &newcomm);
  MPI_Comm_rank( newcomm, &newrank );
  MPI_Comm_size( newcomm, &newsize );

  // Inter group subcommunicator
  MPI_Comm_split (MPI_COMM_WORLD, rank%(P/no_groups), rank, &headcomm);
  MPI_Comm_rank( headcomm, &headrank );
  MPI_Comm_size( headcomm, &headsize );

  eTime = MPI_Wtime();
  time = eTime - sTime;

  MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if (!rank) {
      comm_time = maxtime;
  }


  //Default MPI_Bcast
  alltime = 0;
  c=5;
  while(c--) {
      //double *buf=(double*)malloc(D* sizeof(double));
      double buf[D];
      for (int i=0; i<D; i++)
          buf[i] = (double)rand() / (double)RAND_MAX;

      sTime = MPI_Wtime();

      MPI_Bcast(buf, D, MPI_DOUBLE, 0, MPI_COMM_WORLD);

      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {
          alltime+=maxtime;
      }
      //free(buf);
  }
  if (!rank) {
      printf ("Default MPI_Bcast:%lf\n", alltime/5.0);
      fprintf(fbcast, "%d,%d,%d,0,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }

  //Optimized MPI_Bcast
  alltime = 0;
  c=5;
  while(c--) {
      //double *buf=(double*)malloc(D* sizeof(double));
      double buf[D];
      for (int i=0; i<D; i++)
          buf[i] = (double)rand() / (double)RAND_MAX;

      sTime = MPI_Wtime();

      if(rank%(P/no_groups)==0) { 
        MPI_Bcast(buf, D, MPI_DOUBLE, 0, headcomm); 
      }
      MPI_Bcast(buf, D, MPI_DOUBLE, 0, newcomm);
      
      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {
        alltime+=maxtime;
      }
      //free(bufp);
  }
  if (!rank) {
      printf ("Optimized MPI_Bcast:%lf\n", alltime/5.0);
      fprintf(fbcast, "%d,%d,%d,1,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }


  //Default MPI_Reduce
  alltime = 0;
  c=5;
  while(c--) {
      double sendval[D],maxval[D];
      //double *sendval=(double*)malloc(D* sizeof(double));
      //double *maxval=(double*)malloc(D* sizeof(double));

      //initialize data
      for (int i=0; i<D; i++)
         sendval[i] = (double)rand() / (double)RAND_MAX;
      
      sTime=MPI_Wtime();
      
      MPI_Reduce(&sendval, &maxval, D, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); // find max of sendvals
      
      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {
        alltime+=maxtime;
      }
      //free(sendval);
      //free(maxval);
  }
  if (!rank) {
      printf ("Default MPI_Reduce:%lf\n", alltime/5.0);
      fprintf(freduce, "%d,%d,%d,0,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }


  //Optimized MPI_Reduce
  alltime = 0;
  c=5;
  while(c--) {
      //	double bufr[D],maxvalr[D];
      //  double *bufr=(double*)malloc(D* sizeof(double));
      //  double *maxvalr=(double*)malloc(D* sizeof(double));
      double sendval[D],maxval[D];
      for (int i=0; i<D; i++)
          sendval[i] = (double)rand() / (double)RAND_MAX;

      sTime = MPI_Wtime();
      
      MPI_Reduce(&sendval, &maxval, D, MPI_DOUBLE, MPI_SUM, 0, newcomm);
      if(rank%(P/no_groups)==0) { 
          MPI_Reduce(&maxval, &sendval, D, MPI_DOUBLE, MPI_SUM, 0, headcomm);	
      }

      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {
        alltime+=maxtime;
      }
      //free(bufr);
      //free(maxvalr);
  }
  if (!rank) {
      printf ("Optimized MPI_Reduce:%lf\n", alltime/5.0);
      fprintf(freduce, "%d,%d,%d,1,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }



  //Default MPI_Gather
  alltime = 0;
  c=5;
  while(c--) {
      // double sendval[D];
      double *sendval=(double*)malloc(D* sizeof(double));
      for (int i = 0; i < D; i++) {
        sendval[i] = (double)rand() / (double)RAND_MAX;
      }

      double *recvMessage=(double*)malloc(D*P* sizeof(double)); //significant at the root process
      sTime = MPI_Wtime();

      MPI_Gather(sendval, D, MPI_DOUBLE, recvMessage, D, MPI_DOUBLE, 0, MPI_COMM_WORLD);
      
      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {
        alltime+=maxtime;
      }
      free(recvMessage);
      free(sendval);
  }
  if (!rank) {
      printf ("Default MPI_Gather:%lf\n", alltime/5.0);
      fprintf(fgather, "%d,%d,%d,0,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }


  //Optimized MPI_Gather
  alltime = 0;
  c=5;
  while(c--) {
      // double headBuf[D * P/no_groups];
      // double sendval[D];
      double *sendval=(double*)malloc(D* sizeof(double));
      double *recvBuff=(double*)malloc(D*P* sizeof(double));
      double *headBuf  = (double*)malloc(D*(P/no_groups)* sizeof(double));
      for (int i=0; i<D; i++)
          sendval[i] = (double)rand() / (double)RAND_MAX;

      sTime = MPI_Wtime();

      MPI_Gather(sendval, D, MPI_DOUBLE, headBuf, D, MPI_DOUBLE, 0, newcomm);
      if(rank%(P/no_groups)==0) { 
          MPI_Gather(headBuf, D*P/no_groups, MPI_DOUBLE, recvBuff, D*P/no_groups, MPI_DOUBLE, 0, headcomm);
      }

      eTime = MPI_Wtime();
      time = eTime - sTime;

      MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
      if (!rank) {        
        alltime+=maxtime;
      }
      free(recvBuff);
      free(headBuf);
      free(sendval);
  }
  if (!rank) {
      printf ("Optimized MPI_Gather:%lf\n", alltime/5.0);
      fprintf(fgather, "%d,%d,%d,1,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
  }



   // Default MPI_Alltoallv
   alltime=0;
   c=5;
   while(c--){
        double buf[D];
        //double headBuf[D * P/no_groups];
        double *recvBuff=(double*)malloc(D*P* sizeof(double));
        double *headBuf  = (double*)malloc(D*(P/no_groups)* sizeof(double));
        
        int sendcounts[P], recvcounts[P], sdispls[P], rdispls[P];
        for (int i=0; i<P; i++) {
            sendcounts[i] = D/P;
            recvcounts[i] = D/P;
            rdispls[i] = (D/P) * rank;
            sdispls[i] = D/P*rank;
        }

        double message2[D];
        for (int i = 0; i < sdispls[P-1]+sendcounts[P-1]; i++) {
            message2[i] = (double)rand() / (double)RAND_MAX;
        }
        double recvMessage2[D]; 
        sTime = MPI_Wtime();

        MPI_Alltoallv(message2, sendcounts, sdispls, MPI_DOUBLE, recvMessage2, recvcounts, rdispls, MPI_DOUBLE, MPI_COMM_WORLD);

        eTime = MPI_Wtime();
        time = eTime - sTime;

        MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        if (!rank) {
            // printf ("Default MPI_Alltoallv:%lf\n", maxtime);
            alltime+=maxtime;
        }
    }
    if (!rank) {
        printf ("Default MPI_Alltoallv:%lf\n", alltime/5.0);
        fprintf(falltoallv, "%d,%d,%d,0,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
    }




   //Optimized MPI_Alltoallv
   alltime=0;
   c=5;
   while(c--){
        double buf[D];
        //double headBuf[D * P/no_groups];
        double *recvBuff=(double*)malloc(D*P* sizeof(double));
        double *headBuf  = (double*)malloc(D*(P/no_groups)* sizeof(double));
        
        int sendcounts[P], recvcounts[P], sdispls[P], rdispls[P];
        for (int i=0; i<P; i++) {
            sendcounts[i] = D/P;
            recvcounts[i] = D/P;
            rdispls[i] = (D/P) * rank;
            sdispls[i] = D/P*rank;
        }

        double message2[D];
        for (int i = 0; i < sdispls[P-1]+sendcounts[P-1]; i++) {
            message2[i] = (double)rand() / (double)RAND_MAX;
        }
        double recvMessage2[D]; 

        sTime = MPI_Wtime();

        for(int i=0;i<newsize;i++)
        MPI_Gather (message2+sdispls[rank], D/P, MPI_DOUBLE, recvMessage2, D/P, MPI_DOUBLE, i, newcomm);

        for(int i=0;i<headsize;i++)
        MPI_Gather (message2+sdispls[rank], D/P*newsize, MPI_DOUBLE, recvMessage2, D/P*newsize, MPI_DOUBLE, i, headcomm);

        eTime = MPI_Wtime();
        time = eTime - sTime;

        MPI_Reduce (&time, &maxtime,1 , MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        if (!rank) {
            // printf ("Optimized MPI_Alltoallv:%lf\n", maxtime);
            alltime+=maxtime;
        }
    }
    if (!rank) {
        printf ("Optimized MPI_Alltoallv:%lf\n", alltime/5.0);
        fprintf(falltoallv, "%d,%d,%d,1,%lf,\"(%d, %d)\"\n", D/128,P,ppn,alltime/5.0,P,ppn);
    }

  MPI_Comm_free(&headcomm);
  MPI_Comm_free(&newcomm);

  MPI_Finalize();
  return 0;
}



