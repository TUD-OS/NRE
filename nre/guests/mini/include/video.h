/*
 * Copyright (C) 2012, Nils Asmussen <nils@os.inf.tu-dresden.de>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * This file is part of NRE (NOVA runtime environment).
 *
 * NRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NRE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 */

#pragma once

#include "util.h"

class Video {
private:
    static char* const SCREEN;
    enum {
        COLS        = 80,
        ROWS        = 25,
        TAB_WIDTH   = 4
    };

public:
    enum {
        BLACK, BLUE, GREEN, CYAN, RED, MARGENTA, ORANGE, WHITE, GRAY, LIGHTBLUE
    };

public:
    static void clear();
    static void set_color(int fg, int bg) {
        color = ((bg & 0xF) << 4) | (fg & 0xF);
    }
    static void putc(char c);

private:
    static void move();

private:
    static int col;
    static int row;
    static int color;
    static const char *chars;
};

