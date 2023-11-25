#include <utility>
#include <cstring>

#include "PacketManager.h"
#include "UserManager.h"



void PacketManager::Init(const UINT32 maxClient_)
{
	mUserManager = new UserManager;
	mUserManager->Init(maxClient_);
	mUserManager->SendPacketFunc = SendPacketFunc;

}


bool PacketManager::Run()
{

	//이 부분을 패킷 처리 부분으로 이동 시킨다.
	mIsRunProcessThread = true;
	mProcessThread = std::thread([this]() { ProcessPacket(); });

	return true;
}

void PacketManager::End()
{

	mIsRunProcessThread = false;

	if (mProcessThread.joinable())
	{
		mProcessThread.join();
	}
}

void PacketManager::PushSystemPacket(PacketInfo packet_)
{
	std::lock_guard<std::mutex> guard(mLock);
	mSystemPacketQueue.push_back(packet_);
}

PacketInfo PacketManager::DequeSystemPacketData()
{

	std::lock_guard<std::mutex> guard(mLock);
	if (mSystemPacketQueue.empty())
	{
		return PacketInfo();
	}

	auto packetData = mSystemPacketQueue.front();
	mSystemPacketQueue.pop_front();

	return packetData;
}

void PacketManager::ReceivePacketData(const UINT32 clientIndex_, const UINT32 size_, char* pData_)
{
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	pUser->SetPacketData(size_, pData_);

	EnqueuePacketData(clientIndex_);
}


void PacketManager::ClearConnectionInfo(INT32 clientIndex_)
{
	
	User* Clear_Requested_User = mUserManager->GetUserByConnIdx(clientIndex_);
	
	if (Clear_Requested_User->GetDomainState() != User::DOMAIN_STATE::NONE)
	{
		mUserManager->DeleteUserInfo(Clear_Requested_User);
	}

}


void PacketManager::EnqueuePacketData(const UINT32 clientIndex_)
{
	std::lock_guard<std::mutex> guard(mLock);
	mInComingPacketUserIndex.push_back(clientIndex_);
}


PacketInfo PacketManager::DequePacketData()
{
	UINT32 userIndex = 0;

	std::lock_guard<std::mutex> guard(mLock);
	if (mInComingPacketUserIndex.empty())
	{
		return PacketInfo();
	}

	userIndex = mInComingPacketUserIndex.front();
	mInComingPacketUserIndex.pop_front();
	
	User* pUser = mUserManager->GetUserByConnIdx(userIndex);
	PacketInfo packetData = pUser->GetPacket();
	packetData.ClientIndex = userIndex;
	return packetData;

}

void PacketManager::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		bool isIdle = true;

		PacketInfo packetData = DequePacketData();
		PacketInfo sys_packetData = DequeSystemPacketData();

		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		if (packetData.PacketId != 0)
		{
			isIdle = false;
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}

		if (isIdle)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

	}


}

void PacketManager::ProcessRecvPacket(const UINT32 clientIndex_, const UINT16 packetId_, const UINT16 packetSize_, char* pPacket_)
{

	

	switch (clientIndex_)
	{

	case (int)PACKET_ID::USER_CONNECT:
		ProcessUserConnect(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::USER_DISCONNECT:
		ProcessUserDisConnect(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::LOGIN_REQUEST:
		ProcessLogin(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::OBJECT_SPAWN_REQUEST:
		// spawn
		break;
	case (int)PACKET_ID::OBJECT_MOVEMENT_REQUEST:
		//movement
		break;
	case (int)PACKET_ID::OBJECT_ATTACK_REQUEST:
		//ATTACK
		break;

	default:
		printf("[WRONG PACKET ID] packedid: %d\n", clientIndex_);

	}
	
	

}


void PacketManager::ProcessUserConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	printf("[ProcessUserConnect] clientIndex: %d\n", clientIndex_);
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	pUser->Clear();
}

void PacketManager::ProcessUserDisConnect(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	printf("[ProcessUserDisConnect] clientIndex: %d\n", clientIndex_);
	ClearConnectionInfo(clientIndex_);
}

void PacketManager::ProcessLogin(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_)
	{
		return;
	}

	auto pLoginReqPacket = reinterpret_cast<LOGIN_REQUEST_PACKET*>(pPacket_);

	auto pUserID = pLoginReqPacket->UserID;
	printf("requested user id = %s\n", pUserID);

	LOGIN_RESPONSE_PACKET loginResPacket;
	loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
	loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);

	if (mUserManager->GetCurrentUserCnt() >= mUserManager->GetMaxUserCnt())
	{
		//접속자수가 최대수를 차지해서 접속불가
		loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USERPOOL_FULL;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
		return;
	}

	//여기에서 이미 접속된 유저인지 확인하고, 접속된 유저라면 실패한다.
	if (mUserManager->FindUserIndexByID(pUserID) == -1)
	{

		mUserManager->AddUser(pLoginReqPacket->UserID, pLoginReqPacket->UserPW, clientIndex_);

		/*
		RedisLoginReq dbReq;
		CopyMemory(dbReq.UserID, pLoginReqPacket->UserID, (MAX_USER_ID_LEN + 1));
		CopyMemory(dbReq.UserPW, pLoginReqPacket->UserPW, (MAX_USER_PW_LEN + 1));

		RedisTask task;
		task.UserIndex = clientIndex_;
		task.TaskID = RedisTaskID::REQUEST_LOGIN;
		task.DataSize = sizeof(RedisLoginReq);
		task.pData = new char[task.DataSize];
		CopyMemory(task.pData, (char*)&dbReq, task.DataSize);
		mRedisMgr->PushTask(task);
		*/

		loginResPacket.Result = (UINT16)ERROR_CODE::NONE;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);

		printf("Login User Count = %d\n", mUserManager->GetCurrentUserCnt());
	}
	else
	{
		//접속중인 유저여서 실패를 반환한다.
		loginResPacket.Result = (UINT16)ERROR_CODE::LOGIN_USER_ALREADY;
		SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
		return;
	}
}



