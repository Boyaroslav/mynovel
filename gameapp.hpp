/*
 * This file is part of CnCn (mynovel).
 * Copyright (C) 2026 Iaroslav Bobylev
 * CnCn (mynovel) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * CnCn (mynovel) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CnCn (mynovel). If not, see <https://www.gnu.org/licenses/>.
 */


#pragma once

#include "menus.hpp"



bool Screen::init_()
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

        #ifdef __ANDROID__
        SDL_DisplayMode dm;
        if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
            real_width = dm.w;
            real_height = dm.h;
            width = real_width;
            height = real_height; 
            LOGI("Android display mode: %dx%d", width, height);
        } else {
            LOGE("SDL_GetCurrentDisplayMode failed: %s", SDL_GetError());
        }
        #endif

        window = SDL_CreateWindow(
            "lcnovel",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            real_width,
            real_height,
            #ifdef __ANDROID__
                SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
            #else
                SDL_WINDOW_SHOWN
            #endif
            );

        if (!window)
        {
            log("SDL_CreateWindow error");
            return false;
        }

        renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | /*SDL_RENDERER_PRESENTVSYNC |*/ SDL_RENDERER_TARGETTEXTURE);

        if (!renderer)
        {
            log("SDL_CreateRenderer error");
            return false;
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


        //SDL_RenderSetLogicalSize(renderer, width, height);
        vars_init();

        main_font = Font();
        main_font.load();

        lua_runtime = LuaRuntime();
        lua_runtime.init();

        interface = make_pause_menu(width, height, this);


        textbox = std::make_unique<TextBox>();
        camera = std::make_unique<Camera>();
        audio = std::make_unique<Audio>();
        camera->init(renderer, width, height);

        SDL_GetRendererInfo(renderer, &textbox->info);

        textbox->run_lua_func = [&](std::string s){ lua_runtime.run_string(s);};
        if (if_its_game == 0){
            textbox->hide();
        }

        SDL_SetWindowTitle(window, (const char *)get_value("WINDOW_TITLE"));

        

        lua_runtime.TXT = [this](const std::string& text)
        {
            textbox->addMessage(text);
            textbox->update_position(width, height);
        };

        lua_runtime.CLEAR = [this]()
        {
            textbox->cl();
        };

        lua_runtime.CLEAR_LAST = [this]()
        {
            textbox->cllast();
        };

        lua_runtime.INPUT = [this](const std::string& text)
        {
            SDL_StartTextInput();
            textbox->IS_INPUT = true;

            textbox->addMessage(text);
            textbox->input_header_size = textbox->get_last()->size();
        };

        lua_runtime.CHSC = [this](const std::string& text)
        {
            change_scene(text.c_str());
            NEED_MORE_EVENTS = 1;
        };

        lua_runtime.LD = [this](std::string & file, int x, int y, int w, int h)
        {
            last_id++;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(renderer, nullptr);
            log("LD: " + std::string(file));
            sprites.emplace_back(renderer, file.c_str(), x, y, w, h);
            std::cout << sprites.size() << "\n";
            log(sprites[sprites.size() - 1].get_rect());
        };
        lua_runtime.MOVE = [this](int i, int x, int y, int t)
        {
            if (i < 0){
                i = sprites.size() + i;
            }

            sprites[i].move(x, y, t);
        };

        lua_runtime.CAMERA_COMMAND = [this](int tx, int ty, double zoom, double angle, int type, double duration){
            camera->add_operation(tx, ty, zoom, angle, type, duration);
        };
        return true;
}

void Screen::hide_interface(){
    interface->hide();
}

void Screen::show_interface(){
    if (!need_to_do){
        need_to_do = [this]{
            interface = make_pause_menu(width, height, this);
            interface->show();
        };
    } 
}

void Screen::open_settings(){
    if (!need_to_do){
        need_to_do = [this]{
            interface = make_settings_menu(width, height, this);
            interface->show();
        };
    }
}

void Screen::start_game(){
    if (!need_to_do){
        need_to_do = [this]{
            interface = make_pause_menu(width, height, this);
            interface->hide();
            set_if_its_game(1);
            audio->reset();
            IS_CCNVL = 0;
            epos = 0;
            apos = 0;
            spos = 0;
            ccnvl_resources.clear();
            ccnvl_scenes.clear();
            load_(file_name.data());
            textbox->show();
            //load_(file_name.data());
            change_scene("main");
        };
    }
}

