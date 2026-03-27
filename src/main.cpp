#include <SDL.h>
#include <box2d/box2d.h>
#include <cstdlib>
#include <cstdio>

static const int   WIN_W  = 1920;
static const int   WIN_H  = 1080;
static const float PPM    = 100.0f;  // pixels per metre
static const int   BALL_PX = 180;
static const float BALL_R  = (BALL_PX / PPM) * 0.5f;  // radius in metres
static const float WALL_M  = 1.0f;   // wall thickness in metres

// Draw a filled circle using horizontal scan lines
static void fillCircle(SDL_Renderer* renderer, int cx, int cy, int r)
{
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)SDL_sqrt((double)(r * r - dy * dy));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// Create a dynamic ball body with perfect bounce
static b2Body* makeBall(b2World& world, float cx, float cy, float vx, float vy)
{
    b2BodyDef bd;
    bd.type           = b2_dynamicBody;
    bd.position.Set(cx, cy);
    bd.linearVelocity.Set(vx, vy);
    bd.linearDamping  = 0.0f;
    bd.angularDamping = 0.0f;
    b2Body* body = world.CreateBody(&bd);

    b2CircleShape shape;
    shape.m_radius = BALL_R;

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.density     = 1.0f;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
    return body;
}

// Create a static wall body (position is centre, extents are half-widths)
static void makeWall(b2World& world, float cx, float cy, float hw, float hh)
{
    b2BodyDef bd;
    bd.position.Set(cx, cy);
    b2Body* body = world.CreateBody(&bd);

    b2PolygonShape shape;
    shape.SetAsBox(hw, hh);

    b2FixtureDef fd;
    fd.shape       = &shape;
    fd.friction    = 0.0f;
    fd.restitution = 1.0f;
    body->CreateFixture(&fd);
}

int main(int argc, char* argv[])
{
    // --- Box2D world ---
    const float wW = WIN_W / PPM;
    const float wH = WIN_H / PPM;

    b2Vec2  gravity(0.0f, 8.0f);
    b2World world(gravity);

    // Four static walls just outside the play area
    makeWall(world, wW * 0.5f,          -WALL_M * 0.5f,          wW * 0.5f + WALL_M, WALL_M * 0.5f); // top
    makeWall(world, wW * 0.5f,           wH + WALL_M * 0.5f,     wW * 0.5f + WALL_M, WALL_M * 0.5f); // bottom
    makeWall(world, -WALL_M * 0.5f,      wH * 0.5f,              WALL_M * 0.5f, wH * 0.5f + WALL_M); // left
    makeWall(world,  wW + WALL_M * 0.5f, wH * 0.5f,              WALL_M * 0.5f, wH * 0.5f + WALL_M); // right

    // Balls — positions are centres in metres, velocities in m/s
    b2Body* body1 = makeBall(world, 240.0f / PPM, 290.0f / PPM,  2.0f,  1.5f);
    b2Body* body2 = makeBall(world, 560.0f / PPM, 290.0f / PPM, -1.5f,  2.2f);

    // --- SDL ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Stream Overlay",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    bool      running = true;
    SDL_Event event;
    Uint64    prev    = SDL_GetPerformanceCounter();
    double    freq    = (double)SDL_GetPerformanceFrequency();

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        float  dt  = (float)((now - prev) / freq);
        prev = now;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        world.Step(dt, 8, 3);

        // Box2D body position is the centre; convert to pixel centre
        auto toCenterPx = [](b2Body* body, int& cx, int& cy) {
            b2Vec2 pos = body->GetPosition();
            cx = (int)(pos.x * PPM);
            cy = (int)(pos.y * PPM);
        };

        // Clear background to magenta
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderClear(renderer);

        int cx, cy;

        // Ball 1: coral red
        SDL_SetRenderDrawColor(renderer, 220, 80, 60, 255);
        toCenterPx(body1, cx, cy);
        fillCircle(renderer, cx, cy, BALL_PX / 2);

        // Ball 2: steel blue
        SDL_SetRenderDrawColor(renderer, 60, 130, 200, 255);
        toCenterPx(body2, cx, cy);
        fillCircle(renderer, cx, cy, BALL_PX / 2);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
