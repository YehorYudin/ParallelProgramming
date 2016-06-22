#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<errno.h>

typedef struct{
	pthread_mutex_t   mutex;
	pthread_cond_t     cond;
	int value;
} my_struct_t;

my_struct_t data = { PTHREAD_MUTEX_INITIALIZER,
                   PTHREAD_COND_INITIALIZER,
                                            0 };

int hibernation = 1; //Default to 1 second

void* signal_thread(void* arg ) 
{

	sleep( hibernation );
	pthread_mutex_lock( &data.mutex );
	data.value = 1; //set predicate
	pthread_cond_signal( &data.cond );
	pthread_mutex_unlock( &data.mutex );
	return NULL;

}

int main(int argc,char **argv )
{
pthread_t thread;
struct timespec timeout;
pthread_create( &thread, NULL, &signal_thread, NULL);

clock_gettime(CLOCK_REALTIME, &timeout);
timeout.tv_sec += 2; //wait for predicate for 2 seconds

pthread_mutex_lock( &data.mutex );
while ( data.value == 0) {//important!
  if (pthread_cond_timedwait( &data.cond, &data.mutex, &timeout ) == ETIMEDOUT) {
    printf( "Condition wait timed out.\n");
    break;
  }
}
if (data.value != 0)//consider the timeout
  printf( "Condition was signaled.\n");
pthread_mutex_unlock( &data.mutex );
pthread_join( thread, NULL );
return 0;
};
