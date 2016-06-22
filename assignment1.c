#include "emsim.h"
#include <pthread.h>

struct pthread_args
{
  int groupMin;
  int groupMax;
  int numSuccessors;
  int teamsPerGroup;
  team_t* teams;
  team_t** successors;
  team_t** bestThirds;
  match_result *results;
};

void* parallel_calls_to_playGroup(void * ptr) {
  struct pthread_args *arg = ptr;
  int g;

  for (g = arg->groupMin; g < arg->groupMax; ++g) {
    playGroup(g,
              arg->teams + (g * arg->teamsPerGroup),
              arg->teamsPerGroup,
              arg->successors + g * 2,
              arg->successors + (arg->numSuccessors - (g * 2) - 1),
              arg->bestThirds + g);
  }

  return NULL;
}


void playEM(team_t* teams, int numThreads)
{
  team_t* successors[NUMGROUPS * 2 + NUMTHIRDS] = {0};
  team_t* bestThirds[NUMGROUPS];
  int teamsPerGroup, numSuccessors, groupsPerThread, groupsRest;
  int i, g = 0, t;
  int curBestThird = 0;
  pthread_t *threads;
  struct pthread_args * thread_arg;

  threads = malloc(numThreads * sizeof (*threads));
  thread_arg = malloc(numThreads * sizeof (*thread_arg));

  // play groups
  numSuccessors = NUMGROUPS * 2 + NUMTHIRDS;
  teamsPerGroup = NUMTEAMS / NUMGROUPS;
  groupsPerThread = NUMGROUPS / numThreads;
  groupsRest = NUMGROUPS % numThreads;

  initialize();
  for (t = 0; t < numThreads; ++t) {
    thread_arg[t].groupMin = g;
    g = g + groupsPerThread;
    if (groupsRest > 0) {
      g++;
      groupsRest--;
    }
    thread_arg[t].groupMax = g;
    thread_arg[t].teams = teams;
    thread_arg[t].numSuccessors = numSuccessors;
    thread_arg[t].teamsPerGroup = teamsPerGroup;
    thread_arg[t].successors = successors;
    thread_arg[t].bestThirds = bestThirds;
    thread_arg[t].results = results;
    pthread_create(threads+t, NULL, &parallel_calls_to_playGroup, thread_arg+t);
  }

  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  // fill best thirds
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

  free(threads);
  free(thread_arg);
}

struct pthread_args_match {
  int numGames;
  int gameNo;
  team_t* team1;
  team_t* team2;
  int goals1;
  int goals2;
};

void* playMatchInPar(void* ptr) {
  struct pthread_args_match *args = (ptr);
  playFinalMatch(args->numGames, args->gameNo,
                 args->team1, args->team2, &args->goals1, &args->goals2);
  return NULL;
}

void playFinalRound(int numGames, team_t** teams, team_t** successors)
{
  pthread_t *threads;
  struct pthread_args_match* args;
  int i;

  args = calloc(sizeof(*args), numGames);
  threads = malloc(numGames * sizeof (*threads));

  for (i = 0; i < numGames; ++i) {
    args[i].numGames = numGames;
    args[i].gameNo = i;
    args[i].team1 = teams[i*2];
    args[i].team2 = teams[i*2+1];
    pthread_create(threads + i, NULL, &playMatchInPar, &args[i]);
  }

  for (i = 0; i < numGames; ++i) {
    pthread_join(threads[i], NULL);
    if (args[i].goals1 > args[i].goals2)
      successors[i] = args[i].team1;
    else if (args[i].goals1 < args[i].goals2)
      successors[i] = args[i].team2;
    else
    {
      playPenalty(args[i].team1, args[i].team2,
                  &args[i].goals1, &args[i].goals2);
      if (args[i].goals1 > args[i].goals2)
        successors[i] = args[i].team1;
      else
        successors[i] = args[i].team2;
    }
  }

  free(threads);
  free(args);
}
