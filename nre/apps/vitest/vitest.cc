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
#include <services/Console.h>
#include <CPU.h>

#include <util/Date.h>
#include <Exception.h>

using namespace nre;

class MutexGuard {
public:
    explicit MutexGuard(Mutex &m) : _m(m) { _m.acquire(); }
    ~MutexGuard() { _m.release(); }

private:
    MutexGuard(const MutexGuard&);
    MutexGuard& operator=(const MutexGuard&);

    Mutex &_m;
};


static Connection conscon("console");
static ConsoleSession cons(conscon, 0, String("vitest"));
static Mutex mutex_print;
static Mutex mutex_count;

static unsigned long protected_count;

PORTAL static void recall_handler(capsel_t)
{
    // May deadlock...
    Serial::get().writef("CPU%u: Recall!\n", CPU::current().log_id());
}

static void wait_and_print(void *)
{
    auto &serial = Serial::get();
    serial.writef("CPU%u: Waiting for events.\n", CPU::current().log_id());
    while (true) {
        Vi::block();

        for (unsigned u = 100000; u > 0; u--) {
            MutexGuard g(mutex_count);
            protected_count++;
        }

        MutexGuard g(mutex_print);
        serial.writef("CPU%u: Events: %lx.\n", CPU::current().log_id(),
                      Thread::current()->fetch_events());
    }
}

int main()
{
    auto &serial = Serial::get();

    serial.writef("Virtual IRQ test up.\n");

    Vi *irqs[CPU::count()];
    for (CPU::iterator it = CPU::begin(); it != CPU::end(); ++it) {
        ScopedCapSels c;
        serial.writef("Starting thread on CPU%u.\n", it->log_id());

        LocalThread  *lt = LocalThread ::create(it->log_id());
        UNUSED Pt    *pt = new Pt(lt, Hip::get().service_caps() * it->log_id() + CapSelSpace::Caps::EV_RECALL,
                                  recall_handler);
        GlobalThread *gt = GlobalThread::create(wait_and_print, it->log_id(), "vitest-thread");
        serial.writef("Creating Virtual IRQ for CPU%u (cap %u).\n", it->log_id(), c.get());
        irqs[it->log_id()] = new Vi(gt, gt, c.get(), 2 << it->log_id());
        c.release();
        gt->start();
    }

    unsigned long our_count = 0;

    while (true) {
        serial.writef("our %lu vs his %lu\n", our_count, protected_count);

        auto k = cons.receive();
        if (not (k.flags & Keyboard::RELEASE)) continue;
        switch (k.character) {
            case 'q': return 0;
            case '0' ... '9': {
                cpu_t c = k.character - '0';
                if (c >= CPU::count()) break;

                our_count += 100000;
                {
                    MutexGuard g(mutex_print);
                    serial.writef("Triggering CPU%u.\n", c);
                    irqs[c]->trigger();
                    break;
                }
            }
        }
    }

    serial.writef("Virtual IRQ finished successfully.\n");
    return 0;
}
