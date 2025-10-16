#include "src/app/App.h"

int main() {
    try {
        App app(1280, 720, "fbxViewer");
        app.run();
    } catch (const std::exception& e) {
        fprintf(stderr, "Fatal: %s\n", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}