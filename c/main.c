#include <stdio.h>
#include "ws.h"
#include "station.h"

int main(int argc, char** args)
{
    ws_device dev;
    int status = WS_SUCCESS;
    status = station_download_data(&dev);
    if (status != WS_SUCCESS)
    {
        printf("error\n");
    }
    return 0;
}