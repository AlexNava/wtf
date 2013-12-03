/*
 * Agent.h
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#ifndef AGENT_H_
#define AGENT_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"
#include "SDL2/SDL_thread.h"
#include <string>
using namespace std;
#include "AgentsTypes.h"
#include "RxQueue.h"

typedef struct
{
	bool bRxRun;
	bool bTxRun;
} AgentStatus;

class Agent
{
public:
	Agent();
	virtual ~Agent();
	bool init(string name);

	bool setRxObject(void *);
	bool setStepCallback(void *);
	void run();

protected:
	AgentStatus m_xStatus;
	string m_strName;
private:
	SDL_Thread *m_pxRxThread;
};

#endif /* AGENT_H_ */
