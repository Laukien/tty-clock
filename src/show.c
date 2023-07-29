/*
 *     TTY-CLOCK show.c file.
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

#include <stdio.h>
#include "show.h"


void
show_copyright(void)
{
    printf("%s v.%s (c) %s by\n", PRG_TITLE, PRG_VERSION, PRG_DATE);
    printf("\t%s\n", PRG_AUTHOR_1);
    printf("\t%s\n", PRG_AUTHOR_2);
    printf("\t%s\n", PRG_AUTHOR_3);

    return;
}


void
show_version(void)
{
    printf("%s v.%s (c) updated version\n", PRG_TITLE, PRG_VERSION);

    return;
}


void
show_help(void)
{
    printf("usage : tty-clock [-iuvsScbtrahDBxn] [-C [0-7]] [-f format] [-d delay] [-a nsdelay] [-T tty] \n"
          "    -s          Show seconds                            \n"
          "    -S          Screensaver mode                         \n"
          "    -x          Show box                                \n"
          "    -c          Set the clock at the center of the terminal    \n"
          "    -C [0-7]    Set the clock color                       \n"
          "    -b          Use bold colors                          \n"
          "    -t          Set the hour in 12h format                 \n"
          "    -u          Use UTC time                            \n"
          "    -T tty      Display the clock on the specified terminal    \n"
          "    -r          Do rebound the clock                      \n"
          "    -f format   Set the date format                       \n"
          "    -n          Don't quit on keypress                    \n"
          "    -v          Show tty-clock version                    \n"
          "    -i          Show some info about tty-clock              \n"
          "    -h          Show this page                           \n"
          "    -D          Hide date                               \n"
          "    -B          Enable blinking colon                     \n"
          "    -d delay    Set the delay between two redraws of the clock. Default 1s. \n"
          "    -a nsdelay  Additional delay between two redraws in nanoseconds. Default 0ns.\n");

    return;
}

