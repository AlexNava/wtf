/*
 * RxQueue.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"

#include "RxQueue.h"
#include "AgentsTypes.h"

#define STARTPORT 2013
#define ENDPORT 3013

struct AgentStatus;

int rxQueueFunc(void *pxData)
{
	AgentStatus *pxStatus = (AgentStatus*)pxData;
	Uint16 iPort = STARTPORT;

	UDPpacket *pxPacket = SDLNet_AllocPacket(1024);
	UDPsocket xSock = 0;

	while ((xSock == 0) && (iPort < ENDPORT))
	{
		printf("Trying port %d\n", iPort);
		xSock = SDLNet_UDP_Open(iPort);
		if (xSock <= 0)
			iPort++;
	}

	printf("Listening on port %d\n", iPort);
	SDLNet_SocketSet xSockSet;
	xSockSet = SDLNet_AllocSocketSet(1);
	SDLNet_UDP_AddSocket(xSockSet, xSock);

	while (true)
	{
		int iPendingSockets = SDLNet_CheckSockets(xSockSet, 5000);

		if (iPendingSockets <= 0)	// Timeout or error
		{
			printf("No activity on socket\n");
		}
		else
		{
			printf("Activity on %d sockets\n, ", iPendingSockets);

			int iReceivedPackets = SDLNet_UDP_Recv(xSock, pxPacket);
			printf("received 1 packet, size: %d bytes\n", pxPacket->len);
		}
	}

	return 0;	// never
}

