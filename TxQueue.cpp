/*
 * TxQueue.h
 *
 *  Created on: Dec 5, 2013
 *      Author: alessandro
 */

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"

#include "Agent.h"

int txQueueFunc(void *pxData)
{
	AgentStatus *pxStatus = (AgentStatus*)pxData;

	UDPpacket *pxDiscoverPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xDiscoverSock = SDLNet_UDP_Open(0);

	// Wait for RX queue
	SDL_SemWait(pxStatus->pxTxGoSemaphore);
	printf("TX queue started.\n");

	while (true)
	{
		switch(pxStatus->xStatus)
		{
		case statusDiscover:
			{
				// Send announcement messages with a regular interval

				sHeader *pHeader = (sHeader *)pxDiscoverPacket->data;
				pHeader->msgType = msgAnnounce;
				pHeader->spare = 0;

				sAnnounce *pAnnounce = (sAnnounce *)(pxDiscoverPacket->data + sizeof(sHeader));
				memset((char *)(pAnnounce->name), '\0', AGENT_NAME_SIZE);
				memset((char *)(pAnnounce->familyName), '\0', AGENT_NAME_SIZE);
				strncpy((char *)(pAnnounce->name), pxStatus->strName.c_str(), AGENT_NAME_SIZE);
				strncpy((char *)(pAnnounce->familyName), pxStatus->strFamName.c_str(), AGENT_NAME_SIZE);
				pAnnounce->listeningPort = pxStatus->listeningPort;
				pAnnounce->spare = 0;

				Uint32 *structCounter = (Uint32 *)(pxDiscoverPacket->data + sizeof(sHeader) + sizeof (sAnnounce));
				*structCounter = pxStatus->xStructures.size();
				sAnnounceStruct *structArray = (sAnnounceStruct *)(pxDiscoverPacket->data + sizeof(sHeader) + sizeof (sAnnounce) + sizeof(Uint32));
				pHeader->msgSize = sizeof(sHeader) + sizeof (sAnnounce) + sizeof(Uint32) + *structCounter * sizeof(sAnnounceStruct);

				for (map<string, sStructInfo>::iterator it = pxStatus->xStructures.begin(); it != pxStatus->xStructures.end(); it++)
				{
					structArray->id = it->second.id;
					structArray->direction = it->second.eDirection;
					structArray->period = it->second.period;
					memset((char *)(structArray->name), '\0', STRUCT_NAME_SIZE);
					strncpy((char *)(structArray->name), it->first.c_str(), STRUCT_NAME_SIZE);
					structArray++;
				}

				pHeader->checksum = calcChecksum(pHeader);

				for (Uint16 iPort = AGENT_MIN_PORT; iPort < AGENT_MAX_PORT; iPort++)
				{
					int res = SDLNet_ResolveHost(&(pxDiscoverPacket->address), "255.255.255.255", iPort);
					pxDiscoverPacket->len = pHeader->msgSize + sizeof(sHeader);
					res = SDLNet_UDP_Send(xDiscoverSock, -1, pxDiscoverPacket);
					SDL_Delay(5);
				}
			}
			break;
		case statusRun:
			// Wait for the semaphore (tx queue follows rx one)
			SDL_SemWait(pxStatus->pxSendSemaphore);
			if (pxStatus->xStatus != statusRun)	// It can be changed by rx thread
				break;
			// Send all structures

			break;
		}
	}

	return 0;	// never
}

