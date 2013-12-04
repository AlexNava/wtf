/*
 * AgentsTypes.h
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#ifndef PROTOCOLTYPES_H_
#define PROTOCOLTYPES_H_

#include "SDL2/SDL.h"

#define AGENT_NAMES_SIZE	32

typedef enum
{
	msgAnnounce		= 0,
	msgCommand		= 1,
	msgDataStruct	= 2
} eMsgTypes;

typedef enum
{
	cmdReset	= 0,
	cmdPlay		= 1
} eCmdTypes;

typedef struct
{
	Uint16	msgType;
	Uint16	spare;
	Uint32	checksum;
} sHeader;

typedef struct
{
	Uint8	name[AGENT_NAMES_SIZE];
	Uint16	listeningPort;
	Uint16	spare;
} sAnnounce;

typedef struct
{
} sConfig;

typedef struct
{
	Uint16	cmdType;
	Uint16	cmdFlags;
	Uint32	cmdData;
} sCommand;

typedef struct
{
	Uint32	size;
} sDataStruct;

#endif /* PROTOCOLTYPES_H_ */
