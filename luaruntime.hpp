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

/*
для тех функций которым нужен wait, input и тд
1) можно тупо запрещать сохраняться пока они работают
2) можно хер на них забить и просто через сцену чекать работают ли они и заново запускать
возможно сразу оба подхода будут
*/

#include "utils.hpp"

#include "vars.hpp"

#define LUA_COMMAND_ADD_MESSAGE_TO_TEXTBOX "txt"
#define LUA_COMMAND_CLEAR_TEXTBOX "cltb"
#define LUA_COMMAND_CLEAR_ONE_MESSAGE "cllast"
#define LUA_COMMAND_INPUT "input"
#define LUA_COMMAND_LOG "log"
#define LUA_COMMAND_CHANGE_SCENE "change_scene"
#define LUA_COMMAND_LOAD_SPRITE "ld"
#define LUA_COMMAND_MOVE_SPRITE "move"
#define LUA_COMMAND_CAMERA_COMMAND "camera"

struct LuaCoroutine
{
    lua_State *co;
    float wait_timer = 0.0f;
    int lua_ref = LUA_NOREF;
    int input_waiting = false;
    int click_waiting = false;    // короче доп аргумент будет в txt и тд - задерживать ли до нового клика
    int need_to_continue = false; // я хз че это
};

class LuaRuntime
{
public:
    lua_State *L = nullptr;
    std::unordered_map<lua_State *, LuaCoroutine> lstate_coroutine;

    void init()
    {
        L = luaL_newstate();
        luaL_openlibs(L);

        lua_pushlightuserdata(L, this);

        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            const char *text = luaL_checkstring(L, 1);

            if (self->TXT)
                self->TXT(text);
            
            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, LUA_COMMAND_ADD_MESSAGE_TO_TEXTBOX);

        lua_pushlightuserdata(L, this);

        // cl (clear)
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            if (self->CLEAR)
                    self->CLEAR();

