#include "../include/SDL_UI.h"
#include <stdlib.h>

void ui_element_init(UIElement *element, int x, int y, int w, int h) {
    element->rect.x = x;
    element->rect.y = y;
    element->rect.w = w;
    element->rect.h = h;
    element->visible = 1;
    element->focusable = 0;
    element->focused = 0;
    element->draw = NULL;
    element->handle_event = NULL;
    element->parent = NULL;
}

void ui_element_draw(UIElement *element, SDL_Renderer *renderer) {
    if (element->visible && element->draw) {
        element->draw(element, renderer);
    }
}

void ui_element_handle_event(UIElement *element, SDL_Event *event) {
    if (element->handle_event) {
        element->handle_event(element, event);
    }
}

void ui_element_set_visible(UIElement *element, int visible) {
    element->visible = visible;
}

void ui_element_set_focused(UIElement *element, int focused) {
    element->focused = focused;
}

UILayout *ui_layout_create(int x, int y, int w, int h, UILayoutOrientation orientation, int padding) {
    UILayout *layout = (UILayout *)malloc(sizeof(UILayout));
    ui_element_init(&layout->element, x, y, w, h);
    layout->children = NULL;
    layout->child_count = 0;
    layout->orientation = orientation;
    layout->padding = padding;
    layout->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))ui_layout_draw;
    layout->element.handle_event = (void (*)(struct UIElement *, SDL_Event *))ui_layout_handle_event;
    return layout;
}

void ui_layout_add_child(UILayout *layout, UIElement *child) {
    layout->child_count++;
    layout->children = (UIElement **)realloc(layout->children, sizeof(UIElement *) * layout->child_count);
    layout->children[layout->child_count - 1] = child;
    child->parent = (UIElement *)layout;
}

void ui_layout_draw(UIElement *element, SDL_Renderer *renderer) {
    UILayout *layout = (UILayout *)element; 
    for (int i = 0; i < layout->child_count; i++) {
        if (layout->children[i]->visible) {
            ui_element_draw(layout->children[i], renderer);
        }
    }
}

void ui_layout_handle_event(UIElement *element, SDL_Event *event) {
    UILayout *layout = (UILayout *)element;  
    for (int i = 0; i < layout->child_count; i++) {
        ui_element_handle_event(layout->children[i], event);
    }
}

void ui_layout_arrange(UILayout *layout) {
    int offset = layout->padding;
    for (int i = 0; i < layout->child_count; i++) {
        if (layout->orientation == HORIZONTAL) {
            layout->children[i]->rect.x = layout->element.rect.x + offset;
            layout->children[i]->rect.y = layout->element.rect.y;
            offset += layout->children[i]->rect.w + layout->padding;
        } else {
            layout->children[i]->rect.x = layout->element.rect.x;
            layout->children[i]->rect.y = layout->element.rect.y + offset;
            offset += layout->children[i]->rect.h + layout->padding;
        }
    }
}

Button *button_create(int x, int y, int w, int h, const char *text, const char *icon_path, const char *sound_path, SDL_Color color, SDL_Color hover_color, TTF_Font *font, SDL_Renderer *renderer) {
    Button *button = (Button *)malloc(sizeof(Button));
    ui_element_init(&button->element, x, y, w, h);
    button->color = color;
    button->hover_color = hover_color;
    button->hovered = 0;
    button->on_click = NULL;

    SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, color);
    button->text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_SetTextureBlendMode(button->text_texture, SDL_BLENDMODE_BLEND);

    if (icon_path) {
        SDL_Surface *icon_surface = IMG_Load(icon_path);
        button->icon_texture = SDL_CreateTextureFromSurface(renderer, icon_surface);
        SDL_FreeSurface(icon_surface);
    } else {
        button->icon_texture = NULL;
    }

    if (sound_path) {
        button->click_sound = Mix_LoadWAV(sound_path);
    } else {
        button->click_sound = NULL;
    }

    button->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))button_draw;
    button->element.handle_event = (void (*)(struct UIElement *, SDL_Event *))button_handle_event;
    return button;
}

