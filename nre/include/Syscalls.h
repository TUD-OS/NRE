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

#include <arch/SyscallABI.h>
#include <Desc.h>

namespace nre {

/**
 * The architecture independent API for performing system calls.
 */
class Syscalls {
    enum {
        FLAG0   = 1 << 4,
        FLAG1   = 1 << 5,
    };
    enum {
        IPC_CALL,
        IPC_REPLY,
        CREATE_PD,
        CREATE_EC,
        CREATE_SC,
        CREATE_PT,
        CREATE_SM,
        REVOKE,
        LOOKUP,
        EC_CTRL,
        SC_CTRL,
        SM_CTRL,
        ASSIGN_PCI,
        ASSIGN_GSI,
        CREATE_VI,
        VI_CTRL,
        CREATE_EC_GLOBAL = CREATE_EC | FLAG0,
        REVOKE_MYSELF  = REVOKE | FLAG0,
    };

public:
    /**
     * Types of Ecs. Global means that you can bind a Sc to it. Local Ecs are used for portals
     */
    enum ECType {
        EC_GLOBAL,
        EC_LOCAL
    };
    /**
     * Ec operations
     */
    enum EcOp {
        RECALL  = 0
    };
    /**
     * Sm operations
     */
    enum SmOp {
        SM_UP   = 0,
        SM_DOWN = FLAG0,
        SM_ZERO = FLAG0 | FLAG1
    };
    /**
     * Sc operations
     */
    enum ScOp {
        GETTIME = 0
    };

    /**
     * Vi operations
     */
    enum ViOp {
        BLOCK   = 0,
        TRIGGER = FLAG0,
    };

