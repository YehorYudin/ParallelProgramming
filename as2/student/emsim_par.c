#include "emsim.h"
#include <pthread.h>

struct arg_str_PGM{
int groupNo;
team_t* team1;
team_t* team2;
int* goals1;
int* goals2;
};

struct arg_str_PFM{
int numGames; 
int gameNo;
team_t* team1;
team_t* team2;
int* goals1;
int* goals2;
team_t* successor;
};

struct arg_str_Pen{
team_t* team1;
team_t* team2;
int* goals1;
int* goals2;
};

struct my_condstruct{
	pthread_mutex_t * mutex;
	pthread_cond_t  cond;
	int value;
        int synco;
};


struct arg_str_ST{
struct arg_str_PGM * argums;
int numGames;
int cNumTeamsPerGroup;
int numWorker;
team_t* teams;
int* goalsI;
int* goalsJ;
struct my_condstruct * CVprod;
struct my_condstruct * CVcons;
};

struct arg_str_CT{
struct my_condstruct * CVprod;
struct my_condstruct * CVcons;
struct arg_str_PGM * args_PGM;
int numGames;
int NuThr;
};
/**/
void fillarg_str_PGM(void*arguments, int groupNo,
                    team_t* team1, team_t* team2, int* goals1, int* goals2){
struct arg_str_PGM *args = (struct arg_str_PGM *)arguments;
args->groupNo = groupNo;
args->team1 = team1;
args->team2 = team2;
args->goals1 = goals1;
args->goals2 = goals2;
}

void* passarguments_PGM(void* arguments)
{
    struct arg_str_PGM *args = (struct arg_str_PGM *)arguments;
    playGroupMatch(args->groupNo, args->team1, args->team2, args->goals1, args->goals2);
    return NULL;
}

void fillarg_str_PFM(void* arguments, int numGames, int gameNo,
                    team_t* team1, team_t* team2, int* goals1, int* goals2){
struct arg_str_PFM *args = (struct arg_str_PFM *)arguments;
args->numGames = numGames;
args->gameNo = gameNo;
args->team1 = team1;
args->team2 = team2;
args->goals1 = goals1;
args->goals2 = goals2;
}


void* passarguments_PFM(void* arguments)
{
    struct arg_str_PFM *args = (struct arg_str_PFM *)arguments;
    playFinalMatch(args->numGames, args->gameNo, args->team1, args->team2, args->goals1, args->goals2);
    return NULL;
}

void fillarg_str_Pen(void* arguments, team_t* team1, team_t* team2,
                 int* goals1, int* goals2){
struct arg_str_Pen *args = (struct arg_str_Pen *)arguments;
args->team1=team1;
args->team2=team2;
args->goals1=goals1;
args->goals2=goals2;
}

void* passarguments_Pen(void* arguments)
{
    struct arg_str_Pen *args = (struct arg_str_Pen *)arguments;
    playPenalty(args->team1,args->team2,args->goals1,args->goals2);
    return NULL;
}
/*--------------------------------------------------------------------*/ 

void catch_signal(struct my_condstruct * CVprod, struct my_condstruct * CVcons, struct arg_str_PGM * args_PGM, int numGames, int NuThr)
{
//printf("We run new worker no %i\n",NuThr);
  int numcurg;
 while(1)
 {  
      pthread_mutex_lock(CVcons->mutex);
      if(CVcons->synco != 0)
         CVcons->synco = CVcons->synco -1;
      CVcons->value = CVcons->value + 1;
//printf("A Worker no %i waits for a new task! He thinks we have done games, %i\n",NuThr,CVcons->value);
      pthread_cond_signal(&(CVcons->cond));
printf("Worker no %i signaled that it is ready! Workers syncro = %i\n",NuThr,CVcons->synco);
      while(CVprod->synco ==0)
          pthread_cond_wait(&(CVprod->cond),(CVprod->mutex));
      if(CVprod->value >= numGames)
          {printf("Woker no %i stops working\n",NuThr) ;
           pthread_mutex_unlock(CVcons->mutex);
         break;}
      //CVcons->synco = CVcons->synco + 1;
printf("Workers no %i caught a signal from master\n",NuThr);
      numcurg=CVprod->value;
      //CVcons->value = CVcons->value + 1;
      CVprod->synco = 0;
      //pthread_cond_signal(&(CVcons->cond));
      pthread_mutex_unlock(CVcons->mutex);
      passarguments_PGM(args_PGM+numcurg);
printf("Worker no %i proceed a game no %i\n", NuThr, numcurg);
//printf("Worker no %i unlocked a mutex back\n", NuThr);
 }

}


void fillarg_str_CT(void* arguments,struct my_condstruct * CVprod, struct my_condstruct * CVcons,struct arg_str_PGM * args_PGM, int numGames, int NuThr)
{
   struct arg_str_CT *args = (struct arg_str_CT *)arguments;
   args->CVprod = CVprod;
   args->CVcons = CVcons;
   args->args_PGM = args_PGM;
   args->numGames = numGames;
   args->NuThr = NuThr;
}

