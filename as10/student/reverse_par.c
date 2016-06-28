#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include "helper.h"

void reverse(char *str, int strlen)
{
	// parallelize this function and make sure to call reverse_str()
	// on each processor to reverse the substring.
	
	int i, np, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

/*finding lenghts and displaysments*/    
     int* locstrlen = malloc(sizeof(int)*np);
     int* locdispl = malloc(sizeof(int)*np);
     
     int pivotstrlen = strlen /np;
     for (i=0; i<np; i++)
     {
        if (i==0){
            locstrlen[i] = strlen - pivotstrlen* (np-1);
            locdispl[i] = 0;
            }
        else{
            locstrlen[i] = pivotstrlen;
            locdispl[i] = locstrlen[0] + (i-1) * pivotstrlen; 
            }       
     }
     
    /*scattering the string*/
	char* locstr = malloc(sizeof(char)*locstrlen[rank]);
         
            MPI_Scatterv(str,locstrlen,locdispl,MPI_CHAR,locstr,locstrlen[rank],MPI_CHAR,0,MPI_COMM_WORLD);
            /*print(locstr,locstrlen[rank]);*/
            /*reversing*/
            reverse_str(locstr, locstrlen[rank]);
            /*print(locstr,locstrlen[rank]);*/

/*collecting the rversed string*/
    if (rank == 0)
    {   
        /*recieveing for master process*/
    	for(i=np-1; i>0; i--)
    	{
    	    MPI_Recv(str + strlen - locdispl[i] - pivotstrlen,locstrlen[i],MPI_CHAR,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    	}
    	/*adding last piece that was reveresed in master process*/
    	for(i=0;i<locstrlen[0];i++)
        {*(str+strlen-locstrlen[0]+i) = *(locstr+i);}

    }
    else
    {
    /*sending for slave processes*/
        MPI_Send(locstr,locstrlen[rank],MPI_CHAR,0,0,MPI_COMM_WORLD);
    }
    
    free(locstrlen);
    free(locdispl);
    free(locstr);
    
}
