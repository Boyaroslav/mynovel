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

#include "utils.hpp"

struct sprite_texture {
    SDL_Texture* texture;
    int original_texture_width;
    int original_texture_height;
};

struct sprite_region { // for atlas 
    std::shared_ptr<sprite_texture> texture;
    SDL_Rect rect;
};

class Sprite {

private:
    std::vector<sprite_texture> textures;
    std::vector<uint32_t> texture_hashes;
    SDL_Rect rect = {0,0,0,0};
    int x = 0, y=0;
    int texture_size_w, texture_size_h;
    float angle = 0;
    int current_index = 0;
    int future_index = -1;
    float alpha = 1.0f;
    float texture_change_speed = 0.0f;
    float transition_alpha = 1.0f;
    float move_speed = 0.0f; // in sec
    float move_transition = 0.0f;
    float hide_transition = -1.0f;
    float scale_transition = 0.0f;
    float rotate_transition = 0.0f;

    int sx; //start x y (before transition started)
    int sy;
    int tx; // target x y
    int ty;

    int texture_change_percent = 100;

    bool in_fade = true; // есть ли исчезающий спрайт при смене главного спрайта

public:
    ~Sprite() {
        texture_hashes.clear();
        for (auto t : textures) SDL_DestroyTexture(t.texture);
    }

    void clear(){
        texture_hashes.clear();
        for (auto t : textures) SDL_DestroyTexture(t.texture);
    }

    SDL_Surface* load_from_ccnvl(const char* path) {
        // атлас будем грузить по path + ".atlas"
        uint32_t hash = fnv1a_32(path);

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end()) {
            log("CCNVL resource not found: " + std::string(path));
            return nullptr;
        }
        
        auto& res = it->second;
        SDL_RWops* rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        if (!rw) return nullptr;

        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (!surf) {
            log("Failed to load surface from CCNVL: " + std::string(path));
            return nullptr;
        }
        texture_hashes.push_back(hash);

