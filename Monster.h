#pragma once
#include <string>

#include "Packet.h"

class Monster
{

	//ü��
	//��ǥ ������ - ��ġ, motion for animation 
	//����

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
	INT32 mIndex = -1; // User���� �����Ǿ��ִ°�
	std::string mUserID; //UserID
	std::string mMonsterID; //monster �̸�
	//bool mIsConfirm = false;

	Monster_State mCurMonsterState = Monster_State::None;

	INT16 Monster_Lv = 0;
	INT32 Monster_HP = 10;
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

	void Init_Spawn(const VECTOR3 vector3, const INT32 Lv, std::string monsterID)
	{
		VECTOR3 Monster_Spawn_Point = vector3;
		Init_monster_lvl(Lv);
		mCurMonsterState = Monster_State::Live;
		SetMonsterID(monsterID);
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


	void Respawn(const VECTOR3 vector3, const INT32 Lv, std::string monsterID)
	{
		Clear();
		Init_Spawn(vector3, Lv, monsterID);
	}

	std::string GetMonsterID()
	{
		return mMonsterID;
	}

	void SetMonsterID(std::string mMonsterName)
	{
		mMonsterID = mMonsterName;
	}


	void Clear()
	{

		INT32 mIndex = -1;

		mUserID = "";
		mMonsterID = "";
	//	mIsConfirm = false;
		mCurMonsterState = Monster_State::None;

		
		INT16 Monster_Lv = 0;
		INT32 Monster_HP = 10;
		INT32 Monster_DAMAGE = 5;
		VECTOR3 Monster_Spawn_Point = { 0, };
		VECTOR3 Monster_POS = { 0, };
		VECTOR3 Monster_ROT = { 0, };
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

	INT32 SetMonsterHP(UINT32 DAMAGE)
	{
		Monster_HP -= DAMAGE;
		return Monster_HP;
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