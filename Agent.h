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
	eStatus xStatus;
	SDL_sem *pxStepSemaphore;
	SDL_sem *pxSendSemaphore;
} AgentStatus;

class Agent
{
public:
	Agent();
	virtual ~Agent();
	bool init(string name, string famName = "");

	bool setInputObject(string, void *, size_t);
	bool setOutputObject(string, void *, size_t);
	bool setStepCallback(void *);
	void run();

protected:
	AgentStatus m_xStatus;
	string m_strName;
	string m_strFamName;
private:
	SDL_Thread *m_pxRxThread;
	SDL_Thread *m_pxTxThread;
};

#endif /* AGENT_H_ */
