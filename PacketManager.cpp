#include <utility>
#include <cstring>

#include "PacketManager.h"
#include "UserManager.h"



void PacketManager::Init(const UINT32 maxClient_, const UINT32 MonsterNum)
{
	mUserManager = new UserManager;
	mUserManager->Init(maxClient_);
	mUserManager->SendPacketFunc = SendPacketFunc;

	mMonsterManager = new MonsterManager;

	int MonsterLvMaxNum = 10;
	int X_pos_limit = 20;
	int Z_pos_limit = 20;


	mMonsterManager->Init(MonsterNum, MonsterLvMaxNum, X_pos_limit, Z_pos_limit);

	PatrolWithoutAggroPositionSync(mMonsterManager,mUserManager);

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

float Distance_User_Monster(const VECTOR3 user_, const VECTOR3 monster_)
{
	return (user_.X - monster_.X) * (user_.X - monster_.X) + (user_.Y - monster_.Y) * (user_.Y - monster_.Y) + (user_.Z - monster_.Z) * (user_.Z - monster_.Z);
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

		else if (packetData.PacketId != 0)
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

	

	switch (packetId_)
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
		ProcessObjectSpawn(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::OBJECT_MOVEMENT_REQUEST:
		ProcessObjectMovement(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::OBJECT_WEAPON_CHANGE_REQUEST:
		ProcessObjectWeapon(clientIndex_, packetSize_, pPacket_);
		break;

	case (int)PACKET_ID::MONSTER_AGGRO_REQUEST:
		ProcessMonsterAggro(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::MONSTER_MOVEMENT_REQUEST:
		ProcessMonsterMovement(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::OBJECT_ATTACK_REQUEST:
		ProcessObjectAttack(clientIndex_, packetSize_, pPacket_);
		break;
	case (int)PACKET_ID::MONSTER_AGGRO_RESET_REQUEST:
		ProcessMonsterAggroReset(clientIndex_, packetSize_, pPacket_);
		break;





	default:
		printf("[WRONG PACKET ID] packedid: %d\n", packetId_);

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

	PatrolWithoutAggroPositionSynceExcept(mMonsterManager, mUserManager, clientIndex_);

}

void PacketManager::ProcessLogin(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (LOGIN_REQUEST_PACKET_SIZE != packetSize_)
	{
		return;
	}

	PatrolWithoutAggroPositionSynceExcept(mMonsterManager, mUserManager, clientIndex_);

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
float PacketManager::Distance_User_Monster(const VECTOR3 user_, const VECTOR3 monster_)
{
	return (user_.X - monster_.X) * (user_.X - monster_.X) + (user_.Y - monster_.Y) * (user_.Y - monster_.Y) + (user_.Z - monster_.Z) * (user_.Z - monster_.Z);

}

void PacketManager::PatrolWithoutAggroPositionSync(MonsterManager* MonsterPool_, UserManager* UserPool_)
{

	if (UserPool_->GetCurrentUserCnt() == 0) return;

	for (int i = 0; i < MonsterPool_->GetMaxMonsterCnt(); i++)
	{

		if (MonsterPool_->GetMonsterPool()[i]->GetMonsterState() == Monster::Monster_State::Live &&
			MonsterPool_->GetMonsterPool()[i]->GetMonType() == MONSTER_TYPE::GOLEM &&
			MonsterPool_->GetMonsterPool()[i]->GetAggroConnUserID().empty())
		{

			double distance = -1;
			int closest_user = -1;

			for (int j = 0; j < UserPool_->GetMaxUserCnt(); j++)
			{
				if (UserPool_->GetUserPool()[j]->GetDomainState() == User::DOMAIN_STATE::LOGIN)
				{
					if (distance < 0)
					{
						distance = Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos());
						closest_user = j;
					}

					else
					{
						if (Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos()) < distance)
						{
							distance = Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos());
							closest_user = j;

						}

					}

				}
			}

			if (distance > 0 && closest_user >= 0)
			{
				//
				USER_PATROL_AI user_patrol_ai;
				user_patrol_ai.Bulk = false;
				user_patrol_ai.PacketId = (UINT16)PACKET_ID::USER_PATROL_AI;
				user_patrol_ai.PacketLength = sizeof(USER_PATROL_AI);
				memcpy(&user_patrol_ai.MonsterID, MonsterPool_->GetMonsterPool()[i]->GetMonsterID().c_str(),sizeof(user_patrol_ai.MonsterID));
			

				SendPacketFunc(closest_user, sizeof(USER_PATROL_AI), (char*)&user_patrol_ai);

				printf("ai sync %d player\n",i);
		

			}







		}



	}




}

void PacketManager::PatrolWithoutAggroPositionSynceExcept(MonsterManager* MonsterPool_, UserManager* UserPool_,UINT32 clientindex_)
{

	if (UserPool_->GetCurrentUserCnt() == 0) return;

	for (int i = 0; i < MonsterPool_->GetMaxMonsterCnt(); i++)
	{

		if (MonsterPool_->GetMonsterPool()[i]->GetMonsterState() == Monster::Monster_State::Live &&
			MonsterPool_->GetMonsterPool()[i]->GetMonType() == MONSTER_TYPE::GOLEM &&
			MonsterPool_->GetMonsterPool()[i]->GetAggroConnUserID().empty())
		{

			double distance = -1;
			int closest_user = -1;

			for (int j = 0; j < UserPool_->GetMaxUserCnt(); j++)
			{
				if (UserPool_->GetUserPool()[j]->GetDomainState() == User::DOMAIN_STATE::LOGIN && j!=clientindex_)
				{
					if (distance < 0)
					{
						distance = Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos());
						closest_user = j;
					}

					else
					{
						if (Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos()) < distance)
						{
							distance = Distance_User_Monster(UserPool_->GetUserPool()[j]->GetUserPos(), MonsterPool_->GetMonsterPool()[i]->GetMonsterPos());
							closest_user = j;

						}

					}

				}
			}

			if (distance > 0 && closest_user >= 0)
			{
				//
				USER_PATROL_AI user_patrol_ai;
				user_patrol_ai.Bulk = false;
				user_patrol_ai.PacketId = (UINT16)PACKET_ID::USER_PATROL_AI;
				user_patrol_ai.PacketLength = sizeof(USER_PATROL_AI);
				memcpy(&user_patrol_ai.MonsterID, MonsterPool_->GetMonsterPool()[i]->GetMonsterID().c_str(), sizeof(user_patrol_ai.MonsterID));


				SendPacketFunc(closest_user, sizeof(USER_PATROL_AI), (char*)&user_patrol_ai);

				printf("ai sync %d player\n", i);
			}







		}



	}




}



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
	spawnbroadPacket.TARGET_WEAPON = pUser->GetUserWeapon();
	//spawnbroadPacket.TARGET_ROTATION = pUser->GetUserRot();

	// broadcast 합시다.

	mUserManager->SendToAllUser((char*)&spawnbroadPacket, (UINT16)sizeof(OBJECT_SPAWN_BROADCAST_PACKET));

	for (int i = 0; i < mUserManager->GetMaxUserCnt(); i++)
	{

		User* IndexUser = mUserManager->GetUserByConnIdx(i);

		if (i == clientIndex_ || 
			  IndexUser->GetDomainState()!=User::DOMAIN_STATE::LOGIN)
		{
			continue;
		}

		OBJECT_SPAWN_BROADCAST_PACKET spawnbroadPacket;
		spawnbroadPacket.PacketId = (UINT16)PACKET_ID::OBJECT_SPAWN_BROADCAST;
		spawnbroadPacket.PacketLength = sizeof(OBJECT_SPAWN_BROADCAST_PACKET);
		strcpy_s(spawnbroadPacket.CharacterID, IndexUser->GetUserId().c_str());
		spawnbroadPacket.TARGET_HP = IndexUser->GetUserHP();
		spawnbroadPacket.TARGET_POSITION = IndexUser->GetUserPos();
		spawnbroadPacket.TARGET_WEAPON = IndexUser->GetUserWeapon();
		SendPacketFunc(clientIndex_, sizeof(OBJECT_SPAWN_BROADCAST_PACKET), (char*)&spawnbroadPacket);

	}



	// monster spawn

	for (int i = 0; i < mMonsterManager->GetMaxMonsterCnt(); i++)
	{
		Monster* monster = mMonsterManager->GetMonsterPool()[i];
		/*
		SendPacketFunc(SendPacketFunc(clientIndex_, sizeof(LOGIN_RESPONSE_PACKET), (char*)&loginResPacket));
			monster 상태 전송
			aggro 중이면 monster aggro도 전송
		*/

		if (monster->GetMonsterState() == Monster::Monster_State::Live)
		{
			MONSTER_SPAWN_BROADCAST_PACKET monster_spawn_broad_pak;
			strcpy_s(monster_spawn_broad_pak.MonsterID, monster->GetMonsterID().c_str());
			monster_spawn_broad_pak.PacketId = (UINT16)PACKET_ID::MONSTER_SPAWN_BROADCAST;
			monster_spawn_broad_pak.PacketLength = (UINT16)sizeof(MONSTER_SPAWN_BROADCAST_PACKET);
			monster_spawn_broad_pak.TARGET_HP = monster->GetMonsterHP();
			monster_spawn_broad_pak.TARGET_LV = monster->GetMonsterLv();
			monster_spawn_broad_pak.TARGET_TYPE = monster->GetMonType();
			monster_spawn_broad_pak.TARGET_POSITION = monster->GetMonsterPos();
			monster_spawn_broad_pak.TARGET_SPAWNPOINT = monster->GetMonsterSpawnPoint();

			if (monster->GetAggroConnUserID().empty())
			{
				SendPacketFunc(clientIndex_, sizeof(MONSTER_SPAWN_BROADCAST_PACKET), (char*)&monster_spawn_broad_pak);
			}

			else
			{
				MONSTER_SPAWN_AGGRO_BROADCAST_PACKET monster_spawn_aggro_broad_pak;
				memcpy(&monster_spawn_aggro_broad_pak, &monster_spawn_broad_pak, sizeof(MONSTER_SPAWN_BROADCAST_PACKET));
				monster_spawn_aggro_broad_pak.PacketId = (UINT16)PACKET_ID::MONSTER_SPAWN_AGGRO_BROADCAST;
				monster_spawn_aggro_broad_pak.PacketLength = (UINT16)sizeof(MONSTER_SPAWN_AGGRO_BROADCAST_PACKET);
				strcpy_s(monster_spawn_aggro_broad_pak.CharacterID, monster->GetAggroConnUserID().c_str());
				SendPacketFunc(clientIndex_, sizeof(MONSTER_SPAWN_AGGRO_BROADCAST_PACKET), (char*)&monster_spawn_aggro_broad_pak);
			}

		}

		else
		{
			continue;
		}

	}




}
void PacketManager::ProcessObjectMovement(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] OBJECT_MOVEMENT_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pMovementPacket = reinterpret_cast<OBJECT_MOVEMENT_REQUEST_PACKET*>(pPacket_);
	auto pUserID = pMovementPacket->CharacterID;

	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);

	if (strcmp(pUserID, pUser->GetUserId().c_str()) != 0)
	{
		printf("[ERROR] 움직이기 위한 Character ID가 다릅니다.\n");
		return;
	}

	OBJECT_MOVEMENT_BROADCAST_PACKET movebroadPacket;

	

	memcpy(&movebroadPacket, pMovementPacket,sizeof(OBJECT_MOVEMENT_REQUEST_PACKET));
	movebroadPacket.PacketId = (UINT16)PACKET_ID::OBJECT_MOVEMENT_BROADCAST;
	movebroadPacket.PacketLength = sizeof(OBJECT_MOVEMENT_BROADCAST_PACKET);

	pUser->SetUserPos(movebroadPacket.TARGET_POSITION);
	pUser->SetUserRot(movebroadPacket.TARGET_ROTATION);

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

	auto pObjectIDAttack = pAttackPacket->ATTACK_CHARACTER_ID;
	auto pObjectIDDamaged = pAttackPacket->DAMAGED_CHARACTER_ID;

	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);

	OBJECT_ATTACK_BROADCAST_PACKET obj_atk_broad_pak;

	memcpy(&obj_atk_broad_pak, pAttackPacket, sizeof(OBJECT_ATTACK_REQUEST_PACKET));
	obj_atk_broad_pak.PacketId = (UINT16)PACKET_ID::OBJECT_ATTACK_BROADCAST;
	obj_atk_broad_pak.PacketLength = sizeof(OBJECT_ATTACK_BROADCAST_PACKET);

	if (strcmp(pObjectIDAttack, pUser->GetUserId().c_str()) == 0)
	{
		auto pMonster = mMonsterManager->FindMonsterIndexByMonsterID(pObjectIDDamaged);
		if (pMonster != mMonsterManager->NullPointermonster)
		{
			INT32 monsterHP = pMonster->GetMonsterHP();

			
			if (monsterHP <= 0)
			{
				//\경험치 안올린다.
				obj_atk_broad_pak.DAMAGED_HP = 0;
			}

			else
			{
				monsterHP -= pAttackPacket->DAMAGE;

				if (monsterHP > 0)
				{
					obj_atk_broad_pak.DAMAGED_HP = monsterHP;
					pMonster->SetMonsterHP(monsterHP);

			

				}

				else
				{
					//player의 경험치를 올린다.
					//exup(lv,player_index)
					obj_atk_broad_pak.DAMAGED_HP = 0;
					pMonster->SetDead(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
				}


			}

			mUserManager->SendToAllUser((char*)&obj_atk_broad_pak, sizeof(OBJECT_ATTACK_BROADCAST_PACKET));

		}

		else

		{
			printf("[ERROR] OBJECT_ATTACK_REQUEST monster ID가 없습니다.\n");
			return;
		}

	}

	else if (strcmp(pObjectIDDamaged, pUser->GetUserId().c_str()) == 0)
	{
		auto pMonster = mMonsterManager->FindMonsterIndexByMonsterID(pObjectIDAttack);
		if (pMonster != nullptr)
		{
			INT32 PlayerHP = pUser->GetUserHP();
			if (PlayerHP <= 0)
			{
				//이미 죽었다.
				obj_atk_broad_pak.DAMAGED_HP = 0;
				//pUser->SetDead => 죽으면 그냥 냅둘까?
			}

			else
			{
				PlayerHP -= pAttackPacket->DAMAGE;

				if (PlayerHP > 0)
				{
					obj_atk_broad_pak.DAMAGED_HP = PlayerHP;
					pUser->SetHP(PlayerHP);

				}

				else
				{
					//player의 경험치를 올린다.
					//exup(lv,player_index)
					obj_atk_broad_pak.DAMAGED_HP = 0;
					//반드시 구현
					//pUser->SetDead(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

				}
			}

			mUserManager->SendToAllUser((char*)&obj_atk_broad_pak, sizeof(OBJECT_ATTACK_BROADCAST_PACKET));
		}

		else

		{
			printf("[ERROR] OBJECT_ATTACK_REQUEST monster ID가 없습니다.\n");
			return;
		}

	}

	else
	{
		printf("[ERROR] OBJECT_ATTACK_REQUEST ID가 없습니다.\n");
		return;

	}

}

