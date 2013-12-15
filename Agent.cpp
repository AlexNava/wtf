/*
 * Agent.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include "Agent.h"

Agent::Agent()
{
	m_status.name = "";
	m_status.famName = "";
	m_status.listeningPort = 0;
	m_status.eAutomaState = stateDiscover;
	m_status.lastTick = 0;
	m_status.currentTick = 0;
	m_status.pTxGoSemaphore = NULL;
	m_status.pRxGoSemaphore = NULL;
	m_status.pStepSemaphore = NULL;
	m_status.pSendSemaphore = NULL;
	m_status.pInputMutex = NULL;
	m_status.pOutputMutex = NULL;
	m_pRxThread = NULL;
	m_pTxThread = NULL;

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

	m_status.name = name;

	// Check for bad characters, but famName can be empty
	if ((famName.size() > AGENT_NAME_SIZE)
			|| (famName.find(' ') != string::npos))
	{
		return false;
	}

	m_status.famName = famName;

	m_status.pTxGoSemaphore = SDL_CreateSemaphore(0);
	m_status.pRxGoSemaphore = SDL_CreateSemaphore(0);
	m_status.pStepSemaphore = SDL_CreateSemaphore(0);
	m_status.pSendSemaphore = SDL_CreateSemaphore(0);
	m_status.pInputMutex = SDL_CreateMutex();
	m_status.pOutputMutex = SDL_CreateMutex();
	m_pRxThread = SDL_CreateThread(rxQueueFunc, "Rx", &m_status);
	m_pTxThread = SDL_CreateThread(txQueueFunc, "Tx", &m_status);

	return true;
}

void Agent::run()
{
	// Start queues (when RX is ready, it starts TX)
	SDL_SemPost(m_status.pRxGoSemaphore);
	while (true)	// Keep the main process alive
	{
		SDL_SemWait(m_status.pStepSemaphore);

		// Call step callback (if it has been set)
		Sint32 tickDelta = m_status.currentTick - m_status.lastTick;
		if (m_pStepFunc != NULL)
		{
			if (tickDelta > 0)
				for (int iTick = 0; iTick < tickDelta; iTick++)
				{
					SDL_LockMutex(m_status.pInputMutex);
					SDL_LockMutex(m_status.pOutputMutex);
					m_pStepFunc();
					SDL_UnlockMutex(m_status.pInputMutex);
					SDL_UnlockMutex(m_status.pOutputMutex);
				}
			else if (tickDelta < 0)
				for (int iTick = 0; iTick > tickDelta; iTick--)
				{
					SDL_LockMutex(m_status.pInputMutex);
					SDL_LockMutex(m_status.pOutputMutex);
					m_pStepFunc();
					SDL_UnlockMutex(m_status.pInputMutex);
					SDL_UnlockMutex(m_status.pOutputMutex);
				}
		}
	}
}

bool Agent::addStruct(string name, void *pData, size_t size, eDataDirection direction, Uint8 period)
{
	if (name.empty()
			|| (name.size() > STRUCT_NAME_SIZE)
			|| (name.find(' ') != string::npos))
		return false;

	if (m_status.localStructures.find(name) != m_status.localStructures.end())
		return false;

	if ((size + sizeof(sHeader) + sizeof(sDataStruct)) > AGENT_MAX_PACKET_SIZE)
		return false;

	sStructInfo structInfo;
	structInfo.id = m_status.localStructures.size();
	structInfo.pData = pData;
	structInfo.size = (pData != NULL)? size : 0;
	structInfo.eDirection = direction;
	structInfo.period = (direction == dataIn)? period : 0;

	m_status.localStructures.insert(pair<string, sStructInfo>(name, structInfo));

	return true;
}

bool Agent::setStepCallback(void (*stepFunc)())
{
	m_pStepFunc = stepFunc;
	return true;
}