void Screen::main_menu(){
        if (!need_to_do){
        need_to_do = [this]{
            textbox->cl();
            camera->camera_reset();
            sprites.clear();
            
            bg.clear();
            
            set_if_its_game(0);

            #ifdef __ANDROID__
            LOGI("LCNOVELFILE %s", file_name.c_str());
            #endif
            load_("script.bin");
            textbox->move_position(0,0);
            textbox->hide();
            audio->reset();
            change_scene("menu");
            interface = make_main_menu(width, height, this);
            interface->show();

            
        };
    }
}

void Screen::load_(char *name)
    {
        std::string n(name);
        #ifdef __ANDROID__
        LOGI("load_ %s", name);
        #endif
        scenes.clear();
        
        if (!IS_CCNVL){
            file_name = n;

            load_file(file_name.c_str(), scenes, &scenes_number);
        }
        else{ // это смена LD_FILE!!!!!!!!!! и грузит от скрипт
 
            load_scene_by_name(name, scenes);
            current_script_name = name;
        }
        current_scene_name = "main";
}


void Screen::nextEvent()
{
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
bool Screen::change_scene(const char *scene_name)
{
        int index = find_scene_index_by_name(scenes, scene_name);
        if (index < 0)
        {
            #ifdef __ANDROID__
            LOGI("LCNOVEL Scene not found %s", scene_name);
            #endif
            printf("Scene '%s' not found!\n", scene_name);
            return false;
        }

        current_scene_name = scene_name;
        function_commands_left = 0;
        IN_ROW = 0;
        current_event = nullptr;
        if_result = 1;
        NEED_MORE_EVENTS = 1;
        sprites.clear();
        var_waiting = "";

        Scene &sc = scenes[index];
        current_scene = &sc;

        #ifdef __ANDROID__
        LOGI("change_scene: idx=%d event_start=%d epos=%d", index, sc.event_start, epos);
        #endif

        event_pool_position = sc.event_start;

        return true;
}

void Screen::handleEvent(bool isnext_needed)
{
        if (!if_result &&
            current_event->id != 21 &&
            current_event->id != 22)
        {
            NEED_MORE_EVENTS = 1;
            return;
        }

        if (!current_event)
        {
            log("NULL EVENT");
            running = 0;
            return;
        }
        

        switch (current_event->id)
        {

        case 1: // TXT
        {
            earg *a = &apool[current_event->args_offset];
            const char *txt = get_from_spool(a->value);
            std::cout << "[TXT] " << txt << "\n";
            textbox->addMessage(
                std::string(txt));
            textbox->update_position(width, height);
                
        }
        break;

        case 2: // ROW
        {
            if (!IN_ROW)
            {
                earg *a = &apool[current_event->args_offset];
                const char *row = get_from_spool(a->value);
                int n = a->value;
                IN_ROW = 1;
                int executed = 0;

                while (executed < n)
                {
  
                    if (!lua_runtime.has_blocking_coroutine()){
                    nextEvent();
                    handleEvent(false);
                        
                    // считаем только если не внутри функции
                    if (function_commands_left == 0)
                        executed++;

                    }
                }
                IN_ROW = false;
                //NEED_MORE_EVENTS=1;
                // nextEvent(); handleEvent();
            }
        }
        break;

        case 3: // BG
        {
            earg *a = &apool[current_event->args_offset];
            const char *bgname = get_from_spool(a->value);
            log("BG " + std::string(bgname));
            bg.load_texture(renderer, bgname);
            bg.set_texture_change_speed((float)get_value("VAR_BG_CHANGE_SPEED"));
            bg.set_rect(0, 0, width, height);
            bg.start_transition(bg.get_last());
        }
        break;

        case 4: // LD
        {
            last_id++;
            earg *a = &apool[current_event->args_offset];
            const char *file = get_from_spool(a->value);
            // todo - сделать файл sprite.pak
            // где будут храниться все файлы + их имена
            // после этого сделать в спрайте чтобы он искал текстуру там по имени файла
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(renderer, nullptr);
            log("LD: " + std::string(file));
            sprites.emplace_back(renderer, file);
            std::cout << sprites.size() << "\n";
            log(sprites[sprites.size() - 1].get_rect());
        }
        break;

        case 5: // LID
        {
            earg *a = &apool[current_event->args_offset];
            uint32_t index = a->value;
            std::cout << "[LID] " << index << "\n";
        }
        break;

        case 6: // ALIAS name value
        {
            const char *n = get_from_spool(apool[current_event->args_offset].value);
            const char *v = get_from_spool(apool[current_event->args_offset + 1].value);
            std::cout << "[ALIAS] " << n << "=" << v << "\n";
        }
        break;

        case 7: // CHSPR who sprite
        {
            if (sprites.size() > 0){
            earg *a = &apool[current_event->args_offset];
            uint32_t index;
            if (a->type == ARG_STRING)
            {
                index = std::stoi((interpolate((get_from_spool(a->value)))));
            }
            else
            {
                index = a->value;
            }

            const char *sprite_name = get_from_spool(apool[current_event->args_offset + 1].value);

            sprites[index].load_texture(renderer, sprite_name);
            sprites[index].set_texture_change_speed(get_value("LD_SPEED"));
            sprites[index].start_transition(sprites[index].get_last());
        }
        }
        break;

        case 8: // RET code
        {
            int code = apool[current_event->args_offset].value;
            const char *val = get_from_spool(code);
            log("RET " + std::string(val));
            if (strcmp(val, "finale") == 0)
            {
                running = false;
                exit(0);
                return;
            }
            change_scene(val);
            NEED_MORE_EVENTS = 1;
        }
        break;

        case 9: // SET variable value
        {
            const char *var = get_from_spool(apool[current_event->args_offset].value);
            earg &val_arg = apool[current_event->args_offset + 1];

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

        case 10: // MV x y t
        {
            if(sprites.size() > 0){
            int id = apool[current_event->args_offset].value;
            int x = apool[current_event->args_offset + 1].value;
            int y = apool[current_event->args_offset + 2].value;
            float t;
            if (apool[current_event->args_offset + 3].type == ARG_DOUBLE){
            uint32_t i_will_fix_it = apool[current_event->args_offset + 3].value;
            memcpy(&t, &i_will_fix_it, sizeof(float));
            }
            else{
                t = apool[current_event->args_offset + 3].value;
            }
            std::cout << "[MV] x=" << x << " y=" << y << " t=" << t << "\n";
            // t это время за которое надо переместить - todo
            sprites[id].move(x, y, t);
            }
        }
        break;

        case 11: // CLTB
        {
            textbox->cl();
        }
        break;

        case 13:
        { // LOAD with x y w h
            last_id++;
            earg *a = &apool[current_event->args_offset];
            float ld_speed = get_value("LD_SPEED");
            const char *file = get_from_spool(a->value);
            int x = apool[current_event->args_offset + 1].value;
            int y = apool[current_event->args_offset + 2].value;
            int w = apool[current_event->args_offset + 3].value;
            int h = apool[current_event->args_offset + 4].value;
            // todo - сделать файл sprite.pak - done
            // где будут храниться все файлы + их имена
            // после этого сделать в спрайте чтобы он искал текстуру там по имени файла
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(renderer, nullptr);
            log("LDXYWH: " + std::string(file));
            if (ld_speed == 0)
            {
                sprites.emplace_back(renderer, file, x, y, w, h);
            }
            else
            {
                sprites.emplace_back(renderer, file, x, y, w, h, ld_speed);
            }
            log(sprites[sprites.size() - 1].get_rect());
        }
        break;

        case 14:
        { // CALL
            const char *funcname = get_from_spool(apool[current_event->args_offset].value);

            int index = find_scene_index_by_name(scenes, funcname);
            if (index < 0)
            {
                printf("[CALL] Function '%s' not found!\n", funcname);
                break;
            }

            Scene &func = scenes[index];

            // сохраняем позицию, куда вернуться после функции
            event_pool_position_buffer = event_pool_position + 1;

            // сколько команд исполнять
            function_commands_left = func.event_count;

            // переход в функцию
            event_pool_position = func.event_start;

            printf("[CALL] %s (start=%d, count=%d)\n",
                   funcname, func.event_start, func.event_count);
            if (!IN_ROW)
                NEED_MORE_EVENTS = true;
        }
        break;
        case 15: // OPERATION var op val
        {
            const char *var = get_from_spool(apool[current_event->args_offset].value);
            uint8_t op = apool[current_event->args_offset + 1].value; // 0=+ 1=- 2=* 3=/
            earg &val_arg = apool[current_event->args_offset + 2];

            Var cur = get_value(var);
            Var val = val_arg.type == 2
                          ? make_var(std::string(get_from_spool(val_arg.value)))
                          : make_var((uint32_t)val_arg.value);

            if (cur.is_int() && val.is_int())
            {
                uint32_t a = cur.as_int(), b = val.as_int();
                uint32_t res = (op == 0) ? a + b : (op == 1) ? a - b
                                               : (op == 2)   ? a * b
                                                             : (b ? a / b : 0);
                variables[var] = make_var(res);
            }
            else
            {
                double a = cur.is_float() ? cur.as_float() : cur.as_int();
                double b = val.is_float() ? val.as_float() : val.as_int();
                double res = (op == 0) ? a + b : (op == 1) ? a - b
                                             : (op == 2)   ? a * b
                                                           : (b ? a / b : 0.0);
                variables[var] = make_var(res);
            }

            std::cout << "[OPERATION] " << var << " op=" << (int)op << "\n";
        }
        break;
        case 16: // TBCAP - change footer
        {
            std::string t = get_from_spool(apool[current_event->args_offset].value);
            textbox->set_footer(renderer, t);
        }
        break;
        case 17: // WAIT
        {
            float t = apool[current_event->args_offset].value / 1000.0f;
            log("[WAIT] " + std::to_string(t) + "s");
            wait_timer = t;
            WAITING = true;
            isnext_needed = false;
            while (WAITING)
            {
                while (SDL_PollEvent(&e))
                    if (e.type == SDL_QUIT)
                        exit(0);
                update_and_render();
            }
        }
        break;

        case 18: // IF
        {
            earg &f = apool[current_event->args_offset];
            std::string op = get_from_spool(apool[current_event->args_offset + 1].value);
            earg &s = apool[current_event->args_offset + 2];

            Var left, right;

            if (f.type == ARG_STRING)
                left = get_value(get_from_spool(f.value)); // имя переменной → её значение
            else
                left = make_var((uint32_t)f.value);

            if (s.type == ARG_STRING)
                right = get_value(get_from_spool(s.value));
            else
                right = make_var((uint32_t)s.value);

            // сравниваем как строки если хоть один строковый
            if (left.is_string() || right.is_string())
            {
                std::string l = left.as_string();
                std::string r = right.as_string();
                if (op == "==")
                    if_result = l == r;
                else if (op == "!=")
                    if_result = l != r;
                else
                    if_result = false;
            }
            else
            {
                double l = left.is_float() ? left.as_float() : left.as_int();
                double r = right.is_float() ? right.as_float() : right.as_int();
                if (op == "==")
                    if_result = l == r;
                else if (op == "!=")
                    if_result = l != r;
                else if (op == ">")
                    if_result = l > r;
                else if (op == "<")
                    if_result = l < r;
                else if (op == ">=")
                    if_result = l >= r;
                else if (op == "<=")
                    if_result = l <= r;
            }
            NEED_MORE_EVENTS = 1;
        }
        break;

        case 20: // LUA
        {
            std::string code = std::string(get_from_spool(apool[current_event->args_offset].value));
            std::string src;

            if (code[0] == '='){
                src = "__ret=(function() return " + code.substr(1) + " end)()";
            }
            else{
                src = code;
            }

            lua_runtime.run_string(src);
        
        }
        break;
        case 21:
        {
            if_result = !if_result;
            NEED_MORE_EVENTS = 1;
        }
        break;
        case 22:
        { // ENDIF
            if_result = true;
            NEED_MORE_EVENTS = 1;
        }
        break;
        case 23: // BPM
        {
            if (apool[current_event->args_offset].type == ARG_INT)
            {
                audio->play_music("0");
            }
            else
            {
                const char *file = get_from_spool(apool[current_event->args_offset].value);
                audio->play_music(interpolate(std::string(file)).c_str());
            }
        }
        break;
        case 24: // SOUND
        {
            const char *file = get_from_spool(apool[current_event->args_offset].value);
            audio->play_audio(interpolate(std::string(file)).c_str());

        }
        break;
        case 25: // BOMFADEIN
        {
            int t = apool[current_event->args_offset].value;
            audio->fade_in_music(t);
        }
        break;
        case 26: // LUA_IMPORT
        {
            const char *file = get_from_spool(apool[current_event->args_offset].value);
            if (IS_CCNVL){
                int hash = fnv1a_32(file);
                auto it = ccnvl_resources.find(hash);
                if (it != ccnvl_resources.end()) {
                    auto &res = it->second;
                    lua_runtime.lua_load_buffer((const char*)(ccnvl_data + res.offset), res.size, file);
            }
            }
            else{

            std::cout << "[LUA_IMPORT] " << file << "\n";
            lua_runtime.lua_import_file(file);
            }
        }
        break;
        case 27: // LD_FILE
        {
            const char *file = get_from_spool(apool[current_event->args_offset].value);
            log(std::string("LD_FILE ") + file);
            this->load_(const_cast<char *>(file));
            this->change_scene("main");

            NEED_MORE_EVENTS = 1;
        }
        break;
        case 28: // BPMVOL
        {
            audio->set_music_volume(apool[current_event->args_offset].value);
        }
        break;
        case 29: // SOUNDVOL
        {
            const char *file = get_from_spool(apool[current_event->args_offset].value);
            audio->play_audio(interpolate(std::string(file)).c_str(), 0, apool[current_event->args_offset + 1].value);
        }
        break;
        case 33: // BPMFADEOUT
        {
            int t = apool[current_event->args_offset].value;
            audio->fade_out_music(t);
        }
        break;
        case 34: // TALK
        {
            const char *file = get_from_spool(apool[current_event->args_offset].value);
            audio->play_audio(interpolate(std::string(file)).c_str(), 1);

        }
        break;
        case 35: // INPUT
        {
            SDL_StartTextInput();
            textbox->IS_INPUT=1;
            textbox->addMessage(interpolate(get_from_spool(apool[current_event->args_offset].value)));
            textbox->input_header_size = textbox->get_last()->size();
        }
        break;
        case 36: // TBRECT
        {
            int x = apool[current_event->args_offset].value;
            int y = apool[current_event->args_offset + 1].value;
            int w = apool[current_event->args_offset + 2].value;
            int h = apool[current_event->args_offset + 3].value;
            textbox->move_position(x, y);
            if (w != -1 && h != -1)
                textbox->update_position(w, h);
        }
        break;
        case 37: //TBFILL
        {
            const char *hexcol = get_from_spool(apool[current_event->args_offset].value);
            Color c = {hexcol};
            
        }
        break;
        case 38: //HIDE
        {
            int id = apool[current_event->args_offset].value;

            log("HIDE: " + std::to_string(id));
            log("INFO " + std::to_string(sprites.size()));
            sprites[id].hide();


        }
        break;
        case 39: // WAITV
        {
            const char *var = get_from_spool(apool[current_event->args_offset].value);
            var_waiting = std::string(var);
            set_value(var_waiting, 0);
            NEED_MORE_EVENTS=1;
        }
        break;
        case 40: //SETACTIVE
        {

        }
        break;
        case 41: // TBTOGGLE
        {
            if(textbox->hidden)textbox->show();
            else{textbox->hide();}
        }
        break;

        case 43: // SHOW
        {
            int id = apool[current_event->args_offset].value;
            sprites[id].show();
        }
        break;

        case 44: // DEL
        {
            sprites.erase(sprites.begin() + apool[current_event->args_offset].value);
        }
        case 45: // DELSPRITES
        {
            sprites.clear();
        }
        }
}

void Screen::handleMouseEvent(const SDL_Event &e)
{

        if (e.type == SDL_MOUSEBUTTONDOWN)
        {
            px = e.button.x;
            py = e.button.y;
            textbox->check_cosmetic_press(px, py);
        }


        if (e.type == SDL_MOUSEBUTTONUP)
        {
            if (WAS_MOTION){
                WAS_MOTION = 0;
                return;
            }
            px = e.button.x;
            py = e.button.y;


            if (e.button.button == SDL_BUTTON_LEFT)
            {

                interface->handle_click(px, py);
                if (if_its_game){
                textbox->check_press(px, py);
                if (!textbox->is_last_completed())
                {
                    textbox->done_messages();
                    //if (textbox->is_last_completed() && !WAITING)
                    //    handleEvent();
                }
                else if (!WAITING && !textbox->WAS_ACTION && !interface->smth_pressed && !lua_runtime.has_blocking_coroutine())
                    NEED_MORE_EVENTS=1;

                    

            }
            if (textbox->IS_INPUT){
                    SDL_StopTextInput();
                    std::string l = *(textbox->get_last());
                    if (!l.empty()) l.erase(0, textbox->input_header_size);
                    variables["__input"] =  make_var(l);
                    lua_runtime.everybody_inputed();
                    textbox->IS_INPUT = 0;
            }
            lua_runtime.everybody_clicked();
        }
            else if (e.button.button == SDL_BUTTON_RIGHT)
            {
            }
        }
        if (e.type == SDL_MOUSEMOTION){
            auto [px, py] = camera->get_real_cords(e.motion.x, e.motion.y);
            textbox->is_hovered(px, py);
        }
}

void Screen::run(abool &run)
{
        
        last_time = SDL_GetTicks();
        textbox->update_position(width, height);
        //nextEvent();

        //handleEvent();
        textbox->load_toml(renderer, "textbox.toml");



        while (run && running)
        {

            // events
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    set_value("__running__", 0);
                    exit(0);
                }

                if (e.type == SDL_FINGERMOTION){
                    if (std::fabs(e.tfinger.dy)> 0.005f)
                        WAS_MOTION = 1;

                    textbox->handle_mouse_wheel(e);

                }
    
                handleMouseEvent(e);
                if (e.type == SDL_KEYDOWN)
                {
                    SDL_Keymod mod = SDL_GetModState();
                    if ((e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) && textbox->IS_INPUT){
                        if(!lua_runtime.has_blocking_coroutine()){
                            NEED_MORE_EVENTS = 1;
                        }
                        
                        SDL_StopTextInput();
                        std::string l = *(textbox->get_last());
                        if (!l.empty()) l.erase(0, textbox->input_header_size);
                        variables["__input"] =  make_var(l);
                        lua_runtime.everybody_inputed();
                        textbox->IS_INPUT = 0;
                        
                        
                    }
                    else if ((e.key.keysym.sym == SDLK_BACKSPACE) && textbox->IS_INPUT){
                        std::string* last = (textbox->get_last());
                        if (last != nullptr && !(last->empty()) && (last->size() > textbox->input_header_size)){
                            char lp = last->back();
                            while((lp & 0xC0) == 0x80 && last->size() > 0){
                                last->pop_back();
                                lp = last->back();
                            }
                            last->pop_back();
                        }
                        
                    }

                    else if (mod & KMOD_CTRL)
                    {
                        if ((e.key.keysym.sym == SDLK_EQUALS && (mod & KMOD_SHIFT)) || e.key.keysym.sym == SDLK_KP_PLUS)
                        {
                            main_font.setSize(main_font.size + 5);
                        }
                        else if (e.key.keysym.sym == SDLK_MINUS || e.key.keysym.sym == SDLK_KP_MINUS)
                        {
                            main_font.setSize(main_font.size - 5);
                        }
                    }
                    else if ((e.key.keysym.sym == SDLK_ESCAPE) && if_its_game == 1){
                        if (interface->shown()){
                            interface->hide();
                        }
                        else{
                            show_interface();}
                    }
                    else if(e.key.keysym.sym == SDLK_AC_BACK){
                            if (interface->shown()){

                                main_menu();
                            }
                            else{
                                show_interface();}
                    }
                }
                else if (e.type == SDL_TEXTINPUT && textbox->IS_INPUT) {
                    *(textbox->get_last()) += e.text.text;
                    textbox->refresh_last();
                }
                else if ((e.type == SDL_MOUSEWHEEL) && if_its_game == 1){
                    textbox->handle_mouse_wheel(e);
                }
                else if (e.type == SDL_MOUSEMOTION){
                    auto [px, py] = camera->get_real_cords(e.motion.x, e.motion.y);

                }
            }
            if(textbox->WAS_ACTION){
                textbox->WAS_ACTION = 0;
                lua_runtime.run_string(LUA_ACTION_FROM_TEXTBOX);
                LUA_ACTION_FROM_TEXTBOX.clear();
            }
            if (NEED_MORE_EVENTS)
            {
                NEED_MORE_EVENTS = 0;
                if (var_waiting.empty() || get_value(var_waiting).as_int() == 1)
                {
                    var_waiting.clear();
                    if (!lua_runtime.has_blocking_coroutine()){
                    nextEvent();
                    handleEvent();
                    update_snapshot();
                    }
                }
                else
                {
                    NEED_MORE_EVENTS = 1;
                }
            }
            if (interface->smth_pressed){
                interface->smth_pressed = 0;
            }
            if (need_to_do){
                need_to_do();
                need_to_do = nullptr;
            }
            update_and_render();
        }


        return;
}
void Screen::update_and_render()
{
        uint32_t current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;
        lua_runtime.handle_lua_coroutines(delta_time);
        //textbox->is_hovered(px, py);

        if (WAITING)
        {
            wait_timer -= delta_time;
            if (wait_timer <= 0.0f)
            {
                WAITING = false;
                if (!IN_ROW)
                {
                    //nextEvent();
                    NEED_MORE_EVENTS = true;
                }
            }
        }
        textbox->bake_completed(renderer);

        camera->set_renderer(renderer);

        textbox->update(delta_time);
        bg.update(delta_time);
        camera->update(delta_time);


        SDL_RenderClear(renderer);
        bg.draw(renderer);
        if (sprites.size()>0){
        std::for_each(sprites.begin(), sprites.end(), [this, delta_time](Sprite &sprite)
                 { sprite.update(delta_time); sprite.draw(renderer); });
        }
        if (!camera->TEXTBOX_IS_NOT_ZOOMED){
            textbox->draw(renderer);
        }

        camera->draw(renderer);
        camera->finish_renderer(renderer);
        if (camera->TEXTBOX_IS_NOT_ZOOMED){
            textbox->draw(renderer);
        
        }
        interface->draw(renderer, px, py);
        SDL_RenderPresent(renderer);
}