void* passarguments_CT(void* arguments)
{
   struct arg_str_CT *args = (struct arg_str_CT *)arguments;
   catch_signal(args->CVprod, args->CVcons, args->args_PGM,args->numGames,args->NuThr);
   return NULL;
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/

void playGroups(team_t* teams, int numWorker)
{
 /*---initilising constants and data structures*/
  static const int cNumTeamsPerGroup = NUMTEAMS / NUMGROUPS;
  static const int numGames = NUMGROUPS*(cNumTeamsPerGroup*(cNumTeamsPerGroup-1))/2;
//printf("this shit should be number of games: %i\n",numGames);
  int g, i, j, k;

  //pthread_t * master;
  //master = malloc(sizeof(master));
  pthread_t * threadWorkers;
  threadWorkers = malloc(numWorker*sizeof(*threadWorkers));
  //pthread_t THWR[numWorker];
  //threadWorkers = & THWR;
  //struct arg_str_ST * arg; 
  //arg= malloc(sizeof(arg));
  struct arg_str_PGM * arg_PGM;
  arg_PGM = malloc(numGames*sizeof(*arg_PGM));
  struct arg_str_CT * arg_Workers;
  arg_Workers = malloc(numWorker*sizeof(arg_Workers));

  int goalsI[numGames];
  int goalsJ[numGames];

for(i=0;i<numGames;i++)
    {goalsI[i] = 0; 
    goalsJ[i] = 0;}

/*---initialising thread, CV, setting to wait*/

  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  struct my_condstruct condProducer = { & mutex, PTHREAD_COND_INITIALIZER, 0,0 };
  struct my_condstruct condConsumer = { & mutex, PTHREAD_COND_INITIALIZER, -1*numWorker, numWorker};
  //int pthread_cond_init ( pthread_cond_t *condMaster, NULL );
//printf("Initialised successfuly\n");
//printf("Workers not ready = %i\n",condConsumer.synco);

 /*producer thread*/

  //fillarg_str_ST(arg,arg_PGM,numGames,cNumTeamsPerGroup,numWorker,teams,goalsI,goalsJ,&condProducer,&condConsumer);
//printf("Shit happened not during filling the arg_str_ST structure\n");
  //pthread_create( master, NULL, &passarguments_ST, arg);
//printf("Here we should set up master thread successfuly\n");

/*counsumer threads*/

  for (i=0;i<numWorker;i++)
  {
    fillarg_str_CT(arg_Workers+i,&condProducer,&condConsumer,arg_PGM,numGames,i);
    pthread_create(threadWorkers+i, NULL, &passarguments_CT, arg_Workers+i);
//printf("We create worker thread no %i\n",i);
  }
//printf("We initialised all worker threads\n");


/*---playing matches and counting points---*/

k=0;
  for (g = 0; g < NUMGROUPS; ++g)
  {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i)
    {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j)
       {
printf("Master waits for a free worker \n");
       fillarg_str_PGM(arg_PGM+k, g, teams + i, teams + j, &goalsI[k], &goalsJ[k]);

        pthread_mutex_lock( (condProducer).mutex );
        //condProducer.synco = 0;
        while((condConsumer).synco > numWorker-1)
            pthread_cond_wait( &((condConsumer).cond), (condConsumer).mutex);
//printf("Master is really knows that there is a free worker \n");
        (condProducer).value = k; //set predicate
        (condProducer).synco = 1;
        pthread_cond_signal( &((condProducer).cond) );
        (condConsumer).synco = condConsumer.synco + 1;
printf("Master sends a signal to catch game no %i, Master syncro value= %i\n", (condProducer).value, (condProducer).synco);
        pthread_mutex_unlock( (condProducer).mutex );
	k++;  
//printf("Master locked the mutex back\n");
       }  
     }
   }
/*pseuode-barrier*/
  
  pthread_mutex_lock( condProducer.mutex );
printf("Master waits untill all games are played\n");
  while(condConsumer.value < numGames-1)
      pthread_cond_wait(&(condConsumer.cond),condConsumer.mutex);
   condProducer.synco = 1;
   condProducer.value = numGames;
   pthread_cond_broadcast(&(condProducer.cond));
printf("Producer broadcasts to stop working \n");
   pthread_mutex_unlock(condProducer.mutex);

  for(i=0;i<numWorker;i++)
    {pthread_join(threadWorkers[i],NULL);};
//pthread_join(master[0],NULL);
printf("We joined all threads\n");

/*---separate loop to count points?*/
 k=0;
  for (g = 0; g < NUMGROUPS; ++g) {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i) {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j) {

        teams[i].goals += goalsI[k] - goalsJ[k];
        teams[j].goals += goalsJ[k] - goalsI[k];
	k++;    
        if (goalsI[k] > goalsJ[k])
          teams[i].points += 3;
        else if (goalsI[k] < goalsJ[k])
          teams[j].points += 3;
        else {
          teams[i].points += 1;
          teams[j].points += 1;
        }
        k++;
      }
    }
  }
printf("We counted all points\n");
//free(master);
//free(threadWorkers);
//free(arg);
free(arg_PGM);
free(arg_Workers);
//pthread_mutex_destroy(&mutex);
//pthread_cond_destroy(&(condConsumer.cond));
//pthread_cond_destroy(&(condProducer.cond));
printf("We cleaned up everything\n");

}


/*----------------------------------------------------------------*/

void playFinalRound(int numGames, team_t** teams, team_t** successors, int numWorker){
printf("We are playing finals ");
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
    else
    {
      playPenalty(team1, team2, &goals1, &goals2);
      if (goals1 > goals2)
        successors[i] = team1;
      else
        successors[i] = team2;
    }
  }
printf(", we played a final\n");
}
