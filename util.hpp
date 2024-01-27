#pragma once

class UObject;
class AActor;
class APlayerController;
class APlayerState;
class APlayerPawn;
class UKismetSystemLibrary;

inline const char* SelectedHitbox = "HEAD";
inline const char* AllHitboxes[] = { "HEAD", "NECK", "CHEST" };

inline const char* SelectedBoxStyle = "CORNERED-FILLED";
inline const char* AllBoxStyles[] = { "2D", "2D-FILLED", "CORNERED", "CORNERED-FILLED", "3D (ENGINE-BOUNDING)" };

namespace Util
{
	uintptr_t PatternScan(uintptr_t base, DWORD size, const wchar_t* pattern, const wchar_t* mask);
	FVector GetAimWorldLocation(APlayerController* LocalController, APlayerPawn* Player);
	bool IsLocationInScreen(FVector ScreenLocation, int over = 30);
}

class UObject
{
public:
	auto GetReference()
	{
		return reinterpret_cast<uintptr_t>(this);
	}

	auto GetVFTable(uint32_t TableOffset)
	{
		return read<uintptr_t*>(this->GetReference() + TableOffset);
	}

	static auto ProcessEvent(UObject* Class, UObject* Function, uintptr_t Params)
	{
		(*(__int64(__fastcall**)(UObject*, UObject*, uintptr_t))(
			*(uintptr_t*)Class->GetReference() + 0x218i64))(Class, Function, Params);
	}

	static auto StaticFindObject(UObject* Class, UObject* InOuter, const wchar_t* Name, bool ExactClass)
	{
		static uintptr_t StaticFindObjectAddress = 0;
		if (!StaticFindObjectAddress)
		{
			StaticFindObjectAddress = RVA(Util::PatternScan(
				Settings::GameBaseAddress,
				Settings::GameSizeOfImage,
				L"\xE8\x00\x00\x00\x00\x49\x8B\xDD\x41\x8B\xF5",
				L"x????xxxxxx"
			), 5);

			if (!StaticFindObjectAddress) return static_cast<UObject*>(nullptr);
		}

		return reinterpret_cast<UObject * (__fastcall*)(UObject*, UObject*,
			const wchar_t*, bool)>(StaticFindObjectAddress)(Class, InOuter, Name, ExactClass);
	}
};

class UEngine : public UObject
{
public:
	uintptr_t GEngineAddress = 0;

	// constructor
	UEngine(uintptr_t GlobalPointer) : GEngineAddress(GlobalPointer) {};

	// destructor
	~UEngine() {}

	UEngine* GetContext()
	{
		if (!GEngineAddress) return nullptr;
		return read<UEngine*>(GEngineAddress);
	}

	static auto FreeMemory(uintptr_t AllocatedMemery)
	{
		static uintptr_t FreeMemoryAddress = 0;
		if (!FreeMemoryAddress)
		{
			FreeMemoryAddress = RVA(Util::PatternScan(
				Settings::GameBaseAddress,
				Settings::GameSizeOfImage,
				L"\xE8\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x8B\xC7\x48\x83\xC4\x20",
				L"x????xxxx?xxxxxxx"
			), 5);

			if (!FreeMemoryAddress) return;
		}

		reinterpret_cast<void(__fastcall*)(uintptr_t)>(FreeMemoryAddress)(AllocatedMemery);
	}
};

class UWorld : public UObject
{
public:
	// PTR Holder
	uintptr_t GWorldAddress;

	UWorld(uintptr_t GlobalPointer) : GWorldAddress(GlobalPointer) {};
	~UWorld() {}

	UWorld* GetContext()
	{
		if (!GWorldAddress) return nullptr;
		return read<UWorld*>(GWorldAddress);
	}
};

class ULevel : public UObject
{
public:

};

class UKismetSystemLibrary : public UObject
{
public:
	static auto GetStaticClass()
	{
		static UObject* KismetSystemLibraryClassObject = nullptr;
		if (KismetSystemLibraryClassObject == nullptr)
		{
			KismetSystemLibraryClassObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.KismetSystemLibrary", false);

			if (KismetSystemLibraryClassObject == nullptr) return static_cast<UObject*>(nullptr);
		}

		return KismetSystemLibraryClassObject;
	}

