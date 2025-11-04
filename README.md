# ⭐ NGWP_TermProject: 3인 협력 마리오 (네트워크 게임)

## 🎮 1. 프로젝트 개요

본 프로젝트는 **한국공학대학교 네트워크 게임 프로그래밍** 텀 프로젝트로, 고전 2D 사이드 스크롤 플랫폼 액션 게임인 **'SUPER MARIO BROS'의 모작**을 3인 멀티플레이 환경으로 구현합니다. 목표는 점프, 이동, 몬스터 공격 등을 통해 스테이지를 클리어하고 최종 구간까지 도달하는 것입니다.

| 구분 | 내용 |
| :--- | :--- |
| **개발 도구** | Visual Studio 2022 |
| **API** | Windows API (WinAPI) |
| **통신 프로토콜** | TCP/IP |
| **버전 관리** | Git, GitHub |

---

## 👥 2. 멀티플레이어 환경 및 기능

최대 3인까지 참여 가능한 협력 게임으로 설계되었으며, 상태 동기화 및 상호작용 기능을 포함합니다.

* **3인 캐릭터**: 1P (마리오), 2P (루이지), 3P (와리오).
* **독립 카메라**: 각각의 플레이어가 개별 카메라 좌표를 가지고 있어, 각각의 화면이 다르게 보입니다.
* **게임 클리어 조건**: 게임에 참여하고 있는 **플레이어 모두**가 깃발에 충돌해야 게임이 클리어됩니다.
* **목숨 공용**으로 변경됩니다.
* **플레이어 간 상호작용**: 서로의 머리를 밟아 더 높게 뛰어오를 수 있습니다.

---

## ⚙️ 3. 서버 High-Level 디자인 (스레드 모델)

서버는 **메인 루프 + 다중 I/O 스레드** 모델을 채택하여, 로직 처리의 일관성을 **Main Thread**에 집중시키고 네트워크 I/O를 분리 처리합니다.

### A. 3가지 스레드 역할 정의

| 스레드 유형 | 함수 | 역할 및 기능 |
| :--- | :--- | :--- |
| **Main Thread** | `ThreadManager::GameLoop()` | **서버의 심장**. 모든 로직 처리와 상태 일관성을 책임지며, 패킷 처리, 로직 계산, 상태 전파를 실행합니다. |
| **Accept Thread** | `ThreadManager::AcceptLoop()` | 새로운 클라이언트의 **접속 요청만을 전담**하여 논블로킹으로 처리하고, 성공 시 `Client Thread`를 생성합니다. |
| **Client Thread** | `ThreadManager::ClientLoop()` | 할당된 단일 클라이언트와의 **네트워크 I/O**를 전담합니다. 패킷을 수신하고 파싱하여 **패킷 큐**(`m_packetQueue`)에 삽입합니다. |

### B. 스레드 동기화 방법

* **동기화 자원**:
    * `packetQueue`: `ClientLoop()`의 **쓰기**와 `GameLoop()`의 **읽기**가 동시에 접근하는 가장 중요한 공유 자원입니다.
    * `currentmap, monsters, items`: 모든 플레이어가 공유하는 자원으로 동시에 접근합니다.
* **동기화 메커니즘**: `std::mutex m_mtx`를 사용하여 모든 공유 자원 접근 시 **상호 배제(Lock)**를 보장합니다.

---

## 🔨 4. 주요 클래스별 기능 (Low-Level)

| 클래스 | 주요 함수 | 서버 역할 | 클라이언트 역할 |
| :--- | :--- | :--- | :--- |
| **ThreadManager** | `GameLoop()`, `BroadcastState()` | 메인 루프 처리, 상태 동시 전송 | - |
| **NetworkManager** | `InitServer()`, `AcceptClient()` | WinSock 초기화, 리스닝 소켓 관리, 연결 수락 전담 | `InitClient()`, `RecvPacket()`, `SendPacket()` |
| **PacketManager** | `ParsePacket()`, `HandlePacket()` | 수신 바이트를 객체로 변환하고 로직에 반영 | 수신 바이트를 객체로 변환하고 로직에 반영 |
| **GameWorld** | `Update()`, `CheckCollision()` | 게임 맵/오브젝트 업데이트 및 충돌 검사 | `GameInput()`, `DrawWorld()` |

---

## 🧑‍💻 5. 팀원 별 역할분담

| 팀원 | 주요 역할 | 세부 담당 내용 |
| :--- | :--- | :--- |
| **이용재** | 클라이언트 리펙토링 및 NETWORKMANAGER 관리 | 클라이언트 네트워크(`InitClient()`, `SendPacket()`, `RecvPacket()`), `CheckCollision` 연동 작업. |
| **이현석** | 서버-클라이언트 PacketManager 표준 관리 | 패킷 파싱/직렬화(`ParsePacket`, `SerializePacket`), `HandlePacket` 구현 및 패킷 포맷 정의. |
| **황태규** | 서버 Manager, NetworkManager 및 Object 관리 | 서버 스레드 함수(`GameLoop()`, `AcceptLoop()`, `ClientLoop()`) 작성, `BroadcastState()`, 상태 관리. |
