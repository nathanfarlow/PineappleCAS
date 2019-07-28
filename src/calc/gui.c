#ifndef COMPILE_PC

#include "gui.h"

#include <tice.h>
#include <graphx.h>
#include <keypadc.h>

void draw_string_centered(char *text, int x, int y) {
    unsigned len = gfx_GetStringWidth(text);

    gfx_SetTextBGColor(COLOR_TRANSPARENT);
    gfx_SetTextFGColor(COLOR_TEXT);

    gfx_PrintStringXY(text, x - len / 2, y);
}

void draw_background() {

    gfx_SetMonospaceFont(0);

    gfx_FillScreen(COLOR_BACKGROUND);

    gfx_SetTextTransparentColor(COLOR_TRANSPARENT);
    gfx_SetTextBGColor(COLOR_TRANSPARENT);
    gfx_SetTextFGColor(COLOR_TEXT);

    /*Outer border*/
    gfx_SetColor(COLOR_BLUE);
    gfx_Rectangle(1, 1, LCD_WIDTH - 2, LCD_HEIGHT - 2);
    gfx_HorizLine(1, LCD_HEIGHT - 21, LCD_WIDTH - 2);

    draw_string_centered("PineappleCAS v1.0 by Nathan Farlow", LCD_WIDTH / 2, LCD_HEIGHT - 14);

    draw_string_centered("Input", LCD_WIDTH / 4 + 20, 14);
    draw_string_centered("Output", LCD_WIDTH / 4 * 3 - 20, 14);

    /*Outer selection border*/
    gfx_Rectangle(10, 50, LCD_WIDTH - 20, 160);

    /*Function rectangle*/
    gfx_Rectangle(10 + 2, 50 + 2, 100, 160 - 4);
    draw_string_centered("Function", 10 + 2 + 100 / 2, 50 + 10);

    draw_string_centered("Options", 10 + 2 + 100 + (LCD_WIDTH - 10 - 10 - 2 - 100) / 2, 50 + 10);
}

typedef enum {
    GUI_LABEL,
    GUI_CHECKBOX,
    GUI_TEXTBOX,
    GUI_DROPDOWN,
    GUI_BUTTON
} GuiType;

/*I miss oop so much*/
typedef struct {

    GuiType type;

    int x, y, w, h;
    bool active;

    char *text;

    /*For checkboxes*/
    bool checked;
    /*For dropdowns*/
    unsigned index;

    /*For dropdowns*/
    unsigned selected_index;
} view_t;

#define NUM_DROPDOWN_ENTRIES 21
char *dropdown_entries[NUM_DROPDOWN_ENTRIES] = {
    "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y0",
    "Str1", "Str2", "Str3", "Str4", "Str4", "Str6", "Str7", "Str8", "Str9", "Str0",
    "Ans"
};


#define NUM_IO 2
#define NUM_FUNCTION 5
#define NUM_SIMPLIFY 7
#define NUM_EVALUATE 4
#define NUM_EXPAND 3
#define NUM_DERIVATIVE 1
#define NUM_HELP 0

view_t *io_context[NUM_IO];
view_t *function_context[NUM_FUNCTION];
view_t *simplify_context[NUM_SIMPLIFY];
view_t *evaluate_context[NUM_EVALUATE];
view_t *expand_context[NUM_EXPAND];
view_t *derivative_context[NUM_DERIVATIVE];
view_t *help_context[1];

view_t *button_simplify;
view_t *button_evaluate;
view_t *button_expand;
view_t *button_derivative;

typedef enum {
    CONTEXT_IO,
    CONTEXT_FUNCTION,
    CONTEXT_SIMPLIFY,
    CONTEXT_EVALUATE,
    CONTEXT_EXPAND,
    CONTEXT_DERIVATIVE,
    CONTEXT_HELP,
    NUM_CONTEXTS
} Context;

unsigned elements_in_context[NUM_CONTEXTS] = {
    NUM_IO,
    NUM_FUNCTION,
    NUM_SIMPLIFY,
    NUM_EVALUATE,
    NUM_EXPAND,
    NUM_DERIVATIVE,
    NUM_HELP
};