    /**
     * Calls the given portal
     *
     * @param pt the portal capability selector
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void call(capsel_t pt) {
        SyscallABI::syscall(pt << 8 | IPC_CALL);
    }

    /**
     * Creates a new Ec.
     *
     * @param ec the capability selector to use for the Ec
     * @param utcb the address of the utcb to use
     * @param sp the initial stack pointer
     * @param cpu the physical CPU id to bind the Ec to
     * @param event_base the event base for the Ec
     * @param type the type of Ec (global or local)
     * @param dstpd the Pd in which the Ec should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_ec(capsel_t ec, void *utcb, void *sp, cpu_t cpu,
                                 unsigned event_base, ECType type, capsel_t dstpd) {
        SyscallABI::syscall(ec << 8 | (type == EC_LOCAL ? CREATE_EC : CREATE_EC_GLOBAL), dstpd,
                            reinterpret_cast<word_t>(utcb) | cpu,
                            reinterpret_cast<word_t>(sp),
                            event_base);
    }

    /**
     * Creates a new Sc.
     *
     * @param sc the capability selector to use for the Sc
     * @param ec the Ec to bind the Sc to
     * @param qpd the quantum and period for the Sc
     * @param dstpd the Pd in which the Sc should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_sc(capsel_t sc, capsel_t ec, Qpd qpd, capsel_t dstpd) {
        SyscallABI::syscall(sc << 8 | CREATE_SC, dstpd, ec, qpd.value());
    }

    /**
     * Creates a new Pt.
     *
     * @param pt the capability selector to use for the Pt
     * @param ec the Ec to bind the Pt to
     * @param addr the address of the portal
     * @param mtd the message-transfer descriptor
     * @param dstpd the Pd in which the Sc should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_pt(capsel_t pt, capsel_t ec, uintptr_t addr, Mtd mtd, capsel_t dstpd) {
        SyscallABI::syscall(pt << 8 | CREATE_PT, dstpd, ec, mtd.value(), addr);
    }

    /**
     * Creates a new Pd.
     *
     * @param pd the capability selector to use for the Pd
     * @param pt_crd the initial capabilities to pass to the Pd
     * @param dstpd the Pd in which the Pd should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_pd(capsel_t pd, Crd pt_crd, capsel_t dstpd) {
        SyscallABI::syscall(pd << 8 | CREATE_PD, dstpd, pt_crd.value());
    }

    /**
     * Creates a new Sm.
     *
     * @param sm the capability selector to use for the Sm
     * @param initial the initial value of the Sm
     * @param dstpd the Pd in which the Pd should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_sm(capsel_t sm, unsigned initial, capsel_t dstpd) {
        SyscallABI::syscall(sm << 8 | CREATE_SM, dstpd, initial);
    }

    /**
     * Controls the given Ec.
     *
     * @param ec the capability selector for the Ec
     * @param op the operation (currently only RECALL)
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void ec_ctrl(capsel_t ec, EcOp op) {
        SyscallABI::syscall(ec << 8 | EC_CTRL | op);
    }

    /**
     * Controls the given Sm.
     *
     * @param sm the capability selector for the Sm
     * @param op the operation (DOWN, ZERO or UP)
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void sm_ctrl(capsel_t sm, SmOp op) {
        SyscallABI::syscall(sm << 8 | SM_CTRL | op);
    }

    /**
     * Get consumed CPU time of the given Sc
     *
     * @param sc the capability selector for the Sc
     * @return the consumed CPU time in microseconds
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static timevalue_t sc_time(capsel_t sc) {
        word_t out1, out2;
        SyscallABI::syscall(sc << 8 | SC_CTRL | GETTIME, 0, 0, 0, 0, out1, out2);
        return (static_cast<timevalue_t>(out1) << 32) | out2;
    }

    /**
     * Routes the GSI, specified by <sm>, to the given CPU, where it will be signaled on the
     * corresponding interrupt semaphore.
     *
     * @param sm the GSI to assign
     * @param cpu the physical CPU id to which the GSI should be routed
     * @param pci_cfg_mem for GSIs delivered as MSI, this must refer to the memory-mapped PCI
     *  configuration space of the device that generates the interrupt.
     * @param msi_address will be set to the address of the MSI
     * @param msi_value will be set to the value of the MSI
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void assign_gsi(capsel_t sm, cpu_t cpu, void *pci_cfg_mem = 0,
                                  uint64_t *msi_address = 0, word_t *msi_value = 0) {
        word_t out1, out2;
        SyscallABI::syscall(sm << 8 | ASSIGN_GSI, reinterpret_cast<word_t>(pci_cfg_mem),
                            cpu, 0, 0, out1, out2);
        if(msi_address)
            *msi_address = out1;
        if(msi_value)
            *msi_value = out2;
    }

    /**
     * Revokes the capability range described by the given Crd.
     *
     * @param crd the capability range to revoke
     * @param myself true if the caps should be revoked from yourself as well (not only from childs)
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void revoke(Crd crd, bool myself) {
        SyscallABI::syscall(myself ? REVOKE_MYSELF : REVOKE, crd.value());
    }

    /**
     * Looks up the given capability range in the calling Pd, which has to contain at least the
     * base and the type. If existing, the kernel fills the Crd with the other information.
     * Otherwise a NULL-Crd is returned.
     *
     * @param crd the Crd to lookup
     * @return the resulting Crd
     */
    static Crd lookup(Crd crd) {
        word_t out1, out2;
        SyscallABI::syscall(LOOKUP, crd.value(), 0, 0, 0, out1, out2);
        return Crd(out1);
    }

    /**
     * Creates a new Vi (Virtual IRQ).
     *
     * @param vi    the capability selector to use for the Vi
     * @param ec    the ec that handles this Vi
     * @param evt   the event bitmask
     * @param dstpd the Pd in which the Pd should be created
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void create_vi(capsel_t vi, capsel_t ec_handler, capsel_t ec_recall, uint32_t evt, capsel_t dstpd) {
        SyscallABI::syscall(vi << 8 | CREATE_VI, dstpd, ec_handler, ec_recall, evt);
    }


    /**
     * Triggers a Vi or blocks the thread.
     *
     * @param vi the capability selector (may be 0 for BLOCK)
     * @param op the requested operation
     * @throws SyscallException if the system-call failed (result != E_SUCCESS)
     */
    static void vi_ctrl(capsel_t vi, ViOp op, word_t mask) {
        SyscallABI::syscall(vi << 8 | VI_CTRL | op, mask);
    }


private:
    Syscalls();
    ~Syscalls();
    Syscalls(const Syscalls&);
    Syscalls& operator=(const Syscalls&);
};

}
