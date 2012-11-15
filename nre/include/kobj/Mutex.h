/* -*- Mode: C++ -*-
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

#pragma once

#include <kobj/Vi.h>
#include <util/Atomic.h>
#include <kobj/Thread.h>

namespace nre {

    class Mutex {
    private:
        Thread           *_lock_holder;   // Only for debugging.
        Thread * volatile _wait_list;

    public:
        Mutex() : _lock_holder(nullptr), _wait_list(nullptr) { }

        void acquire()
        {
            Thread *current = Thread::current();
            assert(not current->_waiting_for);
            assert(current->fetch_events(1) == 0);

            current->ensure_mutex_irq();

            Thread *newv;
            do {
                current->_waiting_for = newv = _wait_list;
            } while (not Atomic::cmpnswap(&_wait_list, newv, current));

            // We are in the wait list. If we are the tail of the
            // list, we are in the critical section. Otherwise, we
            // must wait.

            // We cannot test current->_waiting_for here, because that
            // can be NULL already, if someone woke us up between the
            // cmpnswap and here.

            if (newv) {
#ifndef NDEBUG
                for (Thread *cur = current->_waiting_for; cur;
                     cur = cur->_waiting_for) {
                    assert(cur != current /* deadlock */);
                }
#endif
                current->mutex_wait();
            }

            assert (current->_waiting_for == NULL);
            assert (current->fetch_events(1) == 0);

            _lock_holder = current;
            Sync::memory_barrier();
        }

        void release()
        {
            Thread *current = Thread::current();
            assert (_lock_holder == current);
            assert (_wait_list);

            // The hopefully common case: The mutex is uncontended.
            while (_wait_list == current) {
                if (Atomic::cmpnswap(&_wait_list, current, (Thread *)nullptr))
                    // Successfull release. Nobody to wake up.
                    return;
                // XXX Actually, this will never loop. Because after
                // cmpnswap fails, the loop condition will be false.
            }

            // It does not matter, if someone enqueues himself at the
            // front of the list at this point.
            Thread *head = _wait_list;

            // XXX The t->next non-NULL test can be removed, if we are
            // sure this is working properly.
            for (; head->_waiting_for and (head->_waiting_for != current); head = head->_waiting_for) { }
            assert (head->_waiting_for == current);

            // No worries about atomic ops here, because the mutex
            // protects the dequeue operation.
            head->_waiting_for = nullptr;
            head->mutex_wakeup();
        }

    };

}

// EOF