view_t **context_lookup[NUM_CONTEXTS] = {
    io_context, function_context, simplify_context,
    evaluate_context, expand_context, derivative_context,
    help_context
};

Context current_context = CONTEXT_FUNCTION;
unsigned active_index = 0;
unsigned function_index = 0;


void draw_label(view_t *v) {
    gfx_PrintStringXY(v->text, v->x, v->y + v->h / 2 - TEXT_HEIGHT / 2);
}

void draw_checkbox(view_t *v) {
    gfx_SetColor(COLOR_PURPLE);
    gfx_Rectangle(v->x, v->y, v->w, v->h);

    if(v->checked) {
        gfx_FillRectangle(v->x + 2, v->y + 2, v->w - 4, v->h - 4);
    }

    if(v->text != NULL) {
        gfx_PrintStringXY(v->text, v->x + v->w + 4, v->y + v->h / 2 - TEXT_HEIGHT / 2);
    }
}

void draw_dropdown(view_t *v) {
    gfx_SetColor(COLOR_PURPLE);
    gfx_Rectangle(v->x, v->y, v->w, v->h);
    draw_string_centered(dropdown_entries[v->index], v->x + v->w / 2, v->y + v->h / 2 - TEXT_HEIGHT / 2);
}

void draw_button(view_t *v) {
    gfx_SetColor(COLOR_PURPLE);
    gfx_Rectangle(v->x, v->y, v->w, v->h);
    draw_string_centered(v->text, v->x + v->w / 2, v->y + v->h / 2 - TEXT_HEIGHT / 2);
}

void view_draw(view_t *v) {

    gfx_SetTextBGColor(COLOR_TRANSPARENT);
    gfx_SetTextFGColor(COLOR_TEXT);

    switch(v->type) {
    case GUI_LABEL:     draw_label(v);      break;
    case GUI_CHECKBOX:  draw_checkbox(v);   break;
    case GUI_DROPDOWN:  draw_dropdown(v);   break;
    case GUI_BUTTON:    draw_button(v);     break;
    default: return;
    }

    if(v->active) {
        gfx_SetTextFGColor(COLOR_PURPLE);
        gfx_PrintStringXY(">", v->x - 10, v->y + v->h / 2 - TEXT_HEIGHT / 2);
    }
}


view_t *view_create(GuiType type, int x, int y, int w, int h, char *text) {
    view_t *v = calloc(1, sizeof(view_t));
    v->type = type;
    v->x = x;
    v->y = y;
    v->w = w;
    v->h = h;
    v->text = text;
    return v;
}

view_t *view_create_checkbox(int x, int y, char *text, bool checked) {
    view_t *v = view_create(GUI_CHECKBOX, x, y, 8, 8, text);
    v->checked = checked;
    return v;
}

view_t *view_create_button(int x, int y, char *text) {
    int width = gfx_GetStringWidth(text);
    return view_create(GUI_BUTTON, x - width / 2, y - TEXT_HEIGHT / 2, width + 8, 20, text);
}

view_t *view_create_dropdown(int x, int y, unsigned index) {
    view_t *v = view_create(GUI_DROPDOWN, x, y, 50, 20, NULL);
    v->index = index;
    return v;
}

view_t *view_create_label(int x, int y, char *text) {
    return view_create(GUI_LABEL, x, y, 0, 8, text);
}


