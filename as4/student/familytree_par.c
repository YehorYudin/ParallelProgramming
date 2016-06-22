#include "familytree.h"
#include <omp.h>

void traverse_inner(tree *node, int numThreads){

		if(node != NULL){

			#pragma omp task shared(genius)
			{
			node->IQ = compute_IQ(node->data);
			genius[node->id] = node->IQ;
			}

			#pragma omp task shared(numThreads) //firstprivate(node)
 			traverse_inner(node->right, numThreads);

			#pragma omp task shared(numThreads) //firstprivate(node)
			traverse_inner(node->left, numThreads);
		}
}


void traverse(tree *node, int numThreads){

	#pragma omp parallel num_threads(numThreads)
{
	#pragma omp single
	{
		if(node != NULL){

			#pragma omp task shared(genius)
			{
			node->IQ = compute_IQ(node->data);
			genius[node->id] = node->IQ;
			}

			#pragma omp task shared(numThreads) //firstprivate(node)
 			traverse_inner(node->right, numThreads/2);

			#pragma omp task shared(numThreads) //firstprivate(node)
			traverse_inner(node->left, numThreads/2);
		}
	}
}
}


