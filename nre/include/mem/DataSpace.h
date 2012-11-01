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
#include <kobj/ObjCap.h>
#include <mem/DataSpaceDesc.h>
#include <Exception.h>

namespace nre {

class DataSpaceException : public Exception {
public:
    DEFINE_EXCONSTRS(DataSpaceException)
};

class ClientSession;
class UtcbFrameRef;
template<class DS>
class DataSpaceManager;

/**
 * A dataspace object represents a piece of memory, that is automatically created at construction
 * and destroyed at destruction. That is, the parent adds this piece of memory to the address
 * space of your Pd and backs it with memory. By delegating sel() to somebody else you can share
 * this memory. The recipient can use the DataSpace(capsel_t) constructor to map this
 * dataspace.
 */
class DataSpace {
    template<class DS>
    friend class DataSpaceManager;

public:
    enum RequestType {
        CREATE,
        JOIN,
        SWITCH_TO,
        DESTROY
    };

    /**
     * Creates a new dataspace with given properties and assigns the received capability selectors
     * to <sel> and <unmapsel>, if not zero. This function is only intended for the malloc-backend,
     * which can't use dynamic memory.
     *
     * @throws DataSpaceException if the creation failed
     */
    static void create(DataSpaceDesc &desc, capsel_t *sel = 0, capsel_t *unmapsel = 0);

    /**
     * Creates a new dataspace with given properties
     *
     * @throws DataSpaceException if the creation failed
     */
    explicit DataSpace(size_t size, DataSpaceDesc::Type type, uint flags, uintptr_t phys = 0,
                       uintptr_t virt = 0, uint align = 0)
        : _desc(size, type, flags, phys, virt, 0, align), _sel(ObjCap::INVALID),
          _unmapsel(ObjCap::INVALID) {
        create();
    }
    /**
     * Creates a new dataspace, described by the given descriptor
     *
     * @throws DataSpaceException if the creation failed
     */
    explicit DataSpace(const DataSpaceDesc &desc)
        : _desc(desc), _sel(ObjCap::INVALID), _unmapsel(ObjCap::INVALID) {
        create();
    }
    /**
     * Attaches to the given dataspace, identified by the given selector
     *
     * @throws DataSpaceException if the attachment failed
     */
    explicit DataSpace(capsel_t sel) : _desc(), _sel(sel), _unmapsel(ObjCap::INVALID) {
        join();
    }
    ~DataSpace() throw() {
        destroy();
    }

    /**
     * @return the selector (=identifier) of this dataspace. It always refers to a semaphore.
     */
    capsel_t sel() const {
        return _sel;
    }
    /**
     * @return the selector to unmap this dataspace. Does also refer to a semaphore
     */
    capsel_t unmapsel() const {
        return _unmapsel;
    }
    /**
     * @return the descriptor for this dataspace
     */
    const DataSpaceDesc &desc() const {
        return _desc;
    }
    /**
     * @return the virtual address
     */
    uintptr_t virt() const {
        return _desc.virt();
    }
    /**
     * @return the physical address
     */
    uintptr_t phys() const {
        return _desc.phys();
    }
    /**
     * @return the size in bytes
     */
    size_t size() const {
        return _desc.size();
    }
    /**
     * @return the permissions (see DataSpaceDesc::Perm)
     */
    uint flags() const {
        return _desc.flags();
    }
    /**
     * @return the type of dataspace
     */
    DataSpaceDesc::Type type() const {
        return _desc.type();
    }

    /**
     * Copies the contents of this dataspace into <dest> and swaps this.desc().origin() with
     * <dest>.desc().origin(). That means, afterwards this will access the memory of <dest> and
     * the other way around.
     */
    void switch_to(DataSpace &dest);

private:
    void create();
    void join();
    void destroy();
    void touch();

    DataSpace(const DataSpace&);
    DataSpace& operator=(const DataSpace&);

    DataSpaceDesc _desc;
    capsel_t _sel;
    capsel_t _unmapsel;
};

static inline OStream &operator<<(OStream &os, const DataSpace &ds) {
    os.writef("DataSpace[sel=%#x, umsel=%#x]: ", ds.sel(), ds.unmapsel());
    os << ds.desc();
    return os;
}

}