void Screen::clean()
{
        if (ccnvl_file){
            fclose(ccnvl_file);
            free(ccnvl_data);
        }
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();

        camera->finish_renderer(renderer);

        if (renderer)
            SDL_DestroyRenderer(renderer);
        if (window)
            SDL_DestroyWindow(window);
        SDL_Quit();
}

SDL_Renderer* Screen::getRenderer() const
{
        return renderer;
}


void Screen::set_if_its_game(bool s){
    if_its_game = s;
}

void Screen::build_n_run_interface(std::function<std::unique_ptr<Menu>(int, int, Screen*)> b){
    interface = b(width, height, this);
    interface->show();
}

void Screen::exit_program(){
        if (!need_to_do){
        need_to_do = [this]{
                set_value("__running__", 0);
                exit(0);
        };
    }
}

bool Screen::can_save() {
    if (WAITING) return false;
    if (textbox->IS_INPUT) return false;
    if (!var_waiting.empty()) return false;
    if (function_commands_left > 0) return false;
    if (IN_ROW) return false;
    if (!lua_runtime.if_available_for_saving()) return false;
    return true;
}

void Screen::qsave(){

    update_snapshot();
    FILE* save_file;

    if (save_snapshot.empty()){
        log("save snapshot is empty!");
        return;}

    if (save_path)
        save_file = fopen(save_path, "wb");
    else
        save_file = fopen("quick_save.ccsave", "wb");
    
    if (!save_file) {
        log("Failed to open save file");
        return;
    }

    fwrite(save_snapshot.data(), 1, save_snapshot.size(), save_file);

    //uint32_t scene_hash = fnv1a_32(current_scene_name);

    fclose(save_file);

}

