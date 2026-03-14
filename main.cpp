#include "gameapp.hpp"
#include "scene.hpp"

int main(int argc, char *argv[])
{
    Screen scr = Screen();
    char n[32] = "cock novel";
    Scene scene;

    abool run = true;

    const char *env = getenv("CCNVL_FILE");

    scr.init_();

    if (env)
    {
        strncpy(n, env, sizeof(n) - 1);
        n[sizeof(n) - 1] = 0;
    }
    else
    {
        strcpy(n, "scene.bin");
    }

    scr.load_(n);
    scr.change_scene("main");

    scr.run(run);

    scr.clean();

    return 0;
}