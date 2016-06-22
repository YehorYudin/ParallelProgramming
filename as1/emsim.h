#ifndef EM_H
#define EM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "db.h"

#define true 1
#define false 0
#define NUMTEAMS 24
#define NUMGROUPS 6
#define NUMTHIRDS 4
#define NUMTHREADS 16

typedef struct{
  char name1[40];
  char name2[40];
  int goals1;
  int goals2;
} match_result;

match_result* results;

void playFinalMatch(int numGames, int gameNo,
                    team_t* team1, team_t* team2, int* goals1, int* goals2);

void playGroup(int groupNo, team_t* teams, int numTeams,
               team_t** first, team_t** second, team_t** third);

void playFinalRound(int numGames, team_t** teams, team_t** successors);

void playPenalty(team_t* team1, team_t* team2, int* goals1, int* goals2);

void sortTeams(int numTeams, team_t** teams);

void playEM(team_t* teams, int numThreads);

int final_pos(int numGames);

void initialize();

#endif // EM_H
