/*
 * Copyright (C) 2012, Julian Stecklina <jsteckli@os.inf.tu-dresden.de>
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

#include <kobj/GlobalThread.h>
#include <kobj/Vi.h>
#include <kobj/Mutex.h>
#include <ipc/Connection.h>
#include <services/Timer.h>
#include <util/Clock.h>
#include <services/Console.h>
#include <CPU.h>

using namespace nre;

static Connection timercon("timer");
static Connection conscon("console");
static ConsoleSession cons(conscon, 0, String("vitest"));

static Mutex     mutex;
static volatile unsigned long critical_int;
static volatile unsigned long acquire_count;


static unsigned long per_cpu[Hip::MAX_CPUS];

static void mutex_torture(void *)
{
    unsigned long acq;
    cpu_t         id = CPU::current().log_id();

    while (true) {
        mutex.acquire();
        acq = ++ acquire_count;
        per_cpu[id] ++;

        assert(critical_int == 0);

        critical_int = id;

        assert(critical_int == id);

        critical_int = 0;
        assert(critical_int == 0);

        assert(acq == acquire_count);
        mutex.release();
    }
}

int main()
{
    auto &serial = Serial::get();

    serial.writef("Mutex stress test up.\n");
    for (CPU::iterator it = CPU::begin(); it != CPU::end(); ++it) {
        serial.writef("Starting thread on CPU%u.\n", it->log_id());

        GlobalThread *gt = GlobalThread::create(mutex_torture, it->log_id(), "mutex-torture");
        gt->start();
    }

    TimerSession timer(timercon);
    Clock clock(1000);

    unsigned long last_acq = 0;
    while (true) {
        timevalue_t   next = clock.source_time(1000);
        unsigned long cur_acq = acquire_count;

        serial.writef("acq %016lx per-sec %08lx critical_int %lx\n",
                      cur_acq, cur_acq - last_acq,  critical_int);

        unsigned i = 0;
        for (CPU::iterator it = CPU::begin(); it != CPU::end(); ++it, i++) {
            serial.writef("%16lx ", per_cpu[i]);
        }
        serial.writef("\n");

        last_acq = cur_acq;

        // wait a second
        timer.wait_until(next);
    }

    return 0;
}
