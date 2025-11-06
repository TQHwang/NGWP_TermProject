#include "NetworkManager.h" 
// 'NetworkManager.h'에서 'Common.h'를 포함하므로 필요한 모든 헤더와 오류 함수 사용 가능

/**
 * WinSock 초기화 및 리스닝 소켓을 생성하고 바인딩, 대기(listen) 상태로 만듭니다.
 * return 성공 시 true, 실패 시 false
 * 역할: WinSock 초기화, 리스닝 소켓 생성, 바인딩 및 대기(listen).
 */
bool NetworkManager::InitServer()
{
    // 1. WinSock 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        err_display("WSAStartup() failed");
        return false;
    }

    // 2. 리스닝 소켓 생성 (TCP/IPv4)
    m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_sock == INVALID_SOCKET) {
        err_display("socket() failed");
        return false;
    }

    // 3. 소켓을 논블로킹 모드로 설정 (AcceptThread의 논블로킹 호출을 위해 )
    u_long non_blocking = 1;
    if (ioctlsocket(m_listen_sock, FIONBIO, &non_blocking) == SOCKET_ERROR) {
        err_display("ioctlsocket(FIONBIO) failed");
        return false;
    }

    // 4. sockaddr_in 구조체 설정 (주소, 포트 바인딩 준비)
    SOCKADDR_IN server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 IP 주소로부터 접속 허용
    server_addr.sin_port = htons(SERVER_PORT);

    // 5. bind()
    if (bind(m_listen_sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        err_display("bind() failed");
        return false;
    }

    // 6. listen() (대기 상태)
    if (listen(m_listen_sock, LISTEN_BACKLOG) == SOCKET_ERROR) {
        err_display("listen() failed");
        return false;
    }

    printf("Server initialization successful on port %d.\n", SERVER_PORT);
    return true;
}


/**
 * 새로운 클라이언트의 연결 요청을 수락합니다.
 * return 성공적으로 연결된 클라이언트 소켓, 실패 또는 연결 대기 중이면 INVALID_SOCKET
 * 역할: accept() 함수를 호출하여 연결 수락, Accept Thread가 호출하여 접속 처리를 전담.
 * 논블로킹 소켓이므로, 연결이 없을 경우 에러코드 WSAEWOULDBLOCK과 함께 INVALID_SOCKET을 반환합니다.
 */
SOCKET NetworkManager::AcceptClient()
{
    // 클라이언트 소켓 및 주소 정보
    SOCKET client_sock;
    SOCKADDR_IN client_addr;
    int addr_len = sizeof(client_addr);

    // 1. accept() 호출
    client_sock = accept(m_listen_sock, (SOCKADDR*)&client_addr, &addr_len);

    // 2. 오류 처리 (논블로킹 환경)
    if (client_sock == INVALID_SOCKET) {
        int err_code = WSAGetLastError();

        // 연결 요청이 아직 없어서 블록되지 않은 상태 (논블로킹 모드에서는 정상)
        if (err_code == WSAEWOULDBLOCK) {
            // printf("Accepting... (No connection yet)\n"); // 디버깅용
            return INVALID_SOCKET;
        }
        // 심각한 오류 발생
        else {
            err_display("accept() failed");
            return INVALID_SOCKET;
        }
    }

    // 3. 성공 시 정보 출력 및 클라이언트 소켓 반환
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("Client connected: IP=%s, Port=%d, Socket=%d\n",
        client_ip,
        ntohs(client_addr.sin_port),
        (int)client_sock);

    return client_sock;
}