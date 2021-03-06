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

#include <kobj/Sm.h>
#include <services/Timer.h>
#include <Logging.h>

#include "HostTimer.h"

using namespace nre;

class TimerService;

static HostTimer *timer;
static TimerService *srv;

class TimerSessionData : public ServiceSession {
public:
    // take care that we do the allocation of ClientData only from the corresponding CPU
    explicit TimerSessionData(Service *s, size_t id, portal_func func)
        : ServiceSession(s, id, func), _sms(new Sm*[CPU::count()]),
          _data(new HostTimer::ClientData*[CPU::count()]()) {
        for(auto it = CPU::begin(); it != CPU::end(); ++it)
            _sms[it->log_id()] = new Sm(0);
    }
    // deletion is ok here because it doesn't touch shared data.
    virtual ~TimerSessionData() {
        for(auto it = CPU::begin(); it != CPU::end(); ++it) {
            delete _data[it->log_id()];
            delete _sms[it->log_id()];
        }
        delete[] _sms;
        delete[] _data;
    }

    Sm &sm(cpu_t cpu) {
        return *_sms[cpu];
    }
    HostTimer::ClientData *data(cpu_t cpu) {
        assert(CPU::current().log_id() == cpu);
        if(_data[cpu] == nullptr)
            _data[cpu] = new HostTimer::ClientData(id(), cpu, timer->get_percpu(cpu), _sms[cpu]);
        return _data[cpu];
    }

private:
    Sm **_sms;
    HostTimer::ClientData **_data;
};

class TimerService : public Service {
public:
    explicit TimerService(const char *name)
        : Service(name, CPUSet(CPUSet::ALL), reinterpret_cast<portal_func>(portal)) {
    }

private:
    virtual ServiceSession *create_session(size_t id, const String &, portal_func func) {
        return new TimerSessionData(this, id, func);
    }

    PORTAL static void portal(TimerSessionData *sess);
};

void TimerService::portal(TimerSessionData *sess) {
    UtcbFrameRef uf;
    try {
        nre::Timer::Command cmd;
        uf >> cmd;

        switch(cmd) {
            case nre::Timer::GET_SMS:
                uf.finish_input();

                for(auto it = CPU::begin(); it != CPU::end(); ++it)
                    uf.delegate(sess->sm(it->log_id()).sel(), it->log_id());
                uf << E_SUCCESS;
                break;

            case nre::Timer::PROG_TIMER: {
                timevalue_t time;
                uf >> time;
                uf.finish_input();

                LOG(TIMER_DETAIL, "TIMER: (" << sess->id() << ") Programming for "
                                             << fmt(time, "#x") << " on "
                                             << CPU::current().log_id() << "\n");
                timer->program_timer(sess->data(CPU::current().log_id()), time);
                uf << E_SUCCESS;
            }
            break;

            case nre::Timer::GET_TIME: {
                uf.finish_input();

                timevalue_t uptime, unixts;
                timer->get_time(uptime, unixts);
                LOG(TIMER_DETAIL, "TIMER: (" << sess->id() << ") Getting time"
                                             << " up=" << fmt(uptime, "#x")
                                             << " unix=" << fmt(unixts, "#x") << "\n");
                uf << E_SUCCESS << uptime << unixts;
            }
            break;
        }
    }
    catch(const Exception &e) {
        uf.clear();
        uf << e;
    }
}

int main(int argc, char *argv[]) {
    bool forcepit = false;
    bool forcehpetlegacy = false;
    bool slowrtc = false;
    for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "forcepit") == 0)
            forcepit = true;
        if(strcmp(argv[i], "forcehpetlegacy") == 0)
            forcehpetlegacy = true;
        if(strcmp(argv[i], "slowrtc") == 0)
            slowrtc = true;
    }

    timer = new HostTimer(forcepit, forcehpetlegacy, slowrtc);
    srv = new TimerService("timer");
    srv->start();
    return 0;
}
