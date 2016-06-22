#include "familytree.h"
#include <omp.h>
#define MYMAXLEVEL 2

void parallel(tree *node){
if (node!=NULL){
 #pragma omp parallel sections
  {
   #pragma omp section
   parallel(node->right);
   #pragma omp section
   parallel(node->left);  
  }
  node->IQ = compute_IQ(node->data);
  genius[node->id] = node->IQ;
 } 
}

void traverse(tree *node, int numThreads){

	/*if(node != NULL){
	#pragma omp parallel sections num_threads(numThreads)
	{

		#pragma omp section
		{
		node->IQ = compute_IQ(node->data);
		genius[node->id] = node->IQ;
		}	
		#pragma omp section
		{
		traverse(node->right, numThreads/2);
		}
		#pragma omp section
		{
		traverse(node->left, numThreads/2);
		}
	}
	}*/

omp_set_num_threads(numThreads);
omp_set_nested(1);
omp_set_max_active_levels(MYMAXLEVEL);

parallel(node);	
}

