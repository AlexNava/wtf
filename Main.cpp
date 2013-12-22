// Test executable, provided only as an example

#include "Agent.h"

Uint8 cazzobuff[3];

void stepFunctionA()
{
	cazzobuff[0]++;
	cazzobuff[1] += 2;
	cazzobuff[2] += 3;
	printf("Step callback!\n");
}

void resetFunctionA()
{
	cazzobuff[0] = 1;
	cazzobuff[1] = 2;
	cazzobuff[2] = 3;
	printf("Reset callback!\n");
}

void stepFunctionB()
{
	printf("Step callback!\n");
	printf("Cazzobuff: %d %d %d\n", cazzobuff[0], cazzobuff[1], cazzobuff[2]);
}

void resetFunctionB()
{
	printf("Reset callback!\n");
}

int main(int argc, char **argv)
{
	bool sender = false;

	Agent agent;
	int initError = 0;

	if (!sender)
	{
		initError = agent.init("BBB");	// Never returns except for errors
		printf ("Init returned %d.\n", initError);

		agent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataIn);
		agent.setStepCallback(&stepFunctionB);
		agent.setResetCallback(&resetFunctionB);
		agent.run();
	}
	else
	{
		initError = agent.init("AAA");	// Never returns except for errors
		printf ("Init returned %d.\n", initError);

		agent.addStruct("cazzo", cazzobuff, sizeof(cazzobuff), dataOut);
		agent.setStepCallback(&stepFunctionA);
		agent.setResetCallback(&resetFunctionA);
		agent.run();
	}
	return initError;
}
