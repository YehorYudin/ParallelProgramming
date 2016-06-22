#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct arg{
int a;
int b;};

void* hello(int a, int b)
{/*int arg = *(int*)ptr;*/
printf("Hello pthread %d,%d\n",a,b);
return NULL;}


void fillarg(void* arguments, int a, int b){
struct arg *args = (struct arg *)arguments;
args->a=a;
args->b=b;}

void* passarg(void* arguments){
struct arg *args = (struct arg *)arguments;
hello(args->a,args->b);
}

int main(int argc, char** argv)
{
int num_threads = 3;
pthread_t* ptr;
struct arg *args;
ptr = malloc(num_threads*sizeof(*ptr));
args = malloc(num_threads*sizeof(*args));

for(int i=0; i<num_threads; i++)
{
	fillarg(args+i,i,2*i);
	pthread_create(ptr+i, NULL, &passarg, args+i);
}

for(int i=0; i<num_threads; i++)
{
	pthread_join(ptr[i],NULL);
}

free(ptr);
free(args);
return 0;
}

