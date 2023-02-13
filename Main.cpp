#include "Engine.hpp"

int main(int argc, char **argv) {
    Engine engine;

    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}
