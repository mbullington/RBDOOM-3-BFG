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
#ifndef __SORT_H__
#define __SORT_H__

/*
================================================================================================
Contains the generic templated sort algorithms for quick-sort, heap-sort and
insertion-sort.

The sort algorithms do not use class operators or overloaded functions to
compare objects because it is often desireable to sort the same objects in
different ways based on different keys (not just ascending and descending but
sometimes based on name and other times based on say priority). So instead, for
each different sort a separate class is implemented with a Compare() function.

This class is derived from one of the classes that implements a sort algorithm.
The Compare() member function does not only define how objects are sorted, the
class can also store additional data that can be used by the Compare() function.
This, for instance, allows a list of indices to be sorted where the indices
point to objects in an array. The base pointer of the array with objects can be
stored on the class that implements the Compare() function such that the
Compare() function can use keys that are stored on the objects.

The Compare() function is not virtual because this would incur significant
overhead. Do NOT make the Compare() function virtual on the derived class! The
sort implementations also explicitely call the Compare() function of the derived
class. This is to avoid various compiler bugs with using overloaded compare
functions and the inability of various compilers to find the right overloaded
compare function.

To sort an array, an idList or an idStaticList, a new sort class, typically
derived from idSort_Quick, is implemented as follows:

class idSort_MySort : public idSort_Quick< idMyObject, idSort_MySort > {
public:
        int Compare( const idMyObject & a, const idMyObject & b ) const {
                if ( a should come before b ) {
                        return -1; // or any negative integer
                } if ( a should come after b ) {
                        return 1;  // or any positive integer
                } else {
                        return 0;
                }
        }
};

To sort an array:

idMyObject array[100];
idSort_MySort().Sort( array, 100 );

To sort an idList:

idList< idMyObject > list;
list.Sort( idSort_MySort() );

The sort implementations never create temporaries of the template type. Only the
'SwapValues' template is used to move data around. This 'SwapValues' template
can be specialized to implement fast swapping of data. For instance, when
sorting a list with objects of some string class it is important to implement a
specialized 'SwapValues' for this string class to avoid excessive re-allocation
and copying of strings.

================================================================================================
*/

#include <utility>
#include "vendor/pdqsort/pdqsort.h"

/*
========================
SwapValues
========================
*/
template <typename _type_>
ID_INLINE void SwapValues(_type_& a, _type_& b) {
  std::swap(a, b);
}

/*
================================================
idSort is an abstract template class for sorting an array of objects of the
specified data type. The array of objects is sorted such that: Compare(
array[i], array[i+1] ) <= 0 for all i
================================================
*/
template <typename _type_>
class idSort {
 public:
  virtual ~idSort() {}
  virtual void Sort(_type_* base, unsigned int num) const = 0;
};

/*
================================================
idSort_Quick is a sort template that implements the
quick-sort algorithm on an array of objects of the specified data type.
================================================
*/
template <typename _type_, typename _derived_>
class idSort_Quick : public idSort<_type_> {
 public:
  virtual void Sort(_type_* base, unsigned int num) const {
    if (num <= 0) {
      return;
    }

    pdqsort(base, base + num, [&](const _type_& a, const _type_& b) {
      return static_cast<const _derived_*>(this)->Compare(a, b) < 0;
    });
  }
};

/*
================================================
Default quick-sort comparison function that can
be used to sort scalars from small to large.
================================================
*/
template <typename _type_>
class idSort_QuickDefault
    : public idSort_Quick<_type_, idSort_QuickDefault<_type_> > {
 public:
  int Compare(const _type_& a, const _type_& b) const { return a - b; }
};

/*
================================================
Specialization for floating point values to avoid an float-to-int
conversion for every comparison.
================================================
*/
template <>
class idSort_QuickDefault<float>
    : public idSort_Quick<float, idSort_QuickDefault<float> > {
 public:
  int Compare(const float& a, const float& b) const {
    if (a < b) {
      return -1;
    }
    if (a > b) {
      return 1;
    }
    return 0;
  }
};

#endif  // !__SORT_H__
