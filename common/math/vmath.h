// ///////////////////////////////////////////////////////////////////////////// //
// The MIT License (MIT)                                                         //
//                                                                               //
// Copyright (c) 2012-2020, Davide Bacchet (davide.bacchet@gmail.com)            //
//                                                                               //
// Permission is hereby granted, free of charge, to any person obtaining a copy  //
// of this software and associated documentation files (the "Software"), to deal //
// in the Software without restriction, including without limitation the rights  //
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     //
// copies of the Software, and to permit persons to whom the Software is         //
// furnished to do so, subject to the following conditions:                      //
//                                                                               //
// The above copyright notice and this permission notice shall be included in    //
// all copies or substantial portions of the Software.                           //
//                                                                               //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   //
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, //
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     //
// THE SOFTWARE.                                                                 //
// ///////////////////////////////////////////////////////////////////////////// //

#pragma once

#include "vmath_types.h"

namespace math {

// /////// //
// Vector2 //
// /////// //

/// get length of vector.
template <typename T> T length(const Vector2<T> &vec);
/// square of length.
template <typename T> T length2(const Vector2<T> &vec);
/// get the normalized vector
template <typename T> Vector2<T> normalized(const Vector2<T> &vec);
/// normalize vector
template <typename T> void normalize(Vector2<T> &vec);
/// linear interpolation of two vectors
template <typename T> Vector2<T> lerp(const Vector2<T> &v1, const Vector2<T> &v2, T fact);

// /////// //
// Vector3 //
// /////// //

/// get length of vector.
template <typename T> T length(const Vector3<T> &vec);
/// square of length.
template <typename T> T length2(const Vector3<T> &vec);
/// get the normalized vector
template <typename T> Vector3<T> normalized(const Vector3<T> &vec);
/// normalize vector
template <typename T> void normalize(Vector3<T> &vec);
/// linear interpolation of two vectors
template <typename T> Vector3<T> lerp(const Vector3<T> &v1, const Vector3<T> &v2, T fact);

// /////// //
// Vector4 //
// /////// //

/// get length of vector.
template <typename T> T length(const Vector4<T> &vec);
/// square of length.
template <typename T> T length2(const Vector4<T> &vec);
/// get the normalized vector
template <typename T> Vector4<T> normalized(const Vector4<T> &vec);
/// normalize vector
template <typename T> void normalize(Vector4<T> &vec);
/// linear interpolation of two vectors
template <typename T> Vector4<T> lerp(const Vector4<T> &v1, const Vector4<T> &v2, T fact);

// /////// //
// Matrix3 //
// /////// //

/// set matrix to zero
template <typename T> void set_zero(Matrix3<T> &mat);
/// set matrix to identity
template <typename T> void set_identity(Matrix3<T> &mat);
/// transpose
template <typename T> void transpose(Matrix3<T> &mat);
/// determinant
template <typename T> T det(const Matrix3<T> &mat);
/// calc inverse matrix
template <typename T> Matrix3<T> inverse(const Matrix3<T> &mat);
/// linear interpolation of two matrices
template <typename T> Matrix3<T> lerp(const Matrix3<T> &m1, const Matrix3<T> &m2, T fact);

// /////// //
// Matrix4 //
// /////// //

/// set matrix to zero
template <typename T> void set_zero(Matrix4<T> &mat);
/// set matrix to identity
template <typename T> void set_identity(Matrix4<T> &mat);
/// get translation vector
template <typename T> Vector3<T> translation(const Matrix4<T> &mat);
/// set translation part of matrix.
template <typename T> void set_translation(Matrix4<T> &mat, const Vector3<T> &v);
/// set matrix rotation part
template <typename T> void set_rotation(Matrix4<T> &mat, const Matrix3<T> &rot);
/// determinant
template <typename T> T det(const Matrix4<T> &m);
/// calc inverse matrix
template <typename T> Matrix4<T> inverse(const Matrix4<T> &m);
/// transpose
template <typename T> void transpose(Matrix4<T> &mat);
/// linear interpolation
template <typename T> Matrix4<T> lerp(const Matrix4<T> &m1, const Matrix4<T> &m2, T fact);

// ////////// //
// Quaternion //
// ////////// //

/// get length
template <typename T> T length(const Quaternion<T> &q);
/// square of length.
template <typename T> T length2(const Quaternion<T> &q);
/// get the normalized quaternion
template <typename T> Quaternion<T> normalized(const Quaternion<T> &q);
/// normalize quaternion
template <typename T> void normalize(Quaternion<T> &q);
/// get the rotation axis (a rotation quaternion is supposed to be normalized, but the function
/// will return the axis also for non-normal quaternions, assuming that w=cos(angle/2), azis=q.[xyz]*sin(angle/2))
template <typename T> Vector3<T> axis(const Quaternion<T> &q);
/// get the rotation angle (a rotation quaternion is supposed to be normalized, but the function
/// will return the angle also for non-normal quaternions, assuming that w=cos(angle/2))
template <typename T> T angle(const Quaternion<T> &q);
/// convert to rotation matrix.
template <typename T> Matrix3<T> rot_matrix(const Quaternion<T> &q);
/// Convert to transformation matrix.
/// @note same operation as rotmatrix() but returns a 4x4 Matrix
template <typename T> Matrix4<T> transform(const Quaternion<T> &q);
/// linear interpolation
template <typename T> Quaternion<T> lerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T fact);
/// spherical interpolation between quaternions (q1, q2). The input quaternions are assumed to be normalized
template <typename T> Quaternion<T> slerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T r);

// ///////// //
// factories //
// ///////// //

/// create identity matrix
template <typename T> Matrix3<T> matrix3_identity();
/// create identity matrix
template <typename T> Matrix4<T> matrix4_identity();
/// create a translation matrix
template <typename T> Matrix4<T> create_translation(const Vector3<T> &v);
/// create a transformation matrix
template <typename T> Matrix4<T> create_transformation(const Vector3<T> &v, const Quaternion<T> &q);
/// create a scaling matrix
template <typename T> Matrix4<T> create_scaling(const Vector3<T> &s);
/// create look-at matrix
template <typename T>
Matrix4<T> create_lookat(const Vector3<T> &eye, const Vector3<T> &to,
                         const Vector3<T> &up = Vector3<T>(T(0), T(0), T(1)));
/// quaternion from eulers angles. The order is Body321, i.e. first yaw (z) then pitch (y) then roll (x)
/// @param x Rotation around x axis (roll, in radians).
/// @param y Rotation around y axis (pitch, in radians).
/// @param z Rotation around z axis (yaw, in radians).
template <typename T> Quaternion<T> quat_from_euler_321(T x, T y, T z);
/// quaternion to eulers angles. The order is Body321, i.e. first yaw (z) then pitch (y) then roll (x)
/// @return vector with angles around x (roll), y (pitch), z(yaw) in radians.
template <typename T> Vector3<T> to_euler_321(Quaternion<T> const &q);
/// quaternion given axis and angle
/// @param axis Unit vector expressing axis of rotation. Note: the axis is NOT normalized internally!
/// @param angle Angle of rotation around axis (in radians).
template <typename T> Quaternion<T> quat_from_axis_angle(Vector3<T> axis, T angle);
/// quaternion from transformation matrix (only rotation part is kept)
template <typename T> Quaternion<T> quat_from_matrix(const Matrix4<T> &m);
/// quaternion from rotation matrix.
template <typename T> Quaternion<T> quat_from_matrix(const Matrix3<T> &m);

} // namespace math

#if not defined(VMATH_COMPILED_LIB)
#include "vmath_impl.h"
#endif

