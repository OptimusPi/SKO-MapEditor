#include "OPI_Timestep.h"
#include "OPI_Clock.h"

OPI_Timestep::OPI_Timestep(unsigned int FPS)
{
   frameTime = (unsigned long int)(1000000000.0/FPS);
   currentTime = OPI_Clock::nanoseconds();
   newTime = 0;
   accumulator = 0;
   ready = false;
}

void OPI_Timestep::Update()
{
     newTime = OPI_Clock::nanoseconds();
     accumulator += newTime - currentTime;
     currentTime = newTime;
}

bool OPI_Timestep::Check()
{
     if ( accumulator >= frameTime )
     {
         accumulator -= frameTime;
         return true;
     }
     
     return false;
}
