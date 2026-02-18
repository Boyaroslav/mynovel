#include "utils.hpp"

struct sprite_texture {
    SDL_Texture* texture;
    int original_texture_width;
    int original_texture_height;
};

class Sprite {

private:
    std::vector<sprite_texture> textures;
    SDL_Rect rect = {0,0,0,0};
    int x = 0, y=0;
    int texture_size_w, texture_size_h;
    float angle = 0;
    int current_index = 0;
    int future_index = -1;
    float alpha = 1.0f;
    float texture_change_speed = 0.0f;
    float transition_alpha = 1.0f;

    int texture_change_percent = 100;

public:
    ~Sprite() {
        for (auto t : textures) SDL_DestroyTexture(t.texture);
    }
    void load_texture(SDL_Renderer* rend, const char* path, int index=-1) {
        SDL_Surface* surf = IMG_Load(path);
        if (!surf) {
            log(std::string("failed to load") + path);
            SDL_FreeSurface(surf);
            return;
        };
        SDL_Texture* new_texture = SDL_CreateTextureFromSurface(rend, surf);
        int w,h;
        SDL_QueryTexture(new_texture, NULL, NULL, &w, &h);
        SDL_SetTextureBlendMode(new_texture, SDL_BLENDMODE_BLEND);
        rect = SDL_Rect(0, 0, w, h);
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
        set_rect(50, 50, 300, 300);
        
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
        if (future_index == -1 || texture_change_speed <= 0.0f) return;

        transition_alpha += (delta_time / texture_change_speed);
        if (transition_alpha >= 1.0f) {
            current_index = future_index;
            future_index = -1;
            transition_alpha = 0.0f;
        }
    }

    void set_texture_change_speed(float n) {
        texture_change_speed = n;
    }

    void draw(SDL_Renderer* rend) {
        SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
        SDL_Rect r_ = placed_rect(rect);
        if (future_index == -1) {
            SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
            SDL_RenderCopyEx(rend, textures[current_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);
        } else {
            SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
            SDL_SetTextureAlphaMod(textures[future_index].texture, Uint8(transition_alpha * 255));
            //set_rect(textures[current_index].rect);

            SDL_RenderCopyEx(rend, textures[current_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);
            //set_rect(textures[future_index].rect);
            SDL_RenderCopyEx(rend, textures[future_index].texture, nullptr, &r_, angle, nullptr, SDL_FLIP_NONE);

            SDL_SetTextureAlphaMod(textures[future_index].texture, 255);
        }
        SDL_SetTextureAlphaMod(textures[current_index].texture, 255);
    }

    void set_rect(int x_, int y_, int w=-1, int h=-1) {
        x = x_;
        y = y_;
        rect.x = x_;
        rect.y = y_;
        if (w>0) rect.w = w;
        if (h>0) rect.h = h;
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
    void move(int x_, int y_=0){
        x += x_;
        y += y_;
        
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
    rect = other.rect;
    x = other.x;
    y = other.y;
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
    rect = other.rect;
    x = other.x;
    y = other.y;
    angle = other.angle;
    current_index = other.current_index;
    future_index = other.future_index;
    alpha = other.alpha;
    texture_change_speed = other.texture_change_speed;
    transition_alpha = other.transition_alpha;

    other.textures.clear();
    return *this;
}


};