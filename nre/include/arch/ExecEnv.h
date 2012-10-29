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

#include <Compiler.h>
#include <arch/Types.h>
#include <arch/Defines.h>

#define PORTAL  REGPARMS(1)

namespace nre {

class Thread;
class Pd;

class ExecEnv {
    // the x86-call instruction is 5 bytes long
    static const size_t CALL_INSTR_SIZE     = 5;

public:
    typedef PORTAL void (*portal_func)(capsel_t);
    typedef void (*startup_func)(void *);

    static const uint PAGE_SHIFT            = ARCH_PAGE_SHIFT;
    static const size_t PAGE_SIZE           = ARCH_PAGE_SIZE;
    static const size_t STACK_SIZE          = ARCH_STACK_SIZE;
    static const size_t PT_ENTRY_COUNT      = PAGE_SIZE / sizeof(uint32_t);
    static const size_t BIG_PAGE_SIZE       = PAGE_SIZE * PT_ENTRY_COUNT;
    static const uintptr_t KERNEL_START     = ARCH_KERNEL_START;
    static const size_t PHYS_ADDR_SIZE      = 40;
    static const size_t EXIT_CODE_NUM       = 0x20;
    // use something in the middle of the first page. this way, we reduce the propability of
    // pagefaults interpreted as voluntary exits
    static const uintptr_t EXIT_START       = 0x800;
    static const uintptr_t EXIT_END         = EXIT_START + EXIT_CODE_NUM - 1;
    static const uintptr_t THREAD_EXIT      = EXIT_END + 1;
    static const uint EXIT_SUCCESS          = 0;
    static const uint EXIT_FAILURE          = 1;

    NORETURN static void exit(int code);
    NORETURN static void thread_exit();

    static Pd *get_current_pd() {
        return static_cast<Pd*>(get_current(2));
    }

    static void set_current_pd(Pd *pd) {
        set_current(2, pd);
    }

    static Thread *get_current_thread() {
        return static_cast<Thread*>(get_current(1));
    }

    static void set_current_thread(Thread *t) {
        set_current(1, t);
    }

    static void *setup_stack(Pd *pd, Thread *t, startup_func start, uintptr_t ret, uintptr_t stack);
    static size_t collect_backtrace(uintptr_t *frames, size_t max);
    static size_t collect_backtrace(uintptr_t stack, uintptr_t bp, uintptr_t *frames, size_t max);

private:
    ExecEnv();

    static void *get_current(size_t no) {
        uintptr_t sp;
        asm volatile ("mov %%" EXPAND(REG(sp)) ", %0" : "=g" (sp));
        return *reinterpret_cast<void**>(((sp & ~(STACK_SIZE - 1)) + STACK_SIZE - no * sizeof(void*)));
    }

    static void set_current(size_t no, void *obj) {
        uintptr_t sp;
        asm volatile ("mov %%" EXPAND(REG(sp)) ", %0" : "=g" (sp));
        *reinterpret_cast<void**>(((sp & ~(STACK_SIZE - 1)) + STACK_SIZE - no * sizeof(void*))) = obj;
    }
};

}
