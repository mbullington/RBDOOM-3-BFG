/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition
Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your option)
any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see
<http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain
additional terms. You should have received a copy of these additional terms
immediately following the terms and conditions of the GNU General Public License
which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a
copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite
120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "precompiled.h"
#pragma hdrstop

#include "vendor/yyjson/src/yyjson.h"

namespace id {

JSON::JSON(const char *text, size_t len)
    : JSON(yyjson_doc_get_root(yyjson_read(text, len, 0))) {}

// Value methods.

bool JSON::IsString() const { return yyjson_is_str((yyjson_val *)root); }
bool JSON::IsInt() const { return yyjson_is_int((yyjson_val *)root); }

int JSON::GetInt() const { return yyjson_get_int((yyjson_val *)root); }
double JSON::GetDouble() const { return yyjson_get_real((yyjson_val *)root); }

uint32_t JSON::GetUint() const {
  return static_cast<uint32_t>(yyjson_get_uint((yyjson_val *)root));
}

idStr JSON::GetString() const { return yyjson_get_str((yyjson_val *)root); }

bool JSON::operator==(const char *str) const {
  return strcmp(yyjson_get_str((yyjson_val *)root), str) == 0;
}

// Object/array methods.

bool JSON::IsObject() const { return yyjson_is_obj((yyjson_val *)root); }
bool JSON::IsArray() const { return yyjson_is_arr((yyjson_val *)root); }

JSON JSON::operator[](const char *key) const {
  return JSON(yyjson_obj_get((yyjson_val *)root, key));
}
JSON JSON::operator[](const int index) const {
  return JSON(yyjson_arr_get((yyjson_val *)root, index));
}

bool JSON::HasMember(const char *key) const {
  return yyjson_obj_get((yyjson_val *)root, key) != nullptr;
}

size_t JSON::Size() const { return yyjson_arr_size((yyjson_val *)root); }

// Pointer methods.

JSON JSON::Pointer(const char *pointer) const {
  return JSON(yyjson_get_pointer((yyjson_val *)root, pointer));
}

}  // namespace id