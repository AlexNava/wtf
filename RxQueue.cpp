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
	AgentStatus *pxStatus = (AgentStatus*)pxData;
	Uint16 iPort = AGENT_MIN_PORT;

	UDPpacket *pxPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xSock = 0;

	// Wait for run()
	SDL_SemWait(pxStatus->pxRxGoSemaphore);
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

	SDL_SemPost(pxStatus->pxTxGoSemaphore);

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
				sHeader		*pxHeader = (sHeader *)pxPacket->data;
				sAnnounce	*pxAnnounce = NULL;
				sCommand	*pxCommand = NULL;
				sAnnounceStruct	*pxDataStruct = NULL;

				switch (pxHeader->msgType)
				{
				case msgAnnounce:
					// Update the network map
					pxAnnounce = (sAnnounce *)(pxPacket->data + sizeof(sHeader));
					if (pxStatus->xStatus == statusDiscover)
					{
						printf("Announcement message.\n");
						//pxDataStruct = (sAnnounceStruct *)(pxPacket->data + sizeof(sHeader));

					}
					else
					{
						// Announce messages are not supposed to arrive
						printf("Unexpected announcement message.\n");
					}
					break;
				case msgCommand:
					// Update the state machine
					pxCommand = (sCommand *)(pxPacket->data + sizeof(sHeader));
					printf("Command message %d.\n", pxCommand->cmdType);

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

