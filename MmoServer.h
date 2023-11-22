#pragma once

#include "IOCPServer.h"
#include "PacketManager.h"
#include "Packet.h"

#include <vector>



class MmoServer : public IOCPServer

{
private :
	std::unique_ptr<PacketManager> m_PacketManager;

public:
	MmoServer() = default;
	virtual ~MmoServer() = default;

	virtual void OnConnect(const UINT32 clientIndex_) override
	{
		printf("[OnConnect] 클라이언트: Index(%d)\n", clientIndex_);

		PacketInfo packet{ clientIndex_, (UINT16)PACKET_ID::USER_CONNECT, 0 };
		m_PacketManager->PushSystemPacket(packet);
	
	}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) override
	{
		printf("[OnReceive] 클라이언트: Index(%d), dataSize(%d)\n", clientIndex_, size_);

		m_PacketManager->ReceivePacketData(clientIndex_, size_, pData_);
	}

	void Run(const UINT32 maxClient)
	{
		auto sendPacketFunc = [&](UINT32 clientIndex_, UINT16 packetSize, char* pSendPacket)
		{
			SendMsg(clientIndex_, packetSize, pSendPacket);
		};

		m_PacketManager = std::make_unique<PacketManager>();
		m_PacketManager->SendPacketFunc = sendPacketFunc;
		m_PacketManager->Init(maxClient);
		m_PacketManager->Run();

		StartServer(maxClient);
	}

	void End()
	{
		m_PacketManager->End();

		DestroyThread();
	}






};