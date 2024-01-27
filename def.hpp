#pragma once

#define M_PI 3.14159265358979323846264338327950288419716939937510f
#define RVA(Address, Size) ((uintptr_t)((UINT_PTR)(Address) + *(PINT)((UINT_PTR)(Address) + ((Size) - sizeof(INT))) + (Size)))

inline ID3D11Device* device = nullptr;
inline ID3D11DeviceContext* immediateContext = nullptr;
inline ID3D11RenderTargetView* renderTargetView = nullptr;

inline HWND hWnd = nullptr;
inline WNDPROC WndProcOriginal = nullptr;

inline auto width = 0.0f, height = 0.0f;

template <typename DataType>
inline DataType read(uintptr_t address)
{
	if (IsBadReadPtr(reinterpret_cast<const void*>(address), sizeof(DataType)))
		return {};

	return *reinterpret_cast<DataType*>(address);
}

template <typename DataType>
inline bool write(uintptr_t address, DataType ValueToWrite)
{
	if (IsBadWritePtr(reinterpret_cast<LPVOID>(address), sizeof(DataType)))
		return false;

	*reinterpret_cast<DataType*>(address) = ValueToWrite;
	return true;
}

// ScriptStruct CoreUObject.Vector
// Size: 0x0c (Inherited: 0x00)
struct FVector
{
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}

	FVector(double x, double y, double z) : X(x), Y(y), Z(z) {}

	FVector operator+(const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }

	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }

	FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }

	FVector operator/(const FVector& V) const { return FVector(X / V.X, Y / V.Y, Z / V.Z); }

	bool operator==(const FVector& V) const { return X == V.X && Y == V.Y && Z == V.Z; }

	bool operator!=(const FVector& V) const { return X != V.X || Y != V.Y || Z != V.Z; }

	FVector operator-() const { return FVector(-X, -Y, -Z); }

	FVector operator+(float Bias) const { return FVector(X + Bias, Y + Bias, Z + Bias); }

	FVector operator-(float Bias) const { return FVector(X - Bias, Y - Bias, Z - Bias); }

	FVector operator*(float Scale) const { return FVector(X * Scale, Y * Scale, Z * Scale); } const

	FVector operator/(float Scale) const { const float RScale = 1.f / Scale; return FVector(X * RScale, Y * RScale, Z * RScale); }

	FVector operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }

	FVector operator*=(const FVector& V) { X *= V.X; Y *= V.Y; Z *= V.Z; return *this; }

	FVector operator/=(const FVector& V) { X /= V.X; Y /= V.Y; Z /= V.Z; return *this; }

	FVector operator*=(float Scale) { X *= Scale; Y *= Scale; Z *= Scale; return *this; }

	FVector operator/=(float V) { const float RV = 1.f / V; X *= RV; Y *= RV; Z *= RV; return *this; }

	inline auto Dot(FVector v)
	{
		return X * v.X + Y * v.Y + Z * v.Z;
	}
};

// ScriptStruct CoreUObject.Vector4
// Size: 0x10 (Inherited: 0x00)
struct FVector4
{
	float X; // 0x00(0x04)
	float Y; // 0x04(0x04)
	float Z; // 0x08(0x04)
	float W; // 0x0c(0x04)
};

// ScriptStruct CoreUObject.Vector2D
// Size: 0x08 (Inherited: 0x00)
struct FVector2D
{
	float X; // 0x00(0x04)
	float Y; // 0x04(0x04)

	FVector2D operator+(const FVector2D& other) const { return FVector2D({ X + other.X, Y + other.Y }); }
	FVector2D operator-(const FVector2D& other) const { return FVector2D({ X - other.X, Y - other.Y }); }
	FVector2D operator+=(const FVector2D& V) { X += V.X; Y += V.Y; return *this; }
};

// ScriptStruct CoreUObject.TwoVectors
// Size: 0x18 (Inherited: 0x00)
struct FTwoVectors
{
	FVector v1; // 0x00(0x0c)
	FVector v2; // 0x0c(0x0c)
};

// ScriptStruct CoreUObject.Plane
// Size: 0x10 (Inherited: 0x0c)
struct FPlane : FVector
{
	float W; // 0x0c(0x04)
};

