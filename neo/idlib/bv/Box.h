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

#ifndef __BV_BOX_H__
#define __BV_BOX_H__

/*
===============================================================================

        Oriented Bounding Box

===============================================================================
*/

class idBox {
 public:
  idBox();
  explicit idBox(const idVec3& center, const idVec3& extents,
                 const idMat3& axis);
  explicit idBox(const idVec3& point);
  explicit idBox(const idBounds& bounds);
  explicit idBox(const idBounds& bounds, const idVec3& origin,
                 const idMat3& axis);

  idBox operator+(const idVec3& t) const;  // returns translated box
  idBox& operator+=(const idVec3& t);      // translate the box
  idBox operator*(const idMat3& r) const;  // returns rotated box
  idBox& operator*=(const idMat3& r);      // rotate the box
  idBox operator+(const idBox& a) const;
  idBox& operator+=(const idBox& a);
  idBox operator-(const idBox& a) const;
  idBox& operator-=(const idBox& a);

  bool Compare(const idBox& a) const;  // exact compare, no epsilon
  bool Compare(const idBox& a,
               const float epsilon) const;  // compare with epsilon
  bool operator==(const idBox& a) const;    // exact compare, no epsilon
  bool operator!=(const idBox& a) const;    // exact compare, no epsilon

  void Clear();  // inside out box
  void Zero();   // single point at origin

  const idVec3& GetCenter() const;   // returns center of the box
  const idVec3& GetExtents() const;  // returns extents of the box
  const idMat3& GetAxis() const;     // returns the axis of the box
  float GetVolume() const;           // returns the volume of the box
  bool IsCleared() const;            // returns true if box are inside out

  bool AddPoint(
      const idVec3& v);  // add the point, returns true if the box expanded
  bool AddBox(const idBox& a);  // add the box, returns true if the box expanded
  idBox Expand(const float d)
      const;  // return box expanded in all directions with the given value
  idBox& ExpandSelf(
      const float d);  // expand box in all directions with the given value
  idBox Translate(const idVec3& translation) const;  // return translated box
  idBox& TranslateSelf(const idVec3& translation);   // translate this box
  idBox Rotate(const idMat3& rotation) const;        // return rotated box
  idBox& RotateSelf(const idMat3& rotation);         // rotate this box

  float PlaneDistance(const idPlane& plane) const;
  int PlaneSide(const idPlane& plane, const float epsilon = ON_EPSILON) const;

  bool ContainsPoint(const idVec3& p) const;  // includes touching
  bool IntersectsBox(const idBox& a) const;   // includes touching
  bool LineIntersection(const idVec3& start, const idVec3& end) const;
  // intersection points are (start + dir * scale1) and (start + dir * scale2)
  bool RayIntersection(const idVec3& start, const idVec3& dir, float& scale1,
                       float& scale2) const;

  // tight box for a collection of points
  void FromPoints(const idVec3* points, const int numPoints);
  // most tight box for a translation
  void FromPointTranslation(const idVec3& point, const idVec3& translation);
  void FromBoxTranslation(const idBox& box, const idVec3& translation);
  // most tight box for a rotation
  void FromPointRotation(const idVec3& point, const idRotation& rotation);
  void FromBoxRotation(const idBox& box, const idRotation& rotation);

  void ToPoints(idVec3 points[8]) const;
  idSphere ToSphere() const;

  // calculates the projection of this box onto the given axis
  void AxisProjection(const idVec3& dir, float& min, float& max) const;
  void AxisProjection(const idMat3& ax, idBounds& bounds) const;

  // calculates the silhouette of the box
  int GetProjectionSilhouetteVerts(const idVec3& projectionOrigin,
                                   idVec3 silVerts[6]) const;
  int GetParallelProjectionSilhouetteVerts(const idVec3& projectionDir,
                                           idVec3 silVerts[6]) const;

 private:
  idVec3 center;
  idVec3 extents;
  idMat3 axis;
};

extern idBox box_zero;

inline idBox::idBox() {}

inline idBox::idBox(const idVec3& center, const idVec3& extents,
                    const idMat3& axis) {
  this->center = center;
  this->extents = extents;
  this->axis = axis;
}

inline idBox::idBox(const idVec3& point) {
  this->center = point;
  this->extents.Zero();
  this->axis.Identity();
}

inline idBox::idBox(const idBounds& bounds) {
  this->center = (bounds[0] + bounds[1]) * 0.5f;
  this->extents = bounds[1] - this->center;
  this->axis.Identity();
}

