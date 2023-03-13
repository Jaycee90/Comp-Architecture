/* Project 3 CS 3339 - Fall 2017
 *********************************/
#include "Stats.h"

Stats::Stats()
{
   cycles = PIPESTAGES - 1;  // pipeline startup cost
   flushes = 0;
   bubbles = 0;

   memops = 0;
   branches = 0;
   taken = 0;

   for(int i = IF1; i < PIPESTAGES; i++) 
   {
      rawhazards[i] = 0;
      resultReg[i] = -1;
      resultStage[i] = 0;
   }
}

void Stats::clock(PIPESTAGE stage) 
{
    cycles++;
        // This method is complete
        // pipeline the register tracking from provided start stage
        // (ops in 'stage' thru end of pipe advance, ops in stages before 'stage'
        // are frozen)
    for(int i = WB; i > stage; i--) 
    {
        resultReg[i] = resultReg[i-1];
        resultStage[i] = resultStage[i-1];
    }
        // inject no-op into 'stage'
    resultReg[stage] = -1;
    resultStage[stage] = 0;
}

void Stats::registerSrc(int r, PIPESTAGE stage) 
{
    int bubbles = 0, valid = 0, need = 0;
    
    for (int i = EXE1; i < WB; i++)
    {
       if (resultReg[i] == r)
       {
          rawhazards[i]++;
          valid = resultStage[i] - i;
          need = stage - ID;

          if (valid > need)
             bubbles = valid - need;
          
          while(bubbles > 0){
             bubble();
             bubbles--;
          }
          break;
       }
    }
}

void Stats::registerDest(int r, PIPESTAGE stage) 
{
  resultReg[ID] = r;
  resultStage[ID] = stage;
}

void Stats::flush(int count) 
{
  for (int i = 0; i < count; i++)
  {
     flushes++;
     clock(IF1);
  }
}

int Stats::getRAWHazards(PIPESTAGE stage) 
{
  return rawhazards[stage];
}

void Stats::bubble() 
{
   bubbles++;
   clock(EXE1);
}
