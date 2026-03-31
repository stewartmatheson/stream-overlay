#pragma once
#include "Mode.h"
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <deque>

struct PopUp {
    std::string title;
    std::string body;
    SDL_Color   borderColor;
    SDL_Color   bgColor;
    float       timeLeft;
    float       totalTime;
};

class PopUpsMode : public Mode {
public:
    PopUpsMode(int winW, int winH);
    ~PopUpsMode() override;

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
    bool handleCommand(const std::vector<std::string>& tokens) override;

private:
    static constexpr float DISPLAY_TIME  = 5.0f;
    static constexpr float FADE_TIME     = 1.0f;
    static constexpr int   PADDING       = 16;
    static constexpr int   BORDER_WIDTH  = 6;
    static constexpr int   BOX_WIDTH     = 400;
    static constexpr int   MARGIN        = 20;

    int winW_, winH_;
    TTF_Font* titleFont_ = nullptr;
    TTF_Font* bodyFont_  = nullptr;

    std::deque<PopUp> popups_;

    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                            const std::string& text, SDL_Color color,
                            int wrapWidth, int* outW, int* outH);
};
