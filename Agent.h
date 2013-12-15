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
#include <vector>
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
	string name;
	string famName;
	IPaddress address;
	vector<sStructInfo> structuresToSend;
} sNeighbor;

typedef map<string, sStructInfo> tStructMap;

typedef struct
{
	string name;
	string famName;
	Uint16 listeningPort;
	eState eAutomaState;
	Uint32 lastTick;
	Uint32 currentTick;
	SDL_sem *pTxGoSemaphore;
	SDL_sem *pRxGoSemaphore;
	SDL_sem *pStepSemaphore;
	SDL_sem *pSendSemaphore;
	SDL_mutex *pInputMutex;
	SDL_mutex *pOutputMutex;
	tStructMap localStructures;
	vector<sNeighbor> neighbors;
} sAgentStatus;

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
	sAgentStatus m_status;
private:
	SDL_Thread *m_pRxThread;
	SDL_Thread *m_pTxThread;
	void (*m_pStepFunc)();
};

#endif /* AGENT_H_ */