void PacketManager::ProcessObjectWeapon(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != OBJECT_WEAPON_CHANGE_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] OBJECT_WEAPON_CHANGE_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;

	}

	auto pWeaponPacket = reinterpret_cast<OBJECT_WEAPON_CHANGE_REQUEST*>(pPacket_);
	auto pUserID = pWeaponPacket->CharacterID;
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);

	if (strcmp(pUserID, pUser->GetUserId().c_str()) != 0)
	{
		printf("[ERROR] MONSTER AGGOR player ID가 다릅니다.\n");
		return;
	}

	pUser->SetUserWeapon(pWeaponPacket->weapon);
	
	OBJECT_WEAPON_CHANGE_BROADCAST obj_wep_broad;
	memcpy(&obj_wep_broad, pWeaponPacket, sizeof(OBJECT_WEAPON_CHANGE_REQUEST));
	obj_wep_broad.PacketId= (UINT16)PACKET_ID::OBJECT_WEAPON_CHANGE_BROADCAST;
	obj_wep_broad.PacketLength = sizeof(OBJECT_WEAPON_CHANGE_BROADCAST);
	
	mUserManager->SendToAllUserExceptMe((char*)&obj_wep_broad, sizeof(OBJECT_WEAPON_CHANGE_BROADCAST), clientIndex_);

}

