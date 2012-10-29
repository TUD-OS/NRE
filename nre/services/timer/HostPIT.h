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

#include <kobj/Ports.h>
#include <kobj/Gsi.h>
#include <services/ACPI.h>

#include "HostTimerDevice.h"

class HostPIT : public HostTimerDevice {
    static const timevalue_t FREQ               = 1193180ULL;
    static const timevalue_t DEFAULT_PERIOD     = 1000ULL; // ms
    static const uint IRQ                       = 0;
    static const uint PORT_BASE                 = 0x40;

    class PitTimer : public Timer {
    public:
        explicit PitTimer() : Timer(), _gsi(irq_to_gsi(IRQ)) {
        }

        virtual nre::Gsi &gsi() {
            return _gsi;
        }
        virtual void init(HostTimerDevice &, cpu_t) {
        }
        virtual void program_timeout(timevalue_t) {
        }

    private:
        static uint irq_to_gsi(uint irq) {
            nre::Connection acpicon("acpi");
            nre::ACPISession acpi(acpicon);
            return acpi.irq_to_gsi(irq);
        }

        nre::Gsi _gsi;
    };

public:
    explicit HostPIT(uint period_us);

    virtual timevalue_t last_ticks() {
        return nre::Atomic::read_uninterruptible(_ticks);
    }
    virtual timevalue_t current_ticks() {
        return nre::Atomic::read_uninterruptible(_ticks);
    }
    virtual timevalue_t update_ticks(bool refresh_only) {
        return refresh_only ? _ticks : ++_ticks;
    }

    virtual bool is_periodic() const {
        return true;
    }
    virtual size_t timer_count() const {
        return 1;
    }
    virtual Timer *timer(size_t) {
        return &_timer;
    }
    virtual timevalue_t freq() const {
        return _freq;
    }

    virtual bool is_in_past(timevalue_t ticks) const {
        return ticks < _ticks;
    }
    virtual timevalue_t next_timeout(timevalue_t, timevalue_t next) {
        return next;
    }
    virtual void start(timevalue_t ticks) {
        _ticks = ticks;
    }
    virtual void enable(Timer *, bool) {
        // nothing to do
    }

private:
    nre::Ports _ports;
    timevalue_t _freq;
    timevalue_t _ticks;
    PitTimer _timer;
};