            if (self->should_wait_click(L))
                return self->yield_until_click(L);
            return 0; }, 1);
        lua_setglobal(L, LUA_COMMAND_CLEAR_TEXTBOX);

        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            if (self->CLEAR_LAST)
                    self->CLEAR_LAST();
            return 0; }, 1);
        lua_setglobal(L, LUA_COMMAND_CLEAR_ONE_MESSAGE);

        lua_pushcfunction(L, [](lua_State *L) -> int
                          {
                              float t = (float)luaL_checknumber(L, 1);
                              lua_pushnumber(L, t);
                              return lua_yield(L, 1); });
        lua_setglobal(L, "wait");
        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
                        auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
                        const char *text = luaL_checkstring(L, 1);
            //self->mark_last_as_waiting_for_input();
            auto it = self->lstate_coroutine.find(L);
            if (it != self->lstate_coroutine.end()){
                it->second.input_waiting = true;
            }

            if (self->INPUT)
                    self->INPUT(text);



            return lua_yield(L, 0); }, 1);
        lua_setglobal(L, LUA_COMMAND_INPUT);

        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            log(luaL_checkstring(L, 1));



            return 0; }, 1);
        lua_setglobal(L, LUA_COMMAND_LOG);

        lua_pushlightuserdata(L, this);
        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            self->sync_vars_to_lua(L);

            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, "sync");

        lua_pushlightuserdata(L, this);

        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            const char *text = luaL_checkstring(L, 1);

            if (self->CHSC)
                self->CHSC(text);
            
            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, LUA_COMMAND_CHANGE_SCENE);

        lua_pushlightuserdata(L, this);

        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            std::string text = const_cast<char*>(luaL_checkstring(L, 1));
            int x = luaL_checkinteger(L, 2);
            int y = luaL_checkinteger(L, 3);
            int w = luaL_checkinteger(L, 4);
            int h = luaL_checkinteger(L, 5);

            if (self->LD)
                self->LD(text, x, y, w, h);
            
            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, LUA_COMMAND_LOAD_SPRITE);


        lua_pushlightuserdata(L, this);

        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            int i = luaL_checknumber(L, 1);
            
            int x = luaL_checknumber(L, 2);
            int y = luaL_checknumber(L, 3);
            int t = luaL_checknumber(L, 4);
            

            if (self->MOVE)
                self->MOVE(i, x, y, t);
            
            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, LUA_COMMAND_MOVE_SPRITE);

                lua_pushlightuserdata(L, this);

        lua_pushcclosure(L, [](lua_State *L) -> int
                         {
            auto *self = (LuaRuntime*)lua_touserdata(L, lua_upvalueindex(1));
            int tx = luaL_checknumber(L, 1);
            
            int ty = luaL_checknumber(L, 2);
            double z = luaL_checknumber(L, 3);
            double a = luaL_checknumber(L, 4);

            int type = luaL_checkinteger(L, 5);
            double dur = luaL_checknumber(L, 6);
            

            if (self->CAMERA_COMMAND)
                self->CAMERA_COMMAND(tx, ty, z, a, type, dur);
            
            if (self->should_wait_click(L))
                return self->yield_until_click(L);

            return 0; }, 1);

        lua_setglobal(L, LUA_COMMAND_CAMERA_COMMAND);
    }
    void shotdown()
    {
    }

    std::function<void(const std::string &)> TXT;
    std::function<void(const std::string &)> INPUT;
    std::function<void(const std::string &)> CHSC;
    std::function<void()> CLEAR;
    std::function<void()> SYNC;
    std::function<void()> CLEAR_LAST;
    std::function<void(const std::string &)> LOG;
    std::function<void(std::string &, int, int, int, int)> LD;
    std::function<void(int, int, int, int)> MOVE;
    std::function<void(int, int, float, float, int, float)>CAMERA_COMMAND;

    void sync_vars_to_lua(lua_State *state)
    {
        for (auto &[key, val] : variables)
        {
            if (val.is_int())
                lua_pushinteger(state, val.as_int());
            else if (val.is_float())
                lua_pushnumber(state, val.as_float());
            else
                lua_pushstring(state, val.as_string().c_str());
            lua_setglobal(state, key.c_str());
        }
    }

    void lua_load_buffer(const char *data, ssize_t size, const char *name)
    {
        if (luaL_loadbuffer(L, data, size, name) != LUA_OK)
        {
            printf("[LUA LOAD ERROR] %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            return;
        }

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            printf("[LUA RUNTIME ERROR] %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            return;
        }
    }

    void lua_import_file(const char *file)
    {
        if (luaL_dofile(L, file) != LUA_OK)
        {
            printf("[LUA_IMPORT ERROR] %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    void run_string(const std::string &s)
    {
        lua_State *co = lua_newthread(L);
        lua_pushthread(co);
        lua_xmove(co, L, 1);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        bool waiting_click = 0;

        sync_vars_to_lua(co);

        if (luaL_loadstring(co, s.c_str()) != LUA_OK)
        {
            printf("[LUA ERROR] %s\n", lua_tostring(co, -1));
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }

        int nres = 0;

        lstate_coroutine[co] = {co, 0.0f, ref};
        int status = lua_resume(co, L, 0, &nres);

        if (status == LUA_YIELD)
        {

            if (nres > 0 && lua_isnumber(co, -1))
            {
                lstate_coroutine[co].wait_timer = (float)lua_tonumber(co, -1);
            }

            lua_pop(co, nres);
            waiting_click = lstate_coroutine[co].click_waiting;
            // lua_active_coroutines.push_back({co, t, ref});
            // lstate_coroutine[co] = &lua_active_coroutines.back();
        }
        else if (status == LUA_OK)
        {
            lstate_coroutine.erase(co);
            sync_vars_from_lua(co);
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }
        else
        {
            printf("[LUA ERROR] %s\n", lua_tostring(co, -1));
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
            lstate_coroutine.erase(co);
        }
        sync_vars_from_lua(L);
    }

    void sync_vars_from_lua(lua_State *state)
    {
        for (auto &[key, val] : variables)
        {
            lua_getglobal(state, key.c_str());
            if (lua_isinteger(state, -1))
                val = make_var((uint32_t)lua_tointeger(state, -1));
            else if (lua_isnumber(state, -1))
                val = make_var((double)lua_tonumber(state, -1));
            else if (lua_isstring(state, -1))
                val = make_var(std::string(lua_tostring(state, -1)));
            lua_pop(state, 1);
        }

        // __ret → __return
        lua_getglobal(state, "__ret");
        if (lua_isinteger(state, -1))
            variables["__return"] = make_var((uint32_t)lua_tointeger(state, -1));
        else if (lua_isnumber(state, -1))
            variables["__return"] = make_var((double)lua_tonumber(state, -1));
        else if (lua_isstring(state, -1))
            variables["__return"] = make_var(std::string(lua_tostring(state, -1)));
        else if (lua_isboolean(state, -1))
            variables["__return"] = make_var((uint32_t)lua_toboolean(state, -1));
        lua_pop(state, 1);
    }

    void handle_lua_coroutines(float delta_time)
    {
        for (auto it = lstate_coroutine.begin(); it != lstate_coroutine.end();)

        {
            LuaCoroutine &coroutine = it->second;
            if (coroutine.input_waiting)
            {
                ++it;
                continue;
            }

            if (coroutine.click_waiting)
            {
                ++it;
                continue;
            }

            if (coroutine.wait_timer > 0.0f)
            {
                coroutine.wait_timer -= delta_time;
                ++it;
                continue;
            }
            coroutine.wait_timer = 0.0f;
            sync_vars_to_lua(coroutine.co);

            int nres = 0;
            int status = lua_resume(coroutine.co, L, 0, &nres);

            if (status == LUA_YIELD)
            {
                if (nres > 0 && lua_isnumber(coroutine.co, -1))
                    coroutine.wait_timer = (float)lua_tonumber(coroutine.co, -1);

                lua_pop(coroutine.co, nres);
                ++it;
            }
            else
            {
                sync_vars_from_lua(coroutine.co);
                luaL_unref(L, LUA_REGISTRYINDEX, coroutine.lua_ref);
                it = lstate_coroutine.erase(it);
            }
        }
    }

    void everybody_inputed()
    {
        for (auto &[co, coroutine] : lstate_coroutine)
        {
            if (coroutine.input_waiting)
            {
                coroutine.input_waiting = false;
                coroutine.wait_timer = 0.016f;
            }
        }
    }
    void everybody_clicked()
    {
        for (auto &[co, coroutine] : lstate_coroutine)
        {
            if (coroutine.click_waiting)
            {
                coroutine.click_waiting = false;
                coroutine.wait_timer = 0.016f;
            }
        }
    }

    bool should_wait_click(lua_State* L) // напрямую берем переменную
    {
        lua_getglobal(L, "wait_click");
        bool result = false;
        if (lua_isnumber(L, -1))
            result = lua_tonumber(L, -1) != 0;
        else
            result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    }


    int yield_until_click(lua_State *L)
    {
        auto it = lstate_coroutine.find(L);
        if (it != lstate_coroutine.end())
            it->second.click_waiting = true;
        return lua_yield(L, 0);
    }


    bool if_available_for_saving()
    {
        // все ли инпутнули
        // все ли вейтанули
        return lstate_coroutine.empty();
    }

    bool has_click_waiting_coroutine() const
    {
        for (auto &[co, coroutine] : lstate_coroutine)
            if (coroutine.click_waiting)
                return true;
        return false;
    }

    bool has_blocking_coroutine() const
    {
        for (auto &[co, coroutine] : lstate_coroutine)
            if (coroutine.input_waiting || coroutine.click_waiting)
                return true;
        return false;
    }

    bool is_click_waiting(lua_State *co) const
    {
        auto it = lstate_coroutine.find(co);
        return it != lstate_coroutine.end() && it->second.click_waiting;
    }
};