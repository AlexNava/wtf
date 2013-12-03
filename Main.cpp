#include "Agent.h"

int main(int argc, char **argv)
{
	Agent xAgent;
	int iInitError = xAgent.init("Test");	// Never returns except for errors
	printf ("Init returned %d.\n", iInitError);
	xAgent.run();
	return iInitError;
}
