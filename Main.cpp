#include "Protocol.h"
#include "Agent.h"

int main(int argc, char **argv)
{
	Agent xAgent;
	int iInitError = xAgent.init("Test");	// Never returns except for errors
	printf ("Init returned %d.\n", iInitError);

	Uint8 cazzobuff[100];
	xAgent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataIn);
	xAgent.addStruct("culo", NULL, 123, dataIn);
	xAgent.addStruct("cazzo", NULL, 0, dataIn);
	xAgent.run();
	return iInitError;
}
