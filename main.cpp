#include <string.h>

#include <algorithm>

#include "auto_table.h"
#include "file.h"

int main(int argc, char* argv[]) {
    if (2 != argc) {
        printf("Invalid argument.\n 1:sql file\n");
        return -1;
    }

    AutoTable oTable;
    oTable.Analysis(argv[1]);

    return 0;
}
