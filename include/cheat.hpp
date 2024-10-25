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

#include "dmntcht.h"
#include "cheat_engine_types.hpp"
#include "result.hpp"
#include "results.hpp"

#include <string>
#include <vector>

namespace edz::cheat {

    // Work around for EResult madness
    bool Succeeded(Result res);

    #define EDIZON_CHEATS_DIR EDIZON_DIR "/cheats"
    #define EMPTY_RESPONSE { }

    class Cheat {
    public:
        Cheat(DmntCheatEntry cheatEntry);

        std::string getName();
        u32 getID();

        bool toggle();
        bool setState(bool state);
        bool isEnabled();

    private:
        std::string m_name;
        u32 m_id;
    };

    class FrozenAddress {
    public:
        FrozenAddress(DmntFrozenAddressEntry);
        FrozenAddress(u64 address, u8 width, u64 value);
        FrozenAddress(u64 address, u8 width);

        u64 getAddress();
        u8 getWidth();
        u64 getValue();
        u64 setValue(u64 value, u8 width);

        bool toggle();
        bool isFrozen();

    private:
        DmntFrozenAddressEntry m_frozenAddressEntry;
        bool m_frozen;
    };


    class CheatManager {
    public:
        static Result initialize();
        static void exit();

        static bool isCheatServiceAvailable();

        static Result forceAttach();
        static bool hasCheatProcess();

        static u64 getTitleID();
        static u64 getProcessID();
        static u64 getBuildID();

        static types::Region getBaseRegion();
        static types::Region getHeapRegion();
        static types::Region getMainRegion();
        static types::Region getAliasRegion();

        //static EResultVal<std::string> getCheatFile();

        //static EResultVal<u32> addCheat(DmntCheatDefinition cheatDefinition, bool enabled);
        static Result removeCheat(u32 cheatID);

        static std::vector<Cheat*>& getCheats();
        static std::vector<FrozenAddress*>& getFrozenAddresses();

        static MemoryInfo queryMemory(u64 address);
        static std::vector<MemoryInfo> getMemoryRegions();

        static Result readMemory(u64 address, void *buffer, size_t bufferSize);
        static Result writeMemory(u64 address, const void *buffer, size_t bufferSize);

        static Result reload();

    private:
        static inline std::vector<Cheat*> s_cheats;
        static inline std::vector<FrozenAddress*> s_frozenAddresses;

        static inline DmntCheatProcessMetadata s_processMetadata;
    };

} 