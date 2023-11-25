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
	SYS_END = 30,

	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	OBJECT_SPAWN_REQUEST = 301,
	OBJECT_SPAWN_BROADCAST = 302,
	OBJECT_MOVEMENT_REQUEST =303,
	OBJECT_MOVEMENT_BROADCAST = 304,
	OBJECT_ATTACK_REQUEST = 305,
	OBJECT_ATTACK_BROADCAST = 306,

};

struct VECTOR3
{
	FLOAT X;
	FLOAT Y;
	FLOAT Z;

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
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);

//- 로그인 요청
const int MAX_USER_ID_LEN = 32;
const int MAX_USER_PW_LEN = 32;
// 캐릭터 ID
const int MAX_CHARACTER_ID_LEN = 32;

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
	VECTOR3 TARGET_ROTATION;
};

struct OBJECT_MOVEMENT_REQUEST_PACKET : public OBJECT_SPAWN_BROADCAST_PACKET
{
	INT16 TARGET_ANIMATION_X;
	INT16 TARGET_ANIMATION_Y;
};

const UINT32 OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH = sizeof(OBJECT_MOVEMENT_REQUEST_PACKET);

struct OBJECT_MOVEMENT_BROADCAST_PACKET : public OBJECT_SPAWN_BROADCAST_PACKET
{
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

#pragma pack(pop) 