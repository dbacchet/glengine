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

#include <cmath>
#include <cassert>
#include <cstring>

#define VMATH_EPSILON (4.37114e-07)

namespace math {

//--------------------------------------
// Implementation
//--------------------------------------

// Vector2<T> implementation

template <typename T>
inline Vector2<T> &Vector2<T>::operator=(const Vector2<T> &rhs) {
    x = rhs.x;
    y = rhs.y;
    return *this;
}

template <typename T> inline T &Vector2<T>::operator[](int n) {
    assert(n >= 0 && n <= 1);
    if (0 == n)
        return x;
    else
        return y;
}

template <typename T> inline const T &Vector2<T>::operator[](int n) const {
    assert(n >= 0 && n <= 1);
    if (0 == n)
        return x;
    else
        return y;
}

template <typename T> inline Vector2<T> Vector2<T>::operator+(const Vector2<T> &rhs) const {
    return Vector2<T>(x + rhs.x, y + rhs.y);
}

template <typename T> inline Vector2<T> Vector2<T>::operator-(const Vector2<T> &rhs) const {
    return Vector2<T>(x - rhs.x, y - rhs.y);
}

template <typename T> inline void Vector2<T>::operator+=(const Vector2<T> &rhs) {
    x += rhs.x;
    y += rhs.y;
}

template <typename T> inline void Vector2<T>::operator-=(const Vector2<T> &rhs) {
    x -= rhs.x;
    y -= rhs.y;
}

template <typename T> inline T Vector2<T>::dot(const Vector2<T> &rhs) const {
    return x * rhs.x + y * rhs.y;
}

template <typename T> inline void Vector2<T>::operator+=(T rhs) {
    x += rhs;
    y += rhs;
}

template <typename T> inline void Vector2<T>::operator-=(T rhs) {
    x -= rhs;
    y -= rhs;
}

template <typename T> inline void Vector2<T>::operator*=(T rhs) {
    x *= rhs;
    y *= rhs;
}

template <typename T> inline void Vector2<T>::operator/=(T rhs) {
    x /= rhs;
    y /= rhs;
}

template <typename T> inline bool Vector2<T>::operator==(const Vector2<T> &rhs) const {
    return (std::fabs(x - rhs.x) < VMATH_EPSILON) && (std::fabs(y - rhs.y) < VMATH_EPSILON);
}

template <typename T> inline bool Vector2<T>::operator!=(const Vector2<T> &rhs) const {
    return !(*this == rhs);
}

template <typename T> inline Vector2<T> Vector2<T>::operator-() const {
    return Vector2<T>(-x, -y);
}


// Vector3<T> implementation //

template <typename T> 
inline Vector3<T> &Vector3<T>::operator=(const Vector3<T> &rhs) {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
}

template <typename T> inline T &Vector3<T>::operator[](int n) {
    assert(n >= 0 && n <= 2);
    if (0 == n)
        return x;
    else if (1 == n)
        return y;
    else
        return z;
}

template <typename T> inline const T &Vector3<T>::operator[](int n) const {
    assert(n >= 0 && n <= 2);
    if (0 == n)
        return x;
    else if (1 == n)
        return y;
    else
        return z;
}

template <typename T> inline Vector3<T> Vector3<T>::operator+(const Vector3<T> &rhs) const {
    return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
}

template <typename T> inline Vector3<T> Vector3<T>::operator-(const Vector3<T> &rhs) const {
    return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
}

template <typename T> inline void Vector3<T>::operator+=(const Vector3<T> &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
}

template <typename T> inline void Vector3<T>::operator-=(const Vector3<T> &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
}

template <typename T> inline T Vector3<T>::dot(const Vector3<T> &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
}

template <typename T> inline Vector3<T> Vector3<T>::cross(const Vector3<T> &rhs) const {
    return Vector3<T>(y * rhs.z - rhs.y * z, z * rhs.x - rhs.z * x, x * rhs.y - rhs.x * y);
}

template <typename T> inline void Vector3<T>::operator+=(T rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
}

template <typename T> inline void Vector3<T>::operator-=(T rhs) {
    x -= rhs;
    y -= rhs;
    z -= rhs;
}

template <typename T> inline void Vector3<T>::operator*=(T rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
}

template <typename T> inline void Vector3<T>::operator/=(T rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
}

template <typename T> inline bool Vector3<T>::operator==(const Vector3<T> &rhs) const {
    return std::abs(x - rhs.x) < VMATH_EPSILON && std::abs(y - rhs.y) < VMATH_EPSILON && std::abs(z - rhs.z) < VMATH_EPSILON;
}

template <typename T> inline bool Vector3<T>::operator!=(const Vector3<T> &rhs) const {
    return !(*this == rhs);
}

template <typename T> inline Vector3<T> Vector3<T>::operator-() const {
    return Vector3<T>(-x, -y, -z);
}

// Vector4<T> implementation

template <typename T>
inline Vector4<T> &Vector4<T>::operator=(const Vector4<T> &rhs) {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    w = rhs.w;
    return *this;
}

template <typename T> inline T &Vector4<T>::operator[](int n) {
    assert(n >= 0 && n <= 3);
    if (0 == n)
        return x;
    else if (1 == n)
        return y;
    else if (2 == n)
        return z;
    else
        return w;
}

template <typename T> inline const T &Vector4<T>::operator[](int n) const {
    assert(n >= 0 && n <= 3);
    if (0 == n)
        return x;
    else if (1 == n)
        return y;
    else if (2 == n)
        return z;
    else
        return w;
}

template <typename T> inline Vector4<T> Vector4<T>::operator+(const Vector4<T> &rhs) const {
    return Vector4<T>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

template <typename T> inline Vector4<T> Vector4<T>::operator-(const Vector4<T> &rhs) const {
    return Vector4<T>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

template <typename T> inline void Vector4<T>::operator+=(const Vector4<T> &rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
}

template <typename T> inline void Vector4<T>::operator-=(const Vector4<T> &rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
}

template <typename T> inline T Vector4<T>::dot(const Vector4<T> &rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
}


template <typename T> inline bool Vector4<T>::operator==(const Vector4<T> &rhs) const {
    return std::abs(x - rhs.x) < VMATH_EPSILON && std::abs(y - rhs.y) < VMATH_EPSILON && std::abs(z - rhs.z) < VMATH_EPSILON &&
           std::abs(w - rhs.w) < VMATH_EPSILON;
}

template <typename T> inline bool Vector4<T>::operator!=(const Vector4<T> &rhs) const {
    return !(*this == rhs);
}

template <typename T> inline Vector4<T> Vector4<T>::operator-() const {
    return Vector4<T>(-x, -y, -z, -w);
}

template <typename T> inline void Vector4<T>::operator+=(T rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
    w += rhs;
}

template <typename T> inline void Vector4<T>::operator-=(T rhs) {
    x -= rhs;
    y -= rhs;
    z -= rhs;
    w -= rhs;
}

template <typename T> inline void Vector4<T>::operator*=(T rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
}

template <typename T> inline void Vector4<T>::operator/=(T rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
}

// Matrix3<T> implementation

template <typename T>
inline Matrix3<T>::Matrix3() ///< default to null matrix
{
    for (int i = 0; i < 9; i++)
        data[i] = T(0);
}

template <typename T> inline Matrix3<T>::Matrix3(const Matrix3<T> &src) {
    std::memcpy(data, src.data, sizeof(T) * 9);
}

template <typename T> inline Matrix3<T>::Matrix3(const T *dt) {
    for (int k=0; k<9; k++) {
        data[k] = dt[(k%3)*3 + k/3];
    }
}

template <typename T> inline Matrix3<T>::Matrix3(std::initializer_list<T> init) {
    assert(init.size()>=9);
    const T *v = init.begin();
    for (int k=0; k<9; k++) {
        data[k] = v[(k%3)*3 + k/3];
    }
}

template <typename T> inline bool Matrix3<T>::operator==(const Matrix3<T> &rhs) const {
    for (int i = 0; i < 9; i++)
        if (std::abs(data[i] - rhs.data[i]) >= VMATH_EPSILON)
            return false;
    return true;
}

template <typename T> inline bool Matrix3<T>::operator!=(const Matrix3<T> &rhs) const {
    return !(*this == rhs);
}

template <typename T> inline T &Matrix3<T>::operator()(int i, int j) {
    assert(i >= 0 && i < 3);
    assert(j >= 0 && j < 3);
    return data[j * 3 + i];
}

template <typename T> inline const T &Matrix3<T>::operator()(int i, int j) const {
    assert(i >= 0 && i < 3);
    assert(j >= 0 && j < 3);
    return data[j * 3 + i];
}

template <typename T> inline T &Matrix3<T>::at(int x, int y) {
    assert(x >= 0 && x < 3);
    assert(y >= 0 && y < 3);
    return data[x * 3 + y];
}

template <typename T> inline const T &Matrix3<T>::at(int x, int y) const {
    assert(x >= 0 && x < 3);
    assert(y >= 0 && y < 3);
    return data[x * 3 + y];
}

template <typename T> inline Matrix3<T> &Matrix3<T>::operator=(const Matrix3<T> &rhs) {
    std::memcpy(data, rhs.data, sizeof(T) * 9);
    return *this;
}

template <typename T> inline void Matrix3<T>::operator+=(T rhs) {
    for (int i = 0; i < 9; i++)
        data[i] += rhs;
}

template <typename T> inline void Matrix3<T>::operator-=(T rhs) {
    for (int i = 0; i < 9; i++)
        data[i] -= rhs;
}

template <typename T> inline void Matrix3<T>::operator*=(T rhs) {
    for (int i = 0; i < 9; i++)
        data[i] *= rhs;
}

template <typename T> inline void Matrix3<T>::operator/=(T rhs) {
    for (int i = 0; i < 9; i++)
        data[i] /= rhs;
}


// Matrix4<T> implementation

template <typename T>
inline Matrix4<T>::Matrix4() // default to identity
{
    for (int i = 0; i < 16; i++)
        data[i] = T(0);
}

template <typename T> inline Matrix4<T>::Matrix4(const T *dt) {
    for (int k=0; k<16; k++) {
        data[k] = dt[(k%4)*4 + k/4];
    }
}

template <typename T> inline Matrix4<T>::Matrix4(std::initializer_list<T> init) {
    assert(init.size()>=16);
    const T *v = init.begin();
    for (int k=0; k<16; k++) {
        data[k] = v[(k%4)*4 + k/4];
    }
}


template <typename T> inline Matrix4<T>::Matrix4(const Matrix4<T> &src) {
    std::memcpy(data, src.data, 16 * sizeof(T));
}

template <typename T> inline bool Matrix4<T>::operator==(const Matrix4<T> &rhs) const {
    for (int i = 0; i < 16; i++) {
        if (std::abs(data[i] - rhs.data[i]) >= VMATH_EPSILON)
            return false;
    }
    return true;
}

template <typename T> inline bool Matrix4<T>::operator!=(const Matrix4<T> &rhs) const {
    return !(*this == rhs);
}

template <typename T> inline T &Matrix4<T>::at(int x, int y) {
    assert(x >= 0 && x < 4);
    assert(y >= 0 && y < 4);
    return data[x * 4 + y];
}

template <typename T> inline const T &Matrix4<T>::at(int x, int y) const {
    assert(x >= 0 && x < 4);
    assert(y >= 0 && y < 4);
    return data[x * 4 + y];
}

template <typename T> inline T &Matrix4<T>::operator()(int i, int j) {
    assert(i >= 0 && i < 4);
    assert(j >= 0 && j < 4);
    return data[j * 4 + i];
}

template <typename T> inline const T &Matrix4<T>::operator()(int i, int j) const {
    assert(i >= 0 && i < 4);
    assert(j >= 0 && j < 4);
    return data[j * 4 + i];
}

template <typename T> inline Matrix4<T> &Matrix4<T>::operator=(const Matrix4<T> &rhs) {
    std::memcpy(data, rhs.data, 16 * sizeof(T));
    return *this;
}

template <typename T> inline void Matrix4<T>::operator+=(T rhs) {
    for (int i = 0; i < 16; i++)
        data[i] += rhs;
}

template <typename T> inline void Matrix4<T>::operator-=(T rhs) {
    for (int i = 0; i < 16; i++)
        data[i] -= rhs;
}

template <typename T> inline void Matrix4<T>::operator*=(T rhs) {
    for (int i = 0; i < 16; i++)
        data[i] *= rhs;
}

template <typename T> inline void Matrix4<T>::operator/=(T rhs) {
    for (int i = 0; i < 16; i++)
        data[i] /= rhs;
}


// Quaternion<T> implementation

template <typename T> 
inline Quaternion<T> &Quaternion<T>::operator=(const Quaternion<T> &rhs) {
    w = rhs.w;
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
}

template <typename T> inline void Quaternion<T>::operator+=(const Quaternion<T> &rhs) {
    w += rhs.w;
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
}

template <typename T> inline void Quaternion<T>::operator-=(const Quaternion<T> &rhs) {
    w -= rhs.w;
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
}

template <typename T> inline void Quaternion<T>::operator+=(T rhs) {
    w += rhs;
    x += rhs;
    y += rhs;
    z += rhs;
}

template <typename T> inline void Quaternion<T>::operator-=(T rhs) {
    w -= rhs;
    x -= rhs;
    y -= rhs;
    z -= rhs;
}

template <typename T> inline void Quaternion<T>::operator*=(T rhs) {
    w *= rhs;
    x *= rhs;
    y *= rhs;
    z *= rhs;
}

template <typename T> inline bool Quaternion<T>::operator==(const Quaternion<T> &rhs) const {
    const Quaternion<T> &lhs = *this;
    return (std::abs(lhs.w - rhs.w) < VMATH_EPSILON) 
        && (std::abs(lhs.x - rhs.x) < VMATH_EPSILON)
        && (std::abs(lhs.y - rhs.y) < VMATH_EPSILON)
        && (std::abs(lhs.z - rhs.z) < VMATH_EPSILON);
}

template <typename T> inline bool Quaternion<T>::operator!=(const Quaternion<T> &rhs) const {
    return !(*this == rhs);
}

} // namespace math
