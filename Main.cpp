#include "Protocol.h"
#include "Agent.h"

void stepFunction()
{
	printf("Step callback!\n");
}

int main(int argc, char **argv)
{
	bool sender = false;

	Agent agent;
	int initError = 0;
	Uint8 cazzobuff[3];

	if (!sender)
	{
		initError = agent.init("Test");	// Never returns except for errors
		printf ("Init returned %d.\n", initError);

		agent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataIn);
		agent.setStepCallback(&stepFunction);
		agent.run();
	}
	else
	{
		initError = agent.init("YOLO");	// Never returns except for errors
		printf ("Init returned %d.\n", initError);

		agent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataOut);
		agent.setStepCallback(&stepFunction);
		agent.run();
	}
	return initError;
}
