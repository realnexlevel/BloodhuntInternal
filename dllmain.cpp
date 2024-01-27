#include <Windows.h>
#include <type_traits>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <optional>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"

#include "def.hpp"
#include "util.hpp"

#include "minhook/include/MinHook.h"

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImGuiWindow& BeginScene()
{
	ImGui_ImplDX11_NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("##scene", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);

	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	return *ImGui::GetCurrentWindow();
}

ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
ImGuiColorEditFlags ColorPickerFlags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs;

auto EndScene(ImGuiWindow& window) -> void
{
	window.DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	if (Settings::bOptions)
	{
		if (ImGui::Begin("Bloodhunt", nullptr, WindowFlags))
		{
			ImGui::SetWindowSize(ImVec2(810.0f, 570.0f));

			static auto CurrentSelectedMenuTab = 0;
			if (ImGui::Button("Targeting"))
				CurrentSelectedMenuTab = 0;

			ImGui::SameLine();
			if (ImGui::Button("Visuals"))
				CurrentSelectedMenuTab = 1;

			switch (CurrentSelectedMenuTab)
			{
			case 0:
				ImGui::Text("Risk");
				ImGui::Checkbox("Aimbot", &Settings::bAimbot);
				ImGui::NewLine();

				ImGui::Text("Smoothing (IF AIM BUGGY, ADJUST)");
				ImGui::SliderFloat(" ", &Settings::Smoothing, 1.0f, 20.0f, "%.2f");

				ImGui::NewLine();
				ImGui::Checkbox("Skip Out-Of-Bounds", &Settings::bSkipTargetsOutOfBounds);
				ImGui::Checkbox("Skip Invisible", &Settings::bDontTargetInvisiblePlayers);
				ImGui::Checkbox("Skip Team", &Settings::bDontTargetTeammates);

				ImGui::NewLine();
				if (ImGui::BeginCombo("Bone", SelectedHitbox))
				{
					for (auto n = 0; n < IM_ARRAYSIZE(AllHitboxes); ++n)
					{
						auto IsSelected = (SelectedHitbox == AllHitboxes[n]);

						if (ImGui::Selectable(AllHitboxes[n], IsSelected)) SelectedHitbox = AllHitboxes[n];

						if (IsSelected) ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::NewLine();
				ImGui::Text("Radius");
				ImGui::SliderFloat("  ", &Settings::FOV, 0.0f, 1000.0f, "%.2f");
				break;
			case 1:
				ImGui::NewLine();

				if (ImGui::BeginChild("Player", ImVec2(260, 490), true))
				{
					ImGui::Text("Player:");
					ImGui::Checkbox("Box", &Settings::bPlayerBoxESP);

					if (Settings::bPlayerBoxESP)
					{
						if (ImGui::BeginCombo("Style", SelectedBoxStyle))
						{
							for (auto n = 0; n < IM_ARRAYSIZE(AllBoxStyles); ++n)
							{
								auto IsSelected = (SelectedBoxStyle == AllBoxStyles[n]);

								if (ImGui::Selectable(AllBoxStyles[n], IsSelected)) SelectedBoxStyle = AllBoxStyles[n];

								if (IsSelected) ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}
					}

					ImGui::Checkbox("Name", &Settings::bPlayerNameESP);
					ImGui::Checkbox("Line", &Settings::bPlayerLineESP);
					ImGui::Checkbox("Head", &Settings::bPlayerHeadESP);
					ImGui::Checkbox("Distance", &Settings::bPlayerDistanceESP);

					ImGui::Text("Max Draw Distance");
					ImGui::SliderFloat("   ", &Settings::MaxPlayerESPDistance, 0.0f, 1000.0f, "%.2f");

					ImGui::NewLine();
					ImGui::ColorPicker3("Visible", Settings::PlayerVisibleColor, ColorPickerFlags);
					ImGui::SameLine();
					ImGui::ColorPicker3("Invisible", Settings::PlayerInvisibleColor, ColorPickerFlags);

					ImGui::EndChild();
				}

				ImGui::SameLine();
				if (ImGui::BeginChild("Misc", ImVec2(260, 490), true))
				{
					ImGui::Text("Misc:");
					ImGui::Checkbox("Crosshair", &Settings::bDrawFakeCrosshair);
					ImGui::Checkbox("Targeting FOV", &Settings::bDrawFOV);
					ImGui::EndChild();
				}
				break;
			default:
				break;
			}

			ImGui::End();
		}
	}

	ImGui::Render();
}

LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_KEYUP && (wParam == VK_INSERT))
	{
		Settings::bOptions = !Settings::bOptions;
		ImGui::GetIO().MouseDrawCursor = Settings::bOptions;
	}

	if (Settings::bOptions)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		return true;
	}

	return CallWindowProcW(WndProcOriginal, hWnd, msg, wParam, lParam);
}

HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
HRESULT PresentHook(IDXGISwapChain* SwapChain, UINT syncInterval, UINT flags)
{
	if (device == nullptr)
	{
		SwapChain->GetDevice(__uuidof(device), reinterpret_cast<PVOID*>(&device));
		device->GetImmediateContext(&immediateContext);

		ID3D11Texture2D* renderTarget = nullptr;
		SwapChain->GetBuffer(0, __uuidof(renderTarget), reinterpret_cast<PVOID*>(&renderTarget));
		device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView);
		renderTarget->Release();

		DXGI_SWAP_CHAIN_DESC desc{};
		SwapChain->GetDesc(&desc);

		hWnd = desc.OutputWindow;
		WndProcOriginal = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProcHook)));

		ImGui_ImplDX11_Init(hWnd, device, immediateContext);
		ImGui_ImplDX11_CreateDeviceObjects();

		// Colors...
		{
			auto& style = ImGui::GetStyle();
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

			auto BlackColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
				PurpleColor = ImVec4(0.56862745f, 0.101960f, 0.835294f, 1.0f);

			style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.0f);
			style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.0f);
			style.Colors[ImGuiCol_WindowBg] = BlackColor;
			style.Colors[ImGuiCol_ChildWindowBg] = BlackColor;
			style.Colors[ImGuiCol_PopupBg] = BlackColor;
			style.Colors[ImGuiCol_Border] = PurpleColor;
			style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.0f);
			style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.0f);
			style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_TitleBg] = BlackColor;
			style.Colors[ImGuiCol_TitleBgActive] = BlackColor;
			style.Colors[ImGuiCol_MenuBarBg] = BlackColor;
			style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.0f);
			style.Colors[ImGuiCol_CheckMark] = PurpleColor;
			style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
			style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.0f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.0f);
			style.Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.0f);
			style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.0f);
			style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.0f);
			style.Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
			style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
			style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.0f);
			style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
			style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.0f);
			style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
			style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.0f);
			style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
			style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
		}

		width = Draw.GetWidth(), height = Draw.GetHeight();
	}

	immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);

	auto& window = BeginScene();
	do
	{
		if (EnemyPawn) EnemyPawn = nullptr;

		auto WorldInstance = World->GetContext();
		if (!UKismetSystemLibrary::IsValid(WorldInstance)) break;

		auto OwningGameInstance = read<UGameInstance*>(WorldInstance->GetReference() + 0x180);
		if (!UKismetSystemLibrary::IsValid(OwningGameInstance)) break;

		auto LocalPlayers = read<uintptr_t>(OwningGameInstance->GetReference() + 0x38);
		if (!LocalPlayers) break;

		auto LocalPlayer = read<uintptr_t>(LocalPlayers);
		if (!LocalPlayer) break;

		LocalPlayerController = read<APlayerController*>(LocalPlayer + 0x30);
		if (!UKismetSystemLibrary::IsValid(LocalPlayerController)) break;

		LocalPawn = read<APlayerPawn*>(LocalPlayerController->GetReference() + 0x2A8);
		if (!UKismetSystemLibrary::IsValid(LocalPawn)) break;

		auto LocalPawnPlayerState = LocalPawn->GetPlayerState();
		if (!UKismetSystemLibrary::IsValid(LocalPawnPlayerState)) break;

		std::vector<APlayerPawn*> PlayerPawns{};
		for (int LevelIndex = 0; LevelIndex <
			read<int>(WorldInstance->GetReference() + 0x140); ++LevelIndex)
		{
			auto Levels = read<uintptr_t>(WorldInstance->GetReference() + 0x138);
			if (Levels == NULL) break;

			auto CurrentLevel = read<uintptr_t>(Levels + (LevelIndex * sizeof(uintptr_t)));
			if (CurrentLevel == NULL) continue;

			// Loop all actors in the current level, inner-loop.
			for (auto i = 0; i < read<int>(
				CurrentLevel + (0x98 + sizeof(uintptr_t))); ++i)
			{
				auto Actors = read<uintptr_t>(CurrentLevel + 0x98);
				if (Actors == NULL) break;

				auto CurrentActor = read<AActor*>(Actors + (i * sizeof(uintptr_t)));
				if (CurrentActor == NULL) continue;

				auto ObjectName = UKismetSystemLibrary::GetObjectName(CurrentActor);
				if (!ObjectName.has_value()) continue;

				auto bIsPlayer = wcsstr(ObjectName.value(), L"TBP_ElysiumPlayer_C") || wcsstr(ObjectName.value(), L"TBP_Player_C_")
					|| wcsstr(ObjectName.value(), L"TBP_NPC_Primogen_C") || wcsstr(ObjectName.value(), L"TBP_NPC_C_");

				if (bIsPlayer) PlayerPawns.push_back(static_cast<APlayerPawn*>(CurrentActor));

				UEngine::FreeMemory(reinterpret_cast<uintptr_t>(ObjectName.value()));
			}
		}

		auto ClosestPlayerPawnDistance = FLT_MAX;
		for (auto CurrentPawn : PlayerPawns)
		{
			if (!UKismetSystemLibrary::IsValid(CurrentPawn)) continue;

			if (CurrentPawn == LocalPawn) continue;

			auto CurrentPawnPlayerState = CurrentPawn->GetPlayerState();
			if (!UKismetSystemLibrary::IsValid(CurrentPawnPlayerState)) continue;

			auto bIsTeammate = LocalPawnPlayerState->IsOfSameTeam(CurrentPawnPlayerState);
			if (bIsTeammate) continue;

			auto MeshComponent = CurrentPawn->GetMesh();
			if (!UKismetSystemLibrary::IsValid(MeshComponent)) continue;

			auto HeadWorldLocation = MeshComponent->GetBoneLocation(e_bones::Head),
				OriginWorldLocation = MeshComponent->GetBoneLocation(e_bones::Root);
			if (HeadWorldLocation == FVector() || OriginWorldLocation == FVector()) continue;

			auto HeadScreenLocation = LocalPlayerController->ProjectWorldToScreen(HeadWorldLocation),
				OriginScreenLocation = LocalPlayerController->ProjectWorldToScreen(OriginWorldLocation);
			if (HeadScreenLocation == FVector() || OriginScreenLocation == FVector()) continue;

			auto DistanceFromLocalPawn = (CurrentPawn->GetDistanceTo(LocalPawn) / 100.0f);
			auto bShouldGetTargetingData = true, bIsVisible = LocalPlayerController->LineOfSightTo(CurrentPawn);

			// If the person does not want to target players out of screen space and that is applicable we will simply not get the targeting data for this player.
			if (Settings::bSkipTargetsOutOfBounds && !Util::IsLocationInScreen(HeadScreenLocation)) bShouldGetTargetingData = false;

			// If the player is invisible and skip is selected for targeting then we won't get the data for this player.
			if (Settings::bDontTargetInvisiblePlayers && !bIsVisible) bShouldGetTargetingData = false;

			// If the player is our teammate and skipping targeting for teammates is selected then we won't get targeting data for this player.
			if (Settings::bDontTargetTeammates && bIsTeammate) bShouldGetTargetingData = false;

			if (bShouldGetTargetingData)
			{
				// Get targeting data.
				auto ScreenLocationX = HeadScreenLocation.X - (width / 2.0f), ScreenLocationY = HeadScreenLocation.Y - (height / 2.0f);
				auto PlayerDistanceFromCenter = UKismetMathLibrary::Sqrt(ScreenLocationX * ScreenLocationX + ScreenLocationY * ScreenLocationY);

				if (PlayerDistanceFromCenter < ClosestPlayerPawnDistance && PlayerDistanceFromCenter < Settings::FOV)
				{
					EnemyPawn = static_cast<AEnemyPawn*>(CurrentPawn);
					ClosestPlayerPawnDistance = PlayerDistanceFromCenter;
				}
			}

			auto DrawColor = ImGui::GetColorU32({ Settings::PlayerInvisibleColor[0], Settings::PlayerInvisibleColor[1], Settings::PlayerInvisibleColor[2], 1.0f });
			if (bIsVisible)
				DrawColor = ImGui::GetColorU32({ Settings::PlayerVisibleColor[0], Settings::PlayerVisibleColor[1], Settings::PlayerVisibleColor[2], 1.0f });

			// If the player is of a greater distance than the one selected for the max player draw distance then we will simply skip over all draw functionality and onto the next player.
			if (DistanceFromLocalPawn > Settings::MaxPlayerESPDistance) continue;

			// If the player is our teammate and skipping drawing for teammates is selected then we will simply skip over all draw functionality and onto the next player.
			if (Settings::bDontDrawTeammates && bIsTeammate) continue;

			// Not going to draw visuals for players who are outside of our screen space.
			if (!Util::IsLocationInScreen(HeadScreenLocation)) continue;

			auto BoxHeight = ((HeadScreenLocation.Y - OriginScreenLocation.Y) * 1.1f);
			auto BoxWidth = (BoxHeight / 2.0f);

			auto BoxPositionX = (HeadScreenLocation.X - (BoxWidth / 2.0f)), BoxPositionY = OriginScreenLocation.Y;

			if (Settings::bPlayerBoxESP)
			{
				if (!strcmp(SelectedBoxStyle, "2D"))
				{
					window.DrawList->AddRect(ImVec2(BoxPositionX, BoxPositionY), ImVec2(BoxPositionX + BoxWidth, BoxPositionY + BoxHeight), DrawColor);
				}
				else if (!strcmp(SelectedBoxStyle, "2D-FILLED"))
				{
					window.DrawList->AddRectFilled(ImVec2(BoxPositionX, BoxPositionY), ImVec2(BoxPositionX + BoxWidth, BoxPositionY + BoxHeight), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 0.4f }));
					window.DrawList->AddRect(ImVec2(BoxPositionX, BoxPositionY), ImVec2(BoxPositionX + BoxWidth, BoxPositionY + BoxHeight), DrawColor);
				}
				else if (!strcmp(SelectedBoxStyle, "CORNERED"))
				{
					Draw.DrawCorneredBox(window, BoxPositionX, BoxPositionY, BoxWidth, BoxHeight, DrawColor, DrawColor);
				}
				else if (!strcmp(SelectedBoxStyle, "CORNERED-FILLED"))
				{
					window.DrawList->AddRectFilled(ImVec2(BoxPositionX, BoxPositionY), ImVec2(BoxPositionX + BoxWidth, BoxPositionY + BoxHeight), ImGui::GetColorU32({ 0.f, 0.f, 0.f, 0.4f }));
					Draw.DrawCorneredBox(window, BoxPositionX, BoxPositionY, BoxWidth, BoxHeight, DrawColor, DrawColor);
				}
				else if (!strcmp(SelectedBoxStyle, "3D (ENGINE-BOUNDING)"))
				{
					Draw.Draw3DBoundingBox(window, CurrentPawn, DrawColor, LocalPlayerController);
				}
			}

			if (Settings::bPlayerDistanceESP || Settings::bPlayerNameESP)
			{
				std::string MessageToRender{};
				if (Settings::bPlayerDistanceESP)
					MessageToRender = std::string("[") + std::to_string(static_cast<int>(DistanceFromLocalPawn)) + std::string("m] ");

				if (Settings::bPlayerNameESP)
				{
					auto PlayerName = CurrentPawnPlayerState->GetPlayerName();
					if (PlayerName.has_value()) MessageToRender += PlayerName.value();
				}

				auto Message = MessageToRender.c_str();
				auto TextSize = ImGui::GetFont()->CalcTextSizeA(window.DrawList->_Data->FontSize, FLT_MAX, 0, Message);

				window.DrawList->AddText(ImVec2(BoxPositionX + BoxWidth, ((BoxPositionY + BoxHeight) - TextSize.y)), DrawColor, Message);
			}

			if (Settings::bPlayerHeadESP)
			{
				auto HeadCircleRadius = 0.0f;

				// We want to adjust how large the circle is based on the distance the player is from us as to make sure it does not look odd.
				if (DistanceFromLocalPawn >= 100.0f) HeadCircleRadius = 0.5f;
				else if (DistanceFromLocalPawn >= 50.0f) HeadCircleRadius = 1.0f;
				else if (DistanceFromLocalPawn >= 20.0f) HeadCircleRadius = 3.0f;
				else HeadCircleRadius = 10.0f;

				window.DrawList->AddCircle(ImVec2(HeadScreenLocation.X, HeadScreenLocation.Y), HeadCircleRadius, DrawColor, 100, 2.0f);
			}

			if (Settings::bPlayerLineESP)
				window.DrawList->AddLine(ImVec2(width / 2.0f, height), ImVec2(OriginScreenLocation.X, OriginScreenLocation.Y), DrawColor, 2.0f);
		}

		if (Settings::bAimbot && EnemyPawn != nullptr &&
			LocalPlayerController != nullptr && GetAsyncKeyState(VK_RBUTTON))
		{
			auto EnemyTargetLocation = Util::GetAimWorldLocation(LocalPlayerController, EnemyPawn);
			if (EnemyTargetLocation == FVector()) break;

			auto ScreenTargetLocation = LocalPlayerController->ProjectWorldToScreen(EnemyTargetLocation);
			if (ScreenTargetLocation == FVector()) break;

			auto MoveToX = ScreenTargetLocation.X - (width / 2),
				MoveToY = ScreenTargetLocation.Y - (height / 2);

			MoveToX /= Settings::Smoothing;
			MoveToY /= Settings::Smoothing;

			mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(MoveToX), static_cast<DWORD>(MoveToY), NULL, NULL);
		}
	} while (false);

	if (Settings::bDrawFOV)
		window.DrawList->AddCircle(ImVec2(width / 2.0f, height / 2.0f), Settings::FOV, ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f }), 100, 2.0f);

	if (Settings::bDrawFakeCrosshair)
	{
		window.DrawList->AddLine(ImVec2((width / 2.0f) + 4.0f, (height / 2.0f)),
			ImVec2((width / 2.0f) + 12.0f, (height / 2.0f)), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f }), 2.0f);
		window.DrawList->AddLine(ImVec2((width / 2.0f) - 4.0f, (height / 2.0f)),
			ImVec2((width / 2.0f) - 12.0f, (height / 2.0f)), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f }), 2.0f);
		
		window.DrawList->AddLine(ImVec2((width / 2.0f), (height / 2.0f) + 4.0f),
			ImVec2((width / 2.0f), (height / 2.0f) + 12.0f), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f }), 2.0f);
		window.DrawList->AddLine(ImVec2((width / 2.0f), (height / 2.0f) - 4.0f),
			ImVec2((width / 2.0f), (height / 2.0f) - 12.0f), ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 1.0f }), 2.0f);
	}

	EndScene(window);
	return PresentOriginal(SwapChain, syncInterval, flags);
}

