#include <stdio.h>

#ifdef WIN32
#include "win_scan.h"
#else
#include "linux_scan.h"
#endif

int main( int argc, char *argv[] )
{
    printf( "Scanning your network to find AE400 ...\n(this will take 20~30 secs)\n\n" );
    show_AE400_info();
    return 0;
}
