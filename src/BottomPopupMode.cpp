#include "BottomPopupMode.h"
#include <cstdio>
#include <algorithm>
#include <sstream>

BottomPopupMode::BottomPopupMode(int winW, int winH)
    : winW_(winW), winH_(winH)
{
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        return;
    }

    const char* fontPath = "C:/Windows/Fonts/seguiemj.ttf";
    titleFont_ = TTF_OpenFont(fontPath, 24);
    bodyFont_  = TTF_OpenFont(fontPath, 20);

    if (!titleFont_ || !bodyFont_) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    }
}

BottomPopupMode::~BottomPopupMode()
{
    if (titleFont_) TTF_CloseFont(titleFont_);
    if (bodyFont_)  TTF_CloseFont(bodyFont_);
}

void BottomPopupMode::update(float dt)
{
    if (hasActive_) {
        active_.phaseTime += dt;

        switch (active_.phase) {
        case Phase::SlideIn:
            if (active_.phaseTime >= SLIDE_TIME) {
                active_.phase     = Phase::Display;
                active_.phaseTime = 0.0f;
            }
            break;
        case Phase::Display:
            if (active_.phaseTime >= active_.data.totalTime) {
                active_.phase     = Phase::SlideOut;
                active_.phaseTime = 0.0f;
            }
            break;
        case Phase::SlideOut:
            if (active_.phaseTime >= SLIDE_TIME) {
                hasActive_ = false;
                gapTimer_  = GAP_TIME;
            }
            break;
        }
    } else if (!queue_.empty()) {
        gapTimer_ -= dt;
        if (gapTimer_ <= 0.0f) {
            active_    = { queue_.front(), Phase::SlideIn, 0.0f };
            hasActive_ = true;
            queue_.pop_front();
        }
    }
}

static float easeOutCubic(float t) {
    float f = 1.0f - t;
    return 1.0f - f * f * f;
}

static float easeInCubic(float t) {
    return t * t * t;
}

void BottomPopupMode::render(SDL_Renderer* renderer)
{
    if (!titleFont_ || !bodyFont_) return;

    if (!hasActive_) return;

    {
        auto& ap = active_;
        auto& popup = ap.data;

        int textAreaWidth = BOX_WIDTH - BORDER_WIDTH - PADDING * 2;

        int titleW = 0, titleH = 0;
        SDL_Texture* titleTex = renderText(renderer, titleFont_,
            popup.title, { 255, 255, 255, 255 }, textAreaWidth, &titleW, &titleH);

        int bodyW = 0, bodyH = 0;
        SDL_Texture* bodyTex = renderText(renderer, bodyFont_,
            popup.body, { 255, 255, 255, 255 }, textAreaWidth, &bodyW, &bodyH);

        int boxHeight = PADDING + titleH + 8 + bodyH + PADDING;
        int boxX = (winW_ - BOX_WIDTH) / 2;

        // Calculate Y position based on phase
        int fullyVisibleY = winH_ - boxHeight;
        int offScreenY    = winH_;

        float slideProgress = 0.0f;
        switch (ap.phase) {
        case Phase::SlideIn:
            slideProgress = easeOutCubic(std::min(1.0f, ap.phaseTime / SLIDE_TIME));
            break;
        case Phase::Display:
            slideProgress = 1.0f;
            break;
        case Phase::SlideOut:
            slideProgress = 1.0f - easeInCubic(std::min(1.0f, ap.phaseTime / SLIDE_TIME));
            break;
        }

        int boxY = offScreenY + (int)((float)(fullyVisibleY - offScreenY) * slideProgress);

        // Draw background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer,
            popup.bgColor.r, popup.bgColor.g, popup.bgColor.b, 255);
        SDL_Rect bgRect = { boxX, boxY, BOX_WIDTH, boxHeight };
        SDL_RenderFillRect(renderer, &bgRect);

        // Draw left border
        SDL_SetRenderDrawColor(renderer,
            popup.borderColor.r, popup.borderColor.g, popup.borderColor.b, 255);
        SDL_Rect borderRect = { boxX, boxY, BORDER_WIDTH, boxHeight };
        SDL_RenderFillRect(renderer, &borderRect);

        // Draw title
        if (titleTex) {
            SDL_Rect titleRect = {
                boxX + BORDER_WIDTH + PADDING,
                boxY + PADDING,
                titleW, titleH
            };
            SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
            SDL_DestroyTexture(titleTex);
        }

        // Draw body
        if (bodyTex) {
            SDL_Rect bodyRect = {
                boxX + BORDER_WIDTH + PADDING,
                boxY + PADDING + titleH + 8,
                bodyW, bodyH
            };
            SDL_RenderCopy(renderer, bodyTex, nullptr, &bodyRect);
            SDL_DestroyTexture(bodyTex);
        }
    }
}

bool BottomPopupMode::handleCommand(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "bottompopup") return false;

    std::string joined;
    for (size_t i = 1; i < tokens.size(); i++) {
        if (i > 1) joined += ' ';
        joined += tokens[i];
    }

    // Split on '|'
    std::vector<std::string> parts;
    size_t start = 0;
    size_t pos;
    while ((pos = joined.find('|', start)) != std::string::npos) {
        parts.push_back(joined.substr(start, pos - start));
        start = pos + 1;
    }
    parts.push_back(joined.substr(start));

    if (parts.empty() || parts[0].empty()) {
        fprintf(stderr, "bottompopup: missing title\n");
        return true;
    }

    BottomPopUp p;
    p.title     = parts[0];
    p.body      = (parts.size() > 1) ? parts[1] : "";
    p.totalTime = DISPLAY_TIME;
    p.timeLeft  = DISPLAY_TIME;

    // Default colors
    p.borderColor = { 100, 180, 255, 255 };
    p.bgColor     = { 30, 30, 30, 255 };

    // Parse optional border color: r,g,b
    if (parts.size() > 2 && !parts[2].empty()) {
        int r, g, b;
        char comma1, comma2;
        std::istringstream iss(parts[2]);
        if (iss >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',')
            p.borderColor = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
    }

    // Parse optional background color: r,g,b
    if (parts.size() > 3 && !parts[3].empty()) {
        int r, g, b;
        char comma1, comma2;
        std::istringstream iss(parts[3]);
        if (iss >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',')
            p.bgColor = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
    }

    printf("BottomPopup: [%s] %s\n", p.title.c_str(), p.body.c_str());
    if (!hasActive_) {
        active_    = { p, Phase::SlideIn, 0.0f };
        hasActive_ = true;
    } else {
        queue_.push_back(p);
    }
    return true;
}

SDL_Texture* BottomPopupMode::renderText(SDL_Renderer* renderer, TTF_Font* font,
                                          const std::string& text, SDL_Color color,
                                          int wrapWidth, int* outW, int* outH)
{
    if (text.empty()) {
        *outW = 0;
        *outH = 0;
        return nullptr;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(),
                                                           color, wrapWidth);
    if (!surface) return nullptr;

    *outW = surface->w;
    *outH = surface->h;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}
