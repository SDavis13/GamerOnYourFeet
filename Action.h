#ifndef _Action_h
#define _Action_h
using namespace std;

class Action
{
public:
	int numofstages; 
	int timelimitms; //max time allowed to complete the action
	double timeofstage0; //the time at which stage 0 was activated
	int currentstage; //keeps track of which stages' criteria have been met... -1 means stage 0 not yet met

	void activatetimer(); //changes the time of stage0
	double timeleft(); //returns time left before violating time limit
	void nextstage(); //increases currentstage by 1


};

#endif