        return surf;
    } 

    void load_hash(SDL_Renderer* rend, uint32_t hash) {

        auto it = ccnvl_resources.find(hash);
        if (it == ccnvl_resources.end()) {
            log("CCNVL resource not found: " + std::to_string(hash));
            return;
        }
        // load hash не добавляет в texture_hash ничего!

        auto& res = it->second;
        SDL_RWops* rw = SDL_RWFromMem(ccnvl_data + res.offset, res.size);
        if (!rw){
            log("!rw!!!");
            return;}

        SDL_Surface* surf = IMG_Load_RW(rw, 1);
        if (!surf) {
            log("Failed to load surface from CCNVL: " + std::to_string(hash));
            return;
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_FreeSurface(surf);
        int w, h;
        SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        textures.push_back(sprite_texture{tex, w, h});

    } 


    void load_texture(SDL_Renderer* rend, const char* path, int index=-1) {
        SDL_Surface* surf;
        if (future_index != -1){
            current_index = future_index;
        }
        
        if (IS_CCNVL){surf = load_from_ccnvl(path);}
        else  surf = IMG_Load(path);
        if (!surf) {
            log(std::string("failed to load") + path);
            SDL_FreeSurface(surf);
            return;
        };
        SDL_Texture* new_texture = SDL_CreateTextureFromSurface(rend, surf);
        int w,h;
        SDL_QueryTexture(new_texture, NULL, NULL, &w, &h);
        SDL_SetTextureBlendMode(new_texture, SDL_BLENDMODE_BLEND);
        if (index == -1) {
            textures.push_back(sprite_texture{new_texture, w, h});
            texture_size_w=w;
            texture_size_h=h;
        }

        SDL_FreeSurface(surf);


    }
    Sprite(){}
    
    Sprite(SDL_Renderer* rend, const char* path){
        load_texture(rend, path);
        //set_rect(50, 50, texture_size_w, texture_size_h);
        
    }
    Sprite(SDL_Renderer* rend, const char* path, int x, int y, int w, int h){
        load_texture(rend, path);
        set_rect(x, y, w, h);
        //set_rect(50, 50, texture_size_w, texture_size_h);
        
    }
    Sprite(SDL_Renderer* rend, const char* path, int x, int y, int w, int h, float ld_speed){
    
        load_texture(rend, path);
        this->set_texture_change_speed(ld_speed);
        if (this->textures.size() < 2)in_fade = false;
        set_rect(x, y, w, h);

        future_index = get_last();
        transition_alpha = 0.0f;

        //set_rect(50, 50, texture_size_w, texture_size_h);
        
    }

    void set_current_texture(int index) {
        if (index < 0 || index >= textures.size()) return;
        current_index = index;
        future_index = -1;
        transition_alpha = 0.0f;
        alpha = 1.0f;
    }
    void start_transition(int index) {
        if (index < 0 || index >= textures.size() || index == current_index) return;
        future_index = index;
        transition_alpha = 0.0f;
    }
    float get_transition_progress(){
        return transition_alpha;
    }
    void update(float delta_time) {
        if (move_transition < 1.0f && move_speed > 0.0f) {
            move_transition += delta_time / move_speed;
            if (move_transition >= 1.0f) move_transition = 1.0f;
            x = sx + (tx - sx) * move_transition;
            y = sy + (ty - sy) * move_transition;
        }
        if (hide_transition > 0.0f){
            hide_transition -= (delta_time / texture_change_speed);
            if (hide_transition <= 0.0f){
                hide_transition = 0.0f;

            }
        }
        if (future_index == -1 || texture_change_speed <= 0.0f) return;

        transition_alpha += (delta_time / texture_change_speed);
        if (transition_alpha >= 1.0f) {
            current_index = future_index;
            future_index = -1;
            transition_alpha = 0.0f;
            in_fade = 1;
        }
    }

    void set_texture_change_speed(float n) {
        texture_change_speed = n;
    }

    void draw(SDL_Renderer* rend) {
        if (!rend){
            log("NO RENDERER");
            return;
        }
        if (textures.empty()){
            return;

        }
        if (textures.size() <= current_index){
            //log("THEY ASK ME TO BREAK GAME  " + std::to_string(current_index));
            return;
        }
        SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
        SDL_Rect r_ = placed_rect(rect);
        if((future_index == -1) && hide_transition >= 0.0f){
            SDL_SetTextureAlphaMod(textures[current_index].texture, Uint8(hide_transition * 255));
            SDL_RenderCopyEx(rend, textures[current_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);     
        }
        else if ((future_index == -1) && in_fade) {
            SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
            SDL_RenderCopyEx(rend, textures[current_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);
        }
        else {
            if(in_fade){SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
                SDL_RenderCopyEx(rend, textures[current_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);
            }
            SDL_SetTextureAlphaMod(textures[future_index].texture, Uint8(transition_alpha * 255));
            //set_rect(textures[current_index].rect);

        
            //set_rect(textures[future_index].rect);
            SDL_RenderCopyEx(rend, textures[future_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);

            SDL_SetTextureAlphaMod(textures[future_index].texture, 255);
        }
        if(in_fade)SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
    }

    void set_rect(int x_, int y_, int w=-1, int h=-1) {
        x = x_;
        y = y_;
        rect.x = x_;
        rect.y = y_;
        if (w>0) rect.w = w;
        if (h>0) rect.h = h;
    }

    void hide(){
        future_index = -1;
        hide_transition = 1.0f;
    }

    void show(){
        future_index = current_index;
        transition_alpha = 0.0f; in_fade = false;
        hide_transition = -1.0f;

    }

    void set_rect(SDL_Rect r){
        rect = r;
    }

    void set_angle(float a) {
        angle = a;
    }
    int get_last() {
        return textures.size()-1;
    }
    SDL_Rect get_rect(){
        return rect;
    }
    void move(int x_, int y_=0, int t_=0){
        if (t_ > 0){
            tx = x_;
            ty = y_;
            sx = x;
            sy = y;
            move_speed = (t_ * 1.0) / 1000.0;
            move_transition = 0.0f;
        }
        else{
        x += x_;
        y += y_;
        }
        
    }

    SDL_Rect placed_rect(SDL_Rect r) const{
        r.x += x;
        r.y += y;
        return r; 
    }
    Sprite(const Sprite&) = delete;
Sprite& operator=(const Sprite&) = delete;

Sprite(Sprite&& other) noexcept {
    textures = std::move(other.textures);
    texture_hashes = std::move(other.texture_hashes);
    rect = other.rect;
    x = other.x;
    y = other.y;
    in_fade = other.in_fade;
    angle = other.angle;
    current_index = other.current_index;
    future_index = other.future_index;
    alpha = other.alpha;
    texture_change_speed = other.texture_change_speed;
    transition_alpha = other.transition_alpha;

    other.textures.clear(); // чтобы старый объект ничего не разрушил
}

Sprite& operator=(Sprite&& other) noexcept {
    textures = std::move(other.textures);
    texture_hashes = std::move(other.texture_hashes);
    rect = other.rect;
    x = other.x;
    y = other.y;
    angle = other.angle;
    in_fade = other.in_fade;
    current_index = other.current_index;
    future_index = other.future_index;
    alpha = other.alpha;
    texture_change_speed = other.texture_change_speed;
    transition_alpha = other.transition_alpha;

    other.textures.clear();
    return *this;
}

void write_yourself(FILE *ptr){ // пишет про себя все в файл чтоб потом прочитаться

    uint32_t tex_count = (uint32_t)texture_hashes.size();
    fwrite(&tex_count, sizeof(uint32_t), 1, ptr);
    if (tex_count > 0)
        fwrite(texture_hashes.data(), sizeof(uint32_t), tex_count, ptr);

    fwrite(&rect, sizeof(SDL_Rect), 1, ptr);
    fwrite(&x, sizeof(int), 1, ptr);
    fwrite(&y, sizeof(int), 1, ptr);
    fwrite(&angle, sizeof(float), 1, ptr);

    fwrite(&current_index, sizeof(int), 1, ptr);
    fwrite(&future_index, sizeof(int), 1, ptr);
    fwrite(&alpha, sizeof(float), 1, ptr);
    fwrite(&texture_change_speed, sizeof(float), 1, ptr);
    fwrite(&transition_alpha, sizeof(float), 1, ptr);
    fwrite(&in_fade, sizeof(bool), 1, ptr);

    fwrite(&move_speed, sizeof(float), 1, ptr);
    fwrite(&move_transition, sizeof(float), 1, ptr);
    fwrite(&hide_transition, sizeof(float), 1, ptr);
    fwrite(&sx, sizeof(int), 1, ptr);
    fwrite(&sy, sizeof(int), 1, ptr);
    fwrite(&tx, sizeof(int), 1, ptr);
    fwrite(&ty, sizeof(int), 1, ptr);
}

void read_yourself(FILE *ptr, SDL_Renderer* rend){
    texture_hashes.clear();
    textures.clear();
    uint32_t tex_count;
    fread(&tex_count, sizeof(uint32_t), 1, ptr);

    texture_hashes.resize(tex_count);
    if (tex_count > 0)
        fread(texture_hashes.data(), sizeof(uint32_t), tex_count, ptr);

    fread(&rect, sizeof(SDL_Rect), 1, ptr);
    fread(&x, sizeof(int), 1, ptr);
    fread(&y, sizeof(int), 1, ptr);
    fread(&angle, sizeof(float), 1, ptr);

    fread(&current_index, sizeof(int), 1, ptr);
    fread(&future_index, sizeof(int), 1, ptr);
    fread(&alpha, sizeof(float), 1, ptr);
    fread(&texture_change_speed, sizeof(float), 1, ptr);
    fread(&transition_alpha, sizeof(float), 1, ptr);
    fread(&in_fade, sizeof(bool), 1, ptr);

    fread(&move_speed, sizeof(float), 1, ptr);
    fread(&move_transition, sizeof(float), 1, ptr);
    fread(&hide_transition, sizeof(float), 1, ptr);
    fread(&sx, sizeof(int), 1, ptr);
    fread(&sy, sizeof(int), 1, ptr);
    fread(&tx, sizeof(int), 1, ptr);
    fread(&ty, sizeof(int), 1, ptr);


    for (uint32_t hash : texture_hashes) {
        load_hash(rend, hash);
    }

    if (current_index >= texture_hashes.size()){
        current_index = 0;
    }
}


};