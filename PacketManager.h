#pragma once

#include "Packet.h"

#include <unordered_map>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>
#include "UserManager.h"
#include "MonsterManager.h"

class PacketManager
{
private:

	UserManager* mUserManager;
	MonsterManager* mMonsterManager;



	bool mIsRunProcessThread = false;

	std::thread mProcessThread;

	std::mutex mLock;

	std::deque<UINT32> mInComingPacketUserIndex;
	std::deque<PacketInfo> mSystemPacketQueue;

	void ClearConnectionInfo(INT32 clientIndex_);

	void EnqueuePacketData(const UINT32 clientIndex_);

	PacketInfo DequePacketData();
	PacketInfo DequeSystemPacketData();

	void ProcessPacket();
	void ProcessRecvPacket(const UINT32 clientIndex_, const UINT16 packetId_, const UINT16 packetSize_, char* pPacket_);
	void ProcessUserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessUserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void ProcessLogin(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	//void ProcessLoginDBResult(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void ProcessObjectSpawn(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessObjectMovement(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessObjectAttack(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessObjectWeapon(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);

	void ProcessMonsterAggro(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessMonsterMovement(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void ProcessMonsterAggroReset(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_);
	void PatrolWithoutAggroPositionSync(MonsterManager* MonsterPool_, UserManager* UserPool_);
	void PatrolWithoutAggroPositionSynceExcept(MonsterManager* MonsterPool_, UserManager* UserPool_, UINT32 clientIndex_);



public:

	void Init(const UINT32 maxClient_, const UINT32 MonsterNum);
	bool Run();

	void End();
	void ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData_);

	std::function<void(UINT32, UINT32, char*)> SendPacketFunc;

	void PushSystemPacket(PacketInfo packet_);

	

	float Distance_User_Monster(const VECTOR3 user_, const VECTOR3 monster_);



};