	static std::optional<const wchar_t*> GetObjectName(UObject* Object)
	{
		static UObject* GetObjectNameObject = nullptr;
		if (GetObjectNameObject == nullptr)
		{
			GetObjectNameObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.KismetSystemLibrary.GetObjectName", false);

			if (GetObjectNameObject == nullptr) return static_cast<const wchar_t*>(nullptr);
		}

		struct UKismetSystemLibrary_GetObjectName_Params
		{
			UObject*									Object;
			FString                                     ReturnValue;
		} params;

		params.Object = Object;

		UObject::ProcessEvent(UKismetSystemLibrary::GetStaticClass(), GetObjectNameObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue.c_str();
	}

	static auto IsValid(UObject* Object)
	{
		static UObject* IsValidObject = nullptr;
		if (IsValidObject == nullptr)
		{
			IsValidObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.KismetSystemLibrary.IsValid", false);

			if (IsValidObject == nullptr) return false;
		}

		struct UKismetSystemLibrary_IsValid_Params
		{
			UObject*	Object;
			bool		ReturnValue;
		} params;

		params.Object = Object;

		UObject::ProcessEvent(UKismetSystemLibrary::GetStaticClass(), IsValidObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}
};

class USceneComponent : public UObject
{
public:

};

class UPrimitiveComponent : public USceneComponent
{
public:

};

class ULocalPlayer : public UObject
{
public:

};

class UGameInstance : public UObject
{
public:

};

class AActor : public UObject
{
public:
	auto K2_GetRootComponent()
	{
		static UObject* K2_GetRootComponentObject = nullptr;
		if (K2_GetRootComponentObject == nullptr)
		{
			K2_GetRootComponentObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.Actor.K2_GetRootComponent", false);

			if (K2_GetRootComponentObject == nullptr) return static_cast<USceneComponent*>(nullptr);
		}

		struct
		{
			USceneComponent* ReturnValue;
		} params;

		UObject::ProcessEvent(this, K2_GetRootComponentObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}

	auto GetDistanceTo(AActor* OtherActor)
	{
		static UObject* GetDistanceToObject = nullptr;
		if (GetDistanceToObject == nullptr)
		{
			GetDistanceToObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.Actor.GetDistanceTo", false);

			if (GetDistanceToObject == nullptr) return 0.0f;
		}

		struct
		{
			AActor* OtherActor;
			float ReturnValue;
		} params;

		params.OtherActor = OtherActor;

		UObject::ProcessEvent(this, GetDistanceToObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}

	auto K2_GetPawn()
	{
		static UObject* K2_GetPawnFunctionObject = nullptr;
		if (K2_GetPawnFunctionObject == nullptr)
		{
			K2_GetPawnFunctionObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.Controller.K2_GetPawn", false);

			if (K2_GetPawnFunctionObject == nullptr) return static_cast<APlayerPawn*>(nullptr);
		}

		struct AController_K2_GetPawn_Params
		{
			APlayerPawn* ReturnValue;
		} params;

		UObject::ProcessEvent(this, K2_GetPawnFunctionObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}

	auto GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent)
	{
		static UObject* GetActorBoundsObject = nullptr;
		if (GetActorBoundsObject == nullptr)
		{
			GetActorBoundsObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.Actor.GetActorBounds", false);

			if (GetActorBoundsObject == nullptr) return;
		}

		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		UObject::ProcessEvent(this, GetActorBoundsObject, reinterpret_cast<uintptr_t>(&params));

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}
};

class APlayerController : public AActor
{
public:
	auto LineOfSightTo(AActor* Other, FVector ViewPoint = FVector(), bool bAlternateChecks = false)
	{
		static UObject* LineOfSightToFunctionObject = nullptr;
		if (LineOfSightToFunctionObject == nullptr)
		{
			LineOfSightToFunctionObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.Controller.LineOfSightTo", false);

			if (LineOfSightToFunctionObject == nullptr) return false;
		}

		struct
		{
			AActor* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;

		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;

		UObject::ProcessEvent(this, LineOfSightToFunctionObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}

	auto ProjectWorldToScreen(const FVector& WorldLocation)
	{
		static UObject* ProjectWorldLocationToScreenFunctionObject = nullptr;
		if (ProjectWorldLocationToScreenFunctionObject == nullptr)
		{
			ProjectWorldLocationToScreenFunctionObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.PlayerController.ProjectWorldLocationToScreen", false);

			if (ProjectWorldLocationToScreenFunctionObject == nullptr) return FVector();
		}

		struct
		{
			FVector WorldLocation;
			FVector ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;

		UObject::ProcessEvent(this, ProjectWorldLocationToScreenFunctionObject, reinterpret_cast<uintptr_t>(&params));
		return params.ScreenLocation;
	}
};

class USkinnedMeshComponent : public UPrimitiveComponent
{
public:

};

class USkeletalMeshComponent : public USkinnedMeshComponent
{
public:
	auto GetBoneLocation(int BoneIndex) -> FVector
	{
		static uintptr_t GetBoneMatrixAddress = 0;
		if (!GetBoneMatrixAddress)
		{
			GetBoneMatrixAddress = RVA(Util::PatternScan(
				Settings::GameBaseAddress,
				Settings::GameSizeOfImage,
				L"\xE8\x00\x00\x00\x00\x48\x8B\x47\x30\xF3\x0F\x10\x45",
				L"x????xxxxxxxx"
			), 5);

			if (!GetBoneMatrixAddress) return FVector();
		}

		FMatrix Matrix{};
		reinterpret_cast<FMatrix* (__fastcall*)(USkeletalMeshComponent*,
			FMatrix*, int)>(GetBoneMatrixAddress)(this, &Matrix, BoneIndex);
		return Matrix.WPlane;
	}
};

class APlayerState : public AActor
{
public:
	std::optional<std::string> GetPlayerName()
	{
		static UObject* GetPlayerNameObject = nullptr;
		if (GetPlayerNameObject == nullptr)
		{
			GetPlayerNameObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.PlayerState.GetPlayerName", false);

			if (GetPlayerNameObject == nullptr) return std::string();
		}

		struct APlayerState_GetPlayerName_Params
		{
			struct FString                                     ReturnValue;
		} params;

		UObject::ProcessEvent(this, GetPlayerNameObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue.ToString();
	}

	auto IsOfSameTeam(APlayerState* OtherState)
	{
		// 40 53 48 83 EC 20 48 8B DA 48 85 D2 75 08 32 C0 48 83 C4 20 5B C3
		static uintptr_t IsOfSameTeamAddress = NULL;
		if (IsOfSameTeamAddress == NULL)
		{
			IsOfSameTeamAddress = Util::PatternScan(
				Settings::GameBaseAddress,
				Settings::GameSizeOfImage,
				L"\x40\x53\x48\x83\xEC\x20\x48\x8B\xDA\x48\x85\xD2\x75\x08\x32\xC0\x48\x83\xC4\x20\x5B\xC3",
				L"xxxxxxxxxxxxxxxxxxxxxx"
			);

			if (IsOfSameTeamAddress == NULL) return false;
		}

		return reinterpret_cast<bool(__fastcall*)(
			APlayerState*, APlayerState*)>(IsOfSameTeamAddress)(this, OtherState);
	}
};

class ACharacter : public AActor
{
public:
	auto GetMesh()
	{
		return read<USkeletalMeshComponent*>(this->GetReference() + 0x288);
	}
};

class APlayerPawn : public ACharacter
{
public:
	static auto GetStaticClass()
	{
		static UObject* FortPlayerPawnClassObject = nullptr;
		if (FortPlayerPawnClassObject == nullptr)
		{
			FortPlayerPawnClassObject = UObject::StaticFindObject(nullptr, nullptr, L"FortniteGame.FortPlayerPawn", false);
			if (FortPlayerPawnClassObject == nullptr) return static_cast<UObject*>(nullptr);
		}

		return FortPlayerPawnClassObject;
	}

	auto GetPlayerState()
	{
		return read<APlayerState*>(this->GetReference() + 0x248);
	}
};

class AEnemyPawn : public APlayerPawn
{
public:

};

class UKismetMathLibrary : public UObject
{
public:
	static auto GetStaticClass()
	{
		static UObject* KismetMathLibraryClassObject = nullptr;
		if (KismetMathLibraryClassObject == nullptr)
		{
			KismetMathLibraryClassObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.KismetMathLibrary", false);
			if (KismetMathLibraryClassObject == nullptr) return static_cast<UObject*>(nullptr);
		}

		return KismetMathLibraryClassObject;
	}

	static auto Sqrt(float A)
	{
		static UObject* SqrtFunctionObject = nullptr;
		if (SqrtFunctionObject == nullptr)
		{
			SqrtFunctionObject = UObject::StaticFindObject(nullptr, nullptr, L"Engine.KismetMathLibrary.Sqrt", false);

			if (SqrtFunctionObject == nullptr) return 0.0f;
		}

		struct
		{
			float A;
			float ReturnValue;
		} params;

		params.A = A;

		UObject::ProcessEvent(UKismetMathLibrary::GetStaticClass(), SqrtFunctionObject, reinterpret_cast<uintptr_t>(&params));
		return params.ReturnValue;
	}
};

namespace Util
{
	auto PatternScan(uintptr_t base, DWORD size,
		const wchar_t* pattern, const wchar_t* mask) -> uintptr_t
	{
		auto MaskCompare = [](void* buffer, const wchar_t* pattern, const wchar_t* mask)
		{
			for (auto b = reinterpret_cast<PBYTE>(buffer); *mask; ++pattern, ++mask, ++b)
			{
				if (*mask == 'x' && *reinterpret_cast<const wchar_t*>(pattern) != *b)
					return false;
			}

			return true;
		};

		size -= static_cast<DWORD>(wcslen(mask));

		for (auto i = 0UL; i < size; ++i)
		{
			auto addr = reinterpret_cast<PBYTE>(base) + i;

			if (MaskCompare(addr, pattern, mask))
				return reinterpret_cast<uintptr_t>(addr);
		}

		return NULL;
	}

	auto GetAimWorldLocation(APlayerController* LocalController, APlayerPawn* Player) -> FVector
	{
		if (!UKismetSystemLibrary::IsValid(LocalController) || !UKismetSystemLibrary::IsValid(Player)) return FVector();

		auto PlayerMesh = Player->GetMesh();
		if (!UKismetSystemLibrary::IsValid(PlayerMesh)) return FVector();

		if (!strcmp(SelectedHitbox, "HEAD"))
			return PlayerMesh->GetBoneLocation(e_bones::Head);
		else if (!strcmp(SelectedHitbox, "NECK"))
			return PlayerMesh->GetBoneLocation(e_bones::neck_01);
		else if (!strcmp(SelectedHitbox, "CHEST"))
			return PlayerMesh->GetBoneLocation(e_bones::spine_01);

		return FVector();
	}

	inline auto IsLocationInScreen(FVector ScreenLocation, int over) -> bool
	{
		if (ScreenLocation.X > -over && ScreenLocation.X < width + over &&
			ScreenLocation.Y > -over && ScreenLocation.Y < height + over)
			return true;

		return false;
	}
}

class Render
{
public:
	Render() {};
	~Render() {};

	// Rendering related...
	inline auto GetWidth()
	{
		D3D11_VIEWPORT vp;
		UINT nvp = 1;

		immediateContext->RSGetViewports(&nvp, &vp);
		return vp.Width;
	}

	inline auto GetHeight()
	{
		D3D11_VIEWPORT vp;
		UINT nvp = 1;

		immediateContext->RSGetViewports(&nvp, &vp);
		return vp.Height;
	}

	inline auto DrawCorneredBox(ImGuiWindow& window, FLOAT X, FLOAT Y, FLOAT W, FLOAT H, ImU32 InlineColor, ImU32 OutLineColor)
	{
		auto lineW = (W / 5.0f), lineH = (H / 6.0f), lineT = 1.0f;

		// inline
		window.DrawList->AddLine(ImVec2(X, Y), ImVec2(X, Y + lineH), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X, Y), ImVec2(X + lineW, Y), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W - lineW, Y), ImVec2(X + W, Y), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lineH), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X, Y + H - lineH), ImVec2(X, Y + H), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X, Y + H), ImVec2(X + lineW, Y + H), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W - lineW, Y + H), ImVec2(X + W, Y + H), InlineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W, Y + H - lineH), ImVec2(X + W, Y + H), InlineColor, 1.0f);

		// outline
		window.DrawList->AddLine(ImVec2(X - lineT, Y - lineT), ImVec2(X + lineW, Y - lineT), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X - lineT, Y - lineT), ImVec2(X - lineT, Y + lineH), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X - lineT, Y + H - lineH), ImVec2(X - lineT, Y + H + lineT), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X - lineT, Y + H + lineT), ImVec2(X + lineW, Y + H + lineT), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W - lineW, Y - lineT), ImVec2(X + W + lineT, Y - lineT), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W + lineT, Y - lineT), ImVec2(X + W + lineT, Y + lineH), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W + lineT, Y + H - lineH), ImVec2(X + W + lineT, Y + H + lineT), OutLineColor, 1.0f);
		window.DrawList->AddLine(ImVec2(X + W - lineW, Y + H + lineT), ImVec2(X + W + lineT, Y + H + lineT), OutLineColor, 1.0f);
	}

	inline auto Draw3DBoundingBox(ImGuiWindow& window, AActor* Actor, ImU32 DrawColor, APlayerController* LocalPlayerController)
	{
		FVector ComponentOrigin{}, ComponentBoxExtent{};
		Actor->GetActorBounds(true, ComponentOrigin, ComponentBoxExtent);

		auto min = (ComponentOrigin - ComponentBoxExtent), max = (ComponentOrigin + ComponentBoxExtent);
		FVector vec1{}, vec2{}, vec3{}, vec4{}, vec5{}, vec6{}, vec7{}, vec8{};

		vec3 = min;
		vec3.X = max.X;
		vec4 = min;
		vec4.Y = max.Y;
		vec5 = min;
		vec5.Z = max.Z;
		vec6 = max;
		vec6.X = min.X;
		vec7 = max;
		vec7.Y = min.Y;
		vec8 = max;
		vec8.Z = min.Z;
		vec1 = LocalPlayerController->ProjectWorldToScreen(min);
		vec2 = LocalPlayerController->ProjectWorldToScreen(max);
		vec3 = LocalPlayerController->ProjectWorldToScreen(vec3);
		vec4 = LocalPlayerController->ProjectWorldToScreen(vec4);
		vec5 = LocalPlayerController->ProjectWorldToScreen(vec5);
		vec6 = LocalPlayerController->ProjectWorldToScreen(vec6);
		vec7 = LocalPlayerController->ProjectWorldToScreen(vec7);
		vec8 = LocalPlayerController->ProjectWorldToScreen(vec8);

		if (vec1 == FVector() || vec2 == FVector() || vec3 == FVector() || vec4 == FVector() || vec5 == FVector() ||
			vec6 == FVector() || vec7 == FVector() || vec8 == FVector()) return;

		window.DrawList->AddLine(ImVec2(vec1.X, vec1.Y), ImVec2(vec5.X, vec5.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec2.X, vec2.Y), ImVec2(vec8.X, vec8.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec3.X, vec3.Y), ImVec2(vec7.X, vec7.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec4.X, vec4.Y), ImVec2(vec6.X, vec6.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec1.X, vec1.Y), ImVec2(vec3.X, vec3.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec1.X, vec1.Y), ImVec2(vec4.X, vec4.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec8.X, vec8.Y), ImVec2(vec3.X, vec3.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec8.X, vec8.Y), ImVec2(vec4.X, vec4.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec2.X, vec2.Y), ImVec2(vec6.X, vec6.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec2.X, vec2.Y), ImVec2(vec7.X, vec7.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec5.X, vec5.Y), ImVec2(vec6.X, vec6.Y), DrawColor, 2.0f);
		window.DrawList->AddLine(ImVec2(vec5.X, vec5.Y), ImVec2(vec7.X, vec7.Y), DrawColor, 2.0f);
	}
};

// General purpose class instances.
inline Render Draw{};

// Game related class instances.
inline UWorld* World = nullptr;

// Local entity specific class instances.
inline APlayerController* LocalPlayerController = nullptr;
inline APlayerPawn* LocalPawn = nullptr;

// Targeting entity specific class instances.
inline AEnemyPawn* EnemyPawn = nullptr;