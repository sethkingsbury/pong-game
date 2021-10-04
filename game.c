/*
# File:   game.c
# Author: Seth Kingsbury and Alex Roach
# Date:   17/10/2019
# Descr:  Two player Pong game
*/

#include "system.h"
#include "pio.h"
#include "pacer.h"
#include "navswitch.h"
#include "ir_uart.h"
#include "tinygl.h"
#include "../fonts/font3x5_1.h"
#include "display.h"


#define PACER_RATE 500
#define MESSAGE_RATE 20

typedef enum {STATE_MENU, STATE_PLAYING, STATE_LOST, STATE_WAITING, STATE_WON} state_t;

/* Initialise gloabal constants */
int centre = 4;
int rowinc = 1;
int row = 3;
int tick_limit = 100;
char recv = 0;
state_t state = STATE_MENU;

/* Sets the pong paddle based off the given centre point and is two LEDs long */
void set_paddle(void)
{
    struct tinygl_point right = {4, centre - 1};
    struct tinygl_point left = {4, centre};
    tinygl_draw_line(left, right, 1);
}

/* Turns the paddle off */
void set_paddle_off(void)
{
    struct tinygl_point right = {4, centre - 1};
    struct tinygl_point left = {4, centre};
    tinygl_draw_line(left, right, 0);
}

/* Formats text for menu given a string */
void menu_text(const char* string)
{
    tinygl_init (PACER_RATE);
    tinygl_font_set (&font3x5_1);
    tinygl_text_speed_set (MESSAGE_RATE);
    tinygl_text_mode_set (TINYGL_TEXT_MODE_SCROLL);
    tinygl_text_dir_set (TINYGL_TEXT_DIR_ROTATE);
    tinygl_text (string);
}

/* Formats text for player selection given a string */
void player_text(const char* string)
{
    tinygl_text_mode_set (TINYGL_TEXT_MODE_STEP);
    tinygl_text (string);
}

/* The menu state displays the title of the game,
 * changes players depending the west or east movement of the navswitch
 * and when the navswitch is pressed, the player is selected and the game starts */
