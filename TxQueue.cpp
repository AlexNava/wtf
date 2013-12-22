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

int txQueueFunc(void *pData)
{
	sAgentStatus *pStatus = (sAgentStatus*)pData;

	UDPpacket *pDiscoverPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket discoverSock = SDLNet_UDP_Open(0);

	UDPpacket *pStructPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket structSock = SDLNet_UDP_Open(0);

	// Wait for RX queue
	SDL_SemWait(pStatus->pTxGoSemaphore);
	printf("TX queue started.\n");

	while (true)
	{
		switch(pStatus->eAutomaState)
		{
		case stateDiscover:
			{
				// Send announcement messages with a regular interval

				sHeader *pHeader = (sHeader *)pDiscoverPacket->data;
				pHeader->msgType = msgAnnounce;
				pHeader->spare = 0;

				sAnnounce *pAnnounce = (sAnnounce *)(pDiscoverPacket->data + sizeof(sHeader));
				memset((char *)(pAnnounce->name), '\0', AGENT_NAME_SIZE);
				memset((char *)(pAnnounce->familyName), '\0', AGENT_NAME_SIZE);
				strncpy((char *)(pAnnounce->name), pStatus->name.c_str(), AGENT_NAME_SIZE);
				strncpy((char *)(pAnnounce->familyName), pStatus->famName.c_str(), AGENT_NAME_SIZE);
				pAnnounce->listeningPort = pStatus->listeningPort;
				pAnnounce->spare = 0;
				pAnnounce->numStructures = pStatus->localStructures.size();

				sAnnounceStruct *structArray = (sAnnounceStruct *)(pDiscoverPacket->data + sizeof(sHeader) + sizeof (sAnnounce));
				pHeader->msgSize = sizeof(sHeader) + sizeof (sAnnounce) + pAnnounce->numStructures * sizeof(sAnnounceStruct);

				for (tStructMapByName::iterator it = pStatus->localStructures.begin(); it != pStatus->localStructures.end(); it++)
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
					int res = SDLNet_ResolveHost(&(pDiscoverPacket->address), "255.255.255.255", iPort);
					pDiscoverPacket->len = pHeader->msgSize + sizeof(sHeader);
					res = SDLNet_UDP_Send(discoverSock, -1, pDiscoverPacket);
					SDL_Delay(5);
				}
			}
			break;
		case stateRun:
			{
				// Wait for the semaphore (tx queue follows rx one)
				SDL_SemWait(pStatus->pSendSemaphore);
				if (pStatus->eAutomaState != stateRun)	// It can be changed by rx thread
					break;

				SDL_LockMutex(pStatus->pOutputMutex);
				// Send all structures
				sHeader *pHeader = (sHeader *)pStructPacket->data;
				pHeader->msgType = msgDataStruct;
				pHeader->spare = 0;

				sDataStruct *pStruct = (sDataStruct *)(pStructPacket->data + sizeof(sHeader));
				pStruct->reserved = 0;
				pStruct->time = pStatus->currentTick;

				for (Uint32 iNb = 0; iNb < pStatus->neighbors.size(); iNb++)
				{
					pStructPacket->address = pStatus->neighbors[iNb].address;

					for (Uint32 iSt = 0; iSt < pStatus->neighbors[iNb].structuresToSend.size(); iSt++)
					{
						pStruct->id = pStatus->neighbors[iNb].structuresToSend[iSt].id;
						memcpy((pStructPacket->data + sizeof(sHeader) + sizeof(sDataStruct)),
								pStatus->neighbors[iNb].structuresToSend[iSt].pFutureData,
								pStatus->neighbors[iNb].structuresToSend[iSt].size);

						pHeader->msgSize = sizeof(sDataStruct) + pStatus->neighbors[iNb].structuresToSend[iSt].size;
						pHeader->checksum = calcChecksum(pHeader);

						pStructPacket->len = pHeader->msgSize + sizeof(sHeader);
						int res = SDLNet_UDP_Send(structSock, -1, pStructPacket);
						pStatus->neighbors[iNb].structuresToSend[iSt].lastTxTick = pStatus->currentTick;
					}
				}
				SDL_UnlockMutex(pStatus->pOutputMutex);
			}
			break;
		}
	}

	return 0;	// never
}

