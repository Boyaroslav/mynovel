#pragma once

#include "utils.hpp"
#include "vars.hpp"
#include "loader.hpp"
#include "textbox.cpp"
#include "sprite.hpp"

struct callframe{
    Scene* s;
    int ret_pos;
};

class Screen
{
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int px, py;
    int event_pool_position = 0;
    uint32_t last_time;
    TextBox textbox;
    Scene* current_scene = nullptr;
    int event_pool_position_buffer = 0;
    int function_commands_left = 0; // когда вызвается функция CALL [func] то на scene->event_count количество эвентов мы берем команды с пула со сдвигом функции, потом возвращаем каретку обратно

    Sprite bg = Sprite();

    std::vector<Sprite> sprites; // персонажы и какие нибудь предметы хз
    bool NEED_MORE_EVENTS = false;
    float bg_alpha = 1.0f;
    float bg_speed = get_value("VAR_BG_CHANGE_SPEED");
    std::string current_bg_path;

    bool running = true;

public:
    SDL_Event e{};

    Screen() = default;
    std::vector<Scene> scenes;
    int scenes_number = 0;
    Event* current_event = nullptr;


    bool init_()
    {
        // SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
            {
                log("SDL_Init error");
                return false;
            }

        // SDL_image
        if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
            {
                log("IMG_Init error");
                return false;
            }

        // SDL_ttf
        if (TTF_Init() == -1)
            {
                log("TTF_Init error");
                return false;
            }

        // SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
            {
                log("Mix_OpenAudio error");
                return false;
            }

        window = SDL_CreateWindow(
                     "lcnovel",
                     SDL_WINDOWPOS_CENTERED,
                     SDL_WINDOWPOS_CENTERED,
                     width,
                     height,
                     SDL_WINDOW_SHOWN
                 );

        if (!window)
            {
                log("SDL_CreateWindow error");
                return false;
            }

        renderer = SDL_CreateRenderer(
                       window,
                       -1,
                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
                   );
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


        if (!renderer)
            {
                log("SDL_CreateRenderer error");
                return false;
            }

        main_font = Font();
        main_font.load();
        vars_init();
        SDL_SetWindowTitle(window, (const char*)get_value("WINDOW_TITLE"));

        textbox = TextBox();
        // bg.load_texture(renderer, "picture.png");

        return true;
    }

    void load_(char* name)
    {
        load_file(name, scenes, &scenes_number);
    }

    void nextEvent(){
        if (event_pool_position >= epos && running)
        {
            clean();
            exit(0);
            return;
        }
        std::cout << function_commands_left << " - fcl \n";

        // 1. Получаем событие из текущей позиции (оно станет текущим для обработки)
        current_event = get_from_epool(event_pool_position);

        // 2. Обрабатываем логику функции и определяем следующую позицию
        if (function_commands_left > 0)
        {
            function_commands_left--;
            if (function_commands_left == 0)
            {
                // Это было последнее событие функции – следующее берём из точки возврата
                event_pool_position = event_pool_position_buffer;
            }
            else
            {
                // Ещё есть команды в функции – сдвигаемся на следующую внутри неё
                event_pool_position++;
            }
        }
        else
        {
            // Обычный режим (не внутри функции)
            if (current_event->id != 14) // для CALL позицию не сдвигаем
                event_pool_position++;
        }
    }
    bool change_scene(const char* scene_name)
    {
        int index = find_scene_index_by_name(scenes, scene_name);
        if (index < 0)
            {
                printf("Scene '%s' not found!\n", scene_name);
                return false;
            }

        Scene& sc = scenes[index];
        current_scene = &sc;

        event_pool_position = sc.event_start;

        return true;
    }


