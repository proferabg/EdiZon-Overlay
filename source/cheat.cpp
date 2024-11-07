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

#include "cheat.hpp"

#include <cstring>

namespace edz::cheat {

    bool Succeeded(Result res){
        return (R_MODULE(res) == 0 && R_DESCRIPTION(res) == 0);
    }

    /////// Cheat Type ///////

    Cheat::Cheat(DmntCheatEntry cheatEntry) : m_name(cheatEntry.definition.readable_name), m_id(cheatEntry.cheat_id) {

    }

    std::string Cheat::getName() {
        return this->m_name;
    }

    u32 Cheat::getID() {
        return this->m_id;
    }

    bool Cheat::toggle() {
        dmntchtToggleCheat(getID());

        return isEnabled();
    }

    bool Cheat::setState(bool state) {
        bool ret = state;
        if (isEnabled() != state)
            ret = this->toggle();
        
        return ret;
    }

    bool Cheat::isEnabled() {
        DmntCheatEntry cheatEntry;

        if (!Succeeded(dmntchtGetCheatById(&cheatEntry, getID()))) {
            return false;
        }

        return cheatEntry.enabled;
    }


    /////// FrozenAddress Type ///////

    FrozenAddress::FrozenAddress(DmntFrozenAddressEntry frozenAddressEntry) : m_frozenAddressEntry(frozenAddressEntry), m_frozen(true) {

    }

    FrozenAddress::FrozenAddress(u64 address, u8 width, u64 value) : m_frozen(false) {
        this->m_frozenAddressEntry.address        = address;
        this->m_frozenAddressEntry.value.width    = width;
        this->m_frozenAddressEntry.value.value    = value;
    }

    FrozenAddress::FrozenAddress(u64 address, u8 width) : m_frozen(false) {
        this->m_frozenAddressEntry.address        = address;
        this->m_frozenAddressEntry.value.width    = width;
    }


    u64 FrozenAddress::getAddress() {
        return this->m_frozenAddressEntry.address;
    }

    u8 FrozenAddress::getWidth() {
        return this->m_frozenAddressEntry.value.width;
    }

    u64 FrozenAddress::getValue() {
        return this->m_frozenAddressEntry.value.value;
    }

    u64 FrozenAddress::setValue(u64 value, u8 width) {
        bool wasFrozen = isFrozen();
        u64 newValue = 0;

        dmntchtDisableFrozenAddress(getAddress());

        dmntchtWriteCheatProcessMemory(getAddress(), &value, width);

        if (wasFrozen) {
            if (Succeeded(dmntchtEnableFrozenAddress(getAddress(), getWidth(), &newValue)))
                return -1;
        }

        // Check if the value was set correctly
        if (std::memcmp(&value, &newValue, width) == 0) {
            this->m_frozenAddressEntry.value.value = newValue;
            this->m_frozenAddressEntry.value.width = width;
                    
            return newValue;
        }

        return -1;
    }


    bool FrozenAddress::toggle() {
        if (isFrozen()) {
            if (Succeeded(dmntchtDisableFrozenAddress(getAddress())))
                this->m_frozen = false;
        }
        else {
            if (Succeeded(dmntchtEnableFrozenAddress(getAddress(), getWidth(), &m_frozenAddressEntry.value.value)))
                this->m_frozen = true;
        }

        return isFrozen();
    }

    bool FrozenAddress::isFrozen() {
        return this->m_frozen;
    }


    /////// FrozenAddress Type ///////

    Result CheatManager::initialize() {
        dmntchtInitialize();
        return CheatManager::reload();
    }   

    void CheatManager::exit() {
        dmntchtExit();
        for (auto &cheat : CheatManager::s_cheats)
            delete cheat;
        CheatManager::s_cheats.clear();

        for (auto &frozenAddress : CheatManager::s_frozenAddresses)
            delete frozenAddress;
        CheatManager::s_frozenAddresses.clear();
    }


