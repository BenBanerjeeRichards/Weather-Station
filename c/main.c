#include <stdio.h>
#include "ws.h"
#include "station.h"
#include "ws_store.h"
#include "config.h"

int main(int argc, char** args)
{

	ws_device dev;
	station_download_data(&dev);
    return 0; 
	
}