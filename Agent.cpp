/*
 * Agent.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include "Agent.h"

Agent::Agent()
{
	m_xStatus.strName = "";
	m_xStatus.strFamName = "";
	m_xStatus.listeningPort = 0;
	m_xStatus.eAutomaState = stateDiscover;
	m_xStatus.lastTick = 0;
	m_xStatus.currentTick = 0;
	m_xStatus.pxTxGoSemaphore = NULL;
	m_xStatus.pxRxGoSemaphore = NULL;
	m_xStatus.pxStepSemaphore = NULL;
	m_xStatus.pxSendSemaphore = NULL;
	m_pxRxThread = NULL;
	m_pxTxThread = NULL;

	m_pStepFunc = NULL;

	if (SDL_Init(0) == -1)
	{
		printf("SDL_Init: %s\n", SDL_GetError());
		exit(1);
	}

	if (SDLNet_Init() == -1)
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}
}

Agent::~Agent()
{
}

bool Agent::init(string name, string famName)
{
	if (name.empty()
			|| (name.size() > AGENT_NAME_SIZE)
			|| (name.find(' ') != string::npos))
	{
		return false;
	}

	m_xStatus.strName = name;

	// Check for bad characters, but famName can be empty
	if ((famName.size() > AGENT_NAME_SIZE)
			|| (famName.find(' ') != string::npos))
	{
		return false;
	}

	m_xStatus.strFamName = famName;

	m_xStatus.pxTxGoSemaphore = SDL_CreateSemaphore(0);
	m_xStatus.pxRxGoSemaphore = SDL_CreateSemaphore(0);
	m_xStatus.pxStepSemaphore = SDL_CreateSemaphore(0);
	m_xStatus.pxSendSemaphore = SDL_CreateSemaphore(0);
	m_pxRxThread = SDL_CreateThread(rxQueueFunc, "Rx", &m_xStatus);
	m_pxTxThread = SDL_CreateThread(txQueueFunc, "Tx", &m_xStatus);

	return true;
}

void Agent::run()
{
	// Start queues (when RX is ready, it starts TX)
	SDL_SemPost(m_xStatus.pxRxGoSemaphore);
	while (true)	// Keep the main process alive
	{
		SDL_SemWait(m_xStatus.pxStepSemaphore);

		// Call step callback (if it has been set)
		Sint32 TickDelta = m_xStatus.currentTick - m_xStatus.lastTick;
		if (m_pStepFunc != NULL)
			m_pStepFunc();
	}
}

bool Agent::addStruct(string name, void *pData, size_t size, eDataDirection direction, Uint8 period)
{
	if (name.empty()
			|| (name.size() > STRUCT_NAME_SIZE)
			|| (name.find(' ') != string::npos))
		return false;

	if (m_xStatus.xStructures.find(name) != m_xStatus.xStructures.end())
		return false;

	if ((size + sizeof(sHeader) + sizeof(sDataStruct)) > AGENT_MAX_PACKET_SIZE)
		return false;

	sStructInfo xStruct;
	xStruct.id = m_xStatus.xStructures.size();
	xStruct.pData = pData;
	xStruct.size = (pData != NULL)? size : 0;
	xStruct.eDirection = direction;
	xStruct.period = period;

	m_xStatus.xStructures.insert(pair<string, sStructInfo>(name, xStruct));

	return true;
}

bool Agent::setStepCallback(void (*stepFunc)())
{
	m_pStepFunc = stepFunc;
	return true;
}
