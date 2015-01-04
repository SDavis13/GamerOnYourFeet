#ifdef _Skip

# include "Action.h"
# include <ctime>
# include <chrono>
#include <iostream>;
#include <ratio>;
using namespace std::chrono;

void Action::activatetimer()
{
	timeofstage0 = high_resolution_clock::now();
	return;
}

double Action::timeleft()
{
	return (high_resolution_clock::now() - timeofstage0).count();
}

void Action::nextstage()
{
	++currentstage;
}

#endif