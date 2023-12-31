#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct PacketData
{
	UINT32 SessionIndex = 0;
	UINT32 DataSize = 0;
	char* pPacketData = nullptr;

	void Set(PacketData& vlaue)
	{
		SessionIndex = vlaue.SessionIndex;
		DataSize = vlaue.DataSize;

		pPacketData = new char[vlaue.DataSize];
		CopyMemory(pPacketData, vlaue.pPacketData, vlaue.DataSize);
	}

	void Set(UINT32 sessionIndex_, UINT32 dataSize_, char* pData)
	{
		SessionIndex = sessionIndex_;
		DataSize = dataSize_;

		pPacketData = new char[dataSize_];
		CopyMemory(pPacketData, pData, dataSize_);
	}

	void Release()
	{
		delete pPacketData;
	}


};

enum class  PACKET_ID : UINT16
{

	USER_CONNECT = 11,
	USER_DISCONNECT = 12,

	USER_PATROL_AI = 21,
	USER_PATROL_AI_RESPONSE = 22,

	SYS_END = 30,



	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	OBJECT_SPAWN_REQUEST = 301,
	OBJECT_SPAWN_BROADCAST = 302,
	OBJECT_MOVEMENT_REQUEST = 303,
	OBJECT_MOVEMENT_BROADCAST = 304,
	OBJECT_WEAPON_CHANGE_REQUEST = 305,
	OBJECT_WEAPON_CHANGE_BROADCAST = 306,

	MONSTER_SPAWN_BROADCAST = 402,
	MONSTER_AGGRO_REQUEST = 403,
	MONSTER_AGGRO_BROADCAST = 404,
	MONSTER_AGGRO_RESET_REQUEST = 405,
	MONSTER_AGGRO_RESET_BROADCAST = 406,

	MONSTER_SPAWN_AGGRO_BROADCAST = 408,
	

	MONSTER_MOVEMENT_REQUEST = 409,
	MONSTER_MOVEMENT_BROADCAST = 410,

	OBJECT_ATTACK_REQUEST = 501,
	OBJECT_ATTACK_BROADCAST = 502,

};

struct VECTOR3
{
	FLOAT X;
	FLOAT Y;
	FLOAT Z;
};

enum class WEAPON : UINT16
{
	nothing = 1,
	twohandedWep = 2,
	Unarmed = 3,
	oneHandwep = 4,
	dagger = 5,
	staff = 6,
};

enum class MONSTER_TYPE : UINT16
{
	NONE = 0,
	GOBLIN = 1,
	GOLEM = 2,
};

struct PacketInfo
{
	UINT32 ClientIndex = 0;
	UINT16 PacketId = 0;
	UINT16 DataSize = 0;
	char* pDataPtr = nullptr;
};

#pragma pack(push,1)
struct PACKET_HEADER
{
	UINT16 PacketLength;
	UINT16 PacketId;
	bool Bulk;
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);

//- 로그인 요청
const int MAX_USER_ID_LEN = 12;
const int MAX_USER_PW_LEN = 12;
// 캐릭터 ID
const int MAX_CHARACTER_ID_LEN = 12;
// 몬스터 ID
const int MAX_MONSTER_ID_LEN = 12;

struct LOGIN_REQUEST_PACKET : public PACKET_HEADER
{
	char UserID[MAX_USER_ID_LEN + 1];
	char UserPW[MAX_USER_PW_LEN + 1];
};
const size_t LOGIN_REQUEST_PACKET_SIZE = sizeof(LOGIN_REQUEST_PACKET);


struct LOGIN_RESPONSE_PACKET : public PACKET_HEADER
{
	UINT16 Result;
};

struct USER_PATROL_AI : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
};

struct USER_PATROL_AI_RESPONSE : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];

	VECTOR3 TARGET_POSITION;

	VECTOR3 TARGET_ROTATION;

	UINT16 WAYPOINT;
};

const UINT32 USER_PATROL_AI_RESPONSE_PACKET_LENGTH = sizeof(USER_PATROL_AI_RESPONSE);

