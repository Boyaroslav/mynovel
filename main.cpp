#include "gameapp.hpp"
#include "scene.hpp"


int main(int argc, char* argv[]) {
    Screen scr = Screen();
    Scene scene = Scene("main");

    abool run = true;

    scr.init_();

    scr.load_("cock");

    scr.run(run);

    scr.clean();

    return 0;
}