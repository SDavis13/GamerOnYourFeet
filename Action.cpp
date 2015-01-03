# include "Action.h"
# include <ctime>

void Action::activatetimer()
{
	timeofstage0 = clock();
	return;
}

double Action::timeleft()
{
	return (clock() - timeofstage0);
}

void Action::nextstage()
{
	++currentstage;
}