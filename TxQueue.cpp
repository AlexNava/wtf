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

int txQueueFunc(void *pxData)
{
	AgentStatus *pxStatus = (AgentStatus*)pxData;

	UDPpacket *pxDiscoverPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xDiscoverSock = SDLNet_UDP_Open(0);

	while (true)
	{
		switch(pxStatus->xStatus)
		{
		case statusDiscover:
			// Send announcement messages with a regular interval
			for (Uint16 iPort = AGENT_MIN_PORT; iPort < AGENT_MAX_PORT; iPort++)
			{

			}
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

