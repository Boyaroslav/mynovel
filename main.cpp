#include "gameapp.hpp"
#include "scene.hpp"


int main(int argc, char* argv[]) {
    Screen scr = Screen();
    char n[32] = "cock novel";
    Scene scene;

    abool run = true;

    scr.init_();
    strcpy(n, "scene.bin");

    scr.load_(n);

    scr.run(run);

    scr.clean();

    return 0;
}