    bool CheatManager::isCheatServiceAvailable() {
        static s8 running = -1;
        if (running == -1){
            Handle handle;
            SmServiceName service_name = smEncodeName("dmnt:cht");
            bool running = R_FAILED(smRegisterService(&handle, service_name, false, 1));

            svcCloseHandle(handle);

            if (!running)
                smUnregisterService(service_name);
        }
        return !!running; 
    }

    Result CheatManager::forceAttach() {
        if (!CheatManager::isCheatServiceAvailable())
            return ResultEdzCheatServiceNotAvailable;

        uint64_t PID = 0;
        for(int i = 0; i < 10; i++) {
            if (R_SUCCEEDED(pmdmntGetApplicationProcessId(&PID))) {
                return dmntchtForceOpenCheatProcess();
            }
        }
        return ResultEdzAttachFailed;
    }

    bool CheatManager::hasCheatProcess() {
        if (R_FAILED(CheatManager::forceAttach()))
            return false;

        bool hasCheatProcess;

        dmntchtHasCheatProcess(&hasCheatProcess);

        return hasCheatProcess;
    }


    u64 CheatManager::getTitleID() {
        return CheatManager::s_processMetadata.title_id;
    }

    u64 CheatManager::getProcessID() {
        return CheatManager::s_processMetadata.process_id;
    }

    u64 CheatManager::getBuildID() {
        u64 buildid = 0;

        std::memcpy(&buildid, CheatManager::s_processMetadata.main_nso_build_id, sizeof(u64));
        
        return __builtin_bswap64(buildid);;
    }


    types::Region CheatManager::getBaseRegion() {
        if (!CheatManager::isCheatServiceAvailable())
            return { };

        return types::Region{ CheatManager::s_processMetadata.address_space_extents.base, CheatManager::s_processMetadata.address_space_extents.size };
    }

    types::Region CheatManager::getHeapRegion() {
        if (!CheatManager::isCheatServiceAvailable())
            return { };

        return types::Region{ CheatManager::s_processMetadata.heap_extents.base, CheatManager::s_processMetadata.heap_extents.size };
    }

    types::Region CheatManager::getMainRegion() {
        if (!CheatManager::isCheatServiceAvailable())
            return { };

        return types::Region{ CheatManager::s_processMetadata.main_nso_extents.base, CheatManager::s_processMetadata.main_nso_extents.size };
    }

    types::Region CheatManager::getAliasRegion() {
        if (!CheatManager::isCheatServiceAvailable())
            return { };

        return types::Region{ CheatManager::s_processMetadata.alias_extents.base, CheatManager::s_processMetadata.alias_extents.size };
    }


    /*EResultVal<std::string> CheatManager::getCheatFile() {
        if (!CheatManager::isCheatServiceAvailable())
            return { ResultEdzCheatServiceNotAvailable, "" };
        
        std::string expectedFileName = hlp::toHexString(CheatManager::getBuildID()) + ".txt";

        for (auto &[fileName, file] : hlp::Folder(EDIZON_CHEATS_DIR).getFiles()) {
            if (strcasecmp(expectedFileName.c_str(), fileName.c_str()) == 0)
                return { ResultSuccess, fileName };
        }
            
        return { ResultEdzNotFound, "" }; 
    }


    EResultVal<u32> CheatManager::addCheat(DmntCheatDefinition cheatDefinition, bool enabled) {
        if (!CheatManager::isCheatServiceAvailable())
            return { ResultEdzCheatServiceNotAvailable, 0 };

        u32 cheatID = 0;
        Result res;
        
        if (res = dmntchtAddCheat(&cheatDefinition, enabled, &cheatID); !Succeeded(res))
            return { res, 0 };

        if (res = CheatManager::reload(); !Succeeded(res))
            return { res, 0 };

        return { res, cheatID };
    }*/

    Result CheatManager::removeCheat(u32 cheatID) {
        if (!CheatManager::isCheatServiceAvailable())
            return ResultEdzCheatServiceNotAvailable;

        Result res;

        if (res = dmntchtRemoveCheat(cheatID); !Succeeded(res))
            return res;

        return CheatManager::reload();
    }