HRESULT(*ResizeBuffersOriginal)(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) = nullptr;
HRESULT ResizeBuffersHook(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	if (renderTargetView)
	{
		immediateContext->OMSetRenderTargets(0, 0, 0);
		renderTargetView->Release();
	}

	HRESULT hr = ResizeBuffersOriginal(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	ID3D11Texture2D* pBuffer;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
	// Perform error handling here!

	device->CreateRenderTargetView(pBuffer, NULL, &renderTargetView);
	// Perform error handling here!
	pBuffer->Release();

	immediateContext->OMSetRenderTargets(1, &renderTargetView, NULL);

	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = Width;
	vp.Height = Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	immediateContext->RSSetViewports(1, &vp);

	return hr;
}

auto SetRenderHook()
{
	IDXGISwapChain* swapChain = nullptr;
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* context = nullptr;
	auto                 featureLevel = D3D_FEATURE_LEVEL_11_0;

	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, 0,
		&featureLevel, 1, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context))) {
		return false;
	}

	auto table = *reinterpret_cast<PVOID**>(swapChain);
	auto present = table[8];
	auto resize = table[13];

	context->Release();
	device->Release();
	swapChain->Release();

	MH_Initialize();

	MH_CreateHook(present, PresentHook, reinterpret_cast<PVOID*>(&PresentOriginal));
	MH_EnableHook(present);

	MH_CreateHook(resize, ResizeBuffersHook, reinterpret_cast<PVOID*>(&ResizeBuffersOriginal));
	MH_EnableHook(resize);

	return true;
}

auto Initialize()
{
	Settings::GameBaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	if (!Settings::GameBaseAddress) return false;

	Settings::GameSizeOfImage = reinterpret_cast<PIMAGE_NT_HEADERS>(
		reinterpret_cast<std::uint8_t*>(Settings::GameBaseAddress) +
		reinterpret_cast<PIMAGE_DOS_HEADER>(Settings::GameBaseAddress)->e_lfanew)->OptionalHeader.SizeOfImage;
	if (!Settings::GameSizeOfImage) return false;

	World = new UWorld(RVA(Util::PatternScan(
		Settings::GameBaseAddress,
		Settings::GameSizeOfImage,
		L"\x48\x8B\x3D\x00\x00\x00\x00\x48\x8B\x5C\x24\x00\x48\x8B\xC7",
		L"xxx????xxxx?xxx"
	), 7));

	return SetRenderHook();
}

bool DllMain(void** hinstDLL, unsigned long fdwReason, void* lpReserved)
{
	return Initialize();
}