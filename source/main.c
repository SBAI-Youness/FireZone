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

#define MAX_PLAYERS 4

SceneType current_scene = SCENE_MAIN_MENU;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *FireZoneTexture = NULL;
SDL_Texture *brickTexture = NULL;
TTF_Font *font = NULL;
UILayout *layout = NULL;

Player players[MAX_PLAYERS];
int local_player_id = 0;

// settings
TCPsocket server_socket = NULL, client_socket = NULL;
TCPsocket client_sockets[MAX_PLAYERS - 1] = {NULL};
int num_clients = 0;
bool is_server = false;
bool is_connected = false;

void ChangeToGameScene()
{
  current_scene = SCENE_GAMEPLAY;
}
void ChangeToOptionScene()
{
  current_scene = SCENE_OPTIONS;
}

void onslide(int val)
{
  printf("Slide value: %d\n", val);
}

bool loadTextures();

bool initialize();
void quit();
void handleEvents(bool *running);
void render();
void renderFireZone();

void handlePlayerMovement();
bool loadPlayer();
void renderPlayer(Player *p);
void renderTerrain();
void start_server(int port);
void start_client(const char *host, int port);
void sync_player_position();
void process_network_data();

int main(int argc, char *argv[])
{
  if (!initialize())
  {
    fprintf(stderr, "Failed to initialize!\n");
    return EXIT_FAILURE;
  }

  if (argc == 2 && strcmp(argv[1], "server") == 0)
  {
    start_server(12345);
  }
  else if (argc == 3 && strcmp(argv[1], "client") == 0)
  {
    start_client(argv[2], 12345);
  }

  bool running = true;
  const int FPS = 60;
  const int frameDelay = 1000 / FPS;
  Uint32 frameStart;
  int frameTime;
  // TODO: Taha Add more controls to font /bg buttons
  layout = ui_layout_create((WINDOW_WIDTH / 3) + 75, 250, 400, 400, VERTICAL, 40);

  SDL_Color button_color = {255, 255, 255, 255};
  SDL_Color hover_color = {10, 20, 25, 255};

  Button *playButton = button_create(0, 0, 150, 50, "Play", "../assets/images/uitest.png", NULL, button_color, hover_color, font, renderer);
  Button *optionButton = button_create(0, 0, 150, 50, "Options", "../assets/images/uitest.png", NULL, button_color, hover_color, font, renderer);
  Button *quitButton = button_create(0, 0, 150, 50, "Exit", "../assets/images/uitest.png", NULL, button_color, hover_color, font, renderer);
  playButton->on_click = ChangeToGameScene;
  optionButton->on_click = ChangeToOptionScene;

  ui_layout_add_child(layout, (UIElement *)playButton);
  ui_layout_add_child(layout, (UIElement *)optionButton);
  ui_layout_add_child(layout, (UIElement *)quitButton);

  ui_layout_arrange(layout);

  if (!loadPlayer())
  {
    fprintf(stderr, "Failed to load player image!\n");
    quit();
    return EXIT_FAILURE;
  }

  assert(loadTextures());
  while (running)
  {
    frameStart = SDL_GetTicks();

    handleEvents(&running);
    if (current_scene == SCENE_GAMEPLAY)
    {
      process_network_data(); // Handle networking data (move player, sync positions, etc.)
      sync_player_position(); // Sync player position over the network
      handlePlayerMovement();
    }

    render();

    frameTime = SDL_GetTicks() - frameStart;
    if (frameDelay > frameTime)
      SDL_Delay(frameDelay - frameTime);
  }

  quit();
  return EXIT_SUCCESS;
}

bool loadTextures() {
    SDL_Surface *brickSurface = IMG_Load("../assets/images/brick.png");
    if (!brickSurface) {
        printf("Failed to load brick texture: %s\n", IMG_GetError());
        return false;
    }
    brickTexture = SDL_CreateTextureFromSurface(renderer, brickSurface);
    SDL_FreeSurface(brickSurface);

    SDL_SetTextureBlendMode(brickTexture, SDL_BLENDMODE_BLEND);

    if (!brickTexture) {
        printf("Failed to create texture from surface: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

bool initialize()
{
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0 || SDLNet_Init() < 0)
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
  SDLNet_Quit();
  SDL_Quit();
}

void handleEvents(bool *running)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
      *running = false;
    switch (current_scene)
    {
    case SCENE_MAIN_MENU:
      ui_layout_handle_event((UIElement *)layout, &event);
      break;
    case SCENE_GAMEPLAY:
      
      break;
    }
  }
}