// ScriptStruct CoreUObject.Matrix
// 0x0040
typedef struct FMatrix
{
	FPlane                                      XPlane;                                                    // 0x0000(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FPlane                                      YPlane;                                                    // 0x0010(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FPlane                                      ZPlane;                                                    // 0x0020(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	FPlane                                      WPlane;                                                    // 0x0030(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

template< class T > struct TArray
{
public:
	T* Data;
	int Count;
	int Max;

public:
	TArray()
	{
		Data = 0;
		Count = Max = 0;
	};

public:
	int Num()
	{
		return this->Count;
	};

	T& operator() (int i)
	{
		return this->Data[i];
	};

	const T& operator() (int i) const
	{
		return this->Data[i];
	};
};

struct FString : private TArray<wchar_t>
{
	FString() = default;

	explicit FString(const wchar_t* other)
	{
		Max = Count = *other ? wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		auto length = std::wcslen(Data);
		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);
		return str;
	}
};

enum e_bones
{
	Head = 28,
	L_null_hand = 92,
	L_null_leg = 16,
	R_null_hand = 137,
	R_null_leg = 23,
	Root = 0,
	VB_root_foot_l = 191,
	VB_root_foot_r = 192,
	VB_root_pelvis = 193,
	VB_thigh_l_calf_l = 189,
	VB_thigh_r_calf_r = 190,
	ball_l = 13,
	ball_r = 20,
	calf_l = 11,
	calf_r = 18,
	calf_twist_01_l = 14,
	calf_twist_01_r = 21,
	camera_mount = 29,
	clavicle_arm_l = 63,
	clavicle_arm_r = 108,
	clavicle_clothes_l = 56,
	clavicle_clothes_r = 101,
	clavicle_l = 55,
	clavicle_r = 100,
	foot_l = 12,
	foot_r = 19,
	hand_l = 66,
	hand_r = 111,
	hipCoat_01_l = 161,
	hipCoat_01_lb = 167,
	hipCoat_01_p_l = 160,
	hipCoat_01_p_lb = 166,
	hipCoat_01_p_r = 160,
	hipCoat_01_p_rb = 172,
	hipCoat_01_r = 161,
	hipCoat_01_rb = 173,
	hipCoat_02_l = 163,
	hipCoat_02_lb = 169,
	hipCoat_02_p_l = 162,
	hipCoat_02_p_lb = 168,
	hipCoat_02_p_r = 162,
	hipCoat_02_p_rb = 174,
	hipCoat_02_r = 163,
	hipCoat_02_rb = 175,
	hipCoat_03_l = 165,
	hipCoat_03_lb = 171,
	hipCoat_03_p_l = 164,
	hipCoat_03_p_lb = 170,
	hipCoat_03_p_r = 164,
	hipCoat_03_p_rb = 176,
	hipCoat_03_r = 165,
	hipCoat_03_rb = 177,
	hoodie = 159,
	ik_foot_l = 6,
	ik_foot_r = 7,
	ik_foot_root = 5,
	ik_hand_gun = 2,
	ik_hand_l = 3,
	ik_hand_r = 3,
	ik_hand_root = 1,
	index_01_l = 68,
	index_01_r = 113,
	index_02_l = 69,
	index_02_r = 114,
	index_03_l = 70,
	index_03_r = 115,
	index_04_l = 71,
	index_04_r = 116,
	index_carpal_l = 67,
	index_carpal_r = 112,
	jacket_01_l = 58,
	jacket_01_lb = 146,
	jacket_01_p_l = 57,
	jacket_01_p_lb = 145,
	jacket_01_p_r = 102,
	jacket_01_p_rb = 151,
	jacket_01_r = 103,
	jacket_01_rb = 152,
	jacket_02_l = 60,
	jacket_02_lb = 148,
	jacket_02_p_l = 59,
	jacket_02_p_lb = 147,
	jacket_02_p_r = 104,
	jacket_02_p_rb = 153,
	jacket_02_r = 105,
	jacket_02_rb = 154,
	jacket_03_l = 62,
	jacket_03_lb = 150,
	jacket_03_p_l = 61,
	jacket_03_p_lb = 149,
	jacket_03_p_r = 106,
	jacket_03_p_rb = 155,
	jacket_03_r = 107,
	jacket_03_rb = 156,
	locator_bottomLip_c = 50,
	locator_bottomLip_l = 52,
	locator_bottomLip_r = 46,
	locator_cheek_l = 51,
	locator_cheek_r = 47,
	locator_chin_c = 36,
	locator_chin_l = 34,
	locator_chin_r = 43,
	locator_ear_l = 48,
	locator_ear_r = 49,
	locator_eyebrowInner_l = 42,
	locator_eyebrowInner_r = 32,
	locator_eyebrowOuter_l = 45,
	locator_eyebrowOuter_r = 54,
	locator_feeding = 185,
	locator_generalPurpose_01 = 186,
	locator_generalPurpose_02 = 187,
	locator_jawBack_l = 35,
	locator_jawBack_r = 33,
	locator_neckFat = 40,
	locator_noseBridge = 53,
	locator_nostril_l = 38,
	locator_nostril_r = 39,
	locator_septum = 44,
	locator_spineIK = 188,
	locator_upperLip_c = 31,
	locator_upperLip_l = 37,
	locator_upperLip_r = 41,
	lowerarm_l = 65,
	lowerarm_r = 110,
	lowerarm_twist_01_l = 93,
	lowerarm_twist_01_r = 138,
	lowerarm_twist_02_l = 94,
	lowerarm_twist_02_r = 139,
	lowerarm_twist_03_l = 95,
	lowerarm_twist_03_r = 140,
	middle_01_l = 73,
	middle_01_r = 118,
	middle_02_l = 74,
	middle_02_r = 119,
	middle_03_l = 75,
	middle_03_r = 120,
	middle_04_l = 76,
	middle_04_r = 121,
	middle_carpal_l = 72,
	middle_carpal_r = 117,
	neck_01 = 27,
	null_back_01 = 184,
	null_back_02 = 158,
	null_prop_01 = 99,
	null_prop_02 = 144,
	null_prop_03 = 157,
	null_prop_04 = 30,
	pelvis = 9,
	pinky_01_l = 83,
	pinky_01_r = 128,
	pinky_02_l = 84,
	pinky_02_r = 129,
	pinky_03_l = 85,
	pinky_03_r = 130,
	pinky_04_l = 86,
	pinky_04_r = 131,
	pinky_carpal_l = 82,
	pinky_carpal_r = 127,
	ring_01_l = 78,
	ring_01_r = 123,
	ring_02_l = 79,
	ring_02_r = 124,
	ring_03_l = 80,
	ring_03_r = 125,
	ring_04_l = 81,
	ring_04_r = 126,
	ring_carpal_l = 77,
	ring_carpal_r = 122,
	spine_01 = 24,
	spine_02 = 25,
	spine_03 = 26,
	thigh_l = 10,
	thigh_r = 17,
	thigh_twist_01_l = 15,
	thigh_twist_01_r = 22,
	thumb_01_l = 88,
	thumb_01_r = 133,
	thumb_02_l = 89,
	thumb_02_r = 134,
	thumb_03_l = 90,
	thumb_03_r = 135,
	thumb_04_l = 91,
	thumb_04_r = 136,
	thumb_carpal_l = 87,
	thumb_carpal_r = 132,
	upperarm_l = 64,
	upperarm_r = 109,
	upperarm_twist_01_l = 96,
	upperarm_twist_01_r = 141,
	upperarm_twist_02_l = 97,
	upperarm_twist_02_r = 142,
	upperarm_twist_03_l = 98,
	upperarm_twist_03_r = 143,
	weapon_mount = 8
};

namespace Settings
{
	auto bAimbot = false, bSkipKnockedTargets = false, bSkipTargetsOutOfBounds = false,
		bDontTargetInvisiblePlayers = false, bDontTargetTeammates = true,
		bPlayerBoxESP = true, bPlayerLineESP = true, bPlayerHeadESP = true,
		bPlayerDistanceESP = true, bPlayerNameESP = false, bDontDrawTeammates = false;

	auto bOptions = true, bDrawFOV = true, bDrawFakeCrosshair = true;

	float Smoothing = 3.0f, FOV = 120.0f,
		PlayerVisibleColor[3] = { 1.0f, 1.0f, 1.0f }, PlayerInvisibleColor[3] = { 0.0f, 1.0f, 0.9019607843137255f },
		MaxPlayerESPDistance = 300.0f, MaxPlayerTargetingDistance = 300.0f;

	uintptr_t GameBaseAddress = 0;
	DWORD GameSizeOfImage = 0;
}