void PacketManager::ProcessMonsterAggro(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != MONSTER_AGGRO_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] MONSTER_AGGRO_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pAggroPacket = reinterpret_cast<MONSTER_AGGRO_REQUEST_PACKET*>(pPacket_);
	auto pUserID = pAggroPacket->CharacterID;
	auto pMonsterID = pAggroPacket->MonsterID;
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex_);
	auto pMonster = mMonsterManager->FindMonsterIndexByMonsterID(pMonsterID);

	if (strcmp(pUserID, pUser->GetUserId().c_str()) != 0)
	{
		printf("[ERROR] MONSTER AGGOR player ID가 다릅니다.\n");
		return;
	}

	if (pMonster == nullptr || pMonster == mMonsterManager->NullPointermonster)
	{
		printf("[ERROR] MONSTER AGGRO MONSTER ID가 없습니다");
		return;
	}

	pMonster->SetAggro_By_Player(clientIndex_, pUserID);

	MONSTER_AGGRO_BROADCAST_PACKET pMonster_Aggro_broadcast_packet;
	pMonster_Aggro_broadcast_packet.PacketId = (UINT16)PACKET_ID::MONSTER_AGGRO_BROADCAST;
	pMonster_Aggro_broadcast_packet.PacketLength = sizeof(MONSTER_AGGRO_BROADCAST_PACKET);
	strcpy_s(pMonster_Aggro_broadcast_packet.CharacterID, pUserID);
	strcpy_s(pMonster_Aggro_broadcast_packet.MonsterID, pMonsterID);

	mUserManager->SendToAllUserExceptMe((char*)&pMonster_Aggro_broadcast_packet, sizeof(MONSTER_AGGRO_BROADCAST_PACKET), clientIndex_);

}

