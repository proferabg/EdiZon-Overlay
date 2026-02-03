/**
 * Copyright (C) 2019 - 2020 WerWolv
 * 
 * This file is part of EdiZon
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

#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <map>
#include <sstream>
#include <switch.h>
#include <algorithm> 
#include <cctype>
#include <vector>
#include "utils.hpp"
#include "cheat.hpp"

// --- UTILS ---
std::string normalize(std::string str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) { 
        return !std::isalnum(c); 
    }), str.end());
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

// --- 1. CUSTOM LIST CLASS ---
class SmartList : public tsl::elm::List {
public:
    tsl::elm::Element* getSelectedItem() {
        if (this->m_items.empty()) return nullptr;
        if (this->m_focusedIndex >= this->m_items.size()) return nullptr;
        return this->m_items[this->m_focusedIndex];
    }
};

// --- 2. GUI NOTE CLASS ---
class GuiNote : public tsl::Gui {
public:
    GuiNote(std::string title, std::string text) : m_title(title), m_text(text) {}

    // Helper to wrap text into multiple lines
    std::vector<std::string> wrapText(const std::string& text, size_t maxLen) {
        std::vector<std::string> lines;
        std::istringstream stream(text);
        std::string line;

        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            
            if (line.length() <= maxLen) {
                lines.push_back(line);
                continue;
            }

            size_t currentPos = 0;
            while (currentPos < line.length()) {
                if (line.length() - currentPos <= maxLen) {
                    lines.push_back(line.substr(currentPos));
                    break;
                }

                size_t splitPos = line.rfind(' ', currentPos + maxLen);
                
                if (splitPos == std::string::npos || splitPos <= currentPos) {
                    splitPos = currentPos + maxLen;
                }

                lines.push_back(line.substr(currentPos, splitPos - currentPos));
                currentPos = splitPos + 1; 
            }
        }
        return lines;
    }

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::HeaderOverlayFrame(97);

        rootFrame->setHeader(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("Cheat Note", false, 20, 50, 32, (tsl::defaultOverlayColor));
            renderer->drawString(m_title.c_str(), false, 20, 82, 20, (tsl::style::color::ColorHighlight));
        }));

        auto list = new tsl::elm::List();
        
        list->addItem(new tsl::elm::CategoryHeader("Details"));

        // UPDATED: Wrap text to 30 characters per line
        std::vector<std::string> wrappedLines = wrapText(m_text, 30);

        for (const auto& line : wrappedLines) {
            auto item = new tsl::elm::ListItem(line);
            item->setClickListener([](u64 keys){ return false; }); 
            list->addItem(item);
        }

        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_B || keysDown & KEY_Y) {
            tsl::goBack();
            return true;
        }
        return tsl::Gui::handleInput(keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);
    }

    virtual void update() override {}

private:
    std::string m_title;
    std::string m_text;
};

// Forward Declaration
class GuiStats;

class GuiMain : public tsl::Gui {
public:
    GuiMain() { }
    ~GuiMain() { }
    virtual tsl::elm::Element* createUI() override;
    virtual void update() { }

public:
    static inline std::string s_runningTitleIDString;
    static inline std::string s_runningProcessIDString;
    static inline std::string s_runningBuildIDString;
};

// --- 3. GUI CHEATS CLASS ---
class GuiCheats : public tsl::Gui {
public:
    GuiCheats(std::string section) { 
        this->m_section = section;
    }
    ~GuiCheats() { }

    std::map<std::string, std::string> parseExternalNotesFile(int& outCount) {
        std::map<std::string, std::string> notes;
        outCount = 0;

        char path[256];
        snprintf(path, sizeof(path), "sdmc:/atmosphere/contents/%016lX/cheats/notes.txt", 
                 edz::cheat::CheatManager::getTitleID());

        std::ifstream file(path);
        if (!file.is_open()) file.open("sdmc:/atmosphere/cheats/notes.txt");
        
        if (!file.is_open()) return notes;

        std::string line;
        std::string currentTargetNorm = "";
        std::string currentContent = "";
        bool readingNote = false;

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            size_t openBracket = line.find("[");
            size_t closeBracket = line.rfind("]");

            if (openBracket == 0 && closeBracket != std::string::npos && closeBracket > openBracket) {
                if (readingNote && !currentTargetNorm.empty() && !currentContent.empty()) {
                    notes[currentTargetNorm] = currentContent;
                    outCount++;
                }

                std::string rawName = line.substr(openBracket + 1, closeBracket - openBracket - 1);
                
                if (rawName.find("--Section") != std::string::npos) {
                    readingNote = false;
                    continue;
                }

                currentTargetNorm = normalize(rawName); 
                currentContent = "";
                readingNote = true;
                continue;
            }

            if (readingNote) {
                std::string processedLine = line;
                if (processedLine.size() >= 2 && processedLine.front() == '"' && processedLine.back() == '"') {
                    processedLine = processedLine.substr(1, processedLine.size() - 2);
                }

                if (!currentContent.empty()) currentContent += "\n";
                currentContent += processedLine;
            }
        }
        
        if (readingNote && !currentTargetNorm.empty() && !currentContent.empty()) {
            notes[currentTargetNorm] = currentContent;
            outCount++;
        }

        return notes;
    }

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::HeaderOverlayFrame(97);
        bool setOnce = true; 

        int notesLoadedCount = 0;
        std::map<std::string, std::string> noteMap = parseExternalNotesFile(notesLoadedCount);

        rootFrame->setHeader(new tsl::elm::CustomDrawer([this, &setOnce, notesLoadedCount](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString(APP_TITLE, false, 20, 50, 32, (tsl::defaultOverlayColor));
            
            if (setOnce) {
                renderer->drawString(APP_VERSION, false, 20, 52+23, 15, (tsl::bannerVersionTextColor));
                setOnce = false;
            } else {
                char buf[64];
                if (notesLoadedCount > 0) {
                    snprintf(buf, sizeof(buf), "Cheats (%d Notes)", notesLoadedCount);
                    renderer->drawString(buf, false, 20, 52+23, 15, (tsl::style::color::ColorHighlight));
                } else {
                    renderer->drawString("Cheats", false, 20, 52+23, 15, (tsl::bannerVersionTextColor));
                }
            }
            
            if (edz::cheat::CheatManager::getProcessID() != 0) {
                renderer->drawString(GuiMain::s_runningTitleIDString.c_str(), false, 250 +14, 40 -6, 15, (tsl::style::color::ColorText));
                renderer->drawString(GuiMain::s_runningBuildIDString.c_str(), false, 250 +14, 60 -6, 15, (tsl::style::color::ColorText));
                renderer->drawString(GuiMain::s_runningProcessIDString.c_str(), false, 250 +14, 80 -6, 15, (tsl::style::color::ColorText));
            }
        }));

        if (edz::cheat::CheatManager::getCheats().size() == 0) {
            auto warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
                renderer->drawString("\uE150", false, 180, 274, 90, (0xFFFF));
                renderer->drawString("No Cheats loaded!", false, 110, 360, 25, (0xFFFF));
            });
            rootFrame->setContent(warning);
        } else {
            this->m_list = new SmartList();
            this->m_elementToNote.clear();

            std::string head = "Section: " + this->m_section;

            if(m_section.length() > 0) m_list->addItem(new tsl::elm::CategoryHeader(head));
            else m_list->addItem(new tsl::elm::CategoryHeader("Available cheats"));

            bool skip = false, inSection = false, submenus = true;
            std::string skipUntil = "";

            for (auto &cheat : edz::cheat::CheatManager::getCheats()) {
                if(cheat->getID() == 1 && cheat->getName().find("--DisableSubmenus--") != std::string::npos)
                    submenus = false;

                if(submenus){
                    if(this->m_section.length() > 0 && !inSection && cheat->getName().find("--SectionStart:" + this->m_section + "--") == std::string::npos) continue;
                    else if(cheat->getName().find("--SectionStart:" + this->m_section + "--") != std::string::npos) { inSection = true; continue; }
                    else if(inSection && cheat->getName().find("--SectionEnd:" + this->m_section + "--") != std::string::npos) break;

                    if(!skip && cheat->getName().find("--SectionStart:") != std::string::npos){
                        std::string name = cheat->getName();
                        size_t startPos = name.find("SectionStart:");
                        if (startPos != std::string::npos) name = name.substr(startPos + 13);
                        size_t endDash = name.find("--");
                        if (endDash != std::string::npos) name = name.substr(0, endDash);

                        auto cheatsSubmenu = new tsl::elm::ListItem(name);
                        cheatsSubmenu->setClickListener([name](s64 keys) {
                            if (keys & KEY_A) {
                                tsl::changeTo<GuiCheats>(name);
                                return true;
                            }
                            return false;
                        });
                        m_list->addItem(cheatsSubmenu);
                        this->m_numCheats++;
                        skip = true;
                        skipUntil = "--SectionEnd:" + name + "--";
                    }
                    else if (skip && cheat->getName().compare(skipUntil) == 0){
                        skip = false;
                        skipUntil = "";
                    }
                    else if(!skip && (inSection || this->m_section.length() < 1)) {
                        addCheatWithNote(cheat, noteMap);
                    }
                } else {
                    if(cheat->getName().find("--SectionStart:") != std::string::npos || cheat->getName().find("--SectionEnd:") != std::string::npos || cheat->getName().find("--DisableSubmenus--") != std::string::npos)
                        continue;
                    addCheatWithNote(cheat, noteMap);
                }
            }

            if(this->m_numCheats < 1){
                auto warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
                    renderer->drawString("\uE150", false, 180, 250, 90, (0xFFFF));
                    renderer->drawString("No Cheats in Submenu!", false, 110, 340, 25, (0xFFFF));
                });
                rootFrame->setContent(warning);
            } else rootFrame->setContent(m_list);
        }

        return rootFrame;
    }

    void addCheatWithNote(auto& cheat, std::map<std::string, std::string>& notes) {
        std::string cheatNameFull = cheat->getName();
        std::string cheatNameClean = cheatNameFull;
        size_t tagPos = cheatNameClean.find(":ENABLED");
        if (tagPos != std::string::npos) cheatNameClean.replace(tagPos, 8, "");
        
        std::string lookupKey = normalize(cheatNameClean);
        bool hasNote = (notes.find(lookupKey) != notes.end());
        std::string displayName = cheatNameClean;
        
        if (hasNote) {
            displayName = "\uE0E3 " + displayName; 
        }

        auto cheatToggleItem = new tsl::elm::ToggleListItem(displayName, cheat->isEnabled());
        cheatToggleItem->setClickListener([&cheat, cheatToggleItem](u64 keys) {
            if (keys & KEY_A) {
                bool newState = !cheat->isEnabled();
                cheat->setState(newState);
                cheatToggleItem->setState(newState);
                return true;
            }
            return false;
        });

        if (hasNote) {
            this->m_elementToNote[cheatToggleItem] = notes[lookupKey];
        }

        this->m_cheatToggleItems.insert({cheat->getID(), cheatToggleItem});
        m_list->addItem(cheatToggleItem);
        this->m_numCheats++;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        if (keysDown & KEY_Y) {
            if (m_list) {
                tsl::elm::Element* selected = m_list->getSelectedItem();
                
                if (selected && m_elementToNote.find(selected) != m_elementToNote.end()) {
                    tsl::elm::ToggleListItem* item = dynamic_cast<tsl::elm::ToggleListItem*>(selected);
                    std::string title = "";
                    if(item) {
                         title = item->getText();
                         if (title.find("\uE0E3 ") == 0) title = title.substr(5);
                    } else {
                        title = "Cheat Note";
                    }

                    tsl::changeTo<GuiNote>(title, m_elementToNote[selected]);
                    return true;
                }
            }
        }
        return tsl::Gui::handleInput(keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);
    }

    virtual void update() override {
        for (auto const& [cheatId, toggleElem] : this->m_cheatToggleItems)
            for(auto &cheat : edz::cheat::CheatManager::getCheats())
                if(cheat->getID() == cheatId)
                    toggleElem->setState(cheat->isEnabled());
    }

private:
    int m_numCheats = 0;
    std::string m_section;
    std::map<u32, tsl::elm::ToggleListItem*> m_cheatToggleItems;
    SmartList* m_list = nullptr;
    std::map<tsl::elm::Element*, std::string> m_elementToNote;
};

// --- GUI STATS (Unchanged) ---
class GuiStats : public tsl::Gui {
public:
    GuiStats() { 
        if (hosversionAtLeast(8,0,0)) {
            clkrstOpenSession(&this->m_clkrstSessionCpu, PcvModuleId_CpuBus, 3);
            clkrstOpenSession(&this->m_clkrstSessionGpu, PcvModuleId_GPU, 3);
            clkrstOpenSession(&this->m_clkrstSessionMem, PcvModuleId_EMC, 3);
        }

        tsl::hlp::doWithSmSession([this]{
            nifmGetCurrentIpAddress(&this->m_ipAddress);
            this->m_ipAddressString = formatString("%d.%d.%d.%d", this->m_ipAddress & 0xFF, (this->m_ipAddress >> 8) & 0xFF, (this->m_ipAddress >> 16) & 0xFF, (this->m_ipAddress >> 24) & 0xFF);
        });
    }
    ~GuiStats() {
        if (hosversionAtLeast(8,0,0)) {
            clkrstCloseSession(&this->m_clkrstSessionCpu);
            clkrstCloseSession(&this->m_clkrstSessionGpu);
            clkrstCloseSession(&this->m_clkrstSessionMem);
        }
     }
            
    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame(APP_TITLE, "System Information");
        auto infos = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
            renderer->drawString("CPU Temperature:", false, 63, 200, 18, (tsl::style::color::ColorText));
            renderer->drawString("PCB Temperature:", false, 63, 230, 18, (tsl::style::color::ColorText));
            renderer->drawRect(x, 243, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("CPU Clock:", false, 63, 270, 18, (tsl::style::color::ColorText));
            renderer->drawString("GPU Clock:", false, 63, 300, 18, (tsl::style::color::ColorText));
            renderer->drawString("MEM Clock:", false, 63, 330, 18, (tsl::style::color::ColorText));
            renderer->drawRect(x, 343, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("Local IP:", false, 63, 370, 18, (tsl::style::color::ColorText));
    
            static char PCB_temperatureStr[10];
            static char SOC_temperatureStr[10];
            static float tempSOC = 0.0f;
            static float tempPCB = 0.0f;

            ult::ReadSocTemperature(&tempSOC, false);
            ult::ReadPcbTemperature(&tempPCB, false);
    
            snprintf(SOC_temperatureStr, sizeof(SOC_temperatureStr) - 1, "%.1f °C", static_cast<double>(tempSOC));
            snprintf(PCB_temperatureStr, sizeof(PCB_temperatureStr) - 1, "%.1f °C", static_cast<double>(tempPCB));
            
            renderer->drawString(SOC_temperatureStr, false, 258, 200, 18, (tsl::style::color::ColorHighlight));
            renderer->drawString(PCB_temperatureStr, false, 258, 230, 18, (tsl::style::color::ColorHighlight));
            
            static u32 cpuClock = 0, gpuClock = 0, memClock = 0;
            if (hosversionAtLeast(8,0,0)) {
                clkrstGetClockRate(&this->m_clkrstSessionCpu, &cpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionGpu, &gpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionMem, &memClock);
            } else {
                pcvGetClockRate(PcvModule_CpuBus, &cpuClock);
                pcvGetClockRate(PcvModule_GPU, &gpuClock);
                pcvGetClockRate(PcvModule_EMC, &memClock);
            }
    
            renderer->drawString(formatString("%.01f MHz", cpuClock / 1'000'000.0F).c_str(), false, 258, 270, 18, (tsl::style::color::ColorHighlight));
            renderer->drawString(formatString("%.01f MHz", gpuClock / 1'000'000.0F).c_str(), false, 258, 300, 18, (tsl::style::color::ColorHighlight));
            renderer->drawString(formatString("%.01f MHz", memClock / 1'000'000.0F).c_str(), false, 258, 330, 18, (tsl::style::color::ColorHighlight));
    
            if (this->m_ipAddressString ==  "0.0.0.0")
                renderer->drawString("Offline", false, 258, 370, 18, (tsl::style::color::ColorHighlight));
            else 
                renderer->drawString(this->m_ipAddressString.c_str(), false, 258, 370, 18, (tsl::style::color::ColorHighlight));
    
            if(hosversionAtLeast(15,0,0)){
                NifmInternetConnectionType conType;
                u32 wifiStrength;
                NifmInternetConnectionStatus conStatus;
                nifmGetInternetConnectionStatus(&conType, &wifiStrength, &conStatus);
                renderer->drawString("Connection:", false, 63, 400, 18, (tsl::style::color::ColorText));
                if(conStatus == NifmInternetConnectionStatus_Connected && conType == NifmInternetConnectionType_WiFi) {
                    std::string wifiStrengthStr = "(Strong)";
                    tsl::Color color = tsl::Color(0x0, 0xF, 0x0, 0xF);
                    if(wifiStrength == 2){
                        wifiStrengthStr = "(Fair)";
                        color = tsl::Color(0xE, 0xE, 0x2, 0xF);
                    } else if(wifiStrength <= 1){
                        wifiStrengthStr = "(Poor)";
                        color = tsl::Color(0xF, 0x0, 0x0, 0xF);
                    }
                    renderer->drawString("WiFi", false, 258, 400, 18, (tsl::style::color::ColorHighlight));
                    renderer->drawString(wifiStrengthStr.c_str(), false, 303, 400, 18, (color));
                } else if(conStatus == NifmInternetConnectionStatus_Connected && conType == NifmInternetConnectionType_Ethernet){
                    renderer->drawString("Ethernet", false, 258, 400, 18, (tsl::style::color::ColorHighlight));
                } else {
                    renderer->drawString("Disconnected", false, 258, 400, 18, (tsl::style::color::ColorHighlight));
                }
            } else {
                s32 signalStrength = 0;
                wlaninfGetRSSI(&signalStrength);
                renderer->drawString("WiFi Signal:", false, 63, 400, 18, (tsl::style::color::ColorText));
                renderer->drawString(formatString("%d dBm", signalStrength).c_str(), false, 258, 400, 18, (tsl::style::color::ColorHighlight)); 
            }
            renderer->drawString("Credits:", false, 63, 600, 18, (tsl::style::color::ColorText));
            renderer->drawString(APP_AUTHOR, false, 75, 630, 18, (tsl::style::color::ColorHighlight)); 
        });
        rootFrame->setContent(infos);
        return rootFrame;
    }
    virtual void update() { }

private:
    ClkrstSession m_clkrstSessionCpu, m_clkrstSessionGpu, m_clkrstSessionMem;
    u32 m_ipAddress;
    std::string m_ipAddressString;
};

// --- IMPLEMENTATION OF GuiMain::createUI ---
tsl::elm::Element* GuiMain::createUI() {
    auto *rootFrame = new tsl::elm::HeaderOverlayFrame();
    rootFrame->setHeader(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString(APP_TITLE, false, 20, 50, 32, (tsl::defaultOverlayColor));
        renderer->drawString(APP_VERSION, false, 20, 52+23, 15, (tsl::bannerVersionTextColor));

        if (edz::cheat::CheatManager::getProcessID() != 0) {
            renderer->drawString("Program ID", false, 150 +14, 40 -6, 15, (tsl::style::color::ColorText));
            renderer->drawString("Build ID", false, 150 +14, 60 -6, 15, (tsl::style::color::ColorText));
            renderer->drawString("Process ID", false, 150 +14, 80 -6, 15, (tsl::style::color::ColorText));
            renderer->drawString(GuiMain::s_runningTitleIDString.c_str(), false, 250 +14, 40 -6, 15, (tsl::style::color::ColorHighlight));
            renderer->drawString(GuiMain::s_runningBuildIDString.c_str(), false, 250 +14, 60 -6, 15, (tsl::style::color::ColorHighlight));
            renderer->drawString(GuiMain::s_runningProcessIDString.c_str(), false, 250 +14, 80 -6, 15, (tsl::style::color::ColorHighlight));
        }
    }));

    auto list = new tsl::elm::List();

    if(edz::cheat::CheatManager::isCheatServiceAvailable()){
        auto cheatsItem = new tsl::elm::ListItem("Cheats");
        cheatsItem->setClickListener([](s64 keys) {
            if (keys & KEY_A) {
                tsl::changeTo<GuiCheats>("");
                return true;
            }
            return false;
        });
        list->addItem(cheatsItem);
    } else {
        auto noDmntSvc = new tsl::elm::ListItem("Cheat Service Unavailable!");
        list->addItem(noDmntSvc);
    }

    auto statsItem  = new tsl::elm::ListItem("System Information");
    statsItem->setClickListener([](s64 keys) {
        if (keys & KEY_A) {
            tsl::changeTo<GuiStats>();
            return true;
        }
        return false;
    });
    list->addItem(statsItem);

    rootFrame->setContent(list);
    return rootFrame;
}

// --- EdiZonOverlay Class ---
class EdiZonOverlay : public tsl::Overlay {
public:
    EdiZonOverlay() { }
    ~EdiZonOverlay() { }

    void initServices() override {
        if(edz::cheat::CheatManager::isCheatServiceAvailable()){
            edz::cheat::CheatManager::initialize();
            for (auto &cheat : edz::cheat::CheatManager::getCheats()) {
                if(cheat->getName().find(":ENABLED") != std::string::npos){
                    cheat->setState(true);
                }
            }
        }
        clkrstInitialize();
        pcvInitialize();
        i2cInitialize();
        nifmInitialize(NifmServiceType_User);
    } 

    virtual void exitServices() override {
        if (edz::cheat::CheatManager::isCheatServiceAvailable())
            edz::cheat::CheatManager::exit();
        nifmExit();
        i2cExit();
        wlaninfExit();
        nifmExit();
        clkrstExit();
        pcvExit();
    }

    virtual void onShow() override {
        edz::cheat::CheatManager::reload();
        GuiMain::s_runningTitleIDString     = formatString("%016lX", edz::cheat::CheatManager::getTitleID());
        GuiMain::s_runningBuildIDString     = formatString("%016lX", edz::cheat::CheatManager::getBuildID());
        GuiMain::s_runningProcessIDString   = formatString("%lu", edz::cheat::CheatManager::getProcessID());
    }

    std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiMain>();
    }
};

int main(int argc, char **argv) {
    return tsl::loop<EdiZonOverlay>(argc, argv);
}