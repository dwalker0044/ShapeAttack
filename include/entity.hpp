#ifndef GAME_ENTITY_HPP
#define GAME_ENTITY_HPP

#include "linalg/matrix.hpp"
#include "recthelper.hpp"
#include "screen.h"
#include <SDL2/SDL.h>

struct Entity {
    SDL_Texture* texture;
    float        w, h;

    // State space representation.
    linalg::Matrixf<2, 2> X; // position, velocity.
    linalg::Matrixf<2, 2> Xdot; // velocity, acceleration.
    linalg::Matrixf<2, 2> Y;

    linalg::Matrixf<2, 2> A;
    linalg::Matrixf<2, 2> B;

    float imass;
    float k, b; // friction.
    float restitution;

    void update(float dt, linalg::Matrixf<2, 2> const& u)
    {
        Xdot = (X + (dt * A * X)) + (dt * B * u);
        Y    = X;
        X    = Xdot;
    }
};

struct EntityStatic {
    SDL_Texture* texture;
    SDL_FRect    r;
    float        restitution;
};

//////////////////////////////////////////////////////////////////////////////


auto sdl_rect(Entity& entity)
{
    SDL_FRect rect;

    rect.x = entity.X[0][0];
    rect.y = entity.X[0][1];
    rect.w = entity.w;
    rect.h = entity.h;

    return rect;
}

auto sdl_rect(EntityStatic& entity)
{
    return entity.r;
}

auto sdl_rect_center(Entity& entity)
{
    auto r = sdl_rect(entity);
    return sdl_rect_center(r);
}

//////////////////////////////////////////////////////////////////////////////

auto is_point_in_rect(float x, float y, SDL_FRect& rect)
{
    bool in_x = (x > rect.x) && (x < (rect.x + rect.w));
    bool in_y = (y > rect.y) && (y < (rect.y + rect.h));

    bool result{in_x && in_y};
    return result;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp>
auto generate_points(Tp&& entity)
{
    linalg::Matrixf<4, 2> result;

    auto r = sdl_rect(entity);

    copy_from(result[0], {r.x + 0.f, r.y}); // tl
    copy_from(result[1], {r.x + r.w, r.y}); // tr
    copy_from(result[2], {r.x + r.w, r.y + r.h}); // br
    copy_from(result[3], {r.x + 0.f, r.y + r.h}); // bl

    return result;
}

//////////////////////////////////////////////////////////////////////////////

template <typename Tp>
auto minkowski_boundary(Tp&& entity, linalg::Vectorf<2> const& origin)
{
    auto points = generate_points(entity);

    float x = origin[0];
    float y = origin[1];
    points[0][0] += x;
    points[0][1] += y;
    points[1][0] += 0.f;
    points[1][1] += y;
    points[2][0] += 0.f;
    points[2][1] += 0.f;
    points[3][0] += x;
    points[3][1] += 0.f;

    float w = points[1][0] - points[0][0];
    float h = points[2][1] - points[0][1];

    return SDL_FRect{
        points[0][0],
        points[0][1],
        w,
        h};
}

//////////////////////////////////////////////////////////////////////////////

auto make_food()
{
    EntityStatic food;
    food.r           = {200.f, 100.f, 40.f, 40.f};
    food.restitution = 1.f;
    return food;
}

auto make_wall()
{
    EntityStatic food;
    food.r           = {300.f, 200.f, 40.f, 40.f};
    food.restitution = 1.f;
    return food;
}

auto make_player()
{
    constexpr float mass  = 1.f;
    constexpr float imass = 1.f / mass;
    constexpr float k     = 0.f * imass;
    constexpr float b     = -3.f * imass; // friction coefficient

    Entity player{0};
    player.w           = 80.f;
    player.h           = 80.f;
    player.imass       = imass;
    player.k           = k;
    player.restitution = 0.5;

    player.X[0][0] = 100;
    player.X[0][1] = 100;
    player.X[1][0] = 0;
    player.X[1][1] = 0;

    player.A = {{{0.f, 1.f}, {k, b}}};
    player.B = linalg::Matrixf<2, 2>::I();
    player.B *= 500.f;
    return player;
}

#endif // GAME_ENTITY_HPP