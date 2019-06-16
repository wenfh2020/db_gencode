#include <string.h>
#include <algorithm>

#include "file.h"
#include "auto_table.h"

int main(int argc, char* argv[]) {
    if (2 != argc) {
        printf("Invalid argument.\n 1:sql file\n");
        return -1;
    }

    AutoTable oTable;
    oTable.Analysis(argv[1]);

    return 0;
}
