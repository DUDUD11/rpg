#pragma once
#include <unordered_map>

#include "ErrorCode.h"
#include "Monster.h"
#include <cstdlib>
#include <ctime>

class MonsterManager
{
private:
	INT32 mMaxMonsterCnt = 0;
	INT32 mCurrentMonsterCnt = 0;
	INT32 mMonster_Default_Height = 2;
	INT32 MOSTER_AUTO_RESAPWN_TIME = 5;
	INT32 mMaxLevel = 0;
	INT32 MAX_X_POS = 0;
	INT32 MAX_Z_POS = 0;



	std::vector<Monster*> mMonsterObjPool;





public:
	MonsterManager() = default;
	~MonsterManager() = default;

	Monster* NullPointermonster = new Monster;

	void Init(const INT32 mMaxMonsterCnt_, const INT32 mMaxLevel_, const INT32 MAX_X_POS_, const INT32 MAX_Z_POS_)
	{
		mMaxMonsterCnt = mMaxMonsterCnt_;
		mMonsterObjPool = std::vector<Monster*>(mMaxMonsterCnt_);
		mMaxLevel = mMaxLevel_;
		MAX_X_POS = MAX_X_POS_;
		MAX_Z_POS = MAX_Z_POS_;

		int coordinate_x[] = { -1,1,0,0 };
		int coordinate_z[] = { 0,0,-1,1 };


		for (int i = 0; i < mMaxMonsterCnt; i++)
		{
	


			int lv = i % mMaxLevel;
			float Pos_x = (1+i/4) % MAX_X_POS;
			float Pos_y = mMonster_Default_Height;
			float Pos_z = (1+i/4) % MAX_Z_POS;

			int random = std::rand() % 4;
			Pos_x *= coordinate_x[random];
			Pos_z *= coordinate_z[random];

			printf("lv %d\n", lv);

			mMonsterObjPool[i] = new Monster();
			mMonsterObjPool[i]->Respawn(VECTOR3({ Pos_x, Pos_y, Pos_z }), lv, "Monster " + std::to_string(i), (MONSTER_TYPE)(i%2+1));

		}

	}

	std::vector<Monster*> GetMonsterPool()
	{
		return mMonsterObjPool;
	}

	void AutoRespawnMonster(UINT64 mLatestDeadTimeSec_)
	{
		for (int i = 0; i < mMaxMonsterCnt; i++)
		{
			if (mMonsterObjPool[i]->GetMonsterState() == Monster::Monster_State::Dead
				&& mLatestDeadTimeSec_ - mMonsterObjPool[i]->GetLatestDeadTimeSec() > MOSTER_AUTO_RESAPWN_TIME)
			{
				srand(static_cast<unsigned int>(time(NULL)));


				int lv = std::rand() % mMaxLevel;
				float Pos_x = std::rand() % MAX_X_POS;
				float Pos_y = mMonster_Default_Height;
				float Pos_z = std::rand() % MAX_Z_POS;

				mMonsterObjPool[i]->Respawn(VECTOR3({ Pos_x, Pos_y, Pos_z }), lv, "Monster " + i,mMonsterObjPool[i]->GetMonType());

			}

		}

	}

	INT32 GetCurrentmMonsterCnt() { return mCurrentMonsterCnt; }

	INT32 GetMaxMonsterCnt() { return mMaxMonsterCnt; }


	ERROR_CODE AggroUser(char* userID_, int clientIndex_, Monster* pMonster)
	{


		int user_idx = clientIndex_;

		return pMonster->SetAggro_By_Player(clientIndex_, userID_);

	}

	Monster* FindMonsterIndexByMonsterID(char* userID_)
	{
		for (int i = 0; i < mMaxMonsterCnt; i++)
		{
			if (strcmp(userID_, mMonsterObjPool[i]->GetMonsterID().c_str()) == 0)
			{
				return mMonsterObjPool[i];
			}
		}

		return NullPointermonster;

	}





};