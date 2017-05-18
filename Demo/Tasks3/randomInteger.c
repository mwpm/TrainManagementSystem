/* This random number generator algorithim was taken from the "rand1.c" 
example provided by the ta on the discussion board.  The link to this example
is here: https://catalyst.uw.edu/gopost/conversation/mishra/767701
*/

#include "header.h"


int seed = 1; 

int randomInteger(int low, int high)
{
	double randNum = 0.0;
 	int multiplier = 2743;
	int addOn = 5923;
	double max = INT_MAX + 1.0;

	int retVal = 0;

	if (low > high)
		retVal = randomInteger(high, low);
	else
	{
   		seed = seed*multiplier + addOn;
   		randNum = seed;

		if (randNum <0)
		{
			randNum = randNum + max;
		}

		randNum = randNum/max;

		retVal =  ((int)((high-low+1)*randNum))+low;
	}
	
	return retVal;
}

