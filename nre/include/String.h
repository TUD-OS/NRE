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
#include <stream/OStream.h>
#include <stream/OStringStream.h>
#include <cstring>

namespace nre {

/**
 * Basic string class which primary purpose is to send strings around via IPC. It does not yet
 * support manipulation of the string.
 */
class String {
public:
    /**
     * Constructor. Creates an empty string (without allocation on the heap)
     */
    explicit String() : _str(0), _len() {
    }
    /**
     * Constructor. Copies the given string onto the heap.
     *
     * @param str the string
     * @param len the length of the string (-1 by default, which means: use strlen())
     */
    String(const char *str, size_t len = static_cast<size_t>(-1))
        : _str(), _len() {
        if(str)
            init(str, len);
    }
    /**
     * Clones the given string
     */
    explicit String(const String& s)
        : _str(), _len() {
        init(s._str, s._len);
    }
    String & operator=(const String& s) {
        if(&s != this)
            reset(s._str, s._len);
        return *this;
    }
    ~String() {
        delete[] _str;
    }

    /**
     * @return the string (always null-terminated)
     */
    const char *str() const {
        return _str ? _str : "";
    }
    /**
     * @return the length of the string
     */
    size_t length() const {
        return _len;
    }

    /**
     * Formats this string according to <fmt>
     *
     * @param size the size of the buffer that is created
     * @param fmt the format-string
     */
    void format(size_t size, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        vformat(size, fmt, ap);
        va_end(ap);
    }
    /**
     * Formats this string according to <fmt>
     *
     * @param size the size of the buffer that is created
     * @param fmt the format-string
     * @param ap the arguments
     */
    void vformat(size_t size, const char *fmt, va_list ap) {
        delete[] _str;
        _str = new char[size];
        OStringStream os(_str, size);
        os.vwritef(fmt, ap);
        _len = os.length();
    }

    /**
     * Resets the string to the given one. That is, it free's the current string and copies
     * the given one into a new place on the heap
     *
     * @param str the string
     * @param len the length of the string (-1 by default, which means: use strlen())
     */
    void reset(const char *str, size_t len = static_cast<size_t>(-1)) {
        delete[] _str;
        init(str, len);
    }

private:
    void init(const char *str, size_t len) {
        _len = len == static_cast<size_t>(-1) ? strlen(str) : len;
        _str = new char[_len + 1];
        memcpy(_str, str, _len);
        _str[_len] = '\0';
    }

    char *_str;
    size_t _len;
};

/**
 * @return true if s1 and s2 are equal
 */
static inline bool operator==(const String &s1, const String &s2) {
    return s1.length() == s2.length() && strcmp(s1.str(), s2.str()) == 0;
}
/**
 * @return true if s1 and s2 are not equal
 */
static inline bool operator!=(const String &s1, const String &s2) {
    return !operator==(s1, s2);
}

/**
 * Writes the string into the given output-stream
 *
 * @param os the stream
 * @param str the string
 * @return the stream
 */
static inline OStream &operator<<(OStream &os, const String &str) {
    return os << str.str();
}

}
