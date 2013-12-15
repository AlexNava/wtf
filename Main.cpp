#include "Protocol.h"
#include "Agent.h"

void stepFunction()
{
	printf("Step callback!\n");
}

int main(int argc, char **argv)
{
	Agent agent;
	int initError = agent.init("Test");	// Never returns except for errors
	printf ("Init returned %d.\n", initError);

	Uint8 cazzobuff[100];
	agent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataIn);
	agent.addStruct("culo", NULL, 123, dataIn);
	agent.addStruct("cazzo", NULL, 0, dataIn);
	agent.setStepCallback(&stepFunction);
	agent.run();
	return initError;
}
