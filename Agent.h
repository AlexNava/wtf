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
#include "Protocol.h"
#include "RxQueue.h"
#include "TxQueue.h"

typedef enum
{
	statusAnnounce,
	statusConfig,
	statusRun
} eStatus;

typedef struct
{
	bool bRxRun;
	bool bTxRun;
	eStatus xStatus;
} AgentStatus;

class Agent
{
public:
	Agent();
	virtual ~Agent();
	bool init(string name);

	bool setInputObject(string, void *, size_t);
	bool setOutputObject(string, void *, size_t);
	bool setStepCallback(void *);
	void run();

protected:
	AgentStatus m_xStatus;
	string m_strName;
private:
	SDL_Thread *m_pxRxThread;
	SDL_Thread *m_pxTxThread;
};

#endif /* AGENT_H_ */
