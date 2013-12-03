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
	typeAnnounce	= 0,
	typeDataStruct	= 1
} eMsgTypes;

typedef struct
{
	Uint16	msgType;
	Uint16	spare;
	Uint32	checksum;
} sHeader;

typedef struct
{
	Uint8	name[AGENT_NAMES_SIZE];
	Uint16	openPort;
	Uint16	spare;
} sAnnounce;

typedef struct
{
	Uint32	size;
} sDataStruct;

#endif /* PROTOCOLTYPES_H_ */
