#include "..\Common.h"

// 서버 설정 상수
#define SERVER_PORT 9000 
#define LISTEN_BACKLOG 5 // listen 함수에 사용될 최대 대기열 크기

class NetworkManager {
private:
    // 서버의 리스닝 소켓
    SOCKET m_listen_sock;

public:
    // 생성자: 소켓 초기화
    NetworkManager() : m_listen_sock(INVALID_SOCKET) {}

    // WinSock 초기화 및 리스닝 소켓 설정 (PDF: InitServer 역할)
    bool InitServer();

    // 클라이언트 연결 요청 수락 (PDF: AcceptClient 역할)
    SOCKET AcceptClient();

    // 소멸자 (필요 시 WSACleanup, closesocket 등을 호출)
    ~NetworkManager()
    {
        if (m_listen_sock != INVALID_SOCKET) {
            closesocket(m_listen_sock);
        }
        WSACleanup();
    }
};