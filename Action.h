#ifdef _Skip

#ifndef _Action_h
#define _Action_h
#include <chrono>;
#include <vector>;
#include <functional>;
using namespace std;

class Action
{
	typedef vector<vector<int> > actionblockstype;
	typedef function<bool(actionblockstype)> stagetype;

	stagetype defaultstage = [](actionblockstype actionblocks) {return true};

public:
	int timelimitms; //max time allowed to complete the action
	chrono::high_resolution_clock::time_point timeofstage0; //the time at which stage 0 was activated
	int currentstage; //keeps track of which stages' criteria have been met... -1 means stage 0 not yet met
	std::vector<stagetype> stages; 

	void activatetimer(); //changes the time of stage0
	double timeleft(); //returns time left before violating time limit
	void nextstage(); //increases currentstage by 1
	Action(int timelimitms) : timelimitms(timelimitms), currentstage(-1)
	{
		stages.push_back(defaultstage);
	}
	Action(int timelimitms, stagetype stage0) : timelimitms(timelimitms), currentstage(-1)
	{
		stages.push_back(stage0);
	}
	Action(int timelimitms, vector<stagetype> stages) : timelimitms(timelimitms), stages(stages), currentstage(-1){};
};

#endif

if (Action.stages[Action.currentstage + 1](actionblocks)) {Action.nextstage}

#endif