void render()
{
  switch (current_scene)
  {
  case SCENE_MAIN_MENU:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderFireZone();
    ui_layout_draw((UIElement *)layout, renderer);
    break;
  case SCENE_GAMEPLAY:
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderTerrain();
    
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
      if (players[i].active)
      {
        renderPlayer(&players[i]);
      }
    }
    break;
  }
  SDL_RenderPresent(renderer);
}

void renderFireZone()
{
  SDL_Rect destRect = {WINDOW_WIDTH / 4, 50, WINDOW_WIDTH / 2, 150};
  SDL_RenderCopy(renderer, FireZoneTexture, NULL, &destRect);
}

bool loadPlayer()
{
  SDL_Surface *playerSurface = IMG_Load("../assets/images/ghost.png");
  if (!playerSurface)
  {
    printf("Failed to load player image: %s\n", IMG_GetError());
    return false;
  }

  for (int i = 0; i < MAX_PLAYERS; i++)
  {
    players[i].texture = SDL_CreateTextureFromSurface(renderer, playerSurface);
    players[i].rect.w = 32;
    players[i].rect.h = 32;
    players[i].x = WINDOW_WIDTH / 2 - players[i].rect.w / 2;
    players[i].y = WINDOW_HEIGHT / 2 - players[i].rect.h / 2;
    players[i].rect.x = players[i].x;
    players[i].rect.y = players[i].y;
    players[i].active = false;
  }
  players[local_player_id].active = true;
  SDL_FreeSurface(playerSurface);

  return true;
}

void renderPlayer(Player *p)
{
  p->rect.x = p->x;
  p->rect.y = p->y;
  SDL_RenderCopy(renderer, p->texture, NULL, &p->rect);
}

void handlePlayerMovement()
{
  const Uint8 *state = SDL_GetKeyboardState(NULL);  
    const int baseSpeed = 5;
    int playerSpeed = baseSpeed;
    const int mapPixelWidth = MAP_WIDTH * TILE_SIZE * 2;
    const int mapPixelHeight = MAP_HEIGHT * TILE_SIZE * 2;
    bool movedX = false, movedY = false;  

    
    if (state[SDL_SCANCODE_W] && players[local_player_id].y > 0) {
        players[local_player_id].y -= playerSpeed;
        movedY = true;
    }

    if (state[SDL_SCANCODE_S] && (players[local_player_id].y + players[local_player_id].rect.h) < mapPixelHeight) {
        players[local_player_id].y += playerSpeed;
        movedY = true;
    }

    if (state[SDL_SCANCODE_A] && players[local_player_id].x > 0) {
        players[local_player_id].x -= playerSpeed;
        movedX = true;
    }

    if (state[SDL_SCANCODE_D] && (players[local_player_id].x + players[local_player_id].rect.w) < mapPixelWidth) {
        players[local_player_id].x += playerSpeed;
        movedX = true;
    }

     if (movedX && movedY) {
        playerSpeed = baseSpeed / 1.414;  
    }

    if ((movedX || movedY) && is_connected)
    {
        char buffer[256];
        sprintf(buffer, "MOVE %d %d %d", local_player_id, players[local_player_id].x, players[local_player_id].y);
        SDLNet_TCP_Send(client_socket, buffer, strlen(buffer) + 1);  // Send updated position to the server
    }
}

// Server to host the game
void start_server(int port)
{
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  server_socket = SDLNet_TCP_Open(&ip);
  if (!server_socket)
  {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return;
  }

  printf("Server started on port %d\n", port);
  is_server = true;
  num_clients = 0; // No clients initially

  players[0].id = 0; // Server player is always ID 0
  players[0].active = true;
}

// Client to join a game
void start_client(const char *host, int port)
{
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, host, port) == -1)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return;
  }

  client_socket = SDLNet_TCP_Open(&ip);
  if (!client_socket)
  {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    return;
  }

  printf("Connected to server at %s:%d\n", host, port);
  is_connected = true;

  // Receive the assigned ID from the server
  char buffer[256];
  int len = SDLNet_TCP_Recv(client_socket, buffer, 256);
  if (len > 0)
  {
    buffer[len] = '\0';
    int assigned_id;
    if (sscanf(buffer, "ID %d", &assigned_id) == 1)
    {
      local_player_id = assigned_id;
      players[local_player_id].active = true;
      printf("Assigned ID: %d\n", local_player_id);
    }
  }
}

// Sync player position between clients and server
void sync_player_position()
{
  if (is_server)
  {
    char buffer[256];
    sprintf(buffer, "MOVE %d %d %d", players[local_player_id].id, players[local_player_id].x, players[local_player_id].y);

    // Send updated position to all clients
    for (int i = 0; i < num_clients; i++)
    {
      SDLNet_TCP_Send(client_sockets[i], buffer, strlen(buffer) + 1);
    }
    printf("Server broadcasted position: Player %d (%d, %d)\n", players[local_player_id].id, players[local_player_id].x, players[local_player_id].y);
  }
}

