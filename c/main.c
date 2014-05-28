#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "ws.h" 

int main(int argc, char** args)
{		
	ws_device dev;
	ws_init(&dev);
	ws_initialise_read(&dev);
	
	
	ws_close(&dev);
	

	return 0;
}
