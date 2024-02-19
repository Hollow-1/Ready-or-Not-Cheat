#pragma once
#include "Types.h"
#include <string>
#include <unordered_map>

using namespace std;


enum EGameActorType
{
	ACTOR_UNDEFINE,

	CyberneticsSwat_V2_C,
	CyberneticsSuspect_V2_C,
	Cybernetics_Civilian_V2_C,

};

unordered_map<string, int32> GetActorTypeMap()
{
#define MAP_ADD(v)	{ #v, v}

	unordered_map<string, int32> map = {
		MAP_ADD(ACTOR_UNDEFINE),

		MAP_ADD(CyberneticsSwat_V2_C),
		MAP_ADD(CyberneticsSuspect_V2_C),
		MAP_ADD(Cybernetics_Civilian_V2_C),
	};

#undef MAP_ADD

	return map;
}

vector<string> boneNameArray = {
		"Head", "Neck_1", "spine_3", "spine_2", "spine_1", "END_LINE",									//头到盆骨
		"hand_LE", "lowerarm_LE", "upperarm_LE", "Neck_1", "upperarm_RI", "lowerarm_RI", "hand_RI", "END_LINE",//左手到右手
		"foot_LE", "calf_LE", "thigh_LE", "spine_1", "thigh_RI", "calf_RI", "foot_RI","END_LINE"		//左腿到右腿
};

enum EPlayerHealthStatus {
	HS_Healthy = 0,
	HS_Injured = 1,
	HS_Downed = 2,
	HS_Incapacitated = 3,
	HS_Dead = 4,
	HS_Arrested = 5,
	HS_NotAvailable = 6,
	HS_MAX = 7
};

string GetStatusString(EPlayerHealthStatus status) {
	switch (status) {
	case HS_Healthy:
		return u8"健康";
	case HS_Injured:
		return u8"受伤";
	case HS_Downed:
		return u8"倒地";
	case HS_Incapacitated:
		return u8"无意识";
	case HS_Dead:
		return u8"死亡";
	case HS_Arrested:
		return u8"已逮捕";
	case HS_NotAvailable:
		return u8"未知";
	default:
		return u8"未知";
	}
}

struct GamePlayer
{
	EGameActorType type;
	string displayName;

	ULONG_PTR pActor;
	ULONG_PTR rootComponent;
	ULONG_PTR mesh;

	unordered_map<string, int32> boneInfo;
	vector<int32> boneIndexArray;

	ULONG_PTR healthComponent;
	float health;
	float maxHealth;
	EPlayerHealthStatus healthStatus;
};

struct GameEntity
{
	EGameActorType type;
	string displayName;
	ULONG_PTR pActor;
	ULONG_PTR rootComponent;
};