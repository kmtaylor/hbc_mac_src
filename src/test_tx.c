/* gcc -DDEBUG -Wall test_tx.c build_tx.c -o test_tx */

#include <stdio.h>
#include <stdint.h>

#include "build_tx.h"

void fifo_write(uint32_t data) {
    printf("%08X\n", data);
}

int main (void) {
    build_tx_preamble();
    return 0;
}
