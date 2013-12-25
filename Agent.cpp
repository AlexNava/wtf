/*
 * Agent.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include "Agent.h"
#include <fstream>

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
	m_status.pStepMutex = NULL;
	m_pRxThread = NULL;
	m_pTxThread = NULL;

	m_pStepFunc = NULL;
	m_pResetFunc = NULL;

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
	m_status.pResetSemaphore = SDL_CreateSemaphore(0);
	m_status.pInputMutex = SDL_CreateMutex();
	m_status.pOutputMutex = SDL_CreateMutex();
	m_status.pStepMutex = SDL_CreateMutex();
	m_pRxThread = SDL_CreateThread(rxQueueFunc, "Rx", &m_status);
	m_pTxThread = SDL_CreateThread(txQueueFunc, "Tx", &m_status);

	ifstream discoverAddressesFile;
	discoverAddressesFile.open("./discover.txt");
	if ((discoverAddressesFile.rdstate() & ifstream::failbit) == 0)
	{
		while (!discoverAddressesFile.eof())
		{
			string addressLine;
			getline(discoverAddressesFile, addressLine);
			if (addressLine.empty())
				continue;
			m_status.discoverAddresses.push_back(addressLine);
			printf("Addead address to discover: %s\n", addressLine.c_str());
		}
	}
	else
	{
		printf("discover.txt file not found, skipping\n");
	}
	discoverAddressesFile.close();
	return true;
}

void Agent::run()
{
	// Start queues (when RX is ready, it starts TX)
	SDL_SemPost(m_status.pRxGoSemaphore);
	while (true)	// Run in the main process and never return
	{
		SDL_SemWait(m_status.pStepSemaphore);

		SDL_LockMutex(m_status.pStepMutex);
		// Call step callback (if it has been set)
		if ((m_status.eAutomaState == stateRun) && (m_pStepFunc != NULL))
		{
			SDL_LockMutex(m_status.pInputMutex);
			for (Uint32 iSt = 0; iSt < m_status.localStructuresById.size(); iSt++)
			{
				if (m_status.localStructuresById[iSt].eDirection == dataIn)
				{
					memcpy(m_status.localStructuresById[iSt].pData, m_status.localStructuresById[iSt].pFutureData, m_status.localStructuresById[iSt].size);
					// Swap buffers instead of copying (i don't care about buffers in the struct map)
					void *pTemp = m_status.localStructuresById[iSt].pFutureData;
					m_status.localStructuresById[iSt].pFutureData = m_status.localStructuresById[iSt].pWaitingData;
					m_status.localStructuresById[iSt].pWaitingData = pTemp;
				}
			}
			SDL_UnlockMutex(m_status.pInputMutex);

			m_status.lastTick = m_status.currentTick++;
			m_pStepFunc();

			SDL_LockMutex(m_status.pOutputMutex);
			for (Uint32 iSt = 0; iSt < m_status.localStructuresById.size(); iSt++)
			{
				if (m_status.localStructuresById[iSt].eDirection == dataOut)
				{
					memcpy(m_status.localStructuresById[iSt].pFutureData, m_status.localStructuresById[iSt].pData, m_status.localStructuresById[iSt].size);
				}
			}
			SDL_UnlockMutex(m_status.pOutputMutex);
			// Release TX queue
			SDL_SemPost(m_status.pSendSemaphore);
		}
		else if ((m_status.eAutomaState == stateSetup) && (m_pResetFunc != NULL))
		{
			SDL_LockMutex(m_status.pOutputMutex);
			m_pResetFunc();
			SDL_UnlockMutex(m_status.pOutputMutex);
		}
		SDL_UnlockMutex(m_status.pStepMutex);
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
	structInfo.id = m_status.localStructuresById.size();
	structInfo.pData = pData;
	structInfo.pWaitingData = (pData != NULL)? new char[size] : NULL;
	structInfo.pFutureData = ((pData != NULL) && (direction == dataIn))? new char[size] : NULL;
	structInfo.size = (pData != NULL)? size : 0;
	structInfo.eDirection = direction;
	structInfo.period = (direction == dataIn)? period : 0;

	m_status.localStructures.insert(pair<string, sStructInfo>(name, structInfo));
	m_status.localStructuresById.push_back(structInfo);

	return true;
}

bool Agent::setStepCallback(void (*pStepFunc)())
{
	m_pStepFunc = pStepFunc;
	return true;
}

bool Agent::setResetCallback(void (*pResetFunc)())
{
	m_pResetFunc = pResetFunc;
	return true;
}
