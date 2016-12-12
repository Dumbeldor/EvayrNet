#include "NetworkManager.h"
#include "socket\windows\WindowsUDPSocket.h"
#include "systems\client\NetworkClient.h"
#include "systems\server\NetworkServer.h"

using namespace EvayrNet;

EvayrNet::NetworkManager* EvayrNet::g_Network = nullptr;

NetworkManager::NetworkManager(uint16_t aPort, bool aIsServer)
{
	g_Network = this;

	CreateSocket(aPort);

	if (aIsServer)
	{
		m_pNetworkSystem = std::make_unique<NetworkServer>();
		m_pUDPSocket->SetConnected(true);
		m_pUDPSocket->SetConnectionID(UDPSocket::kServerConnectionID);
	}
	else
	{
		m_pNetworkSystem = std::make_unique<NetworkClient>();
	}
}

NetworkManager::~NetworkManager()
{
	if (IsConnected())
	{
		Disconnect();
	}
}

void NetworkManager::CreateSocket(uint16_t aPort)
{
#if defined(_WIN64) || defined(_WIN32)
	m_pUDPSocket = std::make_unique<WindowsUDPSocket>(&m_PacketHandler, aPort, kDefaultTickRate);
#else
	// Linux?
#endif
}

void NetworkManager::Update()
{
	m_pUDPSocket->Update();
}

void NetworkManager::ConnectTo(const char* apIP, uint16_t aPort)
{
	m_pUDPSocket->ConnectTo(apIP, aPort);
}

void NetworkManager::Disconnect()
{
	m_pUDPSocket->Disconnect();
}

void NetworkManager::Send(std::shared_ptr<Messages::Message> apMessage, uint16_t aConnectionID)
{
	apMessage->messageType = Messages::EMessageType::MESSAGE_UNRELIABLE;
	m_pUDPSocket->AddMessage(apMessage, aConnectionID, false);
}

void NetworkManager::SendReliable(std::shared_ptr<Messages::Message> apMessage, uint16_t aConnectionID, bool aStoreACK)
{
	apMessage->messageType = Messages::EMessageType::MESSAGE_RELIABLE;
	m_pUDPSocket->AddMessage(apMessage, aConnectionID, aStoreACK);
}

void NetworkManager::SendSequenced(std::shared_ptr<Messages::Message> apMessage, uint16_t aConnectionID, bool aStoreACK)
{
	apMessage->messageType = Messages::EMessageType::MESSAGE_SEQUENCED;
	m_pUDPSocket->AddMessage(apMessage, aConnectionID, aStoreACK);
}

void NetworkManager::RegisterMessage(std::unique_ptr<Messages::Message> apMessage, uint8_t aOpCode)
{
	m_PacketHandler.RegisterMessage(std::move(apMessage), aOpCode);
}

void NetworkManager::SetTickRate(uint8_t aSendTickRate)
{
	m_pUDPSocket->SetTickRate(aSendTickRate);
}

bool NetworkManager::IsConnected() const
{
	return m_pUDPSocket->IsConnected();
}

NetworkSystem* EvayrNet::NetworkManager::GetNetworkSystem()
{
	return m_pNetworkSystem.get();
}

UDPSocket* NetworkManager::GetUDPSocket()
{
	return m_pUDPSocket.get();
}
