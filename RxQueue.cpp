/*
 * RxQueue.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"

#include "Agent.h"

int rxQueueFunc(void *pxData)
{
	sAgentStatus *pxStatus = (sAgentStatus*)pxData;
	Uint16 iPort = AGENT_MIN_PORT;

	UDPpacket *pxPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xSock = 0;

	// Wait for run()
	SDL_SemWait(pxStatus->pRxGoSemaphore);
	printf("RX queue started.\n");

	while ((xSock == 0) && (iPort < AGENT_MAX_PORT))
	{
		printf("Trying port %d\n", iPort);
		xSock = SDLNet_UDP_Open(iPort);
		if (xSock <= 0)
			iPort++;
	}

	printf("Listening on port %d\n", iPort);
	pxStatus->listeningPort = iPort;

	SDLNet_SocketSet xSockSet;
	xSockSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(xSockSet, xSock);

	SDL_SemPost(pxStatus->pTxGoSemaphore);

	while (true)
	{
		// Endless loop.
		// Here the state machine is updated according to received packets.

		int iPendingSockets = SDLNet_CheckSockets(xSockSet, 5000);

		if (iPendingSockets <= 0)	// Timeout or error
		{
			printf("No activity on socket\n");
		}
		else
		{
			printf("Activity on %d sockets\n", iPendingSockets);

			int iReceivedPackets = SDLNet_UDP_Recv(xSock, pxPacket);
			printf("Received 1 packet, size: %d bytes\n", pxPacket->len);

			Uint32 source = pxPacket->address.host;
			printf("Received msg from %d.%d.%d.%d - ",
					source & 0x000000ff,
					(source & 0x0000ff00) >> 8,
					(source & 0x00ff0000) >> 16,
					source >> 24);

			if (messageValid(pxPacket->data, pxPacket->len))
			{
				sHeader		*pHeader = (sHeader *)pxPacket->data;
				sAnnounce	*pAnnounce = NULL;
				sCommand	*pCommand = NULL;
				sAnnounceStruct	*pDataStruct = NULL;

				switch (pHeader->msgType)
				{
				case msgAnnounce:
					// Update the network map
					pAnnounce = (sAnnounce *)(pxPacket->data + sizeof(sHeader));
					if (pxStatus->eAutomaState == stateDiscover)
					{
						printf("Announcement message.\n");
						sNeighbor xNeighbor;
						const char *strIp = SDLNet_ResolveIP(&(pxPacket->address));
						if (strIp == NULL)
						{
							break;
						}
						xNeighbor.name = pAnnounce->name;
						xNeighbor.famName = pAnnounce->familyName;
						if (!xNeighbor.famName.empty()	// connect with agents of the same family or with no family specified
								&& (xNeighbor.famName != pxStatus->famName))
						{
							break;
						}

						SDLNet_ResolveHost(&(xNeighbor.address), strIp, pAnnounce->listeningPort);

						for (int iSt = 0; iSt < pAnnounce->numStructures; iSt++)
						{
							pDataStruct = (sAnnounceStruct *)(pxPacket->data + sizeof(sHeader) + sizeof(sAnnounce) + iSt * sizeof(sAnnounceStruct));
							if (pDataStruct->direction == dataOut)
								continue;

							sStructInfo xRemoteStruct;
							string remoteStructName = pDataStruct->name;
							tStructMap::iterator it = pxStatus->localStructures.find(remoteStructName);
							if (it == pxStatus->localStructures.end())
								continue;	// No local structures with matching name
							if (it->second.eDirection == dataIn)
								continue;	// Local structure is declared as input

							xRemoteStruct.id = pDataStruct->id;
							xRemoteStruct.period = pDataStruct->period;
							xRemoteStruct.pData = it->second.pData;	// Pass the local pointer
							xRemoteStruct.size = it->second.size;

							xNeighbor.structuresToSend.push_back(xRemoteStruct);
						}

						printf("Added neighbor %s (%s)\nWith input structures: ", xNeighbor.name.c_str(), xNeighbor.famName.c_str());
						for (int iSt = 0; iSt < xNeighbor.structuresToSend.size(); iSt++)
						{
							printf("%d ", xNeighbor.structuresToSend[iSt].id);
						}
						printf("\n");
						pxStatus->neighbors.push_back(xNeighbor);
					}
					else
					{
						// Announce messages are not supposed to arrive
						printf("Unexpected announcement message.\n");
					}
					break;

				case msgCommand:
					{
						// Update the state machine
						pCommand = (sCommand *)(pxPacket->data + sizeof(sHeader));
						printf("Command message %d.\n", pCommand->cmdType);
						switch (pCommand->cmdType)
						{
						case cmdReset:
							pxStatus->eAutomaState = stateSetup;
							pxStatus->lastTick = pCommand->cmdData;
							pxStatus->currentTick = pCommand->cmdData;
							break;
						case cmdPlay:
							pxStatus->eAutomaState = stateRun;
							pxStatus->lastTick = pxStatus->currentTick;
							pxStatus->currentTick = pCommand->cmdData;
							break;
						}

						// Release TX queue
						SDL_SemPost(pxStatus->pSendSemaphore);
						// Exec step callback
						SDL_SemPost(pxStatus->pStepSemaphore);
					}
					break;

				case msgDataStruct:
					// Update input structures
					printf("DataStruct message.\n");

					break;
				}

			}
			else
			{
				printf("Invalid message.\n");
			}
		}
	}

	return 0;	// never
}

