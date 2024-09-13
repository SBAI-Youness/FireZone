#include "../SDL2/include/SDL.h"
#include "../SDL2/include/SDL_image.h"
#include "../SDL2/include/SDL_ttf.h"
#include "../SDL2/include/SDL_mixer.h"
#include "../SDL2/include/SDL_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/Game_Config.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *FireZoneTexture = NULL;

bool initialize();
void quit();
void handleEvents(bool *running);
void render();
void renderFireZone();

int main(int argc, char *argv[])
{
  if (initialize() == false)
  {
    fprintf(stderr, "Failed to initialize!\n");
    return EXIT_FAILURE;
  }

  bool running = true;
  const int FPS = 60;
  const int frameDelay = 1000 / FPS;
  Uint32 frameStart;
  int frameTime;
  while (running)
  {
    frameStart = SDL_GetTicks();
    handleEvents(&running);
    render();
    frameTime = SDL_GetTicks() - frameStart;
    if (frameDelay > frameTime)
      SDL_Delay(frameDelay - frameTime);
  }

  quit();
  return EXIT_SUCCESS;
}

bool initialize()
{
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
    return false;
  }

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
    SDL_Quit();
    return false;
  }

  window = SDL_CreateWindow("FireZone",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            WINDOW_WIDTH,
                            WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN);

  if (window == NULL)
  {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
    IMG_Quit();
    SDL_Quit();
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL)
  {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return false;
  }

  SDL_Surface *surface = IMG_Load("../assets/images/firezone.png");

  if (surface == NULL)
  {
    fprintf(stderr, "IMG_Load Error: %s\n", IMG_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return false;
  }

  FireZoneTexture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  if (FireZoneTexture == NULL)
  {
    fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return false;
  }

  return true;
}

void quit()
{
  SDL_DestroyTexture(FireZoneTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

void handleEvents(bool *running)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        *running = false;
        break;
    }
  }
}

void render()
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  renderFireZone();

  SDL_RenderPresent(renderer);
}

void renderFireZone()
{
  SDL_Rect destRect = {WINDOW_WIDTH / 4, 50, WINDOW_WIDTH / 2, 150};
  SDL_RenderCopy(renderer, FireZoneTexture, NULL, &destRect);
}