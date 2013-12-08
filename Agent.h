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
#include <map>
using namespace std;
#include "Protocol.h"
#include "RxQueue.h"
#include "TxQueue.h"

typedef enum
{
	statusDiscover,
	statusSetup,
	statusRun
} eStatus;

typedef struct
{
	Uint16			id;
	eDataDirection	eDirection;
	Uint8			period;
	void			*pData;
	size_t			size;
} sStructInfo;

typedef struct
{
	eStatus xStatus;
	SDL_sem *pxTxGoSemaphore;
	SDL_sem *pxRxGoSemaphore;
	SDL_sem *pxStepSemaphore;
	SDL_sem *pxSendSemaphore;
	map<string, sStructInfo> xStructures;
} AgentStatus;

class Agent
{
public:
	Agent();
	virtual ~Agent();
	bool init(string name, string famName = "");

	bool addStruct(string name, void *pData, size_t size, eDataDirection direction, Uint8 period = 1);
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
