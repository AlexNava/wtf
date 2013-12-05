/*
 * Agent.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include "Agent.h"
#include "Protocol.h"

Agent::Agent()
{
	m_xStatus.bTxRun = false;
	m_xStatus.bRxRun = false;
	m_xStatus.xStatus = statusAnnounce;
	m_strName = "";

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

	m_pxRxThread = SDL_CreateThread(rxQueueFunc, "Rx", &m_xStatus);
	m_pxTxThread = SDL_CreateThread(txQueueFunc, "Tx", &m_xStatus);
}

Agent::~Agent()
{
}

bool Agent::init(string name)
{
	if (name.empty()
			|| (name.size() > AGENT_NAMES_SIZE)
			|| (name.find(' ') != string::npos))
	{
		return false;
	}

	m_strName = name;

	return true;
}

void Agent::run()
{
	m_xStatus.bTxRun = true;
	m_xStatus.bRxRun = true;
	while (true)	// Keep the main process alive
	{
		SDL_Delay(10000);
	}
}
