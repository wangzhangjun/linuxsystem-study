#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(){

    char filename[80] = "/sys/fs/bpf/cgroup-ingress-traffic-uid";

    if( access( filename, F_OK ) != -1 ) {
        // file exists
        if( remove(filename) == 0 )
            printf("Removed %s.\n", filename);
        else
            perror("remove\n");
    } else {
        // file doesn't exist
        printf("no such %s file.\n", filename);
    }
}