    std::vector<Cheat*>& CheatManager::getCheats() {
        return CheatManager::s_cheats;
    }

    std::vector<FrozenAddress*>& CheatManager::getFrozenAddresses() {
        return CheatManager::s_frozenAddresses;
    }


    MemoryInfo CheatManager::queryMemory(u64 address) {
        if (!CheatManager::isCheatServiceAvailable())
            return { 0 };

        MemoryInfo memInfo = { 0 };

        dmntchtQueryCheatProcessMemory(&memInfo, address);

        return memInfo;
    }

    std::vector<MemoryInfo> CheatManager::getMemoryRegions() {
        if (!CheatManager::isCheatServiceAvailable())
            return EMPTY_RESPONSE;

        MemoryInfo memInfo = { 0 };
        std::vector<MemoryInfo> memInfos;

        u64 lastAddress = 0;

        do {
            lastAddress = memInfo.addr;

            memInfo = queryMemory(memInfo.addr + memInfo.size);
            memInfos.push_back(memInfo);
        } while (lastAddress < (memInfo.addr + memInfo.size));

        return memInfos;
    }


    Result CheatManager::readMemory(u64 address, void *buffer, size_t bufferSize) {
        if (!CheatManager::isCheatServiceAvailable())
            return ResultEdzCheatServiceNotAvailable;

        return dmntchtReadCheatProcessMemory(address, buffer, bufferSize);
    }

    Result CheatManager::writeMemory(u64 address, const void *buffer, size_t bufferSize) {
        if (!CheatManager::isCheatServiceAvailable())
            return ResultEdzCheatServiceNotAvailable;

        return dmntchtWriteCheatProcessMemory(address, buffer, bufferSize);
    }

    Result CheatManager::reload() {
        if (!CheatManager::isCheatServiceAvailable())
            return ResultEdzCheatServiceNotAvailable;

        Result res;

        if (R_FAILED(res = CheatManager::forceAttach()))
            return res;

        // Delete local cheats copy if there are any
        for (auto &cheat : CheatManager::s_cheats)
            delete cheat;
        CheatManager::s_cheats.clear();

        // Delete local frozen addresses copy if there are any
        for (auto &frozenAddress : CheatManager::s_frozenAddresses)
            delete frozenAddress;
        CheatManager::s_frozenAddresses.clear();

        // Get process metadata
        if (res = dmntchtGetCheatProcessMetadata(&CheatManager::s_processMetadata); !Succeeded(res))
            return res;


        // Get all loaded cheats
        u64 cheatCnt = 0;
        if (res = dmntchtGetCheatCount(&cheatCnt); !Succeeded(res))
            return res;

        DmntCheatEntry *cheatEntries = new DmntCheatEntry[cheatCnt];

        if (res = dmntchtGetCheats(cheatEntries, cheatCnt, 0, &cheatCnt); !Succeeded(res))
            return res;
        
        CheatManager::s_cheats.reserve(cheatCnt);
        for (u32 i = 0; i < cheatCnt; i++)
            CheatManager::s_cheats.push_back(new Cheat(cheatEntries[i]));


        // Get all frozen addresses
        u64 frozenAddressCnt = 0;
        if (res = dmntchtGetFrozenAddressCount(&frozenAddressCnt); !Succeeded(res))
            return res;

        DmntFrozenAddressEntry frozenAddressEntries[frozenAddressCnt];
        
        if (res = dmntchtGetFrozenAddresses(frozenAddressEntries, frozenAddressCnt, 0, &frozenAddressCnt); !Succeeded(res))
            return res;

        CheatManager::s_frozenAddresses.reserve(frozenAddressCnt);
        for (auto &frozenAddressEntry : frozenAddressEntries)
            CheatManager::s_frozenAddresses.push_back(new FrozenAddress(frozenAddressEntry));

        return res;
    }

}
