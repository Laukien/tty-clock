/*
 *     TTY-CLOCK ttyclock.h file.
 *     Copyright © 2023 Stephan Laukien <software@laukien.com>
 *     Copyright © 2009-2013 tty-clock contributors
 *     Copyright © 2008-2009 Martin Duquesnoy <xorg62@gmail.com>
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

#pragma once
#ifndef TTYCLOCK_H
#define TTYCLOCK_H

/* Definitions */
#define NORMFRAMEW      35
#define SECFRAMEW       54
#define DATEWINH        3
#define AMSIGN          " [AM]"
#define PMSIGN          " [PM]"
#define DATE_SIZE       256
#define FORMAT_SIZE     100
#define DELAY_DEFAULT   1
#define DELAY_MAX       100
#define DELAYNS_DEFAULT 0
#define DELAYNS_MAX     1000000000

/* Global ttyclock struct */
typedef struct {
    /* while() boolean */
    bool running;

    /* exit code of a subroutine */
    int exit;

    /* terminal variables */
    SCREEN *ttyscr;
    int bg;
    char *tty;
    FILE *ftty;

    /* Running option */
    struct {
        long delay;
        long nsdelay;
        int color;
        char format[100];
        bool second:1;
        bool screensaver:1;
        bool twelve:1;
        bool center:1;
        bool rebound:1;
        bool date:1;
        bool utc:1;
        bool box:1;
        bool noquit:1;
        bool bold:1;
        bool blink:1;
    } option;

    /* Clock geometry */
    struct {
        int x, y, w, h;
        /* For rebound use (see clock_rebound())*/
        int a, b;
    } geo;

    /* Date content ([2] = number by number) */
    struct {
        unsigned int hour[2];
        unsigned int minute[2];
        unsigned int second[2];
        char datestr[DATE_SIZE];
        char old_datestr[DATE_SIZE];
    } date;

    /* time.h utils */
    struct tm *tm;
    time_t lt;

    /* Clock member */
    char *meridiem;
    WINDOW *framewin;
    WINDOW *datewin;

} ttyclock_t;

#endif /* TTYCLOCK_H */

// vim: expandtab tabstop=4 softtabstop=4 shiftwidth=4
