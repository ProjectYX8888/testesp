#include "includes.h"
#include "IconsFontAwesome5.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <TlHelp32.h>
#include "dub_esp.h"
#include "imgui/imgui_internal.h"

#include "GameClasses.h"



extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

ImFont* normal;
void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);

	io.Fonts->AddFontDefault();

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	normal = io.Fonts->AddFontFromFileTTF("D:/Project2/sources/HammafiaUI-main/Hammafia UI (FAN ART)/icons.ttf", 16, &icons_config, icons_ranges);




}

	

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

uintptr_t worldAddress = 0;
uintptr_t noReloadAddress = 0;
uintptr_t unlimitedAmmoAddress = 0;
uintptr_t replayInterfaceAddress = 0;
uintptr_t w2sAddress = 0;
uintptr_t bonePosAddress = 0;

uintptr_t base;

bool allowToLoopEntities = false;

static bool menu_show = false;
static int tabs = 0;
bool init = false;
static bool shbypass = false;
static bool esp = false;


namespace Globals
{
	static bool Open = true;
	static int Tab = 0;

	static bool SHBEnabled = false;
	static bool Dumper = false;

	static bool TriggerBot = false;
	static bool GodMode = false;

	static bool UnlimitedAmmo = false;
	static bool NoReload = false;

	static bool ESP = true;
	static bool ESPSnaplines = true;
	static bool ESPSkeleton = true;
}

World* world = nullptr;
Player* LocalPlayer = nullptr;

void killProcessByName(const char* filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, filename) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

ImVec2 WorldToScreen(Vector3 pos)
{
	auto& io = ImGui::GetIO();
	ImVec2 tempVec2;
	reinterpret_cast<bool(__fastcall*)(Vector3*, float*, float*)>(w2sAddress)(&pos, &tempVec2.x, &tempVec2.y);
	tempVec2.x *= io.DisplaySize.x;
	tempVec2.y *= io.DisplaySize.y;
	return tempVec2;
}

ImVec2 GetBonePosW2S(const uint64_t cPed, const int32_t wMask)
{
	__m128 tempVec4;
	reinterpret_cast<void* (__fastcall*)(uint64_t, __m128*, int32_t)>(bonePosAddress)(cPed, &tempVec4, wMask);
	return WorldToScreen(Vector3::FromM128(tempVec4));
}

bool IsOnScreen(ImVec2 coords)
{
	if (coords.x < 0.1F || coords.y < 0.1F)
	{
		return false;
	}
	else {
		return true;
	}
}
namespace Visuals
{
	void Hook(void)
	{
		if (allowToLoopEntities)
		{
			world = *(World**)(worldAddress);
			if (!world)
				return;

			Player* localPlayer = world->LocalPlayer;
			if (!localPlayer)
				return;

			ReplayInterface* replayInterface = *(ReplayInterface**)(replayInterfaceAddress);
			if (!replayInterface)
				return;

			PedInterface* pedInterface = replayInterface->pedInterface;
			if (!pedInterface)
				return;

			if (Globals::ESP)
			{
				for (int i = 0; i < pedInterface->pedMaximum; i++)
				{
					Player* currentPed = pedInterface->pedList->GetCurrentPlayer(i);
					if (!currentPed) continue;

					ImVec2 pedLocation = WorldToScreen(currentPed->Coords);
					// add isonscree check

					ImVec2 localPlayerLocation = WorldToScreen(localPlayer->Coords);

					if (Globals::ESPSnaplines)
					{
						ImVec2 WindowSize = ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

						if (!IsOnScreen(pedLocation)) continue;

						ImGui::GetBackgroundDrawList()->AddLine(ImVec2(WindowSize.x / 2, WindowSize.y), pedLocation, ImColor(255, 255, 255, 255), 1.f);
					}

					if (Globals::ESPSkeleton)
					{
						// SKELETON

						/*ImColor color = {255, 255, 255 ,255};

						ImVec2 Pelvis = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x2e28);
						if (!IsOnScreen(Pelvis)) continue;

						ImVec2 Neck = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x9995);
						if (!IsOnScreen(Neck)) continue;
						ImVec2 Left_Hand = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x49D9);
						if (!IsOnScreen(Left_Hand)) continue;

						ImVec2 Right_Hand = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xDEAD);
						if (!IsOnScreen(Right_Hand)) continue;

						ImVec2 Left_Foot = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x3779);
						if (!IsOnScreen(Left_Foot)) continue;

						ImVec2 Right_Foot = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xCC4D);
						if (!IsOnScreen(Right_Foot)) continue;


						//ImGui::GetBackgroundDrawList()->AddLine(Head,Neck,ImColor(255, 255, 255, 255), 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Neck, Left_Hand, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Neck, Right_Hand, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Neck, Pelvis, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Pelvis, Left_Foot, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Pelvis, Right_Foot, color, 1.f);*/

						ImColor color = { 255, 255, 255 ,255 };

						ImVec2 Pelvis = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x2e28);
						if (!IsOnScreen(Pelvis)) continue;

						ImVec2 Neck = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x9995);
						if (!IsOnScreen(Neck)) continue;

						ImVec2 Clavicle_Left = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xFCD9);
						if (!IsOnScreen(Clavicle_Left)) continue;

						ImVec2 Clavicle_Right = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x29D2);
						if (!IsOnScreen(Clavicle_Right)) continue;

						ImVec2 Uperarm_left = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xB1C5);
						if (!IsOnScreen(Uperarm_left)) continue;