void draw_context(Context c) {
    unsigned i;
    
    gfx_SetColor(COLOR_BACKGROUND);

    switch(c) {
        case CONTEXT_IO:
            gfx_FillRectangle(5, 14 + 8, LCD_WIDTH - 10, 26);
            break;
        case CONTEXT_FUNCTION:
            gfx_FillRectangle(10 + 4, 70, 100 - 4, 120);
            break;
        default:
            gfx_FillRectangle(112, 70, 195, 138);
            break;
    }

    if(c == CONTEXT_EVALUATE) {
        gfx_PrintStringXY("From: ", 124 + 25, 96 + 10 - TEXT_HEIGHT / 2);
        gfx_PrintStringXY("To: ", 124 + 25, 96 + 24 + 10 - TEXT_HEIGHT / 2);
    } else if(c == CONTEXT_HELP) {
        gfx_SetTextFGColor(COLOR_TEXT);
        gfx_PrintStringXY("View https://github.com/", 115, 80 + 10 * 0);
        gfx_PrintStringXY("nathanfarlow/PineappleCAS", 115, 80 + 10 * 1);
        gfx_PrintStringXY("for usage instructions.", 115, 80 + 10 * 2);

        gfx_PrintStringXY("PineappleCAS uses the imath", 115, 80 + 10 * 4);
        gfx_PrintStringXY("library by Michael J.", 115, 80 + 10 * 5);
        gfx_PrintStringXY("Fromberger.", 115, 80 + 10 * 6);
        
        gfx_PrintStringXY("Thanks Mateo and Andriweb", 115, 80 + 10 * 8);
        gfx_PrintStringXY("for help and inspiration", 115, 80 + 10 * 9);
        gfx_PrintStringXY("for this project.", 115, 80 + 10 * 10);
    }

    for(i = 0; i < elements_in_context[c]; i++) {
        view_draw(context_lookup[c][i]);
    }
}

void execute_simplify();
void execute_evaluate();
void execute_expand();
void execute_derivative();

void handle_input(uint8_t key) {
    
    switch(current_context) {
    case CONTEXT_IO:
        if(key == sk_Down) {
            io_context[active_index]->active = false;

            current_context = CONTEXT_FUNCTION;
            active_index = 0;
            function_context[0]->active = true;
            
            draw_context(CONTEXT_IO);
            draw_context(CONTEXT_FUNCTION);
            draw_context(CONTEXT_SIMPLIFY);

        } else if(key == sk_Left && active_index == 1) {
            active_index = 0;
            io_context[1]->active = false;
            io_context[0]->active = true;
            draw_context(CONTEXT_IO);
        } else if(key == sk_Right && active_index == 0) {
            active_index = 1;
            io_context[0]->active = false;
            io_context[1]->active = true;
            draw_context(CONTEXT_IO);
        } else if(key == sk_Enter) {
            view_t *v = context_lookup[current_context][active_index];
            v->index = (v->index + 1) % NUM_DROPDOWN_ENTRIES;
            draw_context(current_context);
        }

        break;
    case CONTEXT_FUNCTION:

        if((key == sk_Right || key == sk_Enter) && elements_in_context[CONTEXT_SIMPLIFY + active_index] > 0) {
            function_index = active_index;

            current_context = (Context)(CONTEXT_SIMPLIFY + active_index);
            function_context[active_index]->active = false;
            active_index = 0;
            context_lookup[current_context][active_index]->active = true;

            draw_context(CONTEXT_FUNCTION);
            draw_context(current_context);

            break;
        }

    default:
        if(key == sk_Up) {
            if(active_index == 0) {
                context_lookup[current_context][active_index]->active = false;
                active_index = 0;
                io_context[active_index]->active = true;
                draw_context(current_context);
                current_context = CONTEXT_IO;
                draw_context(CONTEXT_IO);
                break;
            }

            context_lookup[current_context][active_index]->active = false;
            active_index--;
            context_lookup[current_context][active_index]->active = true;
            draw_context(current_context);

            if(current_context == CONTEXT_FUNCTION)
                draw_context((Context)(CONTEXT_SIMPLIFY + active_index));
        } else if(key == sk_Down) {
            if(active_index == elements_in_context[current_context] - 1)
                break;
            context_lookup[current_context][active_index]->active = false;
            active_index++;
            context_lookup[current_context][active_index]->active = true;
            draw_context(current_context);

            if(current_context == CONTEXT_FUNCTION)
                draw_context((Context)(CONTEXT_SIMPLIFY + active_index));
        } else if(key == sk_Left && current_context != CONTEXT_FUNCTION) {
            context_lookup[current_context][active_index]->active = false;
            draw_context(current_context);
            active_index = function_index;
            current_context = CONTEXT_FUNCTION;
            context_lookup[current_context][active_index]->active = true;
            draw_context(CONTEXT_FUNCTION);
        } else if(key == sk_Enter) {
            view_t *v = context_lookup[current_context][active_index];

            switch(v->type) {
            case GUI_CHECKBOX:
                v->checked = !v->checked;
                draw_context(current_context);
                break;
            case GUI_DROPDOWN:
                v->index = (v->index + 1) % NUM_DROPDOWN_ENTRIES;
                draw_context(current_context);
                break;
            case GUI_BUTTON:
                if(v == button_simplify)        execute_simplify();
                else if(v == button_evaluate)   execute_evaluate();
                else if(v == button_expand)     execute_expand();
                else if(v == button_derivative) execute_derivative();
                break;
            default:
                break;
            }
            
        }

        break;
    }
}