    void handleEvent(bool isnext_needed=true)
    {

        switch(current_event->id)
            {

            case 1:   // TXT
            {
                earg* a = &apool[current_event->args_offset];
                const char* txt = get_from_spool(a->value);
                std::cout << "[TXT] " << txt << "\n";
                textbox.addMessage(
                    std::string(txt)
                );
            }
            break;

            case 2:   // ROW
            {
                earg* a = &apool[current_event->args_offset];
                const char* row = get_from_spool(a->value);
                int n = a->value;
                for(int i=0; i<n; i++)
                    {
                        nextEvent();
                        handleEvent(false);
                    }
            }
            break;

            case 3:   // BG
            {
                earg* a = &apool[current_event->args_offset];
                const char* bgname = get_from_spool(a->value);
                bg.load_texture(renderer, bgname);
                bg.set_texture_change_speed((float)get_value("VAR_BG_CHANGE_SPEED"));
                bg.set_rect(0, 0, width, height);
                bg.start_transition(bg.get_last());
            }
            break;

            case 4:   // LD
            {
                earg* a = &apool[current_event->args_offset];
                const char* file = get_from_spool(a->value);
                // todo - сделать файл sprite.pak
                // где будут храниться все файлы + их имена
                // после этого сделать в спрайте чтобы он искал текстуру там по имени файла
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderTarget(renderer, nullptr);
                log("LD: " + std::string(file));
                sprites.emplace_back(renderer, file);
                std::cout<<sprites.size()<<"\n";
                log(sprites[sprites.size()-1].get_rect());
            }
            break;

            case 5:   // LID
            {
                earg* a = &apool[current_event->args_offset];
                uint32_t index = a->value;
                std::cout << "[LID] " << index << "\n";
            }
            break;

            case 6:   // ALIAS name value
            {
                const char* n = get_from_spool(apool[current_event->args_offset].value);
                const char* v = get_from_spool(apool[current_event->args_offset+1].value);
                std::cout << "[ALIAS] " << n << "=" << v << "\n";
            }
            break;

            case 7:   // CHSPR who sprite
            {
                const char* n = get_from_spool(apool[current_event->args_offset].value);
                const char* s = get_from_spool(apool[current_event->args_offset+1].value);
                std::cout << "[CHSPR] " << n << " -> " << s << "\n";
            }
            break;

            case 8:   // RET code
            {
                int code = apool[current_event->args_offset].value;
                const char* val = get_from_spool(code);
                if (strcmp(val, "finale") == 0)
                    {
                        running = false;
                        exit(0);
                    }
                change_scene(val);
                NEED_MORE_EVENTS = 1;
            }
            break;

            case 9:   // SET variable value
            {
                const char* var = get_from_spool(apool[current_event->args_offset].value);
                earg& val_arg = apool[current_event->args_offset + 1];

                Var result;
                if (val_arg.type == ARG_INT) // int
                {
                    result = make_var((uint32_t)val_arg.value);
                }
                else if (val_arg.type == ARG_DOUBLE) // float — value хранит биты float
                {
                    float f;
                    memcpy(&f, &val_arg.value, sizeof(float));
                    result = make_var((double)f);
                }
                else if (val_arg.type == ARG_STRING) // string
                {
                    result = make_var(std::string(get_from_spool(val_arg.value)));
                }

                variables[var] = result;
                std::cout << "[SET] " << var << "=" << result.as_string() << "\n";
            }
            break;

            case 10:   // MV x y t
            {
                int id = apool[current_event->args_offset].value; 
                int x = apool[current_event->args_offset + 1].value;
                int y = apool[current_event->args_offset+2].value;
                int t = apool[current_event->args_offset+3].value;
                std::cout << "[MV] x=" << x << " y=" << y << " t=" << t << "\n";
                // t это время за которое надо переместить - todo
                sprites[id].move(x, y);
            }
            break;

            case 11:   // CLTB
            {
                textbox.cl();

            }
            break;

            case 14 : { // CALL
                const char* funcname = get_from_spool(apool[current_event->args_offset].value);

                int index = find_scene_index_by_name(scenes, funcname);
                if (index < 0)
                {
                    printf("[CALL] Function '%s' not found!\n", funcname);
                    break;
                }

                Scene& func = scenes[index];

                // сохраняем позицию, куда вернуться после функции
                event_pool_position_buffer = event_pool_position + 1;

                // сколько команд исполнять
                function_commands_left = func.event_count;

                // переход в функцию
                event_pool_position = func.event_start;

                printf("[CALL] %s (start=%d, count=%d)\n",
                    funcname, func.event_start, func.event_count);
                NEED_MORE_EVENTS = true;

            }
            break;

            }
        if(isnext_needed)nextEvent();
    }

    void handleMouseEvent(const SDL_Event& e)
    {
        if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                px = e.button.x;
                py = e.button.y;
            }

        if (e.type == SDL_MOUSEBUTTONUP)
            {
                int px = e.button.x;
                int py = e.button.y;

                if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        handleEvent();
                    }
                else if (e.button.button == SDL_BUTTON_RIGHT)
                    {
                    }
            }
    }

    void run(abool& run)
    {
        int i=0;
        do
            {
                std::cout<<get_from_spool(i)<<strlen(get_from_spool(i))<<"\n";
                while(*get_from_spool(i) != ENDSTR) i+=1;
                i+=1;

            }
        while(i < spos);
        i=0;
        do
            {
                std::cout<<i<<int(get_from_epool(i)->id)<<"\n";
                i+=1;

            }
        while(i < event_pool_position);
        last_time = SDL_GetTicks();
        list_scenes(scenes);

        nextEvent();
        handleEvent();
        textbox.update_position(width, height);
        while (run && running)
            {

                // events
                while (SDL_PollEvent(&e))
                    {
                        if (e.type == SDL_QUIT)
                            {
                                exit(0);
                            }
                        handleMouseEvent(e);
                        if (e.type == SDL_KEYDOWN){
                            SDL_Keymod mod = SDL_GetModState();
                            if (mod & KMOD_CTRL){
                                if ((e.key.keysym.sym == SDLK_EQUALS && (mod & KMOD_SHIFT)) || e.key.keysym.sym == SDLK_KP_PLUS){
                                    main_font.setSize(main_font.size + 5);

                                }
                                else if (e.key.keysym.sym == SDLK_MINUS || e.key.keysym.sym == SDLK_KP_MINUS){
                                    main_font.setSize(main_font.size - 5);

                                }
                            }
                        }
                    }
                if(NEED_MORE_EVENTS)
                    {
                        NEED_MORE_EVENTS=0;
                        handleEvent();
                    }

                uint32_t current_time = SDL_GetTicks();
                float delta_time = (current_time - last_time) / 1000.0f;
                last_time = current_time;
                textbox.update(delta_time);
                bg.update(delta_time);

                SDL_RenderClear(renderer);
                bg.draw(renderer);
                for_each(sprites.begin(), sprites.end(), [this, delta_time](Sprite& sprite) {sprite.update(delta_time);
                
                    sprite.draw(renderer);});

                textbox.draw(renderer);

                SDL_RenderPresent(renderer);
            }
    }

    void clean()
    {
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();

        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);

        SDL_Quit();
    }

    SDL_Renderer* getRenderer() const
    {
        return renderer;
    }
};
