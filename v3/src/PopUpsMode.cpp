#include "PopUpsMode.h"
#include <print>
#include <algorithm>

PopUpsMode::PopUpsMode(int winW, int winH)
    : winW_(winW), winH_(winH)
{
    if (TTF_Init() == -1) {
        std::println(stderr, "TTF_Init failed: {}", TTF_GetError());
        return;
    }

    const char* fontPath = "C:/Windows/Fonts/seguiemj.ttf";
    titleFont_ = TTF_OpenFont(fontPath, 22);
    bodyFont_  = TTF_OpenFont(fontPath, 16);

    if (!titleFont_ || !bodyFont_) {
        std::println(stderr, "Failed to load font: {}", TTF_GetError());
    }
}

PopUpsMode::~PopUpsMode()
{
    if (titleFont_) TTF_CloseFont(titleFont_);
    if (bodyFont_)  TTF_CloseFont(bodyFont_);
    TTF_Quit();
}

void PopUpsMode::update(float dt)
{
    for (auto& p : popups_)
        p.timeLeft -= dt;

    while (!popups_.empty() && popups_.front().timeLeft <= 0.0f)
        popups_.pop_front();
}

void PopUpsMode::render(SDL_Renderer* renderer)
{
    if (!titleFont_ || !bodyFont_) return;

    int yOffset = MARGIN;

    for (auto& popup : popups_) {
        // Calculate alpha for fade out
        float alpha = 1.0f;
        if (popup.timeLeft < FADE_TIME)
            alpha = std::max(0.0f, popup.timeLeft / FADE_TIME);
        // Lerp colors toward the chroma key (magenta) for fade-out.
        // Color-key transparency is binary, so alpha blending would
        // reveal the magenta background. Instead we move toward it.
        auto lerp = [](Uint8 from, Uint8 to, float t) -> Uint8 {
            return (Uint8)((int)from + (int)((int)to - (int)from) * t);
        };
        float fade = 1.0f - alpha; // 0 = fully visible, 1 = fully magenta
        SDL_Color keyColor = { 255, 0, 255, 255 };

        SDL_Color fadedBg = {
            lerp(popup.bgColor.r, keyColor.r, fade),
            lerp(popup.bgColor.g, keyColor.g, fade),
            lerp(popup.bgColor.b, keyColor.b, fade), 255
        };
        SDL_Color fadedBorder = {
            lerp(popup.borderColor.r, keyColor.r, fade),
            lerp(popup.borderColor.g, keyColor.g, fade),
            lerp(popup.borderColor.b, keyColor.b, fade), 255
        };
        SDL_Color fadedText = {
            lerp(255, keyColor.r, fade),
            lerp(255, keyColor.g, fade),
            lerp(255, keyColor.b, fade), 255
        };

        // Render title and body text to textures
        int textAreaWidth = BOX_WIDTH - BORDER_WIDTH - PADDING * 2;

        int titleW = 0, titleH = 0;
        SDL_Texture* titleTex = renderText(renderer, titleFont_,
            popup.title, fadedText, textAreaWidth, &titleW, &titleH);

        int bodyW = 0, bodyH = 0;
        SDL_Texture* bodyTex = renderText(renderer, bodyFont_,
            popup.body, fadedText, textAreaWidth, &bodyW, &bodyH);

        int boxHeight = PADDING + titleH + 8 + bodyH + PADDING;
        int boxX = winW_ - BOX_WIDTH - MARGIN;
        int boxY = yOffset;

        // Draw background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer,
            fadedBg.r, fadedBg.g, fadedBg.b, 255);
        SDL_Rect bgRect = { boxX, boxY, BOX_WIDTH, boxHeight };
        SDL_RenderFillRect(renderer, &bgRect);

        // Draw left border
        SDL_SetRenderDrawColor(renderer,
            fadedBorder.r, fadedBorder.g, fadedBorder.b, 255);
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

        yOffset += boxHeight + 8;
    }
}

bool PopUpsMode::handleCommand(const std::vector<std::string>& tokens)
{
    if (tokens[0] != "popup") return false;

    // Rejoin everything after "popup" and split on '|'
    // Format: popup <title>|<body>|<border_r,g,b>|<bg_r,g,b>
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
        std::println(stderr, "popup: missing title");
        return true;
    }

    PopUp p;
    p.title     = parts[0];
    p.body      = (parts.size() > 1) ? parts[1] : "";
    p.totalTime = DISPLAY_TIME;
    p.timeLeft  = DISPLAY_TIME;

    // Default colors
    p.borderColor = { 100, 180, 255, 255 };
    p.bgColor     = { 30, 30, 30, 220 };

    // Parse optional border color: r,g,b
    if (parts.size() > 2 && !parts[2].empty()) {
        int r, g, b;
        if (sscanf(parts[2].c_str(), "%d,%d,%d", &r, &g, &b) == 3)
            p.borderColor = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };
    }

    // Parse optional background color: r,g,b
    if (parts.size() > 3 && !parts[3].empty()) {
        int r, g, b;
        if (sscanf(parts[3].c_str(), "%d,%d,%d", &r, &g, &b) == 3)
            p.bgColor = { (Uint8)r, (Uint8)g, (Uint8)b, 220 };
    }

    std::println("Popup: [{}] {}", p.title, p.body);
    popups_.push_back(p);
    return true;
}

SDL_Texture* PopUpsMode::renderText(SDL_Renderer* renderer, TTF_Font* font,
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
