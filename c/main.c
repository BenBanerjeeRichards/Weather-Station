#include <stdio.h>
#include "ws.h" 

int main(int argc, char** args)
{		
	ws_device dev;
	ws_init(&dev);
	ws_initialise_read(&dev);

	unsigned char data[256];
	int read;
	ws_read_fixed_block_data(&dev, data, &read);

	printf("\n");

	ws_close(&dev);  
	return 0;
}
