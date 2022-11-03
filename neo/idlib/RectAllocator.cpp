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

/*

This routine performs a tight packing of a list of rectangles, attempting to
minimize the area of the rectangle that encloses all of them.

Contrast with idBitBlockAllocator, which is used incrementally with either fixed
size or size-doubling target areas.

Typical uses:
packing glyphs into a font image
packing model surfaces into a skin atlas
packing images into swf atlases

If you want a minimum alignment, ensure that all the sizes are multiples of that
alignment, or scale the input sizes down by that alignment and scale the
outputPositions back up.

Please see LICENSE_EXEMPTIONS.md included with this repository, as this code is
based off of 'potpack', a JavaScript library by Volodymyr Agafonkin under the
ISC License.
*/

class idSortBoxesHeight : public idSort_Quick<int, idSortBoxesHeight> {
 public:
  int SizeMetric(idVec2i v) const { return v.y; }
  int Compare(const int& a, const int& b) const {
    return SizeMetric((*inputSizes)[b]) - SizeMetric((*inputSizes)[a]);
  }
  const idList<idVec2i>* inputSizes;
};

struct idBoxSpace {
  int x, y, w, h;
};

void RectAllocator(const idList<idVec2i>& inputSizes,
                   idList<idVec2i>& outputPositions, idVec2i& totalSize,
                   float* packingFraction = NULL) {
  outputPositions.SetNum(inputSizes.Num());
  if (inputSizes.Num() == 0) {
    totalSize.Set(0, 0);
    return;
  }
  idList<int> sizeRemap;
  sizeRemap.SetNum(inputSizes.Num());
  for (int i = 0; i < inputSizes.Num(); i++) {
    sizeRemap[i] = i;
  }

  // calculate total box area and maximum box width
  int area = 0;
  int maxWidth = 0;

  for (const auto& box : inputSizes) {
    area += box.x * box.y;
    maxWidth = Max(maxWidth, box.x);
  }

  // sort the boxes for insertion by height, descending
  idSortBoxesHeight sortrectsBySize;
  sortrectsBySize.inputSizes = &inputSizes;
  sizeRemap.SortWithTemplate(sortrectsBySize);

  // aim for a squarish resulting container,
  // slightly adjusted for sub-100% space utilization
  const int startWidth =
      Max(static_cast<int>(ceil(sqrt(area / 0.95f))), maxWidth);

  // start with a single empty space, unbounded at the bottom
  idList<idBoxSpace> spaces;
  spaces.Append(idBoxSpace{.x = 0, .y = 0, .w = startWidth, .h = INT_MAX});

  int width = 0;
  int height = 0;

  for (int boxIdx : sizeRemap) {
    const idVec2i& box = inputSizes[boxIdx];

    // look through spaces backwards so that we check smaller spaces first
    for (int i = spaces.Size() - 1; i >= 0; i--) {
      idBoxSpace& space = spaces[i];

      // look for empty spaces that can accommodate the current box
      if (box.x > space.w || box.y > space.h) continue;

      // found the space; add the box to its top-left corner
      // |-------|-------|
      // |  box  |       |
      // |_______|       |
      // |         space |
      // |_______________|
      auto& outputPos = outputPositions[boxIdx];
      outputPos.Set(space.x, space.y);

      height = Max(height, outputPos.y + box.x);
      width = Max(width, outputPos.x + box.y);

      if (box.x == space.w && box.y == space.h) {
        // space matches the box exactly; remove it
        int lastIdx = spaces.Size() - 1;
        if (i == lastIdx) {
          spaces[i] = spaces[lastIdx];
        }
        spaces.RemoveIndexFast(lastIdx);
      } else if (box.x == space.h) {
        // space matches the box height; update it accordingly
        // |-------|---------------|
        // |  box  | updated space |
        // |_______|_______________|
        space.x += box.x;
        space.w -= box.x;

      } else if (box.y == space.w) {
        // space matches the box width; update it accordingly
        // |---------------|
        // |      box      |
        // |_______________|
        // | updated space |
        // |_______________|
        space.y += box.x;
        space.h -= box.y;

      } else {
        // otherwise the box splits the space into two spaces
        // |-------|-----------|
        // |  box  | new space |
        // |_______|___________|
        // | updated space     |
        // |___________________|
        spaces.Append({
            .x = space.x + box.x,
            .y = space.y,
            .w = space.w - box.x,
            .h = box.y,
        });
        space.y += box.y;
        space.h -= box.y;
      }
      break;
    }
  }

  totalSize.Set(width, height);
  if (packingFraction) {
    if (width * height == 0) {
      *packingFraction = 0;
    } else {
      *packingFraction = ((area * 1.0f) / (width * height));
    }
  }
}
