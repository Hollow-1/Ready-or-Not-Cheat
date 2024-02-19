#pragma once
#include "Settings.h"
#include "Memory.h"

//函数声明
void AddEntity(string displayName, EGameActorType type, ULONG_PTR pActor);
void AddPlayer(string displayName, EGameActorType type, ULONG_PTR pActor);


Vector3 WorldToScreen(Vector3 objPos)
{
	GameVariables& v = GameVars;
	return ProjectWorldToScreen(v.camera, objPos, (float)v.wndWidth, (float)v.wndHeight);
}

//获取距离相机的距离
float GetDistance(Vector3 vec) {
	return CalcDistance(GameVars.camera.POV.Location, vec);
}

FTransform GetTransform(ULONG_PTR rootComponent) {
	return Read<FTransform>(rootComponent + Offsets.ComponentToWorld);
}

string FName::GetStringWithoutNumber() {
	static bool isDecrypt = false;
	//Cache index and name
	static unordered_map<UINT32, string> nameCacheMap;

	if (nameCacheMap.find(index) != nameCacheMap.end()) {
		return nameCacheMap[index];
	}
	UINT32 block = index >> 16;
	UINT32 offset = index & 65535;

	offset = offset * (UINT32)Offsets.FNameStride;
	uintptr_t FNamePool = ((GameVars.moduleBase + Offsets.GNames) + Offsets.GNamesToFNamePool);
	UINT32 CurrentBlock = Read<UINT32>(FNamePool + Offsets.FNamePoolToCurrentBlock);
	UINT32 CurrentByteCursor = Read<UINT32>(FNamePool + Offsets.FNamePoolToCurrentByteCursor);
	if (block > CurrentBlock || (block == CurrentBlock && offset >= CurrentByteCursor)) {
		cout << "Invalid index. => 0x" << std::hex << index << endl;
		return "None";
	}

	uintptr_t NamePoolChunk = Read<uintptr_t>(FNamePool + Offsets.FNamePoolToBlocks + (block * Offsets.pointerSize));
	uintptr_t FNameEntry = NamePoolChunk + offset;

	UINT16 FNameEntryHeader = Read<UINT16>(FNameEntry + Offsets.FNameEntryHeader);
	uintptr_t strPtr = FNameEntry + Offsets.FNameEntryToString;
	UINT16 strLen = FNameEntryHeader >> Offsets.FNameEntryToLenBit;

	if (strLen > 0) {
		string name = ReadString(strPtr, strLen);

		if (isDecrypt) {
			name = FName::XorCypher(name);
		}
		nameCacheMap[index] = name;
		return name;
	}
	else {
		cout << "Invalid string length. index=0x" << std::hex << index << endl;
		return "None";
	}
}

struct UObject {
	static int32 getIndex(ULONG_PTR object) {
		return Read<int32>(object + Offsets.UObjectToInternalIndex);
	}

	static ULONG_PTR getClass(ULONG_PTR object) {//UClass*
		return Read<ULONG_PTR>(object + Offsets.UObjectToClassPrivate);
	}

	static FName getFName(ULONG_PTR object) {
		return Read<FName>(object + Offsets.UObjectToFNameIndex);
	}

	static ULONG_PTR getOuter(ULONG_PTR object) {//UObject*
		return Read<ULONG_PTR>(object + Offsets.UObjectToOuterPrivate);
	}

	static string getNameString(ULONG_PTR object) {
		return getFName(object).GetString();
	}

	static string getClassName(ULONG_PTR object) {
		return getNameString(getClass(object));
	}

	static bool isValid(ULONG_PTR object) {
		return (object > 0 && getFName(object).index > 0 && getClass(object) > 0);
	}
};

bool IsVisible(DWORD_PTR mesh) {
	float LastSubmitTime = Read<float>(mesh + Offsets.LastSubmitTime);
	float LastRenderTimeOnScreen = Read<float>(mesh + Offsets.LastRenderTime);
	//cout << "LastSubmitTime " << LastSubmitTime << " LastRenderTimeOnScreen" << LastRenderTimeOnScreen << endl;
	const float VisionTick = 0.06f;
	return LastRenderTimeOnScreen + VisionTick >= LastSubmitTime;
}

