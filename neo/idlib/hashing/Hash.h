/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2022 Michael Bullington

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

#include <cstdlib>
#include <cstring>

namespace id {

// ================================================
// HashAccum is a simple hash accumulator that can be used to hash a series of
// values.
//
// It is not intended to be used for hashing large amounts of data as it
// is O(n) in the number of values hashed.
// ================================================
template <typename T>
class HashAccum {
  using ChecksumFunction = T(const void* data, int length);

 public:
  HashAccum(ChecksumFunction* checksumFunction)
      : checksumFunction(checksumFunction) {
    Reset();
  }

  ~HashAccum() {
    if (length > 0) {
      free(data);
    }
  }

  void Reset() {
    data = NULL;
    length = 0;
  }

  void Update(const void* data_, int length_) {
    if (length_ == 0) {
      return;
    }

    if (length == 0) {
      data = malloc(length_);
      memcpy(data, data_, length_);
      length = length_;
    } else {
      data = realloc(data, length + length_);
      memcpy((char*)data + length, data_, length_);
      length += length_;
    }
  }

  T Checksum() const { return checksumFunction(data, length); }

 private:
  ChecksumFunction* checksumFunction;

  void* data;
  int length;
};

}  // namespace id