inline idBox::idBox(const idBounds& bounds, const idVec3& origin,
                    const idMat3& axis) {
  this->center = (bounds[0] + bounds[1]) * 0.5f;
  this->extents = bounds[1] - this->center;
  this->center = origin + this->center * axis;
  this->axis = axis;
}

inline idBox idBox::operator+(const idVec3& t) const {
  return idBox(center + t, extents, axis);
}

inline idBox& idBox::operator+=(const idVec3& t) {
  center += t;
  return *this;
}

inline idBox idBox::operator*(const idMat3& r) const {
  return idBox(center * r, extents, axis * r);
}

inline idBox& idBox::operator*=(const idMat3& r) {
  center *= r;
  axis *= r;
  return *this;
}

inline idBox idBox::operator+(const idBox& a) const {
  idBox newBox;
  newBox = *this;
  newBox.AddBox(a);
  return newBox;
}

inline idBox& idBox::operator+=(const idBox& a) {
  idBox::AddBox(a);
  return *this;
}

inline idBox idBox::operator-(const idBox& a) const {
  return idBox(center, extents - a.extents, axis);
}

inline idBox& idBox::operator-=(const idBox& a) {
  extents -= a.extents;
  return *this;
}

inline bool idBox::Compare(const idBox& a) const {
  return (center.Compare(a.center) && extents.Compare(a.extents) &&
          axis.Compare(a.axis));
}

inline bool idBox::Compare(const idBox& a, const float epsilon) const {
  return (center.Compare(a.center, epsilon) &&
          extents.Compare(a.extents, epsilon) && axis.Compare(a.axis, epsilon));
}

inline bool idBox::operator==(const idBox& a) const { return Compare(a); }

inline bool idBox::operator!=(const idBox& a) const { return !Compare(a); }

inline void idBox::Clear() {
  center.Zero();
  extents[0] = extents[1] = extents[2] = -idMath::INFINITUM;
  axis.Identity();
}

inline void idBox::Zero() {
  center.Zero();
  extents.Zero();
  axis.Identity();
}

inline const idVec3& idBox::GetCenter() const { return center; }

inline const idVec3& idBox::GetExtents() const { return extents; }

inline const idMat3& idBox::GetAxis() const { return axis; }

inline float idBox::GetVolume() const { return (extents * 2.0f).LengthSqr(); }

inline bool idBox::IsCleared() const { return extents[0] < 0.0f; }

inline idBox idBox::Expand(const float d) const {
  return idBox(center, extents + idVec3(d, d, d), axis);
}

inline idBox& idBox::ExpandSelf(const float d) {
  extents[0] += d;
  extents[1] += d;
  extents[2] += d;
  return *this;
}

inline idBox idBox::Translate(const idVec3& translation) const {
  return idBox(center + translation, extents, axis);
}

inline idBox& idBox::TranslateSelf(const idVec3& translation) {
  center += translation;
  return *this;
}

inline idBox idBox::Rotate(const idMat3& rotation) const {
  return idBox(center * rotation, extents, axis * rotation);
}

inline idBox& idBox::RotateSelf(const idMat3& rotation) {
  center *= rotation;
  axis *= rotation;
  return *this;
}

inline bool idBox::ContainsPoint(const idVec3& p) const {
  idVec3 lp = p - center;
  if (idMath::Fabs(lp * axis[0]) > extents[0] ||
      idMath::Fabs(lp * axis[1]) > extents[1] ||
      idMath::Fabs(lp * axis[2]) > extents[2]) {
    return false;
  }
  return true;
}

inline idSphere idBox::ToSphere() const {
  return idSphere(center, extents.Length());
}

inline void idBox::AxisProjection(const idVec3& dir, float& min,
                                  float& max) const {
  float d1 = dir * center;
  float d2 = idMath::Fabs(extents[0] * (dir * axis[0])) +
             idMath::Fabs(extents[1] * (dir * axis[1])) +
             idMath::Fabs(extents[2] * (dir * axis[2]));
  min = d1 - d2;
  max = d1 + d2;
}

inline void idBox::AxisProjection(const idMat3& ax, idBounds& bounds) const {
  for (int i = 0; i < 3; i++) {
    float d1 = ax[i] * center;
    float d2 = idMath::Fabs(extents[0] * (ax[i] * axis[0])) +
               idMath::Fabs(extents[1] * (ax[i] * axis[1])) +
               idMath::Fabs(extents[2] * (ax[i] * axis[2]));
    bounds[0][i] = d1 - d2;
    bounds[1][i] = d1 + d2;
  }
}

#endif /* !__BV_BOX_H__ */