						ImVec2 Uperarm_right = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x9D4D);
						if (!IsOnScreen(Uperarm_right)) continue;

						ImVec2 Elbow_l = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x58B7);
						if (!IsOnScreen(Elbow_l)) continue;

						ImVec2 Elbow_r = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xBB0);
						if (!IsOnScreen(Elbow_r)) continue;

						ImVec2 R_FormArm = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x6E5C);
						if (!IsOnScreen(R_FormArm)) continue;

						ImVec2 L_FormArm = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xEEEB);
						if (!IsOnScreen(L_FormArm)) continue;

						ImVec2 SKEL_R_Hand = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xDEAD);
						if (!IsOnScreen(SKEL_R_Hand)) continue;

						ImVec2 SKEL_L_Hand = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x49D9);
						if (!IsOnScreen(SKEL_L_Hand)) continue;

						ImVec2 SKEL_L_Thigh = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xE39F);
						if (!IsOnScreen(SKEL_L_Thigh)) continue;

						ImVec2 SKEL_L_Calf = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xF9BB);
						if (!IsOnScreen(SKEL_L_Calf)) continue;

						ImVec2 SKEL_R_Thigh = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xCA72);
						if (!IsOnScreen(SKEL_R_Thigh)) continue;

						ImVec2 SKEL_R_Calf = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x9000);
						if (!IsOnScreen(SKEL_R_Calf)) continue;

						ImVec2 SKEL_L_Foot = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0x3779);
						if (!IsOnScreen(SKEL_L_Foot)) continue;

						ImVec2 SKEL_R_Foot = GetBonePosW2S(reinterpret_cast<uint64_t>(currentPed), 0xCC4D);
						if (!IsOnScreen(SKEL_R_Foot)) continue;

						ImGui::GetBackgroundDrawList()->AddLine(Neck, Clavicle_Right, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Neck, Clavicle_Left, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Clavicle_Right, Uperarm_right, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Clavicle_Left, Uperarm_left, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Uperarm_right, Elbow_r, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Uperarm_left, Elbow_l, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Elbow_r, R_FormArm, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Elbow_l, L_FormArm, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(R_FormArm, SKEL_R_Hand, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(L_FormArm, SKEL_L_Hand, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Neck, Pelvis, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(Pelvis, SKEL_L_Thigh, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(Pelvis, SKEL_R_Thigh, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(SKEL_L_Thigh, SKEL_L_Calf, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(SKEL_R_Thigh, SKEL_R_Calf, color, 1.f);

						ImGui::GetBackgroundDrawList()->AddLine(SKEL_L_Calf, SKEL_L_Foot, color, 1.f);
						ImGui::GetBackgroundDrawList()->AddLine(SKEL_R_Calf, SKEL_R_Foot, color, 1.f);
					}
				}
			}
		}
	}
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{




	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}


	if (GetAsyncKeyState(VK_F8) & 1) {

		menu_show = !menu_show;
		//return 0;
	}



	if (menu_show) {

		ImGui::GetMouseCursor();
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
		ImGui::GetIO().WantCaptureMouse = menu_show;
		ImGui::GetIO().MouseDrawCursor = menu_show;
	

		ImColor mainColor = ImColor(255, 0, 0);
		ImColor bodyColor = ImColor(25, 25, 25, 255);
		ImColor bodyColor1 = ImColor(63, 70, 96, 255);
		ImColor bodyColor12 = ImColor(101, 110, 139, 255);
		ImColor fontColor = ImColor(255, 255, 255, 255);
		ImVec4 mainColorHovered = ImVec4(mainColor.Value.x + 0.1f, mainColor.Value.y + 0.1f, mainColor.Value.z + 0.1f, mainColor.Value.w);
		ImVec4 mainColorActive = ImVec4(mainColor.Value.x + 0.2f, mainColor.Value.y + 0.2f, mainColor.Value.z + 0.2f, mainColor.Value.w);
		ImVec4 menubarColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w - 0.8f);
		ImVec4 frameBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .1f);
		ImVec4 tooltipBgColor = ImVec4(bodyColor.Value.x, bodyColor.Value.y, bodyColor.Value.z, bodyColor.Value.w + .05f);

		ImGuiStyle& style = ImGui::GetStyle();

		
		style.Alpha = 1.0f;
		style.WindowPadding = ImVec2(8, 8);
		style.WindowMinSize = ImVec2(32, 32);
		style.WindowRounding = 0.f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

		style.FramePadding = ImVec2(6.5, 6.5);
		style.FrameRounding = 0.0f;
		style.ItemSpacing = ImVec2(4, 9);
		style.ItemInnerSpacing = ImVec2(8, 8);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 3.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 0.0f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;

		style.CurveTessellationTol = 1.25f;

		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
		style.Colors[ImGuiCol_WindowBg] = ImColor(15, 14, 15, 255);
		style.Colors[ImGuiCol_ChildBg] = ImColor(25, 25, 25, 255); 
		style.Colors[ImGuiCol_Border] = ImColor(64, 64, 64, 0);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
		style.Colors[ImGuiCol_TitleBg] = mainColor;
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		style.Colors[ImGuiCol_TitleBgActive] = mainColor; 
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(205, 92, 92, 175);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImColor(31, 30, 31, 255); 
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);

		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 0.40f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 0.97f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.98f, 0.26f, 0.26f, 0.35f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);



		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		Visuals::Hook();

		static bool menu_show = true;
		static TextEditor		 editor;

	
		ImGui::Begin("ProjectYX", &menu_show, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
		ImGui::SetWindowSize(ImVec2(750, 380), ImGuiCond_Always);

		ImGui::Columns(2);

		ImGui::SetColumnOffset(1, 223.4);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + -9); //
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + -9); //
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(200 / 255.f, 200 / 255.f, 200 / 255.f, 0.0f));
		if (ImGui::BeginChild("wondoa", ImVec2(223.4, 380)))
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 9); //
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13); //
			if (ImGui::Button(ICON_FA_CODE  "  Executor", ImVec2(200, 37)))
			{
				tabs = 0;
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13); //
			if (ImGui::Button(ICON_FA_DOWNLOAD  "  Dumper", ImVec2(200, 37)))
			{
				tabs = 1;
			}
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13); //
			if (ImGui::Button(ICON_FA_BUG  "  Blocker", ImVec2(200, 37)))
			{
				tabs = 2;
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 13); //
			if (ImGui::Button(ICON_FA_BOOK  "  Menus", ImVec2(200, 37)))
			{
				tabs = 3;
			}

	
			ImGui::EndChild();

		}
		
		ImGui::PopStyleColor();
		
		
		ImGui::NextColumn();
		
		static const char* current_menu = NULL;
		static const char* current_resource = NULL;
		
		
		

		if (tabs == 0) {

			

			editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
			editor.SetShowWhitespaces(false);
			editor.SetPalette(TextEditor::GetFakePalette());
			editor.SetReadOnly(false);
			
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5); //
			if (ImGui::Button(ICON_FA_FILE  "  Execute From File", ImVec2(155, 28)))
			{
		
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Isolated", ImVec2(80, 28)))
			{

			}

			ImGui::SameLine();
			ImGui::Checkbox("Skelet", &skeletonesploop);
			ImGui::SameLine();
			ImGui::Checkbox("ESP", &dubesploop);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5); //
			editor.Render("TextEditor", ImVec2(510, 262), false);
		
		
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8); //
			ImGui::PushItemWidth(405);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5); //
			
			if (ImGui::BeginCombo("##cheatlist", current_resource))
			{

				if (ImGui::Button("Menu1"))
				{
		
				};

				ImGui::EndCombo();
			}

			ImGui::SameLine();
		
			if (ImGui::Button(ICON_FA_CHECK "  Execute", ImVec2(100, 28)))
			{
				std::string text = editor.GetText();
				remove("C:\\Projectyx\\menu.js");
				std::ofstream out("C:\\Projectyx\\menu.js");
				out << text;
				out.close();
				
			
				
			};




		}

		if (tabs == 1)
		{


			ImGui::Spacing();
			ImGui::Checkbox(("Enable ESP"), &Globals::ESP); ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Checkbox(("Snaplines"), &Globals::ESPSnaplines); ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Checkbox(("Skeleton"), &Globals::ESPSkeleton); ImGui::Spacing();
			ImGui::Spacing();
		}

		if (tabs == 2) {


			ImGui::Text("Soon");
		}

		if (tabs == 3) {
			
			ImGui::Text("Soon");
			



		}


		


		ImGui::End();

		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}



	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{

	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);


	base = (uintptr_t)GetModuleHandleA(nullptr);

	if (GetModuleHandleA(("FiveM_GTAProcess.exe")))
	{
		allowToLoopEntities = true;
		worldAddress = base + 0x603AF59;
		replayInterfaceAddress = base + 0x1EFD4C8;
		w2sAddress = base + 0xA09344;
		bonePosAddress = base + 0x779d64;
	}


	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hMod, 0, nullptr);
		
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}