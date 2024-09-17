#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define TILE_SIZE 16

typedef struct {
    int id;
    int x, y;
    SDL_Texture *texture;
    SDL_Rect rect;
    bool active;
} Player;

typedef enum {
    SCENE_MAIN_MENU,
    SCENE_OPTIONS,
    SCENE_GAMEPLAY
} SceneType;

#endif