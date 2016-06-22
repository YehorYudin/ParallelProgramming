#include "emsim.h"
#include <pthread.h>

struct arg_str_PG{
int groupNo;
team_t* teams;
int numTeams;
team_t** first;
team_t** second;
team_t** third;
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

void fillarg_str_PG(void* arguments,int groupNo, team_t* teams, int numTeams,
               team_t** first, team_t** second, team_t** third){
struct arg_str_PG *args = (struct arg_str_PG *)arguments;
args->groupNo = groupNo;
args->teams = teams;
args->numTeams = numTeams;
args->first = first;
args->second = second;
args->third = third;
}

void* passarguments_PG(void* arguments)
{
    struct arg_str_PG *args = (struct arg_str_PG *)arguments;
    playGroup(args->groupNo, args->teams, args->numTeams, args->first, args->second, args->third);
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


/*
void* playParFinTog(void* arguments){
    struct arg_str_PFM *args = (struct arg_str_PFM *)arguments;

    playFinalMatch(args->numGames, args->gameNo, args->team1, args->team2, args->goals1, args->goals2);

    if (args->goals1 > args->goals2)
      args->successor = args->team1;
    else if (args->goals1 < args->goals2)
      args->successor = args->team2;
    else
    {
      playPenalty(args->team1, args->team2, args->goals1, args->goals2);
      if (args->goals1 > args->goals2)
        args->successor = args->team1;
      else
        args->successor = args->team2;
    }

    return NULL;
}*/

/*---------------------------------------------------*/

void playEM(team_t* teams, int numThreads)
{
  // put your code here
  static const int cInitialNumSuccessors = NUMGROUPS * 2 + NUMTHIRDS;
  static const int cTeamsPerGroup = NUMTEAMS / NUMGROUPS;
  team_t* successors[NUMGROUPS * 2 + NUMTHIRDS] = {0};
  team_t* bestThirds[NUMGROUPS];
  int g, curBestThird = 0, numSuccessors = cInitialNumSuccessors;

  initialize();
	
  pthread_t* thread;
  struct arg_str_PG *arg;

  /*thread = malloc(numThreads*sizeof(*thread));
  arg = malloc(numThreads*sizeof(*arg));*/
  thread = malloc(NUMGROUPS*sizeof(*thread));
  arg = malloc(NUMGROUPS*sizeof(*arg));
  
  for( g=0; g<NUMGROUPS; ++g)
  {
    fillarg_str_PG(arg+g, g,
              teams + (g * cTeamsPerGroup),
              cTeamsPerGroup,
              successors + g * 2,
              successors + (numSuccessors - (g * 2) - 1),
              bestThirds + g);
    pthread_create(thread+g, NULL, &passarguments_PG, arg+g);
  }
   
  for(g=0;g<NUMGROUPS; ++g)
  {
     pthread_join(thread[g], NULL);
  }


   //thirds
   sortTeams(NUMGROUPS, bestThirds);

   for (g = 0; g < numSuccessors; ++g) {
     if (successors[g] == NULL) {
         successors[g] = bestThirds[curBestThird++];
    }
  }



  // play final rounds
  while (numSuccessors > 1) {
    playFinalRound(numSuccessors / 2, successors, successors);
    numSuccessors /= 2;
  }

   free(thread);
   free(arg);
}

void playFinalRound(int numGames, team_t** teams, team_t** successors)
{
  // put your code here
  int i;

  pthread_t* threadPFM;
  pthread_t* threadPen;
  struct arg_str_PFM *argPFM;
  struct arg_str_Pen *argPen;
  threadPFM = malloc(numGames*sizeof(*threadPFM));
  threadPen = malloc(numGames*sizeof(*threadPen));
  argPFM = malloc(numGames*sizeof(*argPFM));
  argPen = malloc(numGames*sizeof(*argPen));
  
  team_t* teams1[numGames];
  team_t* teams2[numGames];
  int goals1[numGames];
  int goals2[numGames];
  int b[numGames];
  int numactpenthr=0;

  for(i=0; i <numGames; ++i){
    teams1[i]=teams[i*2];
    teams2[i]=teams[i*2+1];
    goals1[i] = 0; 
    goals2[i] = 0;
    b[i] = 0;
    fillarg_str_PFM(argPFM+i,numGames, i, teams1[i], teams2[i], &goals1[i], &goals2[i]);
  }


  for (i = 0; i < numGames; ++i) {
    pthread_create(threadPFM+i, NULL, &passarguments_PFM, argPFM+i);
   }

   for (i=0; i<numGames; ++i)
   {
      pthread_join(threadPFM[i],NULL);
   }

//final counting
  for (i = 0; i < numGames; ++i) {

    if (goals1[i] > goals2[i])
      successors[i] = teams1[i];
    else if (goals1[i] < goals2[i])
      successors[i] = teams2[i];
    else
    {
      b[i]=1;
      fillarg_str_Pen(argPen+numactpenthr,teams1[i], teams2[i], &goals1[i], &goals2[i]);
      pthread_create(threadPen+i, NULL, &passarguments_Pen, argPen+i);
      numactpenthr++;
    }
  }

 /*  for (i=0; i<numGames; ++i){
     if(b[i]==1){
      fillarg_str_Pen(argPen+i,teams1[i], teams2[i], &goals1[i], &goals2[i]);
      pthread_create(threadPen+i, NULL, &passarguments_Pen, argPen+i);
     }
   }*/

   for (i=0; i<numactpenthr; ++i){
      pthread_join(threadPen[i],NULL);
   }

   for(i=0; i<numGames; ++i){
   if(b[i]==1){
      if (goals1[i] > goals2[i])
        successors[i] = teams1[i];
      else
        successors[i] = teams2[i];
      }
   }

   
   free(threadPFM);
   free(argPFM);
   free(threadPen);
   free(argPen);
   /*free(goals1);
   free(goals2);
   free(teams1);
   free(teams2);*/
}

