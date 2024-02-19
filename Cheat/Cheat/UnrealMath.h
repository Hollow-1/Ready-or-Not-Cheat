#pragma once

#define M_PI 3.14159265358979323846264338327950288419716939937510f
#include <cmath>

struct FQuat;
class Vector3;

//弧度转角度
inline float ToDegree(float radian) {
	return radian * 180.f / M_PI;
}

//角度转弧度
inline float ToRadian(float degree) {
	return degree * M_PI / 180.0f;
}

class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f) { }

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }

	float x;
	float y;
	float z;

	inline float Dot(Vector3 v) {
		return x * v.x + y * v.y + z * v.z;
	}

	//将欧拉角转换为四元数 x = pitch, y = yaw, z = roll
	FQuat ToQuat();

	Vector3 operator+(Vector3 v) {
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v) {
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 operator*(float number) const {
		return Vector3(x * number, y * number, z * number);
	}
};

struct FQuat
{
	float x;
	float y;
	float z;
	float w;

	//将四元数转换为欧拉角
	Vector3 ToEulerAngles();
};

Vector3 FQuat::ToEulerAngles()
{
	Vector3 e;

	e.x = asin(2 * (z * x - y * w));							//pitch
	e.y = atan2(2 * (x * y + z * w), 1 - 2 * (y * y + z * z));	//yaw
	e.z = atan2(2 * (z * y + x * w), 1 - 2 * (y * y + x * x));	//roll

	e.x = ToDegree(e.x);
	e.y = ToDegree(e.y);
	e.z = ToDegree(e.z);
	return e;
}

FQuat Vector3::ToQuat()
{
	//deg to rad
	float xx = ToRadian(x); //pitch
	float yy = ToRadian(y); //yaw
	float zz = ToRadian(z); //roll
	
	float cy = (float)cos(yy * 0.5);
	float sy = (float)sin(yy * 0.5);
	float cx = (float)cos(xx * 0.5);
	float sx = (float)sin(xx * 0.5);
	float cz = (float)cos(zz * 0.5);
	float sz = (float)sin(zz * 0.5);

	FQuat q;

	q.x = sz * cx * cy + cz * sx * sy;
	q.y = sz * cx * sy - cz * sx * cy;
	q.z = cz * cx * sy + sz * sx * cy;
	q.w = cz * cx * cy - sz * sx * sy;

	return q;
}


struct FMatrix {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
	};
};

struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];

	FMatrix ToMatrixWithScale()
	{
		FMatrix m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

FMatrix MatrixMultiplication(FMatrix pM1, FMatrix pM2)
{
	FMatrix pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}


struct FMinimalViewInfo
{
	Vector3 Location; // 0x00(0x0c)
	Vector3 Rotation; // 0x0c(0x0c)
	float FOV; // 0x18(0x04)
	float DesiredFOV; // 0x1c(0x04)
	float OrthoWidth; // 0x20(0x04)
	float OrthoNearClipPlane; // 0x24(0x04)
	float OrthoFarClipPlane; // 0x28(0x04)
	float AspectRatio; // 0x2c(0x04)
};

struct FCameraCacheEntry
{
	float Timestamp; // 0x00(0x04)
	char pad_4[0xc]; // 0x04(0x0c)
	FMinimalViewInfo POV; // 0x10(0x5e0)
};

FMatrix to_matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radpitch = rot.x * M_PI / 180.0f;
	float radyaw = rot.y * M_PI / 180.0f;
	float radroll = rot.z * M_PI / 180.0f;
	float sp = sinf(radpitch);
	float cp = cosf(radpitch);
	float sy = sinf(radyaw);
	float cy = cosf(radyaw);
	float sr = sinf(radroll);
	float cr = cosf(radroll);
	FMatrix matrix{};
	matrix.m[0][0] = cp * cy;
	matrix.m[0][1] = cp * sy;
	matrix.m[0][2] = sp;
	matrix.m[0][3] = 0.f;
	matrix.m[1][0] = sr * sp * cy - cr * sy;
	matrix.m[1][1] = sr * sp * sy + cr * cy;
	matrix.m[1][2] = -sr * cp;
	matrix.m[1][3] = 0.f;
	matrix.m[2][0] = -(cr * sp * cy + sr * sy);
	matrix.m[2][1] = cy * sr - cr * sp * sy;
	matrix.m[2][2] = cr * cp;
	matrix.m[2][3] = 0.f;
	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;
	return matrix;
}


Vector3 ProjectWorldToScreen(FCameraCacheEntry& CameraCache, Vector3 world_location, float ScreenWidth, float ScreenHeight)
{
	Vector3 Screenlocation = Vector3(0, 0, 0);

	FMatrix temp_matrix = to_matrix(CameraCache.POV.Rotation);
	Vector3 vaxisx = Vector3(temp_matrix.m[0][0], temp_matrix.m[0][1], temp_matrix.m[0][2]);
	Vector3 vaxisy = Vector3(temp_matrix.m[1][0], temp_matrix.m[1][1], temp_matrix.m[1][2]);
	Vector3 vaxisz = Vector3(temp_matrix.m[2][0], temp_matrix.m[2][1], temp_matrix.m[2][2]);

	Vector3 vdelta = world_location - CameraCache.POV.Location;
	Vector3 vtransformed = Vector3(vdelta.Dot(vaxisy), vdelta.Dot(vaxisz), vdelta.Dot(vaxisx));
	//std::cout << vtransformed.z << std::endl;

	Screenlocation.z = vtransformed.z;
	if (vtransformed.z < 1.0f) {
		vtransformed.z = 1.0f;
	}

	auto screen_center_x = ScreenWidth / 2.0f;
	auto screen_center_y = ScreenHeight / 2.0f;

	Screenlocation.x = screen_center_x + vtransformed.x * ((screen_center_x / tanf(CameraCache.POV.FOV * M_PI / 360))) / vtransformed.z;
	Screenlocation.y = screen_center_y - vtransformed.y * ((screen_center_x / tanf(CameraCache.POV.FOV * M_PI / 360))) / vtransformed.z;

	return Screenlocation;
}

inline float CalcDistance(float x1, float y1, float x2, float y2)
{
	float dx = x1 - x2;
	float dy = y1 - y2;
	return sqrt(dx * dx + dy * dy);
}

inline float CalcDistance(Vector3 v1, Vector3 v2)
{
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dz = v1.z - v2.z;
	return sqrt(dx * dx + dy * dy + dz * dz);
}

//两个角度相加
inline float AnglesAdd(float a1, float a2) {
	float angle = a1 + a2;

	//确保角度在 -180 到 180 之间
	while (angle <= -180.0) {
		angle += 360.0;
	}
	while (angle > 180.0) {
		angle -= 360.0;
	}
	return angle;
}