void gui_Init() {
    io_context[0] = view_create_dropdown(LCD_WIDTH / 4 + 20 - 25, 14 + 8 + 4, 0);
    io_context[1] = view_create_dropdown(LCD_WIDTH / 4 * 3 - 20 - 25, 14 + 8 + 4, 1);

    function_context[0] = view_create_label(26, 80, "Simplify");
    function_context[1] = view_create_label(26, 96, "Evaluate");
    function_context[2] = view_create_label(26, 112, "Expand");
    function_context[3] = view_create_label(26, 128, "Derivative");
    function_context[4] = view_create_label(26, 144, "Help");

    simplify_context[0] = view_create_checkbox(124, 80 + 12 * 0, "Like terms", true);
    simplify_context[1] = view_create_checkbox(124, 80 + 12 * 1, "Basic identities", true);
    simplify_context[2] = view_create_checkbox(124, 80 + 12 * 2, "Trig identities", true);
    simplify_context[3] = view_create_checkbox(124, 80 + 12 * 3, "Hyperbolic identities", true);
    simplify_context[4] = view_create_checkbox(124, 80 + 12 * 4, "Complex identities", true);
    simplify_context[5] = view_create_checkbox(124, 80 + 12 * 5, "Evaluate trig constants", true);
    simplify_context[6] = button_simplify = view_create_button(10 + 2 + 100 + (LCD_WIDTH - 10 - 10 - 2 - 100) / 2, 185, "Simplify");

    evaluate_context[0] = view_create_checkbox(124, 80, "Substitue expression:", false);
    evaluate_context[1] = view_create_dropdown(124 + 80, 96, 10);
    evaluate_context[2] = view_create_dropdown(124 + 80, 96 + 24, 11);
    evaluate_context[3] = button_evaluate = view_create_button(10 + 2 + 100 + (LCD_WIDTH - 10 - 10 - 2 - 100) / 2, 185, "Evaluate");

    expand_context[0] = view_create_checkbox(124, 80 + 12 * 0, "Expand multiplication", true);
    expand_context[1] = view_create_checkbox(124, 80 + 12 * 1, "Expand powers", true);
    expand_context[2] = button_expand = view_create_button(10 + 2 + 100 + (LCD_WIDTH - 10 - 10 - 2 - 100) / 2, 185, "Expand");

    derivative_context[0] = button_derivative = view_create_button(10 + 2 + 100 + (LCD_WIDTH - 10 - 10 - 2 - 100) / 2, 185, "Differentiate");

    current_context = CONTEXT_IO;
    active_index = 0;
    io_context[0]->active = true;
}

void gui_Cleanup() {
    unsigned i, j;

    for(i = 0; i < NUM_CONTEXTS; i++) {
        for(j = 0; j < elements_in_context[i]; j++) {
            free(context_lookup[i][j]);
        }
    }
}

void gui_Run() {

    gui_Init();

    os_ClrHome();
    gfx_Begin();

    draw_background();
    draw_context(CONTEXT_IO);
    draw_context(CONTEXT_FUNCTION);

    while(true) {
        uint8_t key = os_GetCSC();

        if(key == sk_Clear)
            break;

        handle_input(key);
    }

    gfx_End();

    gui_Cleanup();
}

void execute_simplify() {

}

void execute_evaluate() {

}

void execute_expand() {

}

void execute_derivative() {

}

#else
typedef int make_iso_compilers_happy;
#endif