struct OBJECT_SPAWN_REQUEST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];

};

const UINT32 OBJECT_SPAWN_REQUEST_PACKET_LENGTH = sizeof(OBJECT_SPAWN_REQUEST_PACKET);

struct OBJECT_SPAWN_BROADCAST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	UINT32 TARGET_HP;
	VECTOR3 TARGET_POSITION;
	WEAPON TARGET_WEAPON; 
};

struct OBJECT_MOVEMENT_REQUEST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	VECTOR3 TARGET_POSITION;

	VECTOR3 TARGET_ROTATION;
	INT16 TARGET_ANIMATION_X;
	INT16 TARGET_ANIMATION_Y;
};

const UINT32 OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH = sizeof(OBJECT_MOVEMENT_REQUEST_PACKET);

struct OBJECT_MOVEMENT_BROADCAST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	VECTOR3 TARGET_POSITION;

	VECTOR3 TARGET_ROTATION;
	INT16 TARGET_ANIMATION_X;
	INT16 TARGET_ANIMATION_Y;
};

struct OBJECT_ATTACK_REQUEST_PACKET : public PACKET_HEADER
{
	char ATTACK_CHARACTER_ID[MAX_CHARACTER_ID_LEN + 1];
	char DAMAGED_CHARACTER_ID[MAX_CHARACTER_ID_LEN + 1];
	UINT16 DAMAGE;
};

const UINT32 OBJECT_ATTACK_REQUEST_LENGTH = sizeof(OBJECT_ATTACK_REQUEST_PACKET);

struct OBJECT_ATTACK_BROADCAST_PACKET : public OBJECT_ATTACK_REQUEST_PACKET
{
	UINT32 DAMAGED_HP;
};

struct OBJECT_WEAPON_CHANGE_REQUEST : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	WEAPON weapon;
};

const UINT32 OBJECT_WEAPON_CHANGE_REQUEST_PACKET_LENGTH = sizeof(OBJECT_WEAPON_CHANGE_REQUEST);

struct OBJECT_WEAPON_CHANGE_BROADCAST : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	WEAPON weapon;
};

struct MONSTER_SPAWN_BROADCAST_PACKET : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
	UINT32 TARGET_HP;
	UINT16 TARGET_LV;
	VECTOR3 TARGET_POSITION;
	VECTOR3 TARGET_SPAWNPOINT;
	MONSTER_TYPE TARGET_TYPE;
	UINT16 WAYPOINT;

};

struct MONSTER_SPAWN_AGGRO_BROADCAST_PACKET : public MONSTER_SPAWN_BROADCAST_PACKET
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
};

struct MONSTER_AGGRO_REQUEST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
};

const UINT32 MONSTER_AGGRO_REQUEST_PACKET_LENGTH = sizeof(MONSTER_AGGRO_REQUEST_PACKET);

struct MONSTER_AGGRO_RESET_REQUEST_PACKET : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
};

const UINT32 MONSTER_AGGRO_RESET_REQUEST_PACKET_LENGTH = sizeof(MONSTER_AGGRO_RESET_REQUEST_PACKET);

struct MONSTER_AGGRO_RESET_BROADCAST_PACKET : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
};


struct MONSTER_AGGRO_BROADCAST_PACKET : public PACKET_HEADER
{
	char CharacterID[MAX_CHARACTER_ID_LEN + 1];
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
};

struct MONSTER_MOVEMENT_REQUEST_PACKET : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
	VECTOR3 TARGET_POSITION;
	VECTOR3 TARGET_ROTATION;
	UINT16 ANIMATION;
};

const UINT32 MONSTER_MOVEMENT_REQUEST_PACKET_LENGTH = sizeof(MONSTER_MOVEMENT_REQUEST_PACKET);

struct MONSTER_MOVEMENT_BROADCAST_PACKET : public PACKET_HEADER
{
	char MonsterID[MAX_MONSTER_ID_LEN + 1];
	VECTOR3 TARGET_POSITION;
	VECTOR3 TARGET_ROTATION;
	UINT16 ANIMATION;


};




#pragma pack(pop) 