/*

void PacketManager::ProcessLoginDBResult(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	printf("ProcessLoginDBResult. UserIndex: %d\n", clientIndex_);

	auto pBody = (RedisLoginRes*)pPacket_;

	if (pBody->Result == (UINT16)ERROR_CODE::NONE)
	{
		//로그인 완료로 변경한다
	}

	LOGIN_RESPONSE_PACKET loginResPacket;
	loginResPacket.PacketId = (UINT16)PACKET_ID::LOGIN_RESPONSE;
	loginResPacket.PacketLength = sizeof(LOGIN_RESPONSE_PACKET);
	loginResPacket.Result = pBody->Result;
	SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket);
}

*/


void PacketManager::ProcessObjectSpawn(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	//일단 캐릭터랑 ID랑 같다고 가정.

	if (packetSize_ != OBJECT_SPAWN_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] OBJECT_SPAWN_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pSpawnPacket = reinterpret_cast<OBJECT_SPAWN_REQUEST_PACKET*>(pPacket_);
	auto pUserID = pSpawnPacket->CharacterID;
	
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);

	if (strcmp(pUserID, pUser->GetUserId().c_str()) != 0)
	{
		printf("[ERROR] SPAWN 하기 위한 Character ID가 다릅니다.\n");
		return;
	}

	pUser->Respawn();
	OBJECT_SPAWN_BROADCAST_PACKET spawnbroadPacket;
	spawnbroadPacket.PacketId = (UINT16)PACKET_ID::OBJECT_SPAWN_BROADCAST;
	spawnbroadPacket.PacketLength = sizeof(OBJECT_SPAWN_BROADCAST_PACKET);
	strcpy_s(spawnbroadPacket.CharacterID, pUserID);
	spawnbroadPacket.TARGET_HP = pUser->GetUserHP();
	spawnbroadPacket.TARGET_POSITION = pUser->GetUserPos();
	spawnbroadPacket.TARGET_ROTATION = pUser->GetUserRot();

	// broadcast 합시다.

	mUserManager->SendToAllUser((char*)&spawnbroadPacket, (UINT16)sizeof(OBJECT_SPAWN_BROADCAST_PACKET));


}
void PacketManager::ProcessObjectMovement(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pMovementPacket = reinterpret_cast<OBJECT_SPAWN_REQUEST_PACKET*>(pPacket_);
	auto pUserID = pMovementPacket->CharacterID;

	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);

	if (strcmp(pUserID, pUser->GetUserId().c_str()) != 0)
	{
		printf("[ERROR] 움직이기 위한 Character ID가 다릅니다.\n");
		return;
	}

	OBJECT_MOVEMENT_BROADCAST_PACKET movebroadPacket;
	memcpy(&movebroadPacket, pMovementPacket,sizeof(OBJECT_SPAWN_REQUEST_PACKET));
	movebroadPacket.PacketId = (UINT16)PACKET_ID::OBJECT_MOVEMENT_BROADCAST;
	movebroadPacket.PacketLength = sizeof(OBJECT_MOVEMENT_BROADCAST_PACKET);

	mUserManager->SendToAllUserExceptMe((char*)&movebroadPacket, (UINT16)sizeof(OBJECT_MOVEMENT_BROADCAST_PACKET), clientIndex_);

}
void PacketManager::ProcessObjectAttack(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != OBJECT_ATTACK_REQUEST_LENGTH)
	{
		printf("[ERROR] OBJECT_ATTACK_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pAttackPacket = reinterpret_cast<OBJECT_ATTACK_REQUEST_PACKET*>(pPacket_);
	
	auto pUserIDAttack = pAttackPacket->ATTACK_CHARACTER_ID;
	auto pUserIDDamaged = pAttackPacket->DAMAGED_CHARACTER_ID;

	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);


	OBJECT_ATTACK_BROADCAST_PACKET attackbroadPacket;

	memcpy(&attackbroadPacket, pAttackPacket, sizeof(OBJECT_ATTACK_REQUEST_PACKET));

	attackbroadPacket.PacketId = (UINT16)PACKET_ID::OBJECT_ATTACK_BROADCAST;
	attackbroadPacket.PacketLength = sizeof(OBJECT_ATTACK_BROADCAST_PACKET);
//	attackbroadPacket.DAMAGED_HP
	

	if (strcmp(pUserIDAttack, pUser->GetUserId().c_str()) == 0)
	{
		// player가 monster attack

		//pUser -> deal(monster* mon, pAttackPacket->DAMAGE)
		//죽엇나 살았나 죽으면 lvlup 
		//monster 상태 update 하고 broadcast 


	}

	else if (strcmp(pUserIDDamaged, pUser->GetUserId().c_str()) == 0)
	{
		//monster가 player attack
	}
	
	else
	{
		printf("[ERROR] 공격하거나 받는 캐릭터 ID가 다릅니다.\n");
		return;
	}



	mUserManager->SendToAllUserExceptMe((char*)&attackbroadPacket, (UINT16)sizeof(OBJECT_ATTACK_BROADCAST_PACKET), clientIndex_);


}
void PacketManager::ProcessObjectDamaged(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{

}