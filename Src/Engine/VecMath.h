/**
* @file VecMath.h
*/
#ifndef VECMATH_H_INCLUDED
#define VECMATH_H_INCLUDED
#include <cmath>

// 先行宣言
struct vec2;
struct vec3;
struct vec4;
struct mat3;
struct mat4;

// 円周率
constexpr float PI = 3.1415926535f;

// 度数法を弧度法に変換
constexpr float radians(float deg)
{
	return deg * PI / 180.0f;
}

// 弧度法を度数法に変換
constexpr float degrees(float rad)
{
	return rad * 180.0f / PI;
}

struct vec2
{
	float x, y;

	vec2() = default;

	explicit constexpr vec2(const vec3&);
	explicit constexpr vec2(const vec4&);
	constexpr vec2(float n) : x(n), y(n) {}
	constexpr vec2(float x, float y) : x(x), y(y) {}

	float operator[](size_t i) const { return *(&x + i); }
	float& operator[](size_t i) { return *(&x + i); }
};

struct vec3
{
	float x, y, z;

	vec3() = default;
	explicit constexpr vec3(const vec4&);
	explicit constexpr vec3(float n) : x(n), y(n), z(n) {}
	constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	constexpr vec3(const vec2& v, float z) : x(v.x), y(v.y), z(z) {}

	float operator[](size_t i) const { return *(&x + i); }
	float& operator[](size_t i) { return *(&x + i); }
};

struct vec4
{
  float x, y, z, w;

  vec4() = default;
  explicit constexpr vec4(float n) : x(n), y(n), z(n), w(n) {}
  constexpr vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
  constexpr vec4(const vec2& v, float z, float w) : x(v.x), y(v.y), z(z), w(w) {}
  constexpr vec4(const vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}

  float operator[](size_t i) const { return *(&x + i); }
  float& operator[](size_t i) { return *(&x + i); }
};

constexpr vec2::vec2(const vec3 & v) : x(v.x), y(v.y) {}
constexpr vec2::vec2(const vec4 & v) : x(v.x), y(v.y) {}
constexpr vec3::vec3(const vec4 & v) : x(v.x), y(v.y), z(v.z) {}

// vec2の単項演算子
inline vec2 operator-(const vec2& a) { return { -a.x,-a.y}; }

// vec2同士の加算
inline vec2& operator+=(vec2& a, const vec2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}
inline vec2 operator+(const vec2& a, const vec2& b) { vec2 n = a; return n += b; }

// vec2とfloatの加算
inline vec2& operator+=(vec2& a, float b) { return a += {b, b}; }
inline vec2 operator+(const vec2& a, float b) { vec2 n = a; return n += b; }
inline vec2 operator+(float a, const vec2& b) { return b + a; }

// vec2同士の減算
inline vec2& operator-=(vec2& a, const vec2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}
inline vec2 operator-(const vec2& a, const vec2& b) { vec2 n = a; return n -= b; }

// vec2とfloatの減算
inline vec2& operator-=(vec2& a, float b) { return a -= {b, b}; }
inline vec2 operator-(const vec2& a, float b) { vec2 n = a; return n -= b; }
inline vec2 operator-(float a, const vec2& b) { return vec2{ a,a} - b; }

// vec2同士の乗算
inline vec2& operator*=(vec2& a, const vec2& b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}
inline vec2 operator*(const vec2& a, const vec2& b) { vec2 n = a; return n *= b; }

// vec2とfloatの乗算
inline vec2& operator*=(vec2& a, float b) { return a *= {b, b}; }
inline vec2 operator*(const vec2& a, float b) { vec2 n = a; return n *= b; }
inline vec2 operator* (float a, const vec2& b) { return b * a; }

// vec2同士の除算
inline vec2& operator/=(vec2& a, const vec2& b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}
inline vec2 operator/(const vec2& a, const vec2& b) { vec2 n = a; return n /= b; }

// vec2とfloatの除算
inline vec2& operator/=(vec2& a, float b) { return a /= {b, b}; }
inline vec2 operator/(const vec2& a, float b) { vec2 n = a; return n /= b; }
inline vec2 operator/(float a, const vec2& b) { return vec2{ a,a } / b; }


// vec3の単項演算子
inline vec3 operator-(const vec3& a) { return { -a.x,-a.y,-a.z }; }

