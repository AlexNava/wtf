/*
 * TxQueue.h
 *
 *  Created on: Dec 5, 2013
 *      Author: alessandro
 */

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"

#include "TxQueue.h"
#include "Agent.h"
#include "Protocol.h"

int txQueueFunc(void *pxData)
{
	AgentStatus *pxStatus = (AgentStatus*)pxData;

	UDPpacket *pxPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xSock = 0;

	while (true)
	{
		switch(pxStatus->xStatus)
		{
		case statusAnnounce:
			// Send announcement messages with a regular interval
			break;
		case statusRun:
			// Wait for the semaphore (tx queue follows rx one)
			SDL_SemWait(pxStatus->pxSendSemaphore);
			if (pxStatus->xStatus != statusRun)	// It can be changed by other threads
				break;
			// Send all structures

			break;
		}
	}

	return 0;	// never
}

