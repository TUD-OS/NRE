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

#include <arch/Types.h>

/**
 * Whether the UtcbStack variant should be used (requires a NOVA change)
 */
#define USE_UTCB_STACKING       0
/**
 * Whether the NOVA change exists (changes the UTCB layout)
 */
#define USE_UTCB_KERNEL_EXT     1

namespace nre {

class Utcb;
class UtcbFrame;
class UtcbFrameRef;
class OStream;
OStream &operator<<(OStream &os, const Utcb &utcb);
OStream &operator<<(OStream &os, const UtcbFrameRef &frm);

/**
 * Head of the UTCB that contains the number of items in the UTCB and the receive windows for
 * capability delegation and translation. This class is not supposed to be used directly.
 */
class UtcbHead {
    friend class UtcbFrame;
    friend class UtcbFrameRef;
    friend OStream & operator<<(OStream &os, const Utcb &utcb);
    friend OStream & operator<<(OStream &os, const UtcbFrameRef &frm);

protected:
#if USE_UTCB_KERNEL_EXT
    uint16_t top;           // offset of the current frame from the top in words
    uint16_t bottom;        // offset of the current frame from the bottom in words
#endif
    union {
        struct {
            uint16_t untyped;
            uint16_t typed;
        };
        word_t mtr;
    };
    word_t crd_translate;
    word_t crd;
    word_t events;
};

}
