#ifndef SDL_UI_H
#define SDL_UI_H
#include "../SDL2/include/SDL.h"
#include "../SDL2/include/SDL_image.h"
#include "../SDL2/include/SDL_ttf.h"
#include "../SDL2/include/SDL_mixer.h"

typedef enum { HORIZONTAL, VERTICAL } UILayoutOrientation;

typedef struct UIElement {
    SDL_Rect rect;
    int visible;
    int focusable;
    int focused;
    void (*draw)(struct UIElement *, SDL_Renderer *);
    void (*handle_event)(struct UIElement *, SDL_Event *);
    struct UIElement *parent;
} UIElement;

void ui_element_init(UIElement *element, int x, int y, int w, int h);
void ui_element_draw(UIElement *element, SDL_Renderer *renderer);
void ui_element_handle_event(UIElement *element, SDL_Event *event);
void ui_element_set_visible(UIElement *element, int visible);
void ui_element_set_focused(UIElement *element, int focused);

typedef struct UILayout {
    UIElement element;
    UIElement **children;
    int child_count;
    UILayoutOrientation orientation;
    int padding;
} UILayout;

UILayout *ui_layout_create(int x, int y, int w, int h, UILayoutOrientation orientation, int padding);
void ui_layout_add_child(UILayout *layout, UIElement *child);
void ui_layout_draw(UIElement *element, SDL_Renderer *renderer);  
void ui_layout_handle_event(UIElement *element, SDL_Event *event);  
void ui_layout_arrange(UILayout *layout);

typedef struct Button {
    UIElement element;
    SDL_Color color;
    SDL_Color hover_color;
    SDL_Texture *text_texture;
    SDL_Texture *icon_texture;
    Mix_Chunk *click_sound;
    int hovered;
    void (*on_click)(void);
} Button;

Button *button_create(int x, int y, int w, int h, const char *text, const char *icon_path, const char *sound_path, SDL_Color color, SDL_Color hover_color, TTF_Font *font, SDL_Renderer *renderer);
void button_draw(UIElement *element, SDL_Renderer *renderer);
void button_handle_event(UIElement *element, SDL_Event *event);

typedef struct TextButton {
    UIElement element;
    SDL_Color normal_color;
    SDL_Color hover_color;
    SDL_Texture *text_texture;
    int hovered;
    void (*on_click)(void);
} TextButton;

TextButton *text_button_create(int x, int y, const char *text, SDL_Color normal_color, SDL_Color hover_color, TTF_Font *font, SDL_Renderer *renderer);
void text_button_draw(UIElement *element, SDL_Renderer *renderer);
void text_button_handle_event(UIElement *element, SDL_Event *event);

typedef struct Slider {
    UIElement element;
    int min_value;
    int max_value;
    int current_value;
    SDL_Color slider_color;
    SDL_Color knob_color;
    void (*on_slide)(int value); 
} Slider;

Slider *slider_create(int x, int y, int w, int h, int min_value, int max_value, SDL_Color slider_color, SDL_Color knob_color, SDL_Renderer *renderer);
void slider_draw(UIElement *element, SDL_Renderer *renderer);
void slider_handle_event(UIElement *element, SDL_Event *event);

typedef struct ProgressBar {
    UIElement element;
    int max_value;
    int current_value;
    SDL_Color bg_color;             
    SDL_Color fill_color;        
    SDL_Texture *fill_texture;
    SDL_Rect texture_rect;
} ProgressBar;


ProgressBar *progress_bar_create(int x, int y, int w, int h, int max_value, SDL_Color bg_color, SDL_Color fill_color, SDL_Texture *fill_texture);
void progress_bar_draw(UIElement *element, SDL_Renderer *renderer);
void progress_bar_set_value(ProgressBar *progress_bar, int value);

typedef struct Tooltip {
    UIElement element;
    const char *text;
    SDL_Color bg_color;
    SDL_Color text_color;
    SDL_Texture *text_texture;
    int visible;
} Tooltip;

Tooltip *tooltip_create(int x, int y, int w, int h, const char *text, SDL_Color bg_color, SDL_Color text_color, TTF_Font *font, SDL_Renderer *renderer);
void tooltip_draw(UIElement *element, SDL_Renderer *renderer);
void tooltip_set_position(Tooltip *tooltip, int x, int y);
void tooltip_set_visible(Tooltip *tooltip, int visible);

#endif
