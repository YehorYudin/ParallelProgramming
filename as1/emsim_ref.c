#include "emsim_ref.h"

void playEM_ref(team_t* teams)
{
  static const int cInitialNumSuccessors = NUMGROUPS * 2 + NUMTHIRDS;
  static const int cTeamsPerGroup = NUMTEAMS / NUMGROUPS;
  team_t* successors[NUMGROUPS * 2 + NUMTHIRDS] = {0};
  team_t* bestThirds[NUMGROUPS];
  int g, curBestThird = 0, numSuccessors = cInitialNumSuccessors;

  // play groups
  initialize();
  for (g = 0; g < NUMGROUPS; ++g) {
    playGroup(g,
              teams + (g * cTeamsPerGroup),
              cTeamsPerGroup,
              successors + g * 2,
              successors + (numSuccessors - (g * 2) - 1),
              bestThirds + g);
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
    playFinalRound_ref(numSuccessors / 2, successors, successors);
    numSuccessors /= 2;
  }
}

void playFinalRound_ref(int numGames, team_t** teams, team_t** successors)
{
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
}
