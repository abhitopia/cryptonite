#include "cli/app.h"

int main(int argc, char **argv) {
    CryptoniteApp app{};
    return app.run(argc,argv);
}