// Process network data
void process_network_data()
{
  if (is_server)
  {
    TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);
    if (client_socket && num_clients < MAX_PLAYERS - 1)
    {
      client_sockets[num_clients] = client_socket;
      players[num_clients + 1].id = num_clients + 1; // Assign client a unique ID
      players[num_clients + 1].active = true;
      num_clients++;

      // Send the ID to the client
      char buffer[256];
      sprintf(buffer, "ID %d", players[num_clients].id);
      SDLNet_TCP_Send(client_sockets[num_clients - 1], buffer, strlen(buffer) + 1);

      // Send the new client the positions of all existing players (including host)
      for (int i = 0; i < num_clients; i++)
      {
        if (players[i].active)
        {
          sprintf(buffer, "SYNC %d %d %d", players[i].id, players[i].x, players[i].y);
          SDLNet_TCP_Send(client_sockets[num_clients - 1], buffer, strlen(buffer) + 1);
          printf("Sent SYNC to new client: Player %d at position (%d, %d)\n", players[i].id, players[i].x, players[i].y);
          _sleep(100);
        }
      }

      // Notify all existing clients of the new player (broadcast new player to all)
      sprintf(buffer, "SYNC %d %d %d", players[num_clients].id, players[num_clients].x, players[num_clients].y);
      for (int i = 0; i < num_clients - 1; i++)
      {
        SDLNet_TCP_Send(client_sockets[i], buffer, strlen(buffer) + 1);
      }

      printf("Client %d connected with ID %d\n", num_clients, players[num_clients].id);
    }

    // Non-blocking check for client data
    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(num_clients);
    for (int i = 0; i < num_clients; i++)
    {
      SDLNet_TCP_AddSocket(socket_set, client_sockets[i]);
    }
    int num_ready_sockets = SDLNet_CheckSockets(socket_set, 0); // Non-blocking check
    if (num_ready_sockets > 0)
    {
      for (int i = 0; i < num_clients; i++)
      {
        if (SDLNet_SocketReady(client_sockets[i]))
        {
          char buffer[256];
          int len = SDLNet_TCP_Recv(client_sockets[i], buffer, 256);
          if (len > 0)
          {
            buffer[len] = '\0';
            int id, x, y;
            if (sscanf(buffer, "MOVE %d %d %d", &id, &x, &y) == 3)
            {
              players[id].x = x;
              players[id].y = y;
              printf("Received from client %d: Player %d moved to (%d, %d)\n", i, id, x, y);
              // Broadcast this movement to all clients
              for (int j = 0; j < num_clients; j++)
              {
                if (j != i)
                { // Don't send it back to the client that sent it
                  SDLNet_TCP_Send(client_sockets[j], buffer, strlen(buffer) + 1);
                }
              }
            }
          }
        }
      }
    }
    SDLNet_FreeSocketSet(socket_set);
  }
  else if (is_connected)
  {
    // Non-blocking check for server data
    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(1);
    SDLNet_TCP_AddSocket(socket_set, client_socket);

    int num_ready_sockets = SDLNet_CheckSockets(socket_set, 0); // Non-blocking check
    if (num_ready_sockets > 0)
    {
      if (SDLNet_SocketReady(client_socket))
      {
        char buffer[256];
        int len = SDLNet_TCP_Recv(client_socket, buffer, 256);
        if (len > 0)
        {
          buffer[len] = '\0';
          int id, x, y;
          if (sscanf(buffer, "MOVE %d %d %d", &id, &x, &y) == 3)
          {
            players[id].x = x;
            players[id].y = y;
          }
          else if (sscanf(buffer, "SYNC %d %d %d", &id, &x, &y) == 3)
          {
            players[id].x = x;
            players[id].y = y;
            players[id].active = true; // Activate the player
            printf("Synced player %d to position (%d, %d)\n", id, x, y);
          }
        }
      }
    }
    SDLNet_FreeSocketSet(socket_set);
  }
}

void renderTerrain() {
    SDL_Rect tileRect = {0, 0, TILE_SIZE * 2, TILE_SIZE * 2};
    

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
          SDL_SetTextureColorMod(brickTexture,255,x*y,x-y);
            tileRect.x = x * TILE_SIZE * 2;
            tileRect.y = y * TILE_SIZE * 2;
            SDL_RenderCopy(renderer, brickTexture, NULL, &tileRect);
        }
    }
}