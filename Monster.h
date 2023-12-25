#pragma once
#include <string>

#include "Packet.h"

class Monster
{

	//체력
	//좌표 각도와 - 위치, motion for animation 
	//공격

public:

	Monster() = default;
	~Monster() = default;

	enum class Monster_State
	{
		None = 0,
		Dead = 1,
		Live = 2,
	};

private:
	INT32 mIndex = -1; // User에게 부착되어있는가
	std::string mUserID; //UserID
	std::string mMonsterID; //monster 이름
	//bool mIsConfirm = false;

	Monster_State mCurMonsterState = Monster_State::None;
	MONSTER_TYPE mMonsterType = MONSTER_TYPE::NONE;

	UINT16 Monster_Lv=0;
	INT32 Monster_HP=0;
	INT32 Monster_DAMAGE = 5;
	VECTOR3 Monster_Spawn_Point = { 0, };
	VECTOR3 Monster_POS = { 0, };
	VECTOR3 Monster_ROT = { 0, };
	UINT64 mLatestDeadTimeSec = 0;


public:


	void Init_monster_lvl(const INT32 Lv)
	{
		
		for (int i = Monster_Lv; i < Lv; i++)
		{
			Monster_HP *= 3;
			Monster_HP /= 2;

			Monster_DAMAGE *= 3;
			Monster_DAMAGE /= 2;

		}
		
		Monster_Lv = Lv;

	}

	void Init_Spawn(const VECTOR3 vector3, const INT32 Lv, std::string monsterID, MONSTER_TYPE monstertype_)
	{
		Monster_Spawn_Point = vector3;
		//printf("%f %f %f\n", Monster_Spawn_Point.X, Monster_Spawn_Point.Y, Monster_Spawn_Point.Z);
		Monster_HP = 100;
		Monster_POS = vector3;
		Init_monster_lvl(Lv);
		mCurMonsterState = Monster_State::Live;
		SetMonsterID(monsterID);
		SetMonType(monstertype_);
	}

	ERROR_CODE SetAggro_By_Player(const INT32 mUserIndex, char* UserID)
	{
		if (!mUserID.empty())

		{
			return ERROR_CODE::Monster_ALREADY_ASSIGEND;
		}


		mIndex = mUserIndex;
		mUserID = UserID;

		return ERROR_CODE::NONE;
	}


	void Respawn(const VECTOR3 vector3, const INT32 Lv, std::string monsterID, MONSTER_TYPE monstertype_)
	{
		Clear();
		Init_Spawn(vector3, Lv, monsterID, monstertype_);
		//printf("%f %f %f\n", vector3.X, vector3.Y, vector3.Z );
	}

	std::string GetMonsterID()
	{
		return mMonsterID;
	}

	void SetMonsterID(std::string mMonsterName)
	{
		mMonsterID = mMonsterName;
	}

	void Disattract()
	{
		mIndex = -1;
		mUserID = "";
	}

	UINT16 GetMonsterLv()
	{
		return Monster_Lv;
	
	}


	void Clear()
	{

		mIndex = -1;

		mUserID = "";
		mMonsterID = "";
	//	mIsConfirm = false;
		mCurMonsterState = Monster_State::None;
		mMonsterType = MONSTER_TYPE::NONE;
		
		Monster_Lv = 0;
		Monster_HP = 0;
		Monster_DAMAGE = 5;
		Monster_Spawn_Point = { 0, };
		Monster_POS = { 0, };
		Monster_ROT = { 0, };
	}

	VECTOR3 GetMonsterSpawnPoint()
	{
		return Monster_Spawn_Point;
	}

	void SetMonsterState(Monster_State value_)
	{
		mCurMonsterState= value_;
	}

	Monster_State GetMonsterState()
	{
		return mCurMonsterState;
	}

	INT32 GetAggroConnUserIdx()
	{
		return mIndex;
	}

	std::string GetAggroConnUserID()
	{
		return mUserID;
	}

	void SetMonsterHP(UINT32 HP_)
	{
		Monster_HP = HP_;
	}

	INT32 GetMonsterHP()
	{
		return Monster_HP;
	}

	VECTOR3 GetMonsterPos()
	{
		return Monster_POS;
	}

	VECTOR3 GetMonsterRot()
	{
		return Monster_ROT;
	}

	void SetMonType(MONSTER_TYPE monster_type_)
	{
		mMonsterType = monster_type_;
	}

	MONSTER_TYPE GetMonType()
	{
		return mMonsterType;
	}
	

	void SetMonPos(VECTOR3 vector3_)
	{
		Monster_POS = vector3_;
	}

	void SetMonRot(VECTOR3 vector3_)
	{
		Monster_ROT = vector3_;
	}

	void SetDead(UINT64 mLatestDeadTimeSec_)
	{
		mCurMonsterState = Monster_State::Dead;
		mLatestDeadTimeSec = mLatestDeadTimeSec_;

	}

	UINT64 GetLatestDeadTimeSec()
	{
		return mLatestDeadTimeSec;
	}

};