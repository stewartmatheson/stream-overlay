#pragma once
#include "Mode.h"
#include <SDL_ttf.h>
#include <string>
#include <vector>
#include <deque>

struct BottomPopUp {
    std::string title;
    std::string body;
    SDL_Color   borderColor;
    SDL_Color   bgColor;
    float       timeLeft;
    float       totalTime;
};

class BottomPopupMode : public Mode {
public:
    BottomPopupMode(int winW, int winH);
    ~BottomPopupMode() override;

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;
    bool handleCommand(const std::vector<std::string>& tokens) override;
    bool isIdle() const override { return !hasActive_ && queue_.empty(); }

private:
    enum class Phase { SlideIn, Display, SlideOut };

    static constexpr float SLIDE_TIME   = 0.4f;
    static constexpr float DISPLAY_TIME = 10.0f;
    static constexpr float GAP_TIME     = 0.5f;
    static constexpr int   PADDING      = 16;
    static constexpr int   BORDER_WIDTH = 6;
    static constexpr int   BOX_WIDTH    = 400;

    int winW_, winH_;
    TTF_Font* titleFont_ = nullptr;
    TTF_Font* bodyFont_  = nullptr;

    struct ActivePopup {
        BottomPopUp data;
        Phase       phase;
        float       phaseTime;
    };

    ActivePopup             active_;
    bool                    hasActive_ = false;
    std::deque<BottomPopUp> queue_;
    float                   gapTimer_  = 0.0f;

    SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font,
                            const std::string& text, SDL_Color color,
                            int wrapWidth, int* outW, int* outH);
};