void PacketManager::ProcessMonsterMovement(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != MONSTER_MOVEMENT_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] MONSTER_MOVEMENT_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pMonsterMovePacket = reinterpret_cast<MONSTER_MOVEMENT_REQUEST_PACKET*>(pPacket_);
	auto pMonsterID = pMonsterMovePacket->MonsterID;
	auto pMonster = mMonsterManager->FindMonsterIndexByMonsterID(pMonsterID);


	if (pMonster == mMonsterManager->NullPointermonster)
	{
		printf("[ERROR] MONSTER Movement MONSTER ID가 다릅니다");
		return;
	}

	pMonster->SetMonPos(pMonsterMovePacket->TARGET_POSITION);
	pMonster->SetMonRot(pMonsterMovePacket->TARGET_ROTATION);
	//pMonster->SetMonsterHP(pMonsterMovePacket->TARGET_HP);

	MONSTER_MOVEMENT_BROADCAST_PACKET pMonster_movement_broadcast_packet;
	memcpy(&pMonster_movement_broadcast_packet, pMonsterMovePacket, MONSTER_MOVEMENT_REQUEST_PACKET_LENGTH);
	pMonster_movement_broadcast_packet.PacketId = (UINT16)PACKET_ID::MONSTER_MOVEMENT_BROADCAST;
	pMonster_movement_broadcast_packet.PacketLength = sizeof(MONSTER_MOVEMENT_BROADCAST_PACKET);
	mUserManager->SendToAllUserExceptMe((char*)&pMonster_movement_broadcast_packet, sizeof(MONSTER_MOVEMENT_BROADCAST_PACKET), clientIndex_);


}

