#TTY-Clock MakeFile
#Under BSD License
#See clock.c for the license detail.

SRC = src/ttyclock.c src/show.c
CC ?= cc
BIN ?= bin/tty-clock
PREFIX ?= /usr/local
INSTALLPATH ?= ${DESTDIR}${PREFIX}/bin
MANPATH ?= ${DESTDIR}${PREFIX}/share/man/man1
CFLAGS := -std=c99
CFLAGS += -O2
CFLAGS += -Wall -Wextra -Wpedantic
#CFLAGS += -Weverything
CFLAGS += -fstack-protector-all


ifeq ($(shell sh -c 'which ncurses6-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -g $$(ncurses6-config --cflags)
	LDFLAGS += $$(ncurses6-config --libs)
else ifeq ($(shell sh -c 'which ncursesw6-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -g $$(ncursesw6-config --cflags)
	LDFLAGS += $$(ncursesw6-config --libs)
else ifeq ($(shell sh -c 'which ncurses5-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -g $$(ncurses5-config --cflags)
	LDFLAGS += $$(ncurses5-config --libs)
else ifeq ($(shell sh -c 'which ncursesw5-config>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -g $$(ncursesw5-config --cflags)
	LDFLAGS += $$(ncursesw5-config --libs)
else ifeq ($(shell sh -c 'test `uname -s` == 'OpenBSD'>/dev/null 2>/dev/null && echo y'), y)
	CFLAGS += -Wall -O2
	LDFLAGS += -l ncurses
else
	CFLAGS += -Wall -g $$(pkg-config --cflags ncurses)
	LDFLAGS += $$(pkg-config --libs ncurses)
endif

tty-clock : ${SRC}

	@echo "building ${SRC}"
	@mkdir -p bin
	${CC} ${CFLAGS} ${SRC} -o ${BIN} ${LDFLAGS}

install : ${BIN}

	@echo "installing binary file to ${INSTALLPATH}/${BIN}"
	@mkdir -p ${INSTALLPATH}
	@cp ${BIN} ${INSTALLPATH}
	@chmod 0755 ${INSTALLPATH}/${BIN}
	@echo "installing manpage to ${MANPATH}/${BIN}.1"
	@mkdir -p ${MANPATH}
	@cp ${BIN}.1 ${MANPATH}
	@chmod 0644 ${MANPATH}/${BIN}.1
	@echo "installed"

uninstall :

	@echo "uninstalling binary file (${INSTALLPATH})"
	@rm -f ${INSTALLPATH}/${BIN}
	@echo "uninstalling manpage (${MANPATH})"
	@rm -f ${MANPATH}/${BIN}.1
	@echo "${BIN} uninstalled"

clean :

	@echo "cleaning ${BIN}"
	@rm -f ${BIN}
	@echo "${BIN} cleaned"

