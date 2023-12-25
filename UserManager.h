#pragma once
#include <unordered_map>

#include "ErrorCode.h"
#include "User.h"
#include "MonsterManager.h"

class UserManager
{
private:
	INT32 mMaxUserCnt = 0;
	INT32 mCurrentUserCnt = 0;

	std::vector<User*> mUserObjPool;
	std::unordered_map<std::string, int> mUserID_Dictionary;

	std::unordered_map<std::string, std::string> mLogin_DB;


public:
	UserManager() = default;
	~UserManager() = default;

	std::function<void(UINT32, UINT32, char*)> SendPacketFunc;

	std::vector<User*> GetUserPool()
	{
		return mUserObjPool;
	}

	void Init(const INT32 maxUserCount_)
	{
		mMaxUserCnt = maxUserCount_;
		mUserObjPool = std::vector<User*>(mMaxUserCnt);
		
		for (int i = 0; i < mMaxUserCnt; i++)
		{
			mUserObjPool[i] = new User();
			mUserObjPool[i]->Init(i);

		}

	}

	INT32 GetCurrentUserCnt() { return mCurrentUserCnt; }

	INT32 GetMaxUserCnt() { return mMaxUserCnt; }
	
	void IncreaseUserCnt() { mCurrentUserCnt++; }

	void DecreaseUserCnt()
	{
		if (mCurrentUserCnt > 0)
		{
			mCurrentUserCnt--;
		}

	}


	ERROR_CODE AddUser(char* userID_, char* userPW_, int clientIndex_)
	{
		

		int user_idx = clientIndex_;

		mUserObjPool[user_idx]->SetLogin(userID_);
		mUserID_Dictionary.insert(std::pair< char*, int>(userID_, clientIndex_));
		mLogin_DB.insert(std::pair<char*, char*>(userID_, userPW_));

		IncreaseUserCnt();

		return ERROR_CODE::NONE;
	}

	INT32 FindUserIndexByID(char* userID_)
	{
		auto res= mUserID_Dictionary.find(userID_);

		if ( res != mUserID_Dictionary.end())
		{
			return (*res).second;
		}

		return -1;
	}

	void DeleteUserInfo(User* user_)
	{
		mUserID_Dictionary.erase(user_->GetUserId());
		user_->Clear();
		DecreaseUserCnt();

	}

	User* GetUserByConnIdx(INT32 clientIndex_)
	{
		return mUserObjPool[clientIndex_];
	}

	void SendToAllUser(char* data_, const UINT16 dataSize_)
	{
		for (auto pUser : mUserObjPool)
		{
			if (pUser->GetDomainState() == User::DOMAIN_STATE::NONE)
			{
				continue;
			}

			SendPacketFunc((UINT32)pUser->GetNetConnIdx(), (UINT32)dataSize_, data_);

		}
	
	}

	void SendToAllUserExceptMe(char* data_, const UINT16 dataSize_, const INT32 passUserIndex_)
	{
		for (auto pUser : mUserObjPool)
		{
			if (pUser->GetDomainState() == User::DOMAIN_STATE::NONE)
			{
				continue;
			}

			if (pUser->GetNetConnIdx() == passUserIndex_)
			{
				continue;
			}

			SendPacketFunc((UINT32)pUser->GetNetConnIdx(), (UINT32)dataSize_, data_);

		}

	}





};