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

#include <switch.h>
#include <filesystem>

#include <switch/nro.h>
#include <switch/nacp.h>

#include <edizon.h>
#include <helpers/util.h>
#include "utils.hpp"
#include "cheat.hpp"

#include <unistd.h>
#include <netinet/in.h>


class GuiCheats : public tsl::Gui {
public:
    GuiCheats() { }
    ~GuiCheats() { }

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame("EdiZon", "Cheats");

        if (edz::cheat::CheatManager::getCheats().size() == 0) {
            auto warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){
                renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(0xFFFF));
                renderer->drawString("No Cheats loaded!", false, 110, 340, 25, renderer->a(0xFFFF));
            });

            rootFrame->setContent(warning);

        } else {
            auto list = new tsl::elm::List();

            list->addItem(new tsl::elm::CategoryHeader("Available cheats"));
            
            for (auto &cheat : edz::cheat::CheatManager::getCheats()) {
                auto cheatToggleItem = new tsl::elm::ToggleListItem(cheat->getName(), cheat->isEnabled());
                cheatToggleItem->setStateChangedListener([&cheat](bool state) { cheat->setState(state); });

                this->m_cheatToggleItems.push_back(cheatToggleItem);
                list->addItem(cheatToggleItem);
            }

            rootFrame->setContent(list);

        }

        return rootFrame;
    }

    virtual void update() {
        for (u16 i = 0; i < this->m_cheatToggleItems.size(); i++)
            this->m_cheatToggleItems[i]->setState(edz::cheat::CheatManager::getCheats()[i]->isEnabled());
    }

private:
    std::vector<tsl::elm::ToggleListItem*> m_cheatToggleItems;
};

class GuiStats : public tsl::Gui {
public:
    GuiStats() { 
        if (hosversionAtLeast(8,0,0)) {
            clkrstOpenSession(&this->m_clkrstSessionCpu, PcvModuleId_CpuBus, 3);
            clkrstOpenSession(&this->m_clkrstSessionGpu, PcvModuleId_GPU, 3);
            clkrstOpenSession(&this->m_clkrstSessionMem, PcvModuleId_EMC, 3);
        }

        tsl::hlp::doWithSmSession([this]{
            this->m_ipAddress = gethostid();
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
        auto rootFrame = new tsl::elm::OverlayFrame("EdiZon", "System Information");

        auto infos = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h){

            renderer->drawString("CPU Temparature:", false, 45, 160, 18, renderer->a(tsl::style::color::ColorText));
            renderer->drawString("PCB Temparature:", false, 45, 190, 18, renderer->a(tsl::style::color::ColorText));

            renderer->drawRect(x, 203, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("CPU Clock:", false, 45, 230, 18, renderer->a(tsl::style::color::ColorText));
            renderer->drawString("GPU Clock:", false, 45, 260, 18, renderer->a(tsl::style::color::ColorText));
            renderer->drawString("MEM Clock:", false, 45, 290, 18, renderer->a(tsl::style::color::ColorText));

            renderer->drawRect(x, 303, w, 1, renderer->a(tsl::style::color::ColorFrame));
            renderer->drawString("Local IP:", false, 45, 330, 18, renderer->a(tsl::style::color::ColorText));
            renderer->drawString("WiFi Signal:", false, 45, 360, 18, renderer->a(tsl::style::color::ColorText));

            s32 temparature = 0;
            tsGetTemperatureMilliC(TsLocation_Internal, &temparature);
            renderer->drawString(formatString("%.02f °C", temparature / 1000.0F).c_str(), false, 240, 160, 18, renderer->a(tsl::style::color::ColorHighlight));
            tsGetTemperatureMilliC(TsLocation_External, &temparature);
            renderer->drawString(formatString("%.02f °C", temparature / 1000.0F).c_str(), false, 240, 190, 18, renderer->a(tsl::style::color::ColorHighlight));

            u32 cpuClock = 0, gpuClock = 0, memClock = 0;

            if (hosversionAtLeast(8,0,0)) {
                clkrstGetClockRate(&this->m_clkrstSessionCpu, &cpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionGpu, &gpuClock);
                clkrstGetClockRate(&this->m_clkrstSessionMem, &memClock);
            } else {
                pcvGetClockRate(PcvModule_CpuBus, &cpuClock);
                pcvGetClockRate(PcvModule_GPU, &gpuClock);
                pcvGetClockRate(PcvModule_EMC, &memClock);
            }

            renderer->drawString(formatString("%.01f MHz", cpuClock / 1'000'000.0F).c_str(), false, 240, 230, 18, renderer->a(tsl::style::color::ColorHighlight));
            renderer->drawString(formatString("%.01f MHz", gpuClock / 1'000'000.0F).c_str(), false, 240, 260, 18, renderer->a(tsl::style::color::ColorHighlight));
            renderer->drawString(formatString("%.01f MHz", memClock / 1'000'000.0F).c_str(), false, 240, 290, 18, renderer->a(tsl::style::color::ColorHighlight));

            if (this->m_ipAddress == INADDR_LOOPBACK)
                renderer->drawString("Offline", false, 240, 330, 18, renderer->a(tsl::style::color::ColorHighlight));
            else 
                renderer->drawString(this->m_ipAddressString.c_str(), false, 240, 330, 18, renderer->a(tsl::style::color::ColorHighlight));

            s32 signalStrength = 0;
            wlaninfGetRSSI(&signalStrength);

            renderer->drawString(formatString("%d dBm", signalStrength).c_str(), false, 240, 360, 18, renderer->a(tsl::style::color::ColorHighlight)); 
        });

        rootFrame->setContent(infos);

        return rootFrame;
    }

    virtual void update() { }

private:
    ClkrstSession m_clkrstSessionCpu, m_clkrstSessionGpu, m_clkrstSessionMem;
    long m_ipAddress;
    std::string m_ipAddressString;
};

class GuiMain : public tsl::Gui {
public:
    GuiMain() { }