void PacketManager::ProcessMonsterAggroReset(UINT32 clientIndex_, UINT16 packetSize_, char* pPacket_)
{
	if (packetSize_ != MONSTER_AGGRO_RESET_REQUEST_PACKET_LENGTH)
	{
		printf("[ERROR] MONSTER_AGGRO_RESET_REQUEST_PACKET_LENGTH DIFFERENT.\n");
		return;
	}

	auto pMonsterAggroResetPacket = reinterpret_cast<MONSTER_AGGRO_RESET_REQUEST_PACKET*>(pPacket_);
	auto pMonsterID = pMonsterAggroResetPacket->MonsterID;
	auto pMonster = mMonsterManager->FindMonsterIndexByMonsterID(pMonsterID);

	if (pMonster == mMonsterManager->NullPointermonster)
	{
		printf("[ERROR] MONSTER Aggro Reset MONSTER ID가 다릅니다");
		return;
	}

	pMonster->Disattract();

	MONSTER_AGGRO_RESET_BROADCAST_PACKET pMonsterAggroResetbroadcastPacket;
	memcpy(&pMonsterAggroResetbroadcastPacket, pMonsterAggroResetPacket, MONSTER_AGGRO_RESET_REQUEST_PACKET_LENGTH);
	pMonsterAggroResetbroadcastPacket.PacketId = (UINT16)PACKET_ID::MONSTER_AGGRO_RESET_BROADCAST;
	pMonsterAggroResetbroadcastPacket.PacketLength = sizeof(MONSTER_AGGRO_RESET_BROADCAST_PACKET);
	mUserManager->SendToAllUserExceptMe((char*)&pMonsterAggroResetbroadcastPacket, sizeof(MONSTER_AGGRO_RESET_BROADCAST_PACKET), clientIndex_);

	printf("몬스터 어그로 초기화\n");

	PatrolWithoutAggroPositionSync(mMonsterManager, mUserManager); // 패트롤 설정.

}


