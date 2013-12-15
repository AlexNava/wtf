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
	sAgentStatus *pStatus = (sAgentStatus*)pxData;
	Uint16 iPort = AGENT_MIN_PORT;

	UDPpacket *pxPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xSock = 0;

	// Wait for run()
	SDL_SemWait(pStatus->pRxGoSemaphore);
	printf("RX queue started.\n");

	while ((xSock == 0) && (iPort < AGENT_MAX_PORT))
	{
		printf("Trying port %d\n", iPort);
		xSock = SDLNet_UDP_Open(iPort);
		if (xSock <= 0)
			iPort++;
	}

	printf("Listening on port %d\n", iPort);
	pStatus->listeningPort = iPort;

	SDLNet_SocketSet xSockSet;
	xSockSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(xSockSet, xSock);

	SDL_SemPost(pStatus->pTxGoSemaphore);

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

				switch (pHeader->msgType)
				{
				case msgAnnounce:
					{
						// Update the network map
						sAnnounce	*pAnnounce = (sAnnounce *)(pxPacket->data + sizeof(sHeader));

						if (pStatus->eAutomaState == stateDiscover)
						{
							printf("Announcement message.\n");
							sNeighbor neighbor;
							const char *strIp = SDLNet_ResolveIP(&(pxPacket->address));
							if (strIp == NULL)
							{
								break;
							}
							neighbor.name = pAnnounce->name;
							neighbor.famName = pAnnounce->familyName;
							if (!neighbor.famName.empty()	// Connect with agents of the same family or with no family specified
									&& (neighbor.famName != pStatus->famName))
							{
								break;
							}

							if ((neighbor.name == pStatus->name)	// Don't connect to myself :)
									&& (neighbor.famName == pStatus->famName))
							{
								break;
							}

							SDLNet_ResolveHost(&(neighbor.address), strIp, pAnnounce->listeningPort);

							sAnnounceStruct	*pAnnStruct = NULL;
							for (Uint32 iSt = 0; iSt < pAnnounce->numStructures; iSt++)
							{
								pAnnStruct = (sAnnounceStruct *)(pxPacket->data + sizeof(sHeader) + sizeof(sAnnounce) + iSt * sizeof(sAnnounceStruct));
								if (pAnnStruct->direction == dataOut)
									continue;

								sStructInfo xRemoteStruct;
								string remoteStructName = pAnnStruct->name;
								tStructMapByName::iterator it = pStatus->localStructures.find(remoteStructName);
								if (it == pStatus->localStructures.end())
									continue;	// No local structures with matching name
								if (it->second.eDirection == dataIn)
									continue;	// Local structure is declared as input

								xRemoteStruct.id = pAnnStruct->id;
								xRemoteStruct.period = pAnnStruct->period;
								xRemoteStruct.lastTxTick = 0;
								xRemoteStruct.pData = it->second.pData;	// Pass the local pointer
								xRemoteStruct.size = it->second.size;

								neighbor.structuresToSend.push_back(xRemoteStruct);
							}

							printf("Neighbor %s (%s)\nRegistering input structures: ", neighbor.name.c_str(), neighbor.famName.c_str());
							for (Uint32 iSt = 0; iSt < neighbor.structuresToSend.size(); iSt++)
							{
								printf("%d ", neighbor.structuresToSend[iSt].id);
							}
							printf("\n");

							Sint32 iMatching = -1;
							for (Uint32 iSt = 0; iSt < pStatus->neighbors.size(); iSt++)
							{
								if ((pStatus->neighbors[iSt].name == neighbor.name)
										&& (pStatus->neighbors[iSt].famName == neighbor.famName))
								{
									iMatching = iSt;
									break;
								}
							}
							if (iMatching >= 0)
								pStatus->neighbors[iMatching] = neighbor;	// Update
							else
								pStatus->neighbors.push_back(neighbor);	// Add
						}
						else
						{
							// Announce messages are not supposed to arrive
							printf("Unexpected announcement message.\n");
						}
					}
					break;

				case msgCommand:
					{
						// Update the state machine
						sCommand *pCommand = (sCommand *)(pxPacket->data + sizeof(sHeader));
						printf("Command message %d.\n", pCommand->cmdType);
						switch (pCommand->cmdType)
						{
						case cmdReset:
							pStatus->eAutomaState = stateSetup;
							pStatus->lastTick = pCommand->cmdData;
							pStatus->currentTick = pCommand->cmdData;

							for (Uint32 iNb = 0; iNb < pStatus->neighbors.size(); iNb++)
							{
								for (Uint32 iSt = 0; iSt < pStatus->neighbors[iNb].structuresToSend.size(); iSt++)
								{
									pStatus->neighbors[iNb].structuresToSend[iSt].lastTxTick = pCommand->cmdData;
								}
							}
							break;
						case cmdPlay:
							{
								pStatus->eAutomaState = stateRun;
								Sint32 tickDelta = pStatus->currentTick - pCommand->cmdData;
								if (tickDelta > 0)
									for (int iTick = 0; iTick < tickDelta; iTick++)
									{
										pStatus->lastTick = pStatus->currentTick;
										pStatus->currentTick++;
										// Exec step callback (it will then release the TX queue)
										SDL_SemPost(pStatus->pStepSemaphore);
									}
								else if (tickDelta < 0)
									for (int iTick = 0; iTick > tickDelta; iTick--)
									{
										pStatus->lastTick = pStatus->currentTick;
										pStatus->currentTick--;
										// Exec step callback (it will then release the TX queue)
										SDL_SemPost(pStatus->pStepSemaphore);
									}
							}
							break;
						}
					}
					break;

				case msgDataStruct:
					{
						sDataStruct	*pDataStruct = (sDataStruct *)(pxPacket->data + sizeof(sHeader));
						sStructInfo *pLocalStruct = NULL;
						if (pDataStruct->id < pStatus->localStructures.size())
							pLocalStruct = &(pStatus->localStructuresById.at(pDataStruct->id));

						if (pLocalStruct == NULL)
							break;

						printf("DataStruct message. Struct %d received\n", pDataStruct->id);
						if ((pHeader->msgSize - sizeof(sDataStruct)) != pLocalStruct->size)
						{
							printf("Error: wrong size\n");
							break;
						}

						SDL_LockMutex(pStatus->pInputMutex);
						// Update input structure
						memcpy(pLocalStruct->pData, (pxPacket->data + sizeof(sHeader) + sizeof(sDataStruct)), pLocalStruct->size);
						SDL_UnlockMutex(pStatus->pInputMutex);
					}
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