// vec3同士の加算
inline vec3& operator+=(vec3& a, const vec3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
inline vec3 operator+(const vec3& a, const vec3& b) { vec3 n = a; return n += b; }

// vec3とfloatの加算
inline vec3& operator+=(vec3& a, float b) { return a += {b, b, b}; }
inline vec3 operator+(const vec3& a, float b) { vec3 n = a; return n += b; }
inline vec3 operator+(float a, const vec3& b) { return b + a; }

// vec3同士の減算
inline vec3& operator-=(vec3& a, const vec3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}
inline vec3 operator-(const vec3& a, const vec3& b) { vec3 n = a; return n -= b; }

// vec3とfloatの減算
inline vec3& operator-=(vec3& a, float b) { return a -= {b, b, b}; }
inline vec3 operator-(const vec3& a, float b) { vec3 n = a; return n -= b; }
inline vec3 operator-(float a, const vec3& b) { return vec3{a,a,a} - b; }

// vec3同士の乗算
inline vec3& operator*=(vec3& a, const vec3& b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}
inline vec3 operator*(const vec3& a, const vec3& b) { vec3 n = a; return n *= b; }

// vec3とfloatの乗算
inline vec3& operator*=(vec3& a, float b) { return a *= {b, b, b}; }
inline vec3 operator*(const vec3& a, float b) { vec3 n = a; return n *= b; }
inline vec3 operator* (float a, const vec3& b) { return b * a; }

// vec3同士の除算
inline vec3& operator/=(vec3& a, const vec3& b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}
inline vec3 operator/(const vec3& a, const vec3& b) { vec3 n = a; return n /= b; }

// vec3とfloatの除算
inline vec3& operator/=(vec3& a, float b) { return a /= {b, b, b}; }
inline vec3 operator/(const vec3& a, float b) { vec3 n = a; return n /= b; }
inline vec3 operator/(float a, const vec3& b) { return vec3{ a,a,a } / b; }

// vec3の内積
inline float dot
(
	const vec3& a,
	const vec3& b
)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

// vec3の外積
inline vec3 cross
(
	const vec3& a,
	const vec3& b
)
{
	return
	{
	  a.y * b.z - a.z * b.y,
	  a.z * b.x - a.x * b.z,
	  a.x * b.y - a.y * b.x
	};
}

// vec3の長さ
inline float length(const vec3& a)
{
	return sqrt(dot(a, a));
}

// vec3の正規化
inline vec3 normalize(const vec3& a)
{
	return a / length(a);
}

// vec4の単項演算子
inline vec4 operator-(const vec4& a) { return { -a.x,-a.y,-a.z,-a.w }; }

// vec4同士の加算
inline vec4& operator+=(vec4& a, const vec4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}
inline vec4 operator+(const vec4& a, const vec4& b) { vec4 n = a; return n += b; }

// vec4とfloatの加算
inline vec4& operator+=(vec4& a, float b) { return a += {b, b, b, b}; }
inline vec4 operator+(const vec4& a, float b) { vec4 n = a; return n += b; }
inline vec4 operator+(float a, const vec4& b) { return b + a; }

// vec4同士の減算
inline vec4& operator-=(vec4& a, const vec4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}
inline vec4 operator-(const vec4& a, const vec4& b) { vec4 n = a; return n -= b; }

// vec4とfloatの減算
inline vec4& operator-=(vec4& a, float b) { return a -= {b, b, b, b}; }
inline vec4 operator-(const vec4& a, float b) { vec4 n = a; return n -= b; }
inline vec4 operator-(float a, const vec4& b) { return vec4{ a,a,a,a } - b; }

// vec4同士の乗算
inline vec4& operator*=(vec4& a, const vec4& b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
	return a;
}
inline vec4 operator*(const vec4& a, const vec4& b) { vec4 n = a; return n *= b; }

// vec4とfloatの乗算
inline vec4& operator*=(vec4& a, float b) { return a *= {b, b, b, b}; }
inline vec4 operator*(const vec4& a, float b) { vec4 n = a; return n *= b; }
inline vec4 operator*(float a, const vec4& b) { return b * a; }

// vec4同士の除算
inline vec4& operator/=(vec4& a, const vec4& b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	a.w /= b.w;
	return a;
}
inline vec4 operator/(const vec4& a, const vec4& b) { vec4 n = a; return n /= b; }

// vec4とfloatの除算
inline vec4& operator/=(vec4& a, float b) { return a /= {b, b, b, b}; }
inline vec4 operator/(const vec4& a, float b) { vec4 n = a; return n /= b; }
inline vec4 operator/(float a, const vec4& b) { return vec4{ a,a,a,a } / b; }

// 座標変換ベクトルの配列を計算
inline void GetTransformVectors
(
	vec4* result,
	const vec3& scale,
	const vec3& rotation,
	const vec3& position
)
{
	const float sinX = sin(rotation.x);
	const float cosX = cos(rotation.x);
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);
	result[0] = { scale.x * cosY,  scale.y * sinX * sinY, scale.z * cosX * sinY, position.x };
	result[1] = { 0,               scale.y * cosX,        scale.z * -sinX,       position.y };
	result[2] = { scale.x * -sinY, scale.y * sinX * cosY, scale.z * cosX * cosY, position.z };
	result[3] = { 0,			   0,					  0,                     1 };
}