void menu(void)
{
    int player = 0;

    menu_text("PONG!! ");

    while(state == STATE_MENU)
    {
        navswitch_update();
        tinygl_update();
        pacer_wait();

        if (navswitch_push_event_p(NAVSWITCH_WEST)) {
            player = 1;
            player_text("P1");
        }

        if (navswitch_push_event_p(NAVSWITCH_EAST)) {
            player = 2;
            player_text("P2");
        }

        if (navswitch_push_event_p(NAVSWITCH_PUSH)) {
            if (player == 1) {
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (player == 2) {
                tinygl_clear();
                state = STATE_WAITING;
            }
        }
    }
}

/* Waits for a signal to be recived from the currently playing funkit
 * the paddles remain functional */
void wait(void)
{
    while (state == STATE_WAITING) {

        tinygl_update();
        navswitch_update();
        set_paddle();

        /* checks whether player has moved the paddle right */
        if (navswitch_push_event_p (NAVSWITCH_NORTH) && centre > 1) {
            set_paddle_off();
            centre--;
        }

        /* checks whether player has moved the paddle left */
        if (navswitch_push_event_p (NAVSWITCH_SOUTH) && centre < 6) {
            set_paddle_off();
            centre++;
        }

        if (ir_uart_read_ready_p()) {

            recv = ir_uart_getc();

            if (recv == 'a') {
                rowinc = -1;
                row = 0;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'b') {
                rowinc = -1;
                row = 1;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'c') {
                rowinc = -1;
                row = 2;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'd') {
                rowinc = -1;
                row = 3;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'e') {
                rowinc = -1;
                row = 4;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'f') {
                rowinc = -1;
                row = 5;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'g') {
                rowinc = 1;
                row = 6;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'h') {
                rowinc = 1;
                row = 5;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'i') {
                rowinc = 1;
                row = 4;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'j') {
                rowinc = 1;
                row = 3;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'k') {
                rowinc = 1;
                row = 2;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'l') {
                rowinc = 1;
                row = 1;
                tinygl_clear();
                state = STATE_PLAYING;
            } else if (recv == 'w') {
                tinygl_clear();
                state = STATE_WON;
            }
        }
        pacer_wait();
    }
}

/* Tracks ball through LED matrix, waits for collison between paddle and ball, or ball and ground
 * if ball hits paddle ball will be sent to waiting player.
 * If ball hits ground the game ends */
void play(void)
{
    int tick = 0;
    int colinc = 1;
    int col = 0;

    display_pixel_set (col, row, 1);

    while (state == STATE_PLAYING) {

        tinygl_update();
        navswitch_update();
        set_paddle();

        /* checks whether player has moved the paddle right */
        if (navswitch_push_event_p (NAVSWITCH_NORTH) && centre > 1) {
            set_paddle_off();
            centre--;
        }

        /* checks whether player has moved the paddle left */
        if (navswitch_push_event_p (NAVSWITCH_SOUTH) && centre < 6) {
            set_paddle_off();
            centre++;
        }

        /* code for the ball */
        pacer_wait ();

        tick++;

        if (tick >= tick_limit)
        {
            tick = 0;

            /* Erase previous position.  */
            display_pixel_set (col, row, 0);

            col += colinc;
            row += rowinc;

            if (col == 5) {
                state = STATE_LOST;
            }

            /* check if paddle hits the ball on left */
            if (col == 4 && (row - rowinc) == centre) {
                if (rowinc == -1) {
                   rowinc = 1;
                   row += 2;
                }
                col -= colinc * 2;
                colinc = -colinc;
            }

            /* check if paddle hits the ball on right */
            if (col == 4 && (row - rowinc) == (centre - 1)) {
                if (rowinc == 1) {
                    row -= 2;
                    rowinc = -1;
                }
                col -= colinc * 2;
                colinc = -colinc;
            }

            /* check if ball hits wall of matrix */
            if (row > 6 || row < 0)
            {
                row -= rowinc * 2;
                rowinc = -rowinc;
            }

            /* checks if ball needs to be sent to other player
             * sends encoded char with ball direction and position */
            if (col < 0 && colinc == -1) {

                if (rowinc == 1) {
                    if (row == 6) {
                        ir_uart_putc('a');
                    } else if (row == 5) {
                        ir_uart_putc('b');
                    } else if (row == 4) {
                        ir_uart_putc('c');
                    } else if (row == 3) {
                        ir_uart_putc('d');
                    } else if (row == 2) {
                        ir_uart_putc('e');
                    } else if (row == 1) {
                        ir_uart_putc('f');
                    }

                } else {
                    if (row == 0) {
                        ir_uart_putc('g');
                    } else if (row == 1) {
                        ir_uart_putc('h');
                    } else if (row == 2) {
                        ir_uart_putc('i');
                    } else if (row == 3) {
                        ir_uart_putc('j');
                    } else if (row == 4) {
                        ir_uart_putc('k');
                    } else if (row == 5) {
                        ir_uart_putc('l');
                    }
                }
                /* speeds ball up */
                if (tick_limit > 10) {
                    tick_limit -= 5;
                }
                state = STATE_WAITING;
                tinygl_clear();
                break;
            }

            /* Draw new position.  */
            display_pixel_set (col, row, 1);
        }

        display_update ();
        pacer_wait();
    }

    display_pixel_set (col, row, 0);
}

/* Displays losing text and sends winning text to other player
 * If navswitch pushed send back to main menu */
void lose(void)
{
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text("YOU LOSE");

    ir_uart_putc('w');

    while (state == STATE_LOST) {
        navswitch_update();
        tinygl_update();

        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            state = STATE_MENU;
        }

        pacer_wait();
    }
}

/* Displays winning text
 * If navswitch pushed send back to main menu */
void win(void)
{
    tinygl_text_mode_set(TINYGL_TEXT_MODE_SCROLL);
    tinygl_text("YOU WIN");

    while (state == STATE_WON) {
        navswitch_update();
        tinygl_update();

        if (navswitch_push_event_p (NAVSWITCH_PUSH)) {
            state = STATE_MENU;
        }

        pacer_wait();
    }
}

/* Main game loop */
int main (void)
{

    /* initialise everything */
    system_init();
    display_init ();
    navswitch_init();
    ir_uart_init();
    pacer_init (PACER_RATE);

    while (1) {

        if (state == STATE_MENU) {
            tick_limit = 100;
            menu();
        }

        if (state == STATE_WAITING) {
            wait();
        }

        if (state == STATE_PLAYING) {
            play();
        }

        if (state == STATE_LOST) {
            lose();
        }

        if (state == STATE_WON) {
            win();
        }
    }
}
