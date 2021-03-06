/*
 * Part of ncdc - a discord client for the console
 * Copyright (C) 2019 Florian Stinglmayr <fstinglmayr@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NCDC_H
#define NCDC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wctype.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <pthread.h>

#include <curl/curl.h>

#include <jansson.h>

#include <glib.h>

#include <event.h>
#include <event2/thread.h>

#include <curses.h>
#include <panel.h>

#include <locale.h>

#include <dc/refable.h>
#include <dc/api.h>
#include <dc/loop.h>
#include <dc/account.h>
#include <dc/session.h>

#define return_if_true(v,r) do { if (v) return r; } while(0)
#define goto_if_true(v,l) do { if (v) goto l; } while(0)

/* NCDC_DEBUG variable */
/*#define NCDC_DEBUG*/

typedef enum {
    ncdc_colour_separator = 1,
    ncdc_colour_treehighlight = 2,
} ncdc_colour_pair;

extern GPtrArray *sessions;
extern dc_session_t current_session;

extern dc_api_t api;
extern dc_loop_t loop;

extern char *ncdc_private_dir;
extern void *config;
extern void *mainwindow;

#define KEY_ESCAPE 27

bool is_logged_in(void);

wchar_t *util_readkey(int esc);

void exit_main(void);

wchar_t *s_convert(char const *s);

int strwidth(char const *string);
char *read_char(FILE *stream);

int aswprintf(wchar_t **buffer, wchar_t const *fmt, ...);
char *w_convert(wchar_t const *w);
wchar_t* wcsndup(const wchar_t* string, size_t maxlen);

size_t w_strlenv(wchar_t **s);
void w_strfreev(wchar_t **s);
wchar_t **w_strdupv(wchar_t **s, ssize_t sz);
wchar_t *w_joinv(wchar_t const **v, size_t len);

wchar_t **w_tokenise(wchar_t const *w);
wchar_t *w_next_tok(wchar_t const *w);
wchar_t const *w_next_word(wchar_t const *w, ssize_t len);

#endif
