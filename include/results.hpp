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
#include "result.hpp"

namespace edz {

    #define EDZHBRES(desc)  MAKERESULT(MODULE_EDIZONHB, desc)
    #define EDZSYSRES(desc) MAKERESULT(MODULE_EDIZONSYS, desc)

    /* Common error codes */
    constexpr EResult ResultSuccess                          = MAKERESULT(0, 0);

    /* Homebrew error codes */
    constexpr EResult ResultEdzBorealisInitFailed            = EDZHBRES(1);
    constexpr EResult ResultEdzCurlInitFailed                = EDZHBRES(2);
    constexpr EResult ResultEdzNotYetImplemented             = EDZHBRES(3);
    constexpr EResult ResultEdzErrorDuringErrorHandling      = EDZHBRES(4);
    constexpr EResult ResultEdzNotFound                      = EDZHBRES(5);
    constexpr EResult ResultEdzOperationFailed               = EDZHBRES(6);

    constexpr EResult ResultEdzSysmoduleAlreadyRunning       = EDZHBRES(101);
    constexpr EResult ResultEdzSysmoduleNotRunning           = EDZHBRES(102);
    constexpr EResult ResultEdzSysmoduleLaunchFailed         = EDZHBRES(103);
    constexpr EResult ResultEdzSysmoduleTerminationFailed    = EDZHBRES(104);

    constexpr EResult ResultEdzEditorNoConfigFound           = EDZHBRES(201);
    constexpr EResult ResultEdzEditorNoScriptFound           = EDZHBRES(202);
    constexpr EResult ResultEdzEditorConfigOutdated          = EDZHBRES(203);
    constexpr EResult ResultEdzEditorScriptSyntaxError       = EDZHBRES(204);
    constexpr EResult ResultEdzEditorTooManyRedirects        = EDZHBRES(205);
    constexpr EResult ResultEdzEditorAlreadyLoaded           = EDZHBRES(206);
    constexpr EResult ResultEdzEditorNotLoaded               = EDZHBRES(207);
    constexpr EResult ResultEdzEditorLoadFailed              = EDZHBRES(208);
    constexpr EResult ResultEdzEditorStoreFailed             = EDZHBRES(209);

    constexpr EResult ResultEdzScriptRuntimeError            = EDZHBRES(301);
    constexpr EResult ResultEdzScriptInvalidArgument         = EDZHBRES(302);

    constexpr EResult ResultEdzCurlError                     = EDZHBRES(401);
    constexpr EResult ResultEdzAPIError                      = EDZHBRES(402);

    constexpr EResult ResultEdzSaveNoSaveFS                  = EDZHBRES(501);
    constexpr EResult ResultEdzSaveNoSuchBackup              = EDZHBRES(502);
    constexpr EResult ResultEdzSaveInvalidArgument           = EDZHBRES(503);

    constexpr EResult ResultEdzCheatParsingFailed            = EDZHBRES(601);
    constexpr EResult ResultEdzCheatServiceNotAvailable      = EDZHBRES(602);

    constexpr EResult ResultEdzMemoryReadFailed              = EDZHBRES(701);
    constexpr EResult ResultEdzMemoryOverflow                = EDZHBRES(702);
    constexpr EResult ResultEdzOutOfRange                    = EDZHBRES(703);
    constexpr EResult ResultEdzNoValuesFound                 = EDZHBRES(704);
    constexpr EResult ResultEdzInvalidOperation              = EDZHBRES(705);

    /* SysmoduEle error codes */
    constexpr EResult ResultEdzAlreadyRegistered              = EDZSYSRES(1);
    constexpr EResult ResultEdzUnknownButtonID                = EDZSYSRES(2);
    constexpr EResult ResultEdzInvalidButtonCombination       = EDZSYSRES(3);
    constexpr EResult ResultEdzAttachFailed                   = EDZSYSRES(4);
    constexpr EResult ResultEdzDetachFailed                   = EDZSYSRES(5);
    constexpr EResult ResultEdzInvalidBuffer                  = EDZSYSRES(6);

    constexpr EResult ResultEdzTCPInitFailed                  = EDZSYSRES(101);
    constexpr EResult ResultEdzUSBInitFailed                  = EDZSYSRES(102);
    constexpr EResult ResultEdzScreenInitFailed               = EDZSYSRES(103);
    constexpr EResult ResultEdzFontInitFailed                 = EDZSYSRES(104);

    constexpr EResult ResultEdzSocketInitFailed               = EDZSYSRES(201);

    constexpr EResult ResultEdzAbortFailed                    = EDZSYSRES(301);
}