/*
 *     TTY-CLOCK ttyclock.c file.
 *     Copyright (c) 2023 Stephan Laukien <software@laukien.com>
 *     Copyright (c) 2009-2018 tty-clock contributors
 *     Copyright (c) 2008-2009 Martin Duquesnoy <xorg62@gmail.com>
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions are
 *     met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the  nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *     A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *     OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *     LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *     THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *     OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include "show.h"
#include "ttyclock.h"

/* Global variable */
static ttyclock_t ttyclock;
static option_t option;

/* Number matrix */
static const bool number[][15] = {
    {1,1,1,1,0,1,1,0,1,1,0,1,1,1,1}, /* 0 */
    {0,0,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 1 */
    {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1}, /* 2 */
    {1,1,1,0,0,1,1,1,1,0,0,1,1,1,1}, /* 3 */
    {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1}, /* 4 */
    {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1}, /* 5 */
    {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1}, /* 6 */
    {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1}, /* 7 */
    {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1}, /* 8 */
    {1,1,1,1,0,1,1,1,1,0,0,1,1,1,1}, /* 9 */
};


/* Prototypes */
static bool init_security(void);
static void init_object(void);
static bool init_screen(void);
static void init_signal(void);
static bool init_option(int argc, char **argv);
static void signal_handler(int signal);
static void clean_screen(void);
static void update_hour(void);
static void draw_number(int n, int x, int y);
static void draw_clock(void);
static void clock_move(int x, int y, int w, int h);
static void clock_rebound(void);
static void set_second(void);
static void set_center(bool b);
static void set_box(bool b);
static bool key_event(void);


int
main(int argc, char **argv)
{
    setlocale(LC_TIME,"");

    if (!init_security()) {
        return 1;
    }
    init_object();
    atexit(clean_screen);

    if (!init_option(argc, argv)) {
        return ttyclock.exit;
    }
    if (!init_screen()) {
        return EXIT_FAILURE;
    }
    init_signal();

    while(ttyclock.running && !ttyclock.exit) {
        clock_rebound();
        update_hour();
        draw_clock();
        if (!key_event()) {
            return ttyclock.exit;
        }
    }

    endwin();

    return 0;
}


/**
 * Get the command line arguments and sets internal option-set
 */
static bool init_option(int argc, char **argv)
{
    int c; /* argument option */
    struct stat sbuf; /* for option 'T' */

    while ((c = getopt(argc, argv, "iuvsScbtrhBxnDC:f:d:T:a:")) != -1) {
        switch(c) {
        case 'h':
        default:
            show_help();
            ttyclock.exit = EXIT_SUCCESS;
            return false;
            /* 'break' is unreachable */
        case 'i':
            show_copyright();
            ttyclock.exit = EXIT_SUCCESS;
            return false;
            /* 'break' is unreachable */
        case 'u':
            option.utc = true;
            break;
        case 'v':
            show_version();
            ttyclock.exit = EXIT_SUCCESS;
            return false;
            /* 'break' is unreachable */
        case 's':
            option.second = true;
            break;
        case 'S':
            option.screensaver = true;
            break;
        case 'c':
            option.center = true;
            break;
        case 'b':
            option.bold = true;
            break;
        case 'C':
            if(atoi(optarg) >= 0 && atoi(optarg) < 8) {
                option.color = (short)atoi(optarg);
            }
            break;
        case 't':
            option.twelve = true;
            break;
        case 'r':
            option.rebound = true;
            break;
        case 'f':
            strncpy(option.format, optarg, FORMAT_SIZE - 1);
            break;
        case 'd':
            if(atol(optarg) >= 0 && atol(optarg) < DELAY_MAX) {
                option.delay = atol(optarg);
            }
            break;
        case 'D':
            option.date = false;
            break;
        case 'B':
            option.blink = true;
            break;
        case 'a':
            if(atol(optarg) >= 0 && atol(optarg) < DELAYNS_MAX) {
                option.nsdelay = atol(optarg);
            }
            break;
        case 'x':
            option.box = true;
            break;
        case 'T':
            if (stat(optarg, &sbuf) == -1) {
                fprintf(stderr, "ERROR: couldn't stat '%s': %s.\n",
                        optarg, strerror(errno));

                ttyclock.exit = EXIT_FAILURE;
                return false;
            } else if (!S_ISCHR(sbuf.st_mode)) {
                fprintf(stderr, "ERROR: '%s' doesn't appear to be a character device.\n",
                        optarg);

                ttyclock.exit = EXIT_FAILURE;
                return false;
            } else {
                free(ttyclock.tty);
                ttyclock.tty = strdup(optarg);
            }
            break;
        case 'n':
            option.noquit = true;
            break;
        }
    }

    return true;
}


/**
 * Restrict os access by doing unveil and pledge
 */
static bool
init_security(void)
{
#ifdef __OpenBSD__
    if(unveil(NULL, NULL) == -1) {
        fprintf(stderr, "ERROR: unable to unveil\n");

        return false;
    }

    if(pledge("stdio rpath tty", NULL) == -1) {
        fprintf(stderr, "ERROR: unable to pledge\n");

        return false;
    }
#endif

    return true;
}



/**
 * Init tty-struct
 */
static void
init_object(void)
{
    memset(&ttyclock, 0, sizeof(ttyclock_t));

    option.date = true;

    /* Default date format */
    strncpy(option.format, "%F", FORMAT_SIZE - 1);
    /* Default color */
    option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */
    /* Default delay */
    option.delay = DELAY_DEFAULT; /* 1FPS */
    option.nsdelay = DELAYNS_DEFAULT; /* -0FPS */
    option.blink = false;
    
    return;
}


/**
 * Init ncurses screen
 */
static bool
init_screen(void)
{
    if (ttyclock.tty) {
        ttyclock.ftty = fopen(ttyclock.tty, "r+");
        if (!ttyclock.ftty) {
            fprintf(stderr, "ERROR: '%s' couldn't be opened: %s.\n",
                   ttyclock.tty, strerror(errno));

            ttyclock.exit = EXIT_FAILURE;
            return false;
        }
        ttyclock.ttyscr = newterm(NULL, ttyclock.ftty, ttyclock.ftty);
        assert(ttyclock.ttyscr != NULL);
        set_term(ttyclock.ttyscr);
    } else {
        initscr();
    }

    ttyclock.bg = COLOR_BLACK;

    cbreak();
    noecho();
    keypad(stdscr, true);
    start_color();
    curs_set(false);
    clear();

    /* Init default terminal color */
    if(use_default_colors() == OK) {
        ttyclock.bg = -1;
    }

    /* Init color pair */
    init_pair(0, ttyclock.bg, ttyclock.bg);
    init_pair(1, ttyclock.bg, option.color);
    init_pair(2, option.color, ttyclock.bg);
    refresh();

    /* Init global struct */
    ttyclock.running = true;
    if(!ttyclock.geo.x) {
        ttyclock.geo.x = 0;
    }
    if(!ttyclock.geo.y) {
        ttyclock.geo.y = 0;
    }
    if(!ttyclock.geo.a) {
        ttyclock.geo.a = 1;
    }
    if(!ttyclock.geo.b) {
        ttyclock.geo.b = 1;
    }
    ttyclock.geo.w = (option.second) ? SECFRAMEW : NORMFRAMEW;
    ttyclock.geo.h = 7;
    ttyclock.tm = localtime(&(ttyclock.lt));
    if(option.utc) {
        ttyclock.tm = gmtime(&(ttyclock.lt));
    }
    ttyclock.lt = time(NULL);
    update_hour();

    /* Create clock win */
    ttyclock.framewin = newwin(ttyclock.geo.h,
                          ttyclock.geo.w,
                          ttyclock.geo.x,
                          ttyclock.geo.y);
    if(option.box) {
        box(ttyclock.framewin, 0, 0);
    }

    if (option.bold) {
        wattron(ttyclock.framewin, A_BLINK);
    }

    /* Create the date win */
    ttyclock.datewin = newwin(DATEWINH, (int)(strlen(ttyclock.date.datestr) + 2),
                         (int)(ttyclock.geo.x + ttyclock.geo.h - 1),
                         (int)(ttyclock.geo.y + (ttyclock.geo.w / 2)) -
                         (int)((strlen(ttyclock.date.datestr) / 2) - 1));
    if(option.box && option.date) {
        box(ttyclock.datewin, 0, 0);
    }
    clearok(ttyclock.datewin, true);

    set_center(option.center);

    nodelay(stdscr, true);

    if (option.date) {
        wrefresh(ttyclock.datewin);
    }

    wrefresh(ttyclock.framewin);

    attron(A_BLINK);

    return true;
}


/**
 * Sets signal handler
 */
static void init_signal(void)
{
    struct sigaction sig;

    sig.sa_handler = signal_handler;
    sig.sa_flags   = 0;
    sigaction(SIGTERM,  &sig, NULL);
    sigaction(SIGINT,   &sig, NULL);
    sigaction(SIGSEGV,  &sig, NULL);

    return;
}


/**
 * Signal-callback which will be called if the user wants to stop
 */
static void
signal_handler(int signal)
{
    switch(signal) {
    case SIGINT:
        /* FALLTHROUGH */
    case SIGTERM:
        ttyclock.running = false;
        break;
        /* Segmentation fault signal */
    case SIGSEGV:
        endwin();
        fprintf(stderr, "ERROR: segmentation fault.\n");
        exit(EXIT_FAILURE);
        /* 'break' is unreachable */
    }

    return;
}


/**
 * Clear up the screen and frees the resources
 */
static void
clean_screen(void)
{
    if (ttyclock.ttyscr) {
        delscreen(ttyclock.ttyscr);
        fclose(ttyclock.ftty);
    }

    free(ttyclock.tty);
}


static void
update_hour(void)
{
    int ihour;
    char tmpstr[128];

    ttyclock.lt = time(NULL);
    ttyclock.tm = localtime(&(ttyclock.lt));
    if(option.utc) {
        ttyclock.tm = gmtime(&(ttyclock.lt));
    }

    ihour = ttyclock.tm->tm_hour;

    if(option.twelve) {
        ttyclock.meridiem = ((ihour >= 12) ? PMSIGN : AMSIGN);
    } else {
        ttyclock.meridiem = "\0";
    }

    /* Manage hour for twelve mode */
    ihour = ((option.twelve && ihour > 12)  ? (ihour - 12) : ihour);
    ihour = ((option.twelve && !ihour) ? 12 : ihour);

    /* Set hour */
    ttyclock.date.hour[0] = ihour / 10;
    ttyclock.date.hour[1] = ihour % 10;

    /* Set minutes */
    ttyclock.date.minute[0] = ttyclock.tm->tm_min / 10;
    ttyclock.date.minute[1] = ttyclock.tm->tm_min % 10;

    /* Set date string */
    strncpy(ttyclock.date.old_datestr, ttyclock.date.datestr, DATE_SIZE - 1);
    strftime(tmpstr,
            sizeof(tmpstr),
            option.format,
            ttyclock.tm);
    snprintf(ttyclock.date.datestr, DATE_SIZE, "%s%s", tmpstr, ttyclock.meridiem);

    /* Set seconds */
    ttyclock.date.second[0] = ttyclock.tm->tm_sec / 10;
    ttyclock.date.second[1] = ttyclock.tm->tm_sec % 10;

    return;
}


static void
draw_number(int n, int x, int y)
{
    int i, sy = y;

    for(i = 0; i < 30; ++i, ++sy) {
        if(sy == y + 6) {
            sy = y;
            ++x;
        }

        if (option.bold) {
            wattron(ttyclock.framewin, A_BLINK);
        } else {
            wattroff(ttyclock.framewin, A_BLINK);
        }

        wbkgdset(ttyclock.framewin, (chtype)COLOR_PAIR(number[n][i/2]));
        mvwaddch(ttyclock.framewin, x, sy, ' ');
    }
    wrefresh(ttyclock.framewin);

    return;
}


static void
draw_clock(void)
{
    const int datediff = strcmp(ttyclock.date.datestr, ttyclock.date.old_datestr) ;
    if (option.date && !option.rebound && datediff != 0) {
        clock_move(ttyclock.geo.x,
                 ttyclock.geo.y,
                 ttyclock.geo.w,
                 ttyclock.geo.h);
    }

    /* Draw hour numbers */
    draw_number(ttyclock.date.hour[0], 1, 1);
    draw_number(ttyclock.date.hour[1], 1, 8);
    chtype dotcolor = COLOR_PAIR(1);
    if (option.blink && time(NULL) % 2 == 0) {
        dotcolor = COLOR_PAIR(2);
    }

    /* 2 dot for number separation */
    wbkgdset(ttyclock.framewin, dotcolor);
    mvwaddstr(ttyclock.framewin, 2, 16, "  ");
    mvwaddstr(ttyclock.framewin, 4, 16, "  ");

    /* Draw minute numbers */
    draw_number(ttyclock.date.minute[0], 1, 20);
    draw_number(ttyclock.date.minute[1], 1, 27);

    /* Draw the date */
    if (option.bold) {
        wattron(ttyclock.datewin, A_BOLD);
    } else {
        wattroff(ttyclock.datewin, A_BOLD);
    }

    if (option.date) {
        wbkgdset(ttyclock.datewin, (COLOR_PAIR(2)));
        mvwprintw(ttyclock.datewin, (DATEWINH / 2), 1, "%s", ttyclock.date.datestr);
        wrefresh(ttyclock.datewin);
    }

    /* Draw second if the option is enabled */
    if(option.second) {
        /* Again 2 dot for number separation */
        wbkgdset(ttyclock.framewin, dotcolor);
        mvwaddstr(ttyclock.framewin, 2, NORMFRAMEW, "  ");
        mvwaddstr(ttyclock.framewin, 4, NORMFRAMEW, "  ");

        /* Draw second numbers */
        draw_number(ttyclock.date.second[0], 1, 39);
        draw_number(ttyclock.date.second[1], 1, 46);
    }

    return;
}


static void
clock_move(int x, int y, int w, int h)
{

    /* Erase border for a clean move */
    wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
    wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    werase(ttyclock.framewin);
    wrefresh(ttyclock.framewin);

    if (option.date) {
        wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
        wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        werase(ttyclock.datewin);
        wrefresh(ttyclock.datewin);
    }

    /* Frame win move */
    mvwin(ttyclock.framewin, (ttyclock.geo.x = x), (ttyclock.geo.y = y));
    wresize(ttyclock.framewin, (ttyclock.geo.h = h), (ttyclock.geo.w = w));

    /* Date win move */
    if (option.date) {
        mvwin(ttyclock.datewin,
             ttyclock.geo.x + ttyclock.geo.h - 1,
             ttyclock.geo.y + (ttyclock.geo.w / 2) - (int)((strlen(ttyclock.date.datestr) / 2) - 1));
        wresize(ttyclock.datewin, DATEWINH, (int)(strlen(ttyclock.date.datestr) + 2));

        if (option.box) {
            box(ttyclock.datewin,  0, 0);
        }
    }

    if (option.box) {
        box(ttyclock.framewin, 0, 0);
    }

    wrefresh(ttyclock.framewin);
    wrefresh(ttyclock.datewin);

    return;
}


static void
clock_rebound(void)
{
    if(!option.rebound) {
        return;
    }

    if(ttyclock.geo.x < 1) {
        ttyclock.geo.a = 1;
    }
    if(ttyclock.geo.x > (LINES - ttyclock.geo.h - DATEWINH)) {
        ttyclock.geo.a = -1;
    }
    if(ttyclock.geo.y < 1) {
        ttyclock.geo.b = 1;
    }
    if(ttyclock.geo.y > (COLS - ttyclock.geo.w - 1)) {
        ttyclock.geo.b = -1;
    }

    clock_move(ttyclock.geo.x + ttyclock.geo.a,
             ttyclock.geo.y + ttyclock.geo.b,
             ttyclock.geo.w,
             ttyclock.geo.h);

    return;
}


static void
set_second(void)
{
    int y_adj;
    int new_w = (((option.second = !option.second)) ? SECFRAMEW : NORMFRAMEW);

    for(y_adj = 0; (ttyclock.geo.y - y_adj) > (COLS - new_w - 1); ++y_adj);

    clock_move(ttyclock.geo.x, (ttyclock.geo.y - y_adj), new_w, ttyclock.geo.h);

    set_center(option.center);

    return;
}


static void
set_center(bool b)
{
    if((option.center = b)) {
        option.rebound = false;

        clock_move((LINES / 2 - (ttyclock.geo.h / 2)),
                 (COLS  / 2 - (ttyclock.geo.w / 2)),
                 ttyclock.geo.w,
                 ttyclock.geo.h);
    }

    return;
}


static void
set_box(bool b)
{
    option.box = b;

    wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
    wbkgdset(ttyclock.datewin, COLOR_PAIR(0));

    if(option.box) {
        wbkgdset(ttyclock.framewin, COLOR_PAIR(0));
        wbkgdset(ttyclock.datewin, COLOR_PAIR(0));
        box(ttyclock.framewin, 0, 0);
        box(ttyclock.datewin,  0, 0);
    } else {
        wborder(ttyclock.framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wborder(ttyclock.datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    }

    wrefresh(ttyclock.datewin);
    wrefresh(ttyclock.framewin);

    return;
}


static bool
key_event(void)
{
    struct timespec length = {
        option.delay,
        option.nsdelay,
    };
    fd_set rfds;
    int c;
    short i;

    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    if (option.screensaver) {
        c = wgetch(stdscr);
        if(c != ERR && option.noquit == false) {
            ttyclock.running = false;
        } else {
            nanosleep(&length, NULL);
            for(i = 0; i < 8; ++i) {
                if(c == (i + '0')) {
                    option.color = i;
                    init_pair(1, ttyclock.bg, i);
                    init_pair(2, i, ttyclock.bg);
                }
            }
        }

        return true;
    }

    /* get keystrokes */
    switch(c = wgetch(stdscr)) {
    case KEY_RESIZE:
        endwin();
        if (!init_screen()) {
            ttyclock.exit = EXIT_FAILURE;
            return false;
        }
        break;
    case KEY_UP:
        /* FALLTHROUGH */
    case 'k':
        /* FALLTHROUGH */
    case 'K':
        if(ttyclock.geo.x >= 1
           && !option.center) {
            clock_move(ttyclock.geo.x - 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
        }
        break;
    case KEY_DOWN:
        /* FALLTHROUGH */
    case 'j':
        /* FALLTHROUGH */
    case 'J':
        if(ttyclock.geo.x <= (LINES - ttyclock.geo.h - DATEWINH)
           && !option.center) {
            clock_move(ttyclock.geo.x + 1, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
        }
        break;
    case KEY_LEFT:
        /* FALLTHROUGH */
    case 'h':
        /* FALLTHROUGH */
    case 'H':
        if(ttyclock.geo.y >= 1
           && !option.center) {
            clock_move(ttyclock.geo.x, ttyclock.geo.y - 1, ttyclock.geo.w, ttyclock.geo.h);
        }
        break;
    case KEY_RIGHT:
        /* FALLTHROUGH */
    case 'l':
        /* FALLTHROUGH */
    case 'L':
        if(ttyclock.geo.y <= (COLS - ttyclock.geo.w - 1)
           && !option.center) {
            clock_move(ttyclock.geo.x, ttyclock.geo.y + 1, ttyclock.geo.w, ttyclock.geo.h);
        }
        break;
    case 'q':
        /* FALLTHROUGH */
    case 'Q':
        if (option.noquit == false) {
            ttyclock.running = false;
        }
        break;
    case 's':
        /* FALLTHROUGH */
    case 'S':
        set_second();
        break;
    case 't':
        /* FALLTHROUGH */
    case 'T':
        option.twelve = !option.twelve;
        /* Set the new ttyclock.date.datestr to resize date window */
        update_hour();
        clock_move(ttyclock.geo.x, ttyclock.geo.y, ttyclock.geo.w, ttyclock.geo.h);
        break;
    case 'c':
        /* FALLTHROUGH */
    case 'C':
        set_center(!option.center);
        break;
    case 'b':
        /* FALLTHROUGH */
    case 'B':
        option.bold = !option.bold;
        break;
    case 'r':
        /* FALLTHROUGH */
    case 'R':
        option.rebound = !option.rebound;
        if(option.rebound && option.center) {
            option.center = false;
        }
        break;
    case 'x':
        /* FALLTHROUGH */
    case 'X':
        set_box(!option.box);
        break;
    case '0': case '1': case '2': case '3':
        /* FALLTHROUGH */
    case '4': case '5': case '6': case '7':
        i = (short)c - '0';
        option.color = i;
        init_pair(1, ttyclock.bg, i);
        init_pair(2, i, ttyclock.bg);
        break;
    default:
        pselect(1, &rfds, NULL, NULL, &length, NULL);
    }

    return true;
}

// vim: expandtab tabstop=4 softtabstop=4 shiftwidth=4
