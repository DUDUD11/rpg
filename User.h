#pragma once
#include <string>

#include "Packet.h"

class User
{
	const UINT32 PACKET_DATA_BUFFER_SIZE = 65535;

	//체력
	//좌표 각도와 - 위치, motion for animation 
	//공격



public:

	User() = default;
	~User() = default;

	enum class DOMAIN_STATE
	{
		NONE = 0,
		LOGIN = 1
	};

private:
	INT32 mIndex = -1;
	std::string mUserID;
	bool mIsConfirm = false;

	DOMAIN_STATE mCurDomainState = DOMAIN_STATE::NONE;

	UINT32 mPakcetDataBufferWPos = 0;
	UINT32 mPakcetDataBufferRPos = 0;
	char* mPacketDataBuffer = nullptr;

	INT32 USER_HP = 0;
	INT32 USER_DEALING_DAMAGE = 0;
	VECTOR3 USER_POS = { 0, };
	VECTOR3 USER_ROT = { 0, };

	WEAPON USER_WEAPON=WEAPON::nothing;



public:


	void Init(const INT32 index)
	{
		mIndex = index;
		mPacketDataBuffer = new char[PACKET_DATA_BUFFER_SIZE];

		INT32 USER_HP = 0;
		INT32 USER_DEALING_DAMAGE = 0;
		USER_POS = { 0, };
		USER_ROT = { 0, };
		USER_WEAPON = WEAPON::nothing;



	}

	void Clear()
	{
		mUserID = "";
		mIsConfirm = false;
		mCurDomainState = DOMAIN_STATE::NONE;

		mPakcetDataBufferWPos = 0;
		mPakcetDataBufferRPos = 0;

		INT32 USER_HP = 0;
		INT32 USER_DEALING_DAMAGE = 0;
		USER_POS = { 0, };
		USER_ROT = { 0, };

		USER_WEAPON = WEAPON::nothing;

	}

	int SetLogin(char* userID_)
	{
		mCurDomainState = DOMAIN_STATE::LOGIN;
		mUserID = userID_;

		return 0;
	}

	void SetHP(INT32 UserHP_)
	{
		USER_HP = UserHP_;
	}



	void SetDomainState(DOMAIN_STATE value_)
	{
		mCurDomainState = value_;
	}

	INT32 GetNetConnIdx()
	{
		return mIndex;
	}

	INT32 GetUserHP()
	{
		return USER_HP;
	}

	WEAPON GetUserWeapon()
	{
		return USER_WEAPON;
	}

	void SetUserWeapon(WEAPON weapon_)
	{
		USER_WEAPON = weapon_;
	}




	void Respawn()
	{
		USER_HP = 30;
		USER_DEALING_DAMAGE = 3;
		USER_POS.X = 5.0f * mIndex;
		USER_POS.Y = 2.0f;
		USER_POS.Z = 5.0f * mIndex;
		USER_WEAPON = WEAPON::Unarmed;
	}

	void SetUserPos(VECTOR3 vector3_)
	{
		USER_POS = vector3_;
	}

	void SetUserRot(VECTOR3 vector3_)
	{
		USER_ROT = vector3_;
	}

	VECTOR3 GetUserPos()
	{
		return USER_POS;
	}

	VECTOR3 GetUserRot()
	{
		return USER_ROT;
	}


	std::string GetUserId() const
	{
		return  mUserID;
	}

	DOMAIN_STATE GetDomainState()
	{
		return mCurDomainState;
	}

	//TODO SetPacketData, GetPacket 함수를 멀티스레드에 호출하고 있다면 공유변수에 lock을 걸어야 한다
	void SetPacketData(const UINT32 dataSize_, char* pData_)
	{
		//오류수정

	

		if ((mPakcetDataBufferWPos + dataSize_) >= PACKET_DATA_BUFFER_SIZE)
		{
			auto remainDataSize = mPakcetDataBufferWPos - mPakcetDataBufferRPos;

			if (remainDataSize > 0)
			{
				CopyMemory(&mPacketDataBuffer[0], &mPacketDataBuffer[mPakcetDataBufferRPos], remainDataSize);
				mPakcetDataBufferWPos = remainDataSize;
			}
			else
			{
				mPakcetDataBufferWPos = 0;
			}

			mPakcetDataBufferRPos = 0;
		}

		CopyMemory(&mPacketDataBuffer[mPakcetDataBufferWPos], pData_, dataSize_);
		mPakcetDataBufferWPos += dataSize_;
	}

	PacketInfo GetPacket()
	{
		const int PACKET_SIZE_LENGTH = 2;
		const int PACKET_TYPE_LENGTH = 2;
		short packetSize = 0;

		UINT32 remainByte = mPakcetDataBufferWPos - mPakcetDataBufferRPos;

		if (remainByte < PACKET_HEADER_LENGTH)
		{
			return PacketInfo();
		}

		auto pHeader = (PACKET_HEADER*)&mPacketDataBuffer[mPakcetDataBufferRPos];
		if (pHeader->PacketLength > remainByte)
		{
			return PacketInfo();
		}

		PacketInfo packetInfo;
		packetInfo.PacketId = pHeader->PacketId;
		packetInfo.DataSize = pHeader->PacketLength;
		packetInfo.pDataPtr = &mPacketDataBuffer[mPakcetDataBufferRPos];

		mPakcetDataBufferRPos += pHeader->PacketLength;

		return packetInfo;
	}



};