void button_draw(UIElement *element, SDL_Renderer *renderer) {
    Button *button = (Button *)element;

    SDL_SetRenderDrawColor(renderer,
        button->hovered ? button->hover_color.r : button->color.r,
        button->hovered ? button->hover_color.g : button->color.g,
        button->hovered ? button->hover_color.b : button->color.b,
        255);
    
    if(!button->icon_texture)
        SDL_RenderFillRect(renderer, &button->element.rect);

    if (button->icon_texture) {
        int icon_w, icon_h;
        SDL_QueryTexture(button->icon_texture, NULL, NULL, &icon_w, &icon_h);
        SDL_Rect icon_rect = { button->element.rect.x , button->element.rect.y + (button->element.rect.h - icon_h) / 2, icon_w, icon_h };
        SDL_RenderCopy(renderer, button->icon_texture, NULL, &icon_rect);
    }

    if (button->text_texture) {
        int text_w, text_h;
        SDL_QueryTexture(button->text_texture, NULL, NULL, &text_w, &text_h);
        SDL_SetTextureColorMod(button->text_texture, button->hovered ? button->color.r : button->hover_color.r,button->hovered ? button->color.g : button->hover_color.g,button->hovered ? button->color.b : button->hover_color.b);
        SDL_Rect dstrect = { button->element.rect.x + (button->element.rect.w - text_w) / 2,
                             button->element.rect.y + (button->element.rect.h - text_h) / 2,
                             text_w, text_h };
        SDL_RenderCopy(renderer, button->text_texture, NULL, &dstrect);
    }
}

void button_handle_event(UIElement *element, SDL_Event *event) {
    Button *button = (Button *)element;
    
    if (event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        int inside = (x > button->element.rect.x) && (x < button->element.rect.x + button->element.rect.w) &&
                     (y > button->element.rect.y) && (y < button->element.rect.y + button->element.rect.h);

        if (inside) {
            button->hovered = 1;
            if (event->type == SDL_MOUSEBUTTONDOWN && button->on_click) {
                if (button->click_sound) Mix_PlayChannel(-1, button->click_sound, 0);
                button->on_click();
            }
        } else {
            button->hovered = 0;
        }
    }
}

TextButton *text_button_create(int x, int y, const char *text, SDL_Color normal_color, SDL_Color hover_color, TTF_Font *font, SDL_Renderer *renderer) {
    TextButton *button = (TextButton *)malloc(sizeof(TextButton));
    ui_element_init(&button->element, x, y, 0, 0); 
    button->normal_color = normal_color;
    button->hover_color = hover_color;
    button->hovered = 0;
    button->on_click = NULL;

    SDL_Surface *text_surface = TTF_RenderText_Blended(font, text, normal_color);
    button->text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_SetTextureBlendMode(button->text_texture, SDL_BLENDMODE_BLEND);

    int text_w, text_h;
    SDL_QueryTexture(button->text_texture, NULL, NULL, &text_w, &text_h);
    button->element.rect.w = text_w;
    button->element.rect.h = text_h;

    button->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))text_button_draw;
    button->element.handle_event = (void (*)(struct UIElement *, SDL_Event *))text_button_handle_event;

    return button;
}

void text_button_draw(UIElement *element, SDL_Renderer *renderer) {
    TextButton *button = (TextButton *)element;

    SDL_Color color = button->hovered ? button->hover_color : button->normal_color;

    SDL_SetTextureColorMod(button->text_texture, color.r, color.g, color.b);
    
    SDL_Rect dstrect = { button->element.rect.x, button->element.rect.y, button->element.rect.w, button->element.rect.h };
    SDL_RenderCopy(renderer, button->text_texture, NULL, &dstrect);
}

void text_button_handle_event(UIElement *element, SDL_Event *event) {
    TextButton *button = (TextButton *)element;

    if (event->type == SDL_MOUSEMOTION || event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
        int x, y;
        SDL_GetMouseState(&x, &y);

        int inside = (x > button->element.rect.x) && (x < button->element.rect.x + button->element.rect.w) &&
                     (y > button->element.rect.y) && (y < button->element.rect.y + button->element.rect.h);

        if (inside) {
            button->hovered = 1;
            if (event->type == SDL_MOUSEBUTTONDOWN && button->on_click) {
                button->on_click();
            }
        } else {
            button->hovered = 0;
        }
    }
}

Slider *slider_create(int x, int y, int w, int h, int min_value, int max_value, SDL_Color slider_color, SDL_Color knob_color, SDL_Renderer *renderer) {
    Slider *slider = (Slider *)malloc(sizeof(Slider));
    ui_element_init(&slider->element, x, y, w, h);
    slider->min_value = min_value;
    slider->max_value = max_value;
    slider->current_value = min_value;
    slider->slider_color = slider_color;
    slider->knob_color = knob_color;
    slider->on_slide = NULL;

    slider->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))slider_draw;
    slider->element.handle_event = (void (*)(struct UIElement *, SDL_Event *))slider_handle_event;

    return slider;
}

void slider_draw(UIElement *element, SDL_Renderer *renderer) {
    Slider *slider = (Slider *)element;

    SDL_SetRenderDrawColor(renderer, slider->slider_color.r, slider->slider_color.g, slider->slider_color.b, 255);
    SDL_RenderFillRect(renderer, &slider->element.rect);

    int knob_width = slider->element.rect.w / 10;
    int knob_position = (slider->current_value - slider->min_value) * (slider->element.rect.w - knob_width) / (slider->max_value - slider->min_value);
    SDL_Rect knob_rect = { slider->element.rect.x + knob_position, slider->element.rect.y, knob_width, slider->element.rect.h };
    SDL_SetRenderDrawColor(renderer, slider->knob_color.r, slider->knob_color.g, slider->knob_color.b, 255);
    SDL_RenderFillRect(renderer, &knob_rect);
}