    ~GuiMain() { }

    virtual tsl::elm::Element* createUI() {
        auto *rootFrame = new tsl::elm::HeaderOverlayFrame();
        rootFrame->setHeader(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("EdiZon", false, 20, 50, 30, renderer->a(tsl::style::color::ColorText));
            renderer->drawString("v1.0.1", false, 20, 70, 15, renderer->a(tsl::style::color::ColorDescription));

            if (edz::cheat::CheatManager::getProcessID() != 0) {
                renderer->drawString("Program ID:", false, 150, 40, 15, renderer->a(tsl::style::color::ColorText));
                renderer->drawString("Build ID:", false, 150, 60, 15, renderer->a(tsl::style::color::ColorText));
                renderer->drawString("Process ID:", false, 150, 80, 15, renderer->a(tsl::style::color::ColorText));
                renderer->drawString(GuiMain::s_runningTitleIDString.c_str(), false, 250, 40, 15, renderer->a(tsl::style::color::ColorHighlight));
                renderer->drawString(GuiMain::s_runningBuildIDString.c_str(), false, 250, 60, 15, renderer->a(tsl::style::color::ColorHighlight));
                renderer->drawString(GuiMain::s_runningProcessIDString.c_str(), false, 250, 80, 15, renderer->a(tsl::style::color::ColorHighlight));
            }
        }));

        auto list = new tsl::elm::List();

        auto cheatsItem = new tsl::elm::ListItem("Cheats");
        auto statsItem  = new tsl::elm::ListItem("System information");
        cheatsItem->setClickListener([](s64 keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiCheats>();
                return true;
            }

            return false;
        });

        statsItem->setClickListener([](s64 keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<GuiStats>();
                return true;
            }

            return false;
        });

        list->addItem(cheatsItem);
        list->addItem(statsItem);

        rootFrame->setContent(list);

        return rootFrame;
    }

    virtual void update() { }

public:
    static inline std::string s_runningTitleIDString;
    static inline std::string s_runningProcessIDString;
    static inline std::string s_runningBuildIDString;
};

class EdiZonOverlay : public tsl::Overlay {
public:
    EdiZonOverlay() { }
    ~EdiZonOverlay() { }

    void initServices() override {
        dmntchtInitialize();
        edz::cheat::CheatManager::initialize();
        tsInitialize();
        wlaninfInitialize();
        clkrstInitialize();
        pcvInitialize();

    } 

    virtual void exitServices() override {
        dmntchtExit();
        edz::cheat::CheatManager::exit();
        tsExit();
        wlaninfExit();
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