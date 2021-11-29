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

#include <switch.h>
#include <string>

#define MODULE_EDIZONHB  337
#define MODULE_EDIZONSYS 338

namespace edz {

    class EResult {
    public:
        constexpr EResult() : m_module(0), m_desc(0) {}

        constexpr EResult(u32 module, u32 desc) : m_module(module), m_desc(desc) {}

        constexpr EResult(Result result) : m_module(static_cast<u32>(R_MODULE(result))), m_desc(static_cast<u32>(R_DESCRIPTION(result))) {}

        constexpr u32 getModule() {
            return this->m_module;
        }

        constexpr u32 getDescription() {
            return this->m_desc;
        }

        std::string getString() {
            char buffer[0x20];
            sprintf(buffer, "2%03d-%04d (0x%x)", this->getModule(), this->getDescription(), static_cast<u32>(*this));

            return buffer;
        }


        constexpr bool operator==(EResult &other) {
            return this->getDescription() == other.getDescription();
        }

        constexpr bool operator!=(EResult &other) {
            return this->getDescription() != other.getDescription();
        }

        constexpr bool operator==(Result &other) {
            return this->getDescription() == EResult(other).getDescription() && this->getDescription() == EResult(other).getModule();
        }

        constexpr bool operator!=(Result &other) {
            return this->getDescription() != EResult(other).getDescription() || this->getDescription() != EResult(other).getModule();
        }

        constexpr EResult operator=(u32 &other) {
            return Result(other);
        }

        constexpr EResult operator=(EResult &other) {
            return EResult(other.getDescription());
        }

        constexpr EResult operator=(Result other) {
            return EResult(static_cast<u32>(other));
        }

        constexpr operator u32() const { 
            return MAKERESULT(this->m_module, this->m_desc);
        }

        constexpr bool succeeded() {
            return this->m_module == 0 && this->m_desc == 0;
        }

        constexpr bool failed() {
            return !this->succeeded();
        }

    private:
        const u32 m_module, m_desc;
    };

    template<typename T>
    using EResultVal = std::pair<EResult, T>;
    
}