#include "emsim.h"
#include <pthread.h>

pthread_mutex_t mtx           = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t cond_new_work  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_idle_worker = PTHREAD_COND_INITIALIZER;
pthread_t * threads;
int idle_workers;
int new_work = 0;
int finished_works= 0;
int job_started=0;

typedef struct{
	int g;
	team_t* team1;
	team_t* team2;
}worker_args;

void* worker (void* ptr){

	//declaring the variables.

  	int goalsI, goalsJ;
	
	worker_args *args = (worker_args*)ptr;

	while (finished_works!=36){
	
		printf ("-> worker thread: I am: %d, # of current available works: %d \n", (int)pthread_self(), new_work);
		while (new_work==0 && finished_works!=36){
			printf("-> worker thread: waiting for a work\n");
			pthread_cond_wait(&cond_new_work,&mtx);
		}
		printf("-> worker thread: new work is received\n");
		//set the addresses
		if (new_work > 0){
			new_work--;
			finished_works++;

			int g=args->g;
			team_t *team1 = args->team1;
			team_t *team2 = args->team2;

			// do the computation
			playGroupMatch(g, team1, team2, &goalsI, &goalsJ);
			team1->goals += goalsI - goalsJ;
			team2->goals += goalsJ - goalsI;
			if (goalsI > goalsJ)
			  team1->points += 3;
			else if (goalsI < goalsJ)
			  team2->points += 3;
			else {
			  team1->points += 1;
			  team2->points += 1;
			}
			//inform master thread that one thread is available
			pthread_cond_signal(&cond_idle_worker);
			idle_workers++;
			pthread_mutex_unlock(&mtx);
			printf("-> worker thread: work is done, message is sent to the master\n");
			printf("-> worker thread: total number of finished works: %d\n", finished_works);
		}
		else if (finished_works>=36)
			idle_workers++;
	}
		
	printf("-> worker thread: I am done\n");
	return NULL;
}

// play all (36) group games by utilizing numWorker worker threads
void playGroups(team_t* teams, int numWorker) {
  static const int cNumTeamsPerGroup = NUMTEAMS / NUMGROUPS;
  int g, i, j, t, k=0; 
  worker_args args;
  
  threads = malloc (numWorker*sizeof(pthread_t));
  idle_workers=numWorker;

  for (t=0;t<numWorker;t++){
    pthread_create(threads + t, NULL, &worker, &args);
  }

  for (g = 0; g < NUMGROUPS; ++g) {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i) {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j) {

	printf ("-> master thread: idles: %d \n",idle_workers);
	pthread_mutex_lock(&mtx);
	while (idle_workers==0){
//		printf("-> master thread: waiting for an idle worker\n");
		pthread_cond_wait(&cond_idle_worker,&mtx);
	}
//	printf("-> master thread: some worker is available\n");
	// produce the works
	args.g=g;
        args.team1=teams + i;
	args.team2=teams + j;
	
//	printf("-> master thread: new work is available\n");
	// inform the threads
	
	job_started=0;
	pthread_cond_signal(&cond_new_work);
	job_started=1;
	new_work++;
	idle_workers--;
	pthread_mutex_unlock(&mtx);
	k++;
	printf("-> master thread: work signal is sent to one worker, %d\n", k);
      }
    }
  }


  while (1){
//  printf("-> master thread: waiting for an idle worker\n");
	printf ("%d \n", idle_workers);
 	if (idle_workers==numWorker)
		break;
	pthread_cond_signal(&cond_new_work);
  }

  printf("-> master thread: I am done\n");
  for (t=0;t<numWorker;t++){
    pthread_join(threads[t], NULL);
  }

  free(threads);
}

// play a specific final round by utilizing numWorker worker threads
void playFinalRound(int numGames, team_t** teams, team_t** successors, int numWorker){
  team_t* team1;
  team_t* team2;
  int i, goals1 = 0, goals2 = 0;

  for (i = 0; i < numGames; ++i) {
    team1 = teams[i*2];
    team2 = teams[i*2+1];
    playFinalMatch(numGames, i, team1, team2, &goals1, &goals2);

    if (goals1 > goals2)
      successors[i] = team1;
    else if (goals1 < goals2)
      successors[i] = team2;
    else {
      playPenalty(team1, team2, &goals1, &goals2);
      if (goals1 > goals2)
        successors[i] = team1;
      else
        successors[i] = team2;
    }
  }
}
