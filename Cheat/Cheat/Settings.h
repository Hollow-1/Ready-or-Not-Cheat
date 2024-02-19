#pragma once

#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <cwchar>
#include <string>
#include <unordered_map>
#include <vector>

#include "Types.h"
#include "Singleton.h"
#include "Structures.h"
#include "UnrealMath.h"
#include "GameData.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h> 


#define GameVars	GameVariables::Get()
#define Offsets		GameOffsets::Get()
#define CFG			Settings::Get()

using namespace std;

class GameVariables : public Singleton<GameVariables>
{
public:
	DWORD processID;
	ULONG_PTR moduleBase;

	LPCWSTR game_window_title = L"Ready Or Not  ";
	const char* targetProcess = "ReadyOrNot-Win64-Shipping.exe";

	GLFWwindow* glfw_wnd = NULL;
	HWND game_hwnd = NULL; //游戏窗口句柄
	HWND hwnd = NULL;      //绘制窗口句柄

	//相机视角数据每帧都会更新
	FCameraCacheEntry camera;

	//缓存游戏Actor数据
	vector<GamePlayer>* players = new vector<GamePlayer>();
	vector<GameEntity>* entities = new vector<GameEntity>();
	
	vector<GamePlayer>* tmpPlayers = new vector<GamePlayer>();
	vector<GameEntity>* tmpEntities = new vector<GameEntity>();


	//这些数据不会经常变动，不需要每帧更新
	bool wndChanged = false;
	int32 wndPosX = 100;
	int32 wndPosY = 100;
	int32 wndWidth = 1600;
	int32 wndHeight = 900;

	float wndCenterX = wndWidth / 2.0f;
	float wndCenterY = wndHeight / 2.0f;

	ULONG_PTR g_world = NULL;
	ULONG_PTR game_instance = NULL;
	ULONG_PTR local_player_pawn = NULL;
	ULONG_PTR local_player_array = NULL;
	ULONG_PTR local_player = NULL;
	ULONG_PTR local_player_root = NULL;
	ULONG_PTR local_player_controller = NULL;
	ULONG_PTR local_player_camera_manager = NULL;
	ULONG_PTR local_player_state = NULL;
	ULONG_PTR persistent_level = NULL;
	TArray actor_array;

	//Actor名字字符串 和 枚举映射表，用来加快字符串比较
	const unordered_map<string, int32> actorTypeMap = GetActorTypeMap();

	//程序退出信号，其他线程会读取这个值
	bool shouldExit = false;
};


class Settings : public Singleton<Settings>
{
public:
	bool show_menu = true;
	bool show_menu_changed = false;

	bool drawActorAll = false;
	bool drawBoneIndex = false;
	bool espSkeleton = true;
	bool espBox = true;
	bool espHealthBar = true;
	bool espDist = true;
	int32 boxType = 0; // 0=全框， 1=角框

	//最远透视距离，显示的时候要 除 100
	float maxDistance = 10000;

	//radar
	bool drawRadar = true;
	ImVec2 radarCenter = { 200,200 };  //雷达中心位置
	float radarRadius = 150;     //雷达的显示半径
	int32 radarCircleCount = 4;  //雷达同心圆数量
	float radarDist = 3000;      //雷达的绘制距离，敌人超过这个距离，就不会显示在雷达上

	//aimbot
	bool aimbotActive = true;
	bool drawFOV = false;
	float aimbotFOV = 200.f;

};


class GameOffsets : public Singleton<GameOffsets>
{
public:
	ULONG_PTR GWorld = 0x67efad0;
	ULONG_PTR GNames = 0x666ED80;
	ULONG_PTR TUObjectArray = 0x66ab0e0;

	ULONG_PTR pointerSize = 0x8;//0x8 = 64 Bit and 0x4 = 32 Bit

	//UObject
	ULONG_PTR UObjectToInternalIndex = 0xc;
	ULONG_PTR UObjectToClassPrivate = 0x10;
	ULONG_PTR UObjectToFNameIndex = 0x18;
	ULONG_PTR UObjectToOuterPrivate = 0x20;

	//Class: FNamePool
	ULONG_PTR FNameStride = 0x2;
	ULONG_PTR GNamesToFNamePool = 0x0;
	ULONG_PTR FNamePoolToCurrentBlock = 0x8;
	ULONG_PTR FNamePoolToCurrentByteCursor = 0xC;
	ULONG_PTR FNamePoolToBlocks = 0x10;

	//Class: FNameEntry
	ULONG_PTR FNameEntryHeader = 0x0;//Not needed for most games
	ULONG_PTR FNameEntryToLenBit = 0x6;
	ULONG_PTR FNameEntryToString = 0x2;

	ULONG_PTR GameInstance = 0x180;
	ULONG_PTR LocalPlayers = 0x38;
	ULONG_PTR PlayerController = 0x30;
	ULONG_PTR PlayerCameraManager = 0x2b8;

	ULONG_PTR CameraCachePrivate = 0x1ae0;		//PlayerCameraManager -> CameraCachePrivate

	ULONG_PTR PlayerNamePrivate = 0x300;

	ULONG_PTR ControllerToPawn = 0x250;
	ULONG_PTR RootComponent = 0x130;
	ULONG_PTR ComponentToWorld = 0x1c0;

	ULONG_PTR PlayerState = 0x228;
	ULONG_PTR Owner = 0xe0;

	ULONG_PTR PersistentLevel = 0x30;
	ULONG_PTR AActors = 0x98;

	ULONG_PTR SkinnedMeshToSkeletalMesh = 0x480;
	ULONG_PTR SkeletalMeshToFinalRefBoneInfo = 0x1d0; //UE4.26 => 0x1c8;  UE4.27 => 0x1d0
	ULONG_PTR BoneInfoSize = 0xc;

	ULONG_PTR CharacterToMesh = 0x280;  //Character -> SkeletalMeshComponent* Mesh

	ULONG_PTR MeshToTransformsArray = 0x4b0;					//USkinnedMeshComponent::ComponentSpaceTransformsArray
	ULONG_PTR MeshToCurrentRead = MeshToTransformsArray + 0x44; //USkinnedMeshComponent::CurrentReadComponentTransforms

	ULONG_PTR LastSubmitTime = 0x288;		//UPrimitiveComponent::LastSubmitTime
	ULONG_PTR LastRenderTime = 0x28c;		//UPrimitiveComponent::LastRenderTime
};