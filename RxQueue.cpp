/*
 * RxQueue.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_net.h"

#include "RxQueue.h"
#include "Agent.h"
#include "Protocol.h"

int rxQueueFunc(void *pxData)
{
	AgentStatus *pxStatus = (AgentStatus*)pxData;
	Uint16 iPort = AGENT_MIN_PORT;

	UDPpacket *pxPacket = SDLNet_AllocPacket(AGENT_MAX_PACKET_SIZE);
	UDPsocket xSock = 0;

	while ((xSock == 0) && (iPort < AGENT_MAX_PORT))
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
		// Endless loop.
		// Here the state machine is updated according to received packets.

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

