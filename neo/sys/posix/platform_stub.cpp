/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License which accompanied the
Doom 3 Source Code.  If not, please request a copy in writing from id Software
at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite
120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "../../idlib/precompiled.h"
#include "../posix/posix_public.h"
//#include "../sys_local.h"

const char* Sys_EXEPath() {
  static char path[1024];
  uint32_t size = sizeof(path);
  return path;
}

cpuid_t Sys_GetProcessorId() { return CPUID_GENERIC; }

const char* Sys_GetProcessorString() { return "generic"; }

double Sys_ClockTicksPerSecond() { return 0; }

void Sys_CPUCount(int& numLogicalCPUCores, int& numPhysicalCPUCores,
                  int& numCPUPackages) {}

void Sys_DoStartProcess(const char* exeName, bool dofork) {}

/*
 ==================
 Sys_DoPreferences
 ==================
 */
void Sys_DoPreferences() {}

const char* Sys_GetCmdLine() {
  // DG: don't use this, use cmdargv and cmdargc instead!
  return "TODO Sys_GetCmdLine";
}

void Sys_ReLaunch() {}
