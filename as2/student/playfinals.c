void playFinalRound(int numGames, team_t** teams, team_t** successors, int numWorker)
{

  int i;
printf("We are playing finals\n");
/*---thread and strucures allocation*/
 // pthread_t* threadW;
  //struct arg_str_PFM *argPFM;
  //struct arg_str_Pen *argPen;
  //threadW = malloc(numWorker*sizeof(*threadW));
  //argPFM = malloc(numGames*sizeof(*argPFM));
  //argPen = malloc(numGames*sizeof(*argPen));

  
/*---local arrays allocation*/
  team_t* teams1[numGames];
  team_t* teams2[numGames];
  int goals1[numGames];
  int goals2[numGames];
 // int b[numGames];
  int numactpenthr=0;


/*---assigning teams and initilisation of data structures*/
  for(i=0; i <numGames; ++i)
  {
    teams1[i]=teams[i*2];
    teams2[i]=teams[i*2+1];
    goals1[i] = 0; 
    goals2[i] = 0;
    //b[i] = 0;
    //fillarg_str_PFM(argPFM+i,numGames, i, teams1[i], teams2[i], &goals1[i], &goals2[i]);
    //fillarg_str_Pen(argPen+numactpenthr,teams1[i], teams2[i], &goals1[i], &goals2[i]);
  }


/*playing matches*/
  for (i = 0; i < numGames; ++i) 
  {
    //pthread_create(threadPFM+i, NULL, &passarguments_PFM, argPFM+i);
    //has to be done dynamically!!!!!
     playFinalMatch(numGames, i, teams1[i], teams2[i], &goals1[i], &goals2[i]);
   }

   /*for (i=0; i<numGames; ++i)
   {
      pthread_join(threadPFM[i],NULL);
   }*/


/*---counting goals and checks if penalty needs*/
  for (i = 0; i < numGames; ++i)
  {
    /*if (goals1[i] > goals2[i])
      successors[i] = teams1[i];
    else if (goals1[i] < goals2[i])
      successors[i] = teams2[i];
    else*/
   if(goals1[i] == goals2[i])
    {
      //b[i]=1;
      //fillarg_str_Pen(argPen+numactpenthr,teams1[i], teams2[i], &goals1[i], &goals2[i]);
      //pthread_create(threadPen+i, NULL, &passarguments_Pen, argPen+i);
      // has to be implemented dynamically!!!!!
      playPenalty(teams1[i], teams2[i], &goals1[i], &goals2[i]);
      numactpenthr++;
    }
  }


   /*for (i=0; i<numactpenthr; ++i)
   {
      pthread_join(threadPen[i],NULL);
   }*/


/*---final assignment*/
   for(i=0; i<numGames; ++i)
   {
   //if(b[i]==1){
      if (goals1[i] > goals2[i])
        successors[i] = teams1[i];
      else
        successors[i] = teams2[i];
    //}
   }


/*---free memory*/
   /*free(argPFM);
   free(argPen);
   free(threadW);*/
}

void signal_threads(struct arg_str_PGM * argums, int numGames, int cNumTeamsPerGroup, const int numWorker, team_t* teams, int* goalsI, int* goalsJ,struct my_condstruct * CVprod, struct my_condstruct *CVcons )
{
printf("WE RUN MASTER THREAD!\n");
int i,j,g;
int k=0;
/*---playing matches and counting points---*/
  for (g = 0; g < NUMGROUPS; ++g)
  {
    for (i =  g * cNumTeamsPerGroup; i < (g+1) * cNumTeamsPerGroup; ++i)
    {
      for (j = (g+1) * cNumTeamsPerGroup - 1; j > i; --j)
       {
        pthread_mutex_lock( CVprod->mutex );
        while(CVcons->synco<numWorker)
            pthread_cond_wait(&(CVcons->cond),CVcons->mutex);
        // team i plays against team j in group g[j]
        fillarg_str_PGM(argums+k, g, teams + i, teams + j, &goalsI[k], &goalsJ[k]);
	
	CVprod->synco= 1;
        CVprod->value= k; //set predicate
        pthread_cond_signal( &(CVprod->cond) );
 printf("Producer sends a signal to catch game no %i\n", CVprod->synco);
        pthread_mutex_lock( CVprod->mutex );
	k++;  
       }  
     }
   }

}


void fillarg_str_ST(void* arguments, struct arg_str_PGM * argums, int numGames, int cNumTeamsPerGroup,const int numWorker, team_t* teams,int* goalsI, int* goalsJ,struct my_condstruct * CVprod, struct my_condstruct * CVcons )
{
  struct arg_str_ST *args = (struct arg_str_ST *)arguments;
  args->argums = argums;
  args->numGames = numGames;
  args->cNumTeamsPerGroup = cNumTeamsPerGroup;
  args->numWorker = numWorker;
  args->teams = teams;
  args->goalsI = goalsI;
  args->goalsJ = goalsJ;
  args->CVprod = CVprod;
  args->CVcons = CVcons;
}

void* passarguments_ST(void* arguments)
{
 printf("Is it really here?\n");
 struct arg_str_ST *args = (struct arg_str_ST *)arguments;
 printf("We almost run signal_threads\n");
 signal_threads(args->argums, args->numGames, args->cNumTeamsPerGroup, args->numWorker, args->teams,args->goalsI,args->goalsJ,args->CVprod,args->CVcons);
 //printf("Shit happened I don't even know where\n");
 return NULL;
}