// 座標変換ベクトルの配列をかけ合わせる
inline void MultiplyTransformVectors
(
	vec4* result,
	const vec4* m,
	const vec4* n
)
{
	vec4 mn[4];
	for (int i = 0; i < 4; ++i)
		mn[i] = m[0] * n[i].x + m[1] * n[i].y + m[2] * n[i].z + m[3] * n[i].w;
	for (int i = 0; i < 4; ++i)
		result[i] = mn[i];
}

// 法線変換ベクトルの配列を計算
inline void GetRotationVectors
(
	vec3* result,
	const vec3& rotation
)
{
	const float sinX = sin(rotation.x);
	const float cosX = cos(rotation.x);
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);
	result[0] = { cosY,  sinX * sinY, cosX * sinY };
	result[1] = { 0,     cosX,        -sinX };
	result[2] = { -sinY, sinX * cosY, cosX * cosY };
}

// 法線変換ベクトルの配列をかけ合わせる
inline void MultiplyRotationVectors
(
	vec3* result,
	const vec3* m,
	const vec3* n
)
{
	vec3 mn[3];
	for (int i = 0; i < 3; ++i)
		mn[i] = m[0] * n[i].x + m[1] * n[i].y + m[2] * n[i].z;
	for (int i = 0; i < 3; ++i)
		result[i] = mn[i];
}

// 3x3行列
struct mat3
{
	vec3 data[3];

	// デフォルトコンストラクタ
	mat3() = default;

	// 3個のvec3からmat3を構築するコンストラクタ
	mat3
	(
		const vec3& v0,
		const vec3& v1,
		const vec3& v2
	)
	{
		data[0] = v0;
		data[1] = v1;
		data[2] = v2;
	}

	// 1個のfloatを対角線にコピーするコンストラクタ
	explicit mat3(float f)
	{
		data[0] = { f, 0, 0 };
		data[1] = { 0, f, 0 };
		data[2] = { 0, 0, f };
	}

	// mat4からmat3に変換するコンストラクタ
	explicit mat3(const struct mat4& m);

	// 添字演算子
	vec3& operator[](size_t i) { return data[i]; }
	const vec3& operator[](size_t i) const { return data[i]; }
};

// 4x4行列
struct mat4
{
	vec4 data[4];

	// デフォルトコンストラクタ
	mat4() = default;

	// 4個のvec3からmat4を構築するコンストラクタ
	mat4
	(
		const vec4& v0,
		const vec4& v1,
		const vec4& v2,
		const vec4& v3
	)
	{
		data[0] = v0;
		data[1] = v1;
		data[2] = v2;
		data[3] = v3;
	}

	// 1個のfloatを対角線にコピーするコンストラクタ
	explicit mat4(float f)
	{
		data[0] = { f, 0, 0, 0 };
		data[1] = { 0, f, 0, 0 };
		data[2] = { 0, 0, f, 0 };
		data[3] = { 0, 0, 0, f };
	}

	// mat3からmat4に変換するコンストラクタ
	explicit mat4(const mat3& m)
	{
		data[0] = { m.data[0], 0 };
		data[1] = { m.data[1], 0 };
		data[2] = { m.data[2], 0 };
		data[3] = { 0, 0, 0, 1 };
	}

	// 添字演算子
	vec4& operator[](size_t i) { return data[i]; }
	const vec4& operator[](size_t i) const { return data[i]; }
};

// mat4からmat3に変換するコンストラクタ
inline mat3::mat3(const mat4& m)
{
	data[0] = vec3(m.data[0]);
	data[1] = vec3(m.data[1]);
	data[2] = vec3(m.data[2]);
}

// mat3とvec3の乗算
inline vec3 operator*(const mat3& m, const vec3& v)
{
	return m.data[0] * v.x + m.data[1] * v.y + m.data[2] * v.z;
}

// mat3同士の乗算
inline mat3 operator*(const mat3& a, const mat3& b)
{
	mat3 m;
	m.data[0] = a * b.data[0];
	m.data[1] = a * b.data[1];
	m.data[2] = a * b.data[2];
	return m;
}
inline mat3& operator*=(mat3& a, const mat3& b) { a = a * b; return a; }

// mat4とvec4の乗算
inline vec4 operator*(const mat4& m, const vec4& v)
{
	return m.data[0] * v.x + m.data[1] * v.y + m.data[2] * v.z + m.data[3] * v.w;
}

// mat4同士の乗算
inline mat4 operator*(const mat4& a, const mat4& b)
{
	mat4 m;
	m.data[0] = a * b.data[0];
	m.data[1] = a * b.data[1];
	m.data[2] = a * b.data[2];
	m.data[3] = a * b.data[3];
	return m;
}
inline mat4& operator*=(mat4& a, const mat4& b) { a = a * b; return a; }

