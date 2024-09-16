#include "../SDL2/include/SDL.h"
#include "../SDL2/include/SDL_image.h"
#include "../SDL2/include/SDL_ttf.h"
#include "../SDL2/include/SDL_mixer.h"
#include "../SDL2/include/SDL_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "../include/Game_Config.h"
#include "../include/SDL_Ui.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *FireZoneTexture = NULL;
TTF_Font *font = NULL;
UILayout *layout = NULL;

void on_button_click()
{
  printf("Button clicked!\n");
}
void onslide(int val)
{
  printf("Slide value: %d\n", val);
}

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
  //TODO: Taha Add more controls to font /bg buttons
  layout = ui_layout_create((WINDOW_WIDTH / 3) + 75, 250, 400, 400, VERTICAL, 10);

  SDL_Color button_color = {255, 255, 255, 255};
  SDL_Color hover_color = {10, 20, 25, 255};

  Button *button1 = button_create(0, 0, 150, 50, "Play", NULL,NULL,button_color, hover_color, font, renderer);
  Button *button2 = button_create(0, 0, 150, 50,"Options", NULL,NULL,button_color, hover_color, font, renderer);
  Button *button3 = button_create(0, 0, 150, 50, "Exit", NULL,NULL,button_color, hover_color, font, renderer);
  TextButton *button4 = text_button_create(50, 0, "test",button_color, hover_color, font, renderer);


  ui_layout_add_child(layout, (UIElement *)button1);
  ui_layout_add_child(layout, (UIElement *)button2);
  ui_layout_add_child(layout, (UIElement *)button3);
  ui_layout_add_child(layout, (UIElement *)button4);


  ui_layout_arrange(layout);

  // Test files impl: Not remove
  //   SDL_Color button_color = {0, 128, 255, 255};
  //    SDL_Color hover_color = {255, 128, 0, 255};
  //    Button *button = button_create(100, 100, 200, 50, "Click Me", NULL, NULL, button_color, hover_color, font, renderer);
  //    button->on_click = on_button_click;

  //   layout = ui_layout_create(50, 50, 700, 500, VERTICAL, 10);
  //   ui_layout_add_child(layout, (UIElement *)button);
  //   ui_layout_arrange(layout);

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

  TTF_Init();

  if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
    SDL_Quit();
    return false;
  }

  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

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

  font = TTF_OpenFont("../assets/fonts/Anton-Regular.ttf", 24);
  assert(font != NULL);

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
    ui_layout_handle_event((UIElement *)layout, &event);
  }
}

void render()
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  renderFireZone();
  ui_layout_draw((UIElement *)layout, renderer);
  SDL_RenderPresent(renderer);
}

void renderFireZone()
{
  SDL_Rect destRect = {WINDOW_WIDTH / 4, 50, WINDOW_WIDTH / 2, 150};
  SDL_RenderCopy(renderer, FireZoneTexture, NULL, &destRect);
}