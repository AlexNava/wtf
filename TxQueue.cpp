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
		// Wait for the semaphore (tx queue follows rx one)
		SDL_SemWait(pxStatus->pxSendSemaphore);

		switch(pxStatus->xStatus)
		{
		case statusRun:
			// Send all output structures
			break;
		}
	}

	return 0;	// never
}

