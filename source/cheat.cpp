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
#include <thread>
#include <vector>
#include <future>

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
        if (running == -1) 
            running = isServiceRunning("dmnt:cht");

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
    
        // Check if we can attach to a running process (i.e., if a game is running)
        if (R_FAILED(res = CheatManager::forceAttach())) {
            return res;  // Early exit if no game is running
        }
    
        int numThreads = 4;  // Number of threads to use for parallel processing
        std::vector<std::future<void>> futures;
    
        // Step 1: Parallel deletion of local cheats
        if (!CheatManager::s_cheats.empty()) {
            int cheatSize = CheatManager::s_cheats.size();
            int cheatChunkSize = (cheatSize + numThreads - 1) / numThreads;
            futures.reserve(numThreads);
    
            for (int t = 0; t < numThreads; ++t) {
                futures.emplace_back(std::async(std::launch::async, [t, cheatChunkSize, cheatSize]() {
                    int start = t * cheatChunkSize;
                    int end = std::min(start + cheatChunkSize, cheatSize);
                    for (int i = start; i < end; ++i) {
                        delete CheatManager::s_cheats[i];
                    }
                }));
            }
    
            for (auto& future : futures) {
                future.get();  // Wait for all threads to finish
            }
    
            CheatManager::s_cheats.clear();
        }
    
        // Step 2: Parallel deletion of local frozen addresses
        if (!CheatManager::s_frozenAddresses.empty()) {
            int frozenSize = CheatManager::s_frozenAddresses.size();
            int frozenChunkSize = (frozenSize + numThreads - 1) / numThreads;
            futures.clear();  // Clear the futures before reusing them
    
            for (int t = 0; t < numThreads; ++t) {
                futures.emplace_back(std::async(std::launch::async, [t, frozenChunkSize, frozenSize]() {
                    int start = t * frozenChunkSize;
                    int end = std::min(start + frozenChunkSize, frozenSize);
                    for (int i = start; i < end; ++i) {
                        delete CheatManager::s_frozenAddresses[i];
                    }
                }));
            }
    
            for (auto& future : futures) {
                future.get();  // Wait for all threads to finish
            }
    
            CheatManager::s_frozenAddresses.clear();
        }
    
        // Step 3: Get process metadata
        if (R_FAILED(res = dmntchtGetCheatProcessMetadata(&CheatManager::s_processMetadata))) {
            // Early exit if no game is running (since we won't have any metadata)
            return res;
        }
    
        // Step 4: Load cheats with parallelized processing
        u64 cheatCnt = 0;
        if (R_FAILED(res = dmntchtGetCheatCount(&cheatCnt))) {
            // If no cheats are available, skip the processing
            return res;
        }
    
        if (cheatCnt > 0) {
            DmntCheatEntry* cheatEntries = new DmntCheatEntry[cheatCnt];
            if (R_FAILED(res = dmntchtGetCheats(cheatEntries, cheatCnt, 0, &cheatCnt))) {
                delete[] cheatEntries;
                return res;
            }
    
            CheatManager::s_cheats.reserve(cheatCnt);  // Reserve capacity
    
            int cheatChunkSizeNew = (cheatCnt + numThreads - 1) / numThreads;
            std::vector<std::vector<Cheat*>> localCheats(numThreads);  // Local storage to avoid contention
            futures.clear();  // Clear futures before using them again
    
            for (int t = 0; t < numThreads; ++t) {
                localCheats[t].reserve(cheatChunkSizeNew);  // Reserve the local storage
                futures.emplace_back(std::async(std::launch::async, [t, cheatChunkSizeNew, cheatEntries, cheatCnt, &localCheats]() {
                    int start = t * cheatChunkSizeNew;
                    int end = std::min(start + cheatChunkSizeNew, (int)cheatCnt);
                    for (int i = start; i < end; ++i) {
                        localCheats[t].push_back(new Cheat(cheatEntries[i]));
                    }
                }));
            }
    
            for (auto& future : futures) {
                future.get();  // Wait for all threads to finish
            }
    
            // Merge the local cheats back into the main vector
            for (int t = 0; t < numThreads; ++t) {
                CheatManager::s_cheats.insert(CheatManager::s_cheats.end(), localCheats[t].begin(), localCheats[t].end());
            }
    
            delete[] cheatEntries;
        }
    
        // Step 6: Load frozen addresses with parallelized processing
        u64 frozenAddressCnt = 0;
        if (R_FAILED(res = dmntchtGetFrozenAddressCount(&frozenAddressCnt))) {
            return res;
        }
    
        if (frozenAddressCnt > 0) {
            DmntFrozenAddressEntry* frozenAddressEntries = new DmntFrozenAddressEntry[frozenAddressCnt];
            if (R_FAILED(res = dmntchtGetFrozenAddresses(frozenAddressEntries, frozenAddressCnt, 0, &frozenAddressCnt))) {
                delete[] frozenAddressEntries;
                return res;
            }
    
            CheatManager::s_frozenAddresses.reserve(frozenAddressCnt);  // Reserve capacity
    
            int frozenChunkSizeNew = (frozenAddressCnt + numThreads - 1) / numThreads;
            std::vector<std::vector<FrozenAddress*>> localFrozenAddresses(numThreads);  // Local storage for frozen addresses
            futures.clear();  // Reuse futures for frozen address loading
    
            for (int t = 0; t < numThreads; ++t) {
                localFrozenAddresses[t].reserve(frozenChunkSizeNew);  // Reserve the local storage
                futures.emplace_back(std::async(std::launch::async, [t, frozenChunkSizeNew, frozenAddressEntries, frozenAddressCnt, &localFrozenAddresses]() {
                    int start = t * frozenChunkSizeNew;
                    int end = std::min(start + frozenChunkSizeNew, (int)frozenAddressCnt);
                    for (int i = start; i < end; ++i) {
                        localFrozenAddresses[t].push_back(new FrozenAddress(frozenAddressEntries[i]));
                    }
                }));
            }
    
            for (auto& future : futures) {
                future.get();  // Wait for all threads to finish
            }
    
            // Merge the local frozen addresses back into the main vector
            for (int t = 0; t < numThreads; ++t) {
                CheatManager::s_frozenAddresses.insert(CheatManager::s_frozenAddresses.end(), localFrozenAddresses[t].begin(), localFrozenAddresses[t].end());
            }
    
            delete[] frozenAddressEntries;
        }
    
        return res;
    }
    
    
        

}
