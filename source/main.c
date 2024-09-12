#include "../SDL2/include/SDL.h"
#include "../SDL2/include/SDL_image.h"
#include "../SDL2/include/SDL_ttf.h"
#include "../SDL2/include/SDL_mixer.h"
#include "../SDL2/include/SDL_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/Game_Config.h"

void Quit(SDL_Window *window, SDL_Renderer *renderer);

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window *window = SDL_CreateWindow("FireZone",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        WINDOW_WIDTH,
                                        WINDOW_HEIGHT,
                                        SDL_WINDOW_SHOWN);

  if (NULL == window)
  {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (NULL == renderer)
  {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  bool running = true;
  SDL_Event event;
  while (running)
  {
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_QUIT:
          running = false;
          break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 112, 110, 9, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    SDL_Delay(1000 / 60);
  }

  Quit(window, renderer);

  return EXIT_SUCCESS;
}

void Quit(SDL_Window *window, SDL_Renderer *renderer)
{
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}