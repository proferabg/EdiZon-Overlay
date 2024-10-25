/**
 * Copyright (C) 2019 - 2020 WerWolv
 * 
 * This file is part of EdiZon.
 * 
 * EdiZon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * EdiZon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with EdiZon.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstring>
#include <vector>

namespace edz::cheat::types {

    struct Region {
        u64 baseAddress;
        u64 size;

        bool contains(u64 addr) {
            return addr >= baseAddress && addr < (baseAddress + size);
        }
    };

    enum class Signedness {
        Unsigned,
        Signed
    };

    class Pattern {
    public:
        Pattern() : m_pattern(nullptr), m_size(0) { }
        Pattern(u8 *pattern, std::size_t size) : m_pattern(pattern), m_size(size) {}

        void setPattern(u8 *pattern) { this->m_pattern = pattern; }
        void setSize(std::size_t size) { this->m_size = size; }
        void setSignedness(Signedness signedness) { this->m_signedness = signedness; }

        bool operator==(Pattern& other) {
            if (this->m_size != other.m_size) return false;

            return std::memcmp(this->m_pattern, other.m_pattern, this->m_size) == 0;
        }

        bool operator!=(Pattern& other) {
            return !operator==(other);
        }

        bool operator>(Pattern& other) {
            if (this->m_size != other.m_size) return false;

            if (this->m_signedness == Signedness::Signed) {
                if ((reinterpret_cast<u8*>(this->m_pattern)[this->m_size - 1] & 0x80) > (reinterpret_cast<u8*>(other.m_pattern)[this->m_size - 1] & 0x80))
                    return false;

                for (s8 i = this->m_size - 1; i >= 0; i--)
                    if (reinterpret_cast<u8*>(this->m_pattern)[i] > reinterpret_cast<u8*>(other.m_pattern)[i])
                        return true;
            } else {
                for (s8 i = this->m_size - 1; i >= 0; i--) {
                    if (reinterpret_cast<u8*>(this->m_pattern)[i] > reinterpret_cast<u8*>(other.m_pattern)[i])
                        return true;
                }
            }

            return false;
        }

        bool operator<(Pattern& other) {
            if (this->m_size != other.m_size) return false;

            if (this->m_signedness == Signedness::Signed) {
                if ((reinterpret_cast<u8*>(this->m_pattern)[this->m_size - 1] & 0x80) < (reinterpret_cast<u8*>(other.m_pattern)[this->m_size - 1] & 0x80))
                    return false;

                for (s8 i = 0; i < this->m_size; i++)
                    if (reinterpret_cast<u8*>(this->m_pattern)[i] < reinterpret_cast<u8*>(other.m_pattern)[i])
                        return true;
            } else {
                for (s8 i = 0; i < this->m_size; i++) {
                    if (reinterpret_cast<u8*>(this->m_pattern)[i] < reinterpret_cast<u8*>(other.m_pattern)[i])
                        return true;
                }
            }

            return false;
        }

    private:
        u8 *m_pattern;
        ssize_t m_size;
        Signedness m_signedness;
    };


    using Operation = bool(Pattern::*)(Pattern&);
    #define STRATEGY(operation) &edz::cheat::types::Pattern::operator operation

}