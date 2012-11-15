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

namespace nre {

class Atomic {
public:
    /**
     * Adds <value> to *<ptr> and returns the old value
     */
    template<typename T, typename Y>
    static T add(T volatile *ptr, Y value) {
        return __sync_fetch_and_add(ptr, value);
    }

    template<typename T>
    static void bit_and(T *ptr, T value) {
        __sync_and_and_fetch(ptr, value);
    }
    template<typename T>
    static void bit_or(T *ptr, T value) {
        __sync_or_and_fetch(ptr, value);
    }

    template<typename T>
    static void set_bit(T *vector, uint bit, bool value = true) {
        uint index = bit >> 5;
        T mask = 1 << (bit & 0x1f);
        if(value && (~vector[index] & mask))
            bit_or(vector + index, mask);
        if(!value && (vector[index] & mask))
            bit_and(vector + index, ~mask);
    }

    /**
     * Compare and swap
     */
    template<typename T, typename Y>
    static bool cmpnswap(T volatile *ptr, Y oldval, Y newval) {
        return __sync_bool_compare_and_swap(ptr, oldval, newval);
    }

    static uint32_t cmpxchg4b(unsigned *var, uint32_t oldvalue, uint32_t newvalue) {
        return __sync_val_compare_and_swap(reinterpret_cast<uint32_t*>(var), oldvalue, newvalue);
    }
    static uint32_t cmpxchg4b(volatile void *var, uint32_t oldvalue, uint32_t newvalue) {
        return __sync_val_compare_and_swap(reinterpret_cast<volatile uint32_t*>(var), oldvalue, newvalue);
    }

    static uint64_t cmpxchg8b(void *var, uint64_t oldvalue, uint64_t newvalue) {
        return __sync_val_compare_and_swap(reinterpret_cast<uint64_t*>(var), oldvalue, newvalue);
    }
    static uint64_t cmpxchg8b(volatile void *var, uint64_t oldvalue, uint64_t newvalue) {
        return __sync_val_compare_and_swap(reinterpret_cast<volatile uint64_t*>(var), oldvalue, newvalue);
    }

#ifdef __x86_64__
    static uint64_t read_uninterruptible(volatile uint64_t &v) {
        return v;
    }

    static void write_uninterruptible(volatile uint64_t &to, uint64_t v) {
        to = v;
    }
#elif __i386__
    static uint64_t read_uninterruptible(volatile uint64_t &v) {
        uint32_t hi = 0;
        uint32_t lo = 0;
        // We don't need a lock prefix, because this is only meant to be
        // uninterruptible not atomic. So don't use this for SMP!
        asm ("cmpxchg8b %2\n"
             : "+a" (lo), "+d" (hi)
             : "m" (v), "b" (lo), "c" (hi));
        return (static_cast<uint64_t>(hi) << 32) | lo;
    }

    static void write_uninterruptible(volatile uint64_t &to, uint64_t v) {
        uint32_t nlo = v;
        uint32_t nhi = v >> 32;
        uint32_t olo = 0;
        uint32_t ohi = 0;

        // We iterate at least twice to get the correct old value and then
        // to substitute it.
        asm ("1: cmpxchg8b %0\n"
             "jne 1b"
             : "+m" (to),
             "+a" (olo), "+d" (ohi)
             : "b" (nlo), "c" (nhi));
    }
#endif


    template <typename T>
    static T xchg(T &v, T newv)
    {
        return __sync_lock_test_and_set(&v, newv);
    }

private:
    Atomic();
};

}
