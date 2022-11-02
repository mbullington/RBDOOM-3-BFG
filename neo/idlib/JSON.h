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
#pragma once

namespace id {

// Simple JSON interface that relatively mimicks 'rapidjson', but uses 'yyjson'
// internally.
//
// API reference:
// https://github.com/ibireme/yyjson/blob/master/src/yyjson.h
class JSON {
 public:
  JSON(const char *text, size_t len);

  // Value methods.

  bool IsString() const;
  bool IsInt() const;

  int GetInt() const;
  double GetDouble() const;

  uint32 GetUint() const;

  idStr GetString() const;

  bool operator==(const char *str) const;

  // Object/array methods.

  bool IsObject() const;
  bool IsArray() const;

  JSON operator[](const char *key) const;
  JSON operator[](const int index) const;

  bool HasMember(const char *key) const;

  size_t Size() const;

  // JSON Pointer methods.
  // https://rapidjson.org/md_doc_pointer.html

  JSON Pointer(const char *pointer) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(JSON);

  JSON(void *root) : root(root) {}

  void *root;
};

}  // namespace id