void slider_handle_event(UIElement *element, SDL_Event *event) {
    Slider *slider = (Slider *)element;

    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEMOTION) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (x >= slider->element.rect.x && x <= slider->element.rect.x + slider->element.rect.w &&
            y >= slider->element.rect.y && y <= slider->element.rect.y + slider->element.rect.h) {
            int knob_width = slider->element.rect.w / 10;
            slider->current_value = slider->min_value + (x - slider->element.rect.x) * (slider->max_value - slider->min_value) / (slider->element.rect.w - knob_width);
            if (slider->on_slide) {
                slider->on_slide(slider->current_value);
            }
        }
    }
}

ProgressBar *progress_bar_create(int x, int y, int w, int h, int max_value, SDL_Color bg_color, SDL_Color fill_color, SDL_Texture *fill_texture) {
    ProgressBar *progress_bar = (ProgressBar *)malloc(sizeof(ProgressBar));
    ui_element_init(&progress_bar->element, x, y, w, h);
    progress_bar->max_value = max_value;
    progress_bar->current_value = 0;
    progress_bar->bg_color = bg_color;
    progress_bar->fill_texture = fill_texture;

    SDL_QueryTexture(fill_texture, NULL, NULL, &progress_bar->texture_rect.w, &progress_bar->texture_rect.h);
    progress_bar->texture_rect.x = 0;
    progress_bar->texture_rect.y = 0;

    progress_bar->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))progress_bar_draw;
    return progress_bar;
}


void progress_bar_draw(UIElement *element, SDL_Renderer *renderer) {
    ProgressBar *progress_bar = (ProgressBar *)element;

    SDL_SetRenderDrawColor(renderer, progress_bar->bg_color.r, progress_bar->bg_color.g, progress_bar->bg_color.b, 255);
    SDL_RenderFillRect(renderer, &progress_bar->element.rect);

    int fill_width = progress_bar->current_value * progress_bar->element.rect.w / progress_bar->max_value;
    SDL_Rect fill_rect = { progress_bar->element.rect.x, progress_bar->element.rect.y, fill_width, progress_bar->element.rect.h };

    if (progress_bar->fill_texture != NULL) {
        SDL_Rect src_rect = { 0, 0, fill_width, progress_bar->texture_rect.h };
        SDL_RenderCopy(renderer, progress_bar->fill_texture, &src_rect, &fill_rect);
    } else {
        SDL_SetRenderDrawColor(renderer, progress_bar->fill_color.r, progress_bar->fill_color.g, progress_bar->fill_color.b, 255);
        SDL_RenderFillRect(renderer, &fill_rect);
    }
}


void progress_bar_set_value(ProgressBar *progress_bar, int value) {
    progress_bar->current_value = value;
    if (progress_bar->current_value > progress_bar->max_value) {
        progress_bar->current_value = progress_bar->max_value;
    }
}

Tooltip *tooltip_create(int x, int y, int w, int h, const char *text, SDL_Color bg_color, SDL_Color text_color, TTF_Font *font, SDL_Renderer *renderer) {
    Tooltip *tooltip = (Tooltip *)malloc(sizeof(Tooltip));
    ui_element_init(&tooltip->element, x, y, w, h);
    tooltip->text = text;
    tooltip->bg_color = bg_color;
    tooltip->text_color = text_color;
    tooltip->visible = 0;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, text_color);
    tooltip->text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    tooltip->element.draw = (void (*)(struct UIElement *, SDL_Renderer *))tooltip_draw;

    return tooltip;
}

void tooltip_draw(UIElement *element, SDL_Renderer *renderer) {
    Tooltip *tooltip = (Tooltip *)element;

    if (!tooltip->visible) {
        return;
    }

    SDL_SetRenderDrawColor(renderer, tooltip->bg_color.r, tooltip->bg_color.g, tooltip->bg_color.b, 255);
    SDL_RenderFillRect(renderer, &tooltip->element.rect);

    int text_w, text_h;
    SDL_QueryTexture(tooltip->text_texture, NULL, NULL, &text_w, &text_h);
    SDL_Rect dstrect = { tooltip->element.rect.x + 5, tooltip->element.rect.y + 5, text_w, text_h };
    SDL_RenderCopy(renderer, tooltip->text_texture, NULL, &dstrect);
}

void tooltip_set_position(Tooltip *tooltip, int x, int y) {
    tooltip->element.rect.x = x;
    tooltip->element.rect.y = y;
}

void tooltip_set_visible(Tooltip *tooltip, int visible) {
    tooltip->visible = visible;
}