void Screen::qload(){
    FILE* save_file;
    if (save_path)
        save_file = fopen(save_path, "rb");
    else
        save_file = fopen("quick_save.ccsave", "rb");

    uint32_t save_version;
    fread(&save_version, sizeof(uint32_t), 1, save_file);
    std::cout<<"VERS "<<save_version<<' '<<VERSION<<"\n";
    if (save_version > VERSION){
        log("The game version is too old!");
        return;
    }
    epos = 0;
    apos = 0;
    spos = 0;
    textbox->cl();

    IS_CCNVL = 0;

    sprites.clear();
    uint32_t nl; fread(&nl, sizeof(uint32_t), 1, save_file);
    file_name.resize(nl);
    fread(&file_name[0], sizeof(char), nl, save_file);
    log("file_name " + std::string(file_name));
    load_(const_cast<char*>(file_name.c_str()));
    uint32_t script_len; fread(&script_len, sizeof(uint32_t), 1, save_file);
    current_script_name.resize(script_len);
    fread(&current_script_name[0], sizeof(char), script_len, save_file);
    IS_CCNVL=1;
    log("script_name " + std::string(current_script_name));
    if (current_script_name.size() > 0) load_(const_cast<char*>(current_script_name.c_str()));
    uint32_t scene_hash;
    fread(&scene_hash, sizeof(uint32_t), 1, save_file);
    uint32_t scene_len; fread(&scene_len, sizeof(uint32_t), 1, save_file);
    current_scene_name.resize(scene_len);
    fread(&current_scene_name[0], sizeof(char), scene_len, save_file);
    fread(&event_pool_position, sizeof(uint32_t), 1, save_file);
    load_vars(save_file);


    //load_scene_by_hash(scene_hash, scenes);
    int index = find_scene_index_by_hash(scenes, scene_hash);
    Scene &sc = scenes[index];
    current_scene = &sc;
    std::cout<<"SCENE "<<current_scene->name<<"\n";

    bg.read_yourself(save_file, renderer);
    uint32_t ss; fread(&ss, sizeof(uint32_t), 1, save_file);
    sprites.reserve(ss);
    for(int i=0; i<ss; i++){
        sprites.emplace_back();
        sprites.back().read_yourself(save_file, renderer);
    }
    //change_scene(current_scene_name.c_str());


    function_commands_left = 0;
    IN_ROW = false;
    WAITING = false;
    if_result = 1;
    var_waiting.clear();
    current_event = nullptr;
    NEED_MORE_EVENTS = 0;


    textbox->read_yourself(save_file);

    textbox->set_footer(renderer);

    audio->read_yourself(save_file);

}


