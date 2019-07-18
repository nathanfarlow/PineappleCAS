#ifndef COMPILE_PC

#include "gui.h"

#include <tice.h>
#include <keypadc.h>

#include "../dbg.h"

/*320 x 240*/

/*draw an outline*/
void rect_border(uint24_t x, uint8_t y, uint24_t width, uint8_t height) {
    gfx_HorizLine(x, y, width);
    gfx_HorizLine(x, y + height, width + 1);
    gfx_VertLine(x, y, height);
    gfx_VertLine(x + width, y, height);
}

static void draw_string_centered(char *text, int x, int y) {
    unsigned len = gfx_GetStringWidth(text);
    gfx_PrintStringXY(text, x - len / 2, y);
}

static void render_background() {
    gfx_SetMonospaceFont(0);

    gfx_FillScreen(COLOR_BACKGROUND);

    gfx_SetTextTransparentColor(COLOR_TRANSPARENT);
    gfx_SetTextBGColor(COLOR_TRANSPARENT);
    gfx_SetTextFGColor(COLOR_TEXT);

    /*Outer border*/
    gfx_SetColor(COLOR_BLUE);
    rect_border(1, 1, LCD_WIDTH - 2, LCD_HEIGHT - 2);
    gfx_HorizLine(1, LCD_HEIGHT - 21, LCD_WIDTH - 2);

    draw_string_centered("PineappleCAS written by Nathan Farlow", LCD_WIDTH / 2, LCD_HEIGHT - 14);

    /*Divider*/
    gfx_SetColor(COLOR_PURPLE);
    gfx_HorizLine(50, 50, LCD_WIDTH - 100);
    gfx_VertLine(LCD_WIDTH / 2, 50, 160);

    draw_string_centered("Input", LCD_WIDTH / 4 + 20, 14);
    draw_string_centered("Output", LCD_WIDTH / 4 * 3 - 20, 14);
}

void gui_Run() {


    os_ClrHome();

    gfx_Begin();


    render_background();

    while(true) {
    }

    gfx_End();
}

void gui_Cleanup() {

}

#else
typedef int make_iso_compilers_happy;
#endif