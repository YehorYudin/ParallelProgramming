#include "emsim.h"
#include <omp.h>

void playGroups(team_t* teams)
{
  static const int cNumTeamsPerGroup = NUMTEAMS / NUMGROUPS;
 /* static const int numGames = NUMGROUPS*cNumTeamsPerGroup*(cNumTeamsPerGroup-1)/2;
  static const int cNumGamesPerGroup = cNumTeamsPerGroup*(cNumTeamsPerGroup-1)/2;
  int g, i, j, k;
  int goalsI[numGames];
  int goalsJ[numGames];
  k=0;

  #pragma omp parallel for schedule(dynamic) shared(teams, goalsI, goalsJ) private(i, j)
  for (g = 0; g < NUMGROUPS; ++g) {

    //printf("No of threads is %i, this thread is %i \n",omp_get_num_threads(),omp_get_thread_num());
    k=0;
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i) {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j) {

        playGroupMatch(g, teams + i, teams + j, &goalsI[g*cNumGamesPerGroup+k], &goalsJ[g*cNumGamesPerGroup+k]);
        k++;
      }
    }

  }

k=0;
  for (g = 0; g < NUMGROUPS; ++g) {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i) {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j) {

        teams[i].goals += goalsI[k] - goalsJ[k];
        teams[j].goals += goalsJ[k] - goalsI[k];
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
  }*/

  int g, i, j, goalsI, goalsJ;
  
#pragma omp parallel for schedule(dynamic) private(goalsI, goalsJ,i,j,g) shared(teams)
  for (g = 0; g < NUMGROUPS; ++g) {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i) {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j) {

        // team i plays against team j in group g
        playGroupMatch(g, teams + i, teams + j, &goalsI, &goalsJ);
        teams[i].goals += goalsI - goalsJ;
        teams[j].goals += goalsJ - goalsI;
        if (goalsI > goalsJ)
          teams[i].points += 3;
        else if (goalsI < goalsJ)
          teams[j].points += 3;
        else {
          teams[i].points += 1;
          teams[j].points += 1;
        }
      }
    }
  }


}

void playFinalRound(int numGames, team_t** teams, team_t** successors)
{
  /*team_t* team1[numGames];
  team_t* team2[numGames];
  int i;
  int goals1[numGames];
  int goals2[numGames];

  #pragma omp parallel for schedule(dynamic) shared(team1,team2,goals1,goals2,teams) private(i)
  for (i = 0; i < numGames; ++i) {
    team1[i] = teams[i*2];
    team2[i] = teams[i*2+1];
    playFinalMatch(numGames, i, team1[i], team2[i], &goals1[i], &goals2[i]);
  }
 
 #pragma omp parallel for schedule(dynamic) shared(team1,team2,goals1,goals2) private(i)
 for (i = 0; i < numGames; ++i) {
   if(goals1[i]==goals2[i]){
      playPenalty(team1[i], team2[i], &goals1[i], &goals2[i]);
   }
 }

  for (i = 0; i < numGames; ++i) {
    if (goals1[i] > goals2[i])
      successors[i] = team1[i];
    else if (goals1[i] < goals2[i])
      successors[i] = team2[i];
  }*/

  team_t* team1;
  team_t* team2;
  int i, goals1 = 0, goals2 = 0;
  
  #pragma omp parallel for schedule(dynamic) private(i,goals1,goals2,team1,team2) shared(successors,teams)
  for (i = 0; i < numGames; ++i) {
    team1 = teams[i*2];
    team2 = teams[i*2+1];
    playFinalMatch(numGames, i, team1, team2, &goals1, &goals2);
  
  if(goals1==goals2)
    playPenalty(team1, team2, &goals1, &goals2);

    if (goals1 > goals2)
      successors[i] = team1;
    else if (goals1 < goals2)
      successors[i] = team2;

  }
} 
