/*
 * AgentsTypes.h
 *
 *  Created on: Nov 23, 2013
 *      Author: alessandro
 */

#ifndef PROTOCOLTYPES_H_
#define PROTOCOLTYPES_H_

#include "SDL2/SDL.h"

#define AGENT_NAME_SIZE			32
#define STRUCT_NAME_SIZE		32
#define AGENT_MAX_PACKET_SIZE	1024
#define AGENT_MIN_PORT			2013
#define AGENT_MAX_PORT			2113

Uint32 calcChecksum(void *message);
bool messageValid(void *message, Uint32 size);

typedef enum
{
	msgAnnounce		= 0,
	msgCommand		= 1,
	msgDataStruct	= 2
} eMsgTypes;

typedef enum
{
	dataOut		= 0,
	dataIn		= 1
} eDataDirection;

typedef enum
{
	cmdReset	= 0,
	cmdPlay		= 1
} eCmdTypes;

typedef struct
{
	Uint16	msgType;
	Uint16	spare;
	Uint32	msgSize;
	Uint32	checksum;
} sHeader;

typedef struct
{
	char	name[AGENT_NAME_SIZE];
	char	familyName[AGENT_NAME_SIZE];
	Uint16	listeningPort;
	Uint16	spare;
	Uint32	numStructures;
} sAnnounce;

typedef struct
{
	char	name[STRUCT_NAME_SIZE];
	Uint32	size;
	Uint8	direction;
	Uint8	reserved;
	Uint16	id;		// Used for input structures
	Uint32	period;	// Used for input structures
} sAnnounceStruct;

typedef struct
{
	Uint16	id;
	Uint16	reserved;
	Uint32	time;	// Ticks
} sDataStruct;

typedef struct
{
	Uint16	cmdType;
	Uint16	cmdFlags;
	Uint32	cmdData;
} sCommand;


#endif /* PROTOCOLTYPES_H_ */
