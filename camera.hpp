#include "utils.hpp"


enum curve_type{
    LINEAR,
    PARABOLIC,
    LOGARIPHMIC
};

struct camera_operation{
    int x; int tx;
    int y; int ty;
    double zoom; double target_zoom;
    double angle; double target_angle;

    curve_type type;

    double elapsed; // сколько щас
    double duration; // сколько надо


};

double co_time(camera_operation& co){
    return co.elapsed / co.duration;
}


float parabollic(float t, float start, float end, bool inverted=0){
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    float k = inverted ? (1.0f - (1.0f - t) * (1.0f - t)) : (t * t);

    return start + (end - start) * k;
}

float linear(float t, float start, float end, bool inverted=0){
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return start + (end - start) * t;
}

class Camera{
    private:
    int bx=0, by=0;

    int w, h;
    int moved_x=0, moved_y=0;
    float zoom=1.0;

    int center_x;
    int center_y;
    
    float angle = 0;


    float zoom_speed = 1.0;
    float move_speed = 1.0;

    std::vector<camera_operation> cops;

    SDL_Texture* cam_tex;
    public:

    Camera(){

    }

    ~Camera(){
        SDL_DestroyTexture(cam_tex);

    }

    void init(SDL_Renderer* rend, int w_, int h_){
        cam_tex = SDL_CreateTexture(
            rend,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            w_, h_
        );
        w = w_; h = h_;
        bx = w/2;by=h/2;
        center_x = w/2;
        center_y = h/2;
        SDL_SetTextureBlendMode(cam_tex, SDL_BLENDMODE_BLEND);


    }

    void camera_reset(){
        cops.clear();
        center_x=w/2;
        center_y=h/2;
        moved_x=0;moved_y=0;zoom=1.0;
    }

    void finish_renderer(SDL_Renderer* rend){
        SDL_SetRenderTarget(rend, nullptr);
    }

    void add_operation(int tx, int ty, double tzoom, double tangle, int type, double duration){
        finish_cops();
        std::cout<<tx<<' '<<ty<<' '<<tzoom<<"\n";
        camera_operation co = {
            center_x, center_x + tx,
            center_y, center_y + ty,
            zoom, tzoom,
            angle, tangle,
            (curve_type)type,
            0.0, duration

        };
        cops.emplace_back(co);

    }

    void update(float delta_time){
        for(auto i = cops.begin(); i != cops.end();){
            auto &co = *i;
            co.elapsed += delta_time;
            float t = co_time(co);
            if (t >= 1){
                handle_command(t, co);
                cops.erase(i);
                log("FINISHED");
                continue;

            }

            handle_command(t, co);
            i++;

        }

    }

    std::pair<int, int> get_real_cords(int x, int y){
        return { (double)x / zoom + bx, (double)y / zoom + by  };
    }

    void handle_command(float t, camera_operation& co){

        switch(co.type){
                case LINEAR:
                {
    
                    center_x = linear(t, co.x, co.tx, 0);
                    center_y = linear(t, co.y, co.ty, 0);
                    angle = linear(t, co.angle, co.target_angle, 0);
                    zoom = linear(t, co.zoom, co.target_zoom, 0);





                }
                break;
                case PARABOLIC:
                {
                    center_x = parabollic(t, co.x, co.tx, 0);
                    center_y = parabollic(t, co.y, co.ty, 0);
                    angle = parabollic(t, co.angle, co.target_angle, 0);
                    zoom = parabollic(t, co.zoom, co.target_zoom, 0);
                }
                break;
        }
    }



    void set_renderer(SDL_Renderer* rend){
        SDL_SetRenderTarget(rend, cam_tex);
    }

    void finish_cops(){
        for(auto it = cops.begin(); it != cops.end(); ){
            auto& co = *it;
            handle_command(2.0, co);
            cops.erase(it);
        }
    }

    void draw(SDL_Renderer* rend){
        finish_renderer(rend);
        bx = center_x - ((int)((double)w / zoom)/2); by = center_y - ((int)((double)h / zoom)/2);
        int bw = (int)((double)w / zoom), bh = (int)((double)h / zoom);

        if (bx < 0) bx =0;
        if (by < 0) by = 0;
        if (bx + bw > w) bx = w - bw;
        if (by + bh > h) by = h - bh;


        SDL_Rect source{
            bx, by,
            bw, bh
        };


        SDL_Rect target{
            0, 0, w, h
        };


        SDL_RenderCopy(rend, cam_tex, &source, &target);
        set_renderer(rend);
    }

};