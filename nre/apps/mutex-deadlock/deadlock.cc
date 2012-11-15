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
#include <CPU.h>

using namespace nre;

static Connection timercon("timer");
static Mutex mutex[2];

static void mutex_deadlock(void *)
{
    cpu_t         id = CPU::current().log_id();
    Mutex        &m1 = mutex[0];
    Mutex        &m2 = mutex[1];

    while (true) {
        m1.acquire();
        m2.acquire();

        m2.release();
        m1.release();
    }
}

int main()
{
    auto &serial = Serial::get();

    serial.writef("Mutex deadlock test up.\n");
    for (CPU::iterator it = CPU::begin(); it != CPU::end(); ++it) {
        serial.writef("Starting thread on CPU%u.\n", it->log_id());

        GlobalThread *gt = GlobalThread::create(mutex_deadlock, it->log_id(), "mutex-deadlock");
        gt->start();
    }

    TimerSession timer(timercon);
    Clock clock(1000);

    while (true) {
        timevalue_t   next = clock.source_time(1000);

        serial.writef("Still alive...\n");

        // wait a second
        timer.wait_until(next);
    }

    return 0;
}