//读取骨骼名称和对应index
unordered_map<string, int32> GetBoneInfo(GamePlayer& player)
{
	unordered_map<string, int32> boneInfo;

	ULONG_PTR SkeletalMesh = Read<ULONG_PTR>(player.mesh + Offsets.SkinnedMeshToSkeletalMesh);
	TArray RefBoneInfoArray = Read<TArray>(SkeletalMesh + Offsets.SkeletalMeshToFinalRefBoneInfo);

	if (RefBoneInfoArray.IsValid() == false) {
		return boneInfo;
	}

	for (uint32 i = 0; i < RefBoneInfoArray.count; i++) {
		FName fName = Read<FName>(RefBoneInfoArray.addr + i * Offsets.BoneInfoSize);
		string name = fName.GetString();
		if (!name.empty()) {
			boneInfo[name] = (int32)i;
		}
	}
	return boneInfo;
}


//将一个实体添加到 GameVars.tmpEntities 
void AddEntity(string displayName, EGameActorType type, ULONG_PTR pActor)
{
	GameEntity entity;
	entity.type = type;
	entity.displayName = displayName;
	entity.pActor = pActor;
	entity.rootComponent = Read<ULONG_PTR>(pActor + Offsets.RootComponent);

	if (entity.rootComponent != NULL) {
		(*GameVars.tmpEntities).push_back(entity);
	}
}

void UpdatePlayerStatus(GamePlayer& player) {
	player.health = Read<float>(player.healthComponent + 0xE8);
	player.maxHealth = Read<float>(player.healthComponent + 0xEC);
	player.healthStatus = EPlayerHealthStatus(Read<int8>(player.healthComponent + 0x168));
}

//将一个玩家添加到 GameVars.tmpPlayers
void AddPlayer(string displayName, EGameActorType type, ULONG_PTR pActor) {

	GamePlayer player;
	player.type = type;
	player.displayName = displayName;
	player.pActor = pActor;
	player.rootComponent = Read<ULONG_PTR>(pActor + Offsets.RootComponent);
	player.mesh = Read<ULONG_PTR>(pActor + Offsets.CharacterToMesh);

	if (player.rootComponent == NULL || player.mesh == NULL) {
		return;
	}
	player.boneInfo = GetBoneInfo(player);
	for (string boneName : ::boneNameArray) {
		int32 index = 0;
		if (boneName == "END_LINE") {
			index = -1;
		}
		else if (player.boneInfo.find(boneName) != player.boneInfo.end()) {
			index = player.boneInfo[boneName];
		}
		player.boneIndexArray.push_back(index);
	}
	
	player.healthComponent = Read<ULONG_PTR>(pActor + 0x6B0);
	if (player.healthComponent != NULL) {
		UpdatePlayerStatus(player);
		(*GameVars.tmpPlayers).push_back(player);
	}
}


void ActorFilter(string name, ULONG_PTR pActor)
{
	if (CFG.drawActorAll) {
		AddEntity(name, ACTOR_UNDEFINE, pActor);
		return;
	}

	auto it = GameVars.actorTypeMap.find(name);
	if (it == GameVars.actorTypeMap.end()) {
		return;
	}

	EGameActorType type = EGameActorType(it->second);
	switch (type)
	{
	case ACTOR_UNDEFINE:
		return;
	
	case CyberneticsSwat_V2_C:
		AddPlayer(u8"队友", type, pActor);
		break;
	case CyberneticsSuspect_V2_C:
		AddPlayer(u8"嫌疑人", type, pActor);
		break;
	case Cybernetics_Civilian_V2_C:
		AddPlayer(u8"平民", type, pActor);
		break;

	default:
		return;
	}

}



string RandomString(int32 len)
{
	srand((uint32)time(NULL));
	string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	string newstr;
	int32 pos;
	while (newstr.size() != len) {
		pos = ((rand() % (str.size() - 1)));
		newstr += str.substr(pos, 1);
	}
	return newstr;
}