/*
 * Protocol.cpp
 *
 *  Created on: Dec 7, 2013
 *      Author: alessandro
 */
#include "Protocol.h"

Uint32 calcChecksum(Uint8 *message)
{
	return 0;
}

bool messageValid(Uint8 *message, Uint32 size)
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