void Screen::update_snapshot(){
    if (!can_save()){
        return;
    }

    mem_buffer mb;
    FILE* mem_file = lcnovel_open_mem_stream(&mb);
    if (!mem_file) return;

    write_states(mem_file);
    fflush(mem_file);
    fclose(mem_file);

    save_snapshot.assign(mb.data.begin(), mb.data.end());}

void Screen::write_states(FILE *save_file){
    fwrite(&VERSION, sizeof(const uint32_t), 1, save_file);
    uint32_t name_len = (uint32_t)file_name.size();
    fwrite(&name_len, sizeof(uint32_t), 1, save_file);
    std::cout<<"SAVING "<<file_name<<" "<<current_script_name<<"\n";
    
    fwrite(file_name.data(), sizeof(char), file_name.size(), save_file);
    uint32_t script_len = current_script_name.size();
    fwrite(&script_len, sizeof(uint32_t), 1, save_file);
    fwrite(current_script_name.data(), sizeof(char), script_len, save_file);
    uint32_t scene_hash = fnv1a_32(current_scene_name);
    fwrite(&scene_hash, sizeof(uint32_t), 1, save_file);
    uint32_t scene_name_len = current_scene_name.size();
    fwrite(&scene_name_len, sizeof(uint32_t), 1, save_file);
    fwrite(current_scene_name.data(), sizeof(char), scene_name_len, save_file);

    fwrite(&event_pool_position, sizeof(uint32_t), 1, save_file);

    save_vars(save_file);

    bg.write_yourself(save_file);
    uint32_t ss = sprites.size();
    fwrite(&ss, sizeof(uint32_t), 1, save_file);
    for (Sprite &s: sprites){
        s.write_yourself(save_file);
    }

    textbox->write_yourself(save_file);

    audio->write_yourself(save_file);

}