// 回転角度から回転行列を作成
inline mat3 GetRotationMatrix(const vec3& rotation)
{
	const float sinX = sin(rotation.x);
	const float cosX = cos(rotation.x);
	const float sinY = sin(rotation.y);
	const float cosY = cos(rotation.y);
	const float sinZ = sin(rotation.z);
	const float cosZ = cos(rotation.z);
	mat3 m =
	{
		vec3{ cosY,        0,     -sinY },
		vec3{ sinX * sinY, cosX,  sinX * cosY },
		vec3{ cosX * sinY, -sinX, cosX * cosY } 
	};
	const mat3 n =
	{
		{ cosZ, sinZ, 0 },
		{-sinZ, cosZ, 0 },
		{ 0, 0, 1 }
	};
	return m * n;
}

// 拡大率、回転角度、平行移動量から座標変換行列を作成
inline mat4 GetTransformMatrix
(
	const vec3& scale,
	const vec3& rotation,
	const vec3& position
)
{
	const mat3 m = GetRotationMatrix(rotation);
	return
	{
	  vec4{ scale.x * m.data[0], 0 },
	  vec4{ scale.y * m.data[1], 0 },
	  vec4{ scale.z * m.data[2], 0 },
	  vec4{ position, 1 }
	};
}

// 4x4行列から拡大率を抽出
inline vec3 ExtractScale(const mat4& transform)
{
	const float sx = length(vec3(transform[0]));
	const float sy = length(vec3(transform[1]));
	const float sz = length(vec3(transform[2]));
	return { sx, sy, sz };
}

/// <summary>
/// 座標変換行列を平行移動、拡大率、回転行列の各成分に分解
/// </summary>
/// <param name="transform">分解元の座標変換行列</param>
/// <param name="translate">平行移動の格納先となる変数</param>
/// <param name="scale">拡大率の格納先となる変数</param>
/// <param name="rotation">回転行列の格納先となる変数</param>
inline void Decompose
(
	const mat4& transform,
	vec3& translate,
	vec3& scale,
	mat3& rotation
)
{
	// 平行移動を分解
	translate = vec3(transform[3]);

	// 拡大率を分解
	scale = ExtractScale(transform);

	// 回転行列を分解
	rotation = mat3(transform);
	rotation[0] /= scale.x;
	rotation[1] /= scale.y;
	rotation[2] /= scale.z;
}

/// <summary>
/// ビュー行列を作成
/// </summary>
/// <param name="eye">視点の座標</param>
/// <param name="target">注視点の座標</param>
/// <param name="up">視点の上方向を指す仮のベクトル</param>
/// <returns>eye、target、upから作成したビュー行列</returns>
inline mat4 LookAt
(
	const vec3& eye,
	const vec3& target,
	const vec3& up
)
{
	// ワールド座標系における視点座標系のXYZ軸の向きを計算
	const vec3 axisZ = normalize(eye - target);
	const vec3 axisX = normalize(cross(up, axisZ));
	const vec3 axisY = normalize(cross(axisZ, axisX));

	// 座標を軸ベクトルに射影するように行列を設定
	mat4 m;
	m[0] = { axisX.x, axisY.x, axisZ.x, 0 };
	m[1] = { axisX.y, axisY.y, axisZ.y, 0 };
	m[2] = { axisX.z, axisY.z, axisZ.z, 0 };

	// eyeが原点となるように、eyeを各軸に射影して平行移動量を計算
	m[3] = { -dot(axisX, eye), -dot(axisY, eye), -dot(axisZ, eye), 1 };

	return m;
}

/// <summary>
/// 平行投影行列を作成
/// </summary>
/// <param name="left">描画範囲の左端までの距離</param>
/// <param name="right">描画範囲の右端までの距離</param>
/// <param name="bottom">描画範囲の下端までの距離</param>
/// <param name="top">描画範囲の上端までの距離</param>
/// <param name="zNear">描画範囲に含まれる最小Z座標</param>
/// <param name="zFar">描画範囲に含まれる最大Z座標</param>
/// <returns>平行投影行列</returns>
inline mat4 Orthogonal
(
	float left,
	float right,
	float bottom,
	float top,
	float zNear,
	float zFar
)
{
	// 描画範囲のサイズと中心座標を計算
	const vec3 size = { right - left, top - bottom, zFar - zNear };
	const vec3 center = { right + left, top + bottom, zFar + zNear };

	// 描画範囲内の座標が-1～+1に変換されるように、拡大率と平行移動を設定
	mat4 m;
	m[0] = { 2 / size.x,          0,           0, 0 };
	m[1] = { 0, 2 / size.y,           0, 0 };
	m[2] = { 0,          0, -2 / size.z, 0 };
	m[3] = vec4(-center / size, 1);
	return m;
}

#endif // !VECMATH_H_INCLUDED