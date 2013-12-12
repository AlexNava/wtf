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
	stateDiscover,
	stateSetup,
	stateRun
} eState;

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
	string strName;
	string strFamName;
	Uint16 listeningPort;
	eState eAutomaState;
	Uint32 lastTick;
	Uint32 currentTick;
	SDL_sem *pxTxGoSemaphore;
	SDL_sem *pxRxGoSemaphore;
	SDL_sem *pxStepSemaphore;
	SDL_sem *pxSendSemaphore;
	map<string, sStructInfo> xStructures;
} AgentStatus;

Sint32 diffWithOverflow(Uint32, Uint32);

class Agent
{
public:
	Agent();
	virtual ~Agent();
	bool init(string name, string famName = "");

	bool addStruct(string name, void *pData, size_t size, eDataDirection direction, Uint8 period = 1);
	bool setStepCallback(void (*stepFunc)());
	void run();

protected:
	AgentStatus m_xStatus;
private:
	SDL_Thread *m_pxRxThread;
	SDL_Thread *m_pxTxThread;
	void (*m_pStepFunc)();
};

#endif /* AGENT_H_ */
