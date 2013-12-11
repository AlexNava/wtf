/*
 * Protocol.cpp
 *
 *  Created on: Dec 7, 2013
 *      Author: alessandro
 */
#include "Protocol.h"

Uint32 calcChecksum(void *message)
{
	// TODO: Adler-32 on the complete message (header + data) with the checksum set to 0
	// 0xffffffff is not a valid checksum, so i'll use it in case of erors

	if (message == NULL)
		return 0xffffffff;

	Uint16 sumA = 1;
	Uint16 sumB = 0;
	Uint8 *currentWord = (Uint8 *)message;
	sHeader *pHeader = (sHeader *)message;
	
	Uint32 size = pHeader->msgSize + sizeof(sHeader);
	if (size > AGENT_MAX_PACKET_SIZE)
		return 0xffffffff;

	size_t checksumOffset = (Uint8 *)&(pHeader->checksum) - (Uint8 *)pHeader;

	for (Uint32 i = 0; i < size; i++)
	{
	}

	return (sumB << 16) | sumA;
}

bool messageValid(void *message, Uint32 size)
{
	if (message == NULL)
		return false;

	sHeader *pxHeader = (sHeader *)message;

	if ((pxHeader->msgType != msgAnnounce)
		&& (pxHeader->msgType != msgCommand)
		&& (pxHeader->msgType != msgDataStruct))
		return false;

	if ((pxHeader->msgSize + sizeof(sHeader)) != size)
		return false;

	if (pxHeader->checksum != calcChecksum(message))
		return false;

	return true;
}
