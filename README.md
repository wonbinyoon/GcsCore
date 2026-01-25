# GcsCore (Ground Control Station Core)

**GcsCore**는 Windows 환경에서 지상 관제 시스템(GCS, Ground Control Station)을 개발하기 위한 고성능 C++/WinRT 라이브러리입니다. 로켓, 드론, 로봇 등의 무인 이동체와의 시리얼 통신, 데이터 파싱, 로깅 및 재생(Replay) 기능을 제공합니다.

## 🚀 주요 기능 (Features)

*   **비동기 시리얼 통신 (Serial Communication):**
    *   C++/WinRT `Windows::Devices::SerialCommunication` 기반의 고성능 비동기 I/O.
    *   포트 자동 검색 및 핫플러깅 지원 구조.
*   **이벤트 기반 아키텍처 (Event-Driven Architecture):**
    *   `Signal` 및 `ScopedConnection`을 통한 타입 안전하고 스레드 안전한 옵저버 패턴 구현.
    *   통신, 데이터 수신, 에러 등의 이벤트를 느슨한 결합(Loose Coupling)으로 처리.
*   **유연한 데이터 처리 파이프라인 (Data Pipeline):**
    *   **IParser:** 원본 바이트 스트림을 프로토콜 패킷(`IPacket`)으로 변환.
    *   **IConverter:** 패킷을 애플리케이션 레벨의 텔레메트리 데이터(`TelemetryData`)로 변환.
*   **강력한 로깅 및 재생 (Logging & Replay):**
    *   **BinaryLogWriter:** 원본(Raw) 바이트 및 파싱된 데이터를 효율적인 바이너리 포맷으로 저장.
    *   **LogPlayer:** 저장된 로그를 실제 통신처럼 재생 (배속 조절, 일시 정지, Seek 지원).
*   **텔레메트리 데이터 구조:**
    *   3차원 벡터(Vec3), 쿼터니언(Quat) 등 수학 구조체 및 상태 정보 포함.

## 🛠️ 요구 사항 (Requirements)

*   **OS:** Windows 10 버전 1809 (Build 17763) 이상 또는 Windows 11
*   **IDE:** Visual Studio 2019 또는 2022
*   **SDK:** Windows SDK (최신 버전 권장)
*   **NuGet:** `Microsoft.Windows.CppWinRT`

## 📦 설치 및 빌드 (Installation & Build)

1.  **리포지토리 클론:**
    ```bash
    git clone https://github.com/wonbinyoon/GcsCore.git
    ```
2.  **Visual Studio에서 열기:**
    `GcsCore.sln` 파일을 엽니다.
3.  **NuGet 패키지 복원:**
    솔루션 탐색기에서 솔루션을 우클릭하고 "NuGet 패키지 복원"을 선택합니다.
4.  **빌드:**
    `Build` > `Build Solution` (Ctrl+Shift+B)을 실행합니다.

## 💡 사용 예제 (Usage Examples)

### 1. 시리얼 포트 연결 및 데이터 수신

```cpp
#include "transport/serial_manager.h"
#include "common/event.h"

using namespace gcs::communication;

int main() {
    auto serial = std::make_shared<SerialManager>();

    // 포트 목록 조회
    auto ports = SerialManager::GetPortList();
    if (ports.empty()) return -1;

    // 데이터 수신 이벤트 구독
    auto connection = serial->OnRawDataReceived.Connect([](const std::vector<uint8_t>& data) {
        printf("Received %zu bytes\n", data.size());
    });

    // 첫 번째 포트로 연결
    serial->OpenAsync(ports[0].id).get();

    // ... 애플리케이션 실행 ...
    
    // 종료 시
    serial->Close();
}
```

### 2. 파서 및 컨버터 연동

사용자는 자신의 프로토콜에 맞게 `IParser`와 `IConverter`를 상속받아 구현해야 합니다.

```cpp
// 사용자 구현 클래스 예시
class MyProtocolParser : public gcs::communication::IParser { /* ... */ };
class MyDataConverter : public gcs::communication::IConverter { /* ... */ };

// 메인 로직
auto parser = std::make_unique<MyProtocolParser>();
auto converter = std::make_unique<MyDataConverter>();

// 시리얼 -> 파서 데이터 전달 연결
serial->OnRawDataReceived.Connect([&](const auto& data) {
    winrt::array_view<const uint8_t> view(data);
    parser->PushData(view);
});

// 파서 -> 컨버터 -> 텔레메트리 이벤트 연결
parser->OnPacketReceived.Connect([&](auto packet) {
    converter->Convert(packet);
});

converter->OnTelemetryConverted.Connect([](const gcs::data::TelemetryData& tm) {
    printf("Altitude: %f\n", tm.pos.z());
});
```

### 3. 로그 재생 (Replay)

```cpp
#include "logging/log_player.h"

auto player = std::make_unique<gcs::logging::LogPlayer>(
    std::make_unique<MyProtocolParser>(),
    std::make_unique<MyDataConverter>()
);

player->Load("logs/flight_log.bin", gcs::logging::LogType::kRaw);
player->SetSpeed(2.0); // 2배속 재생
player->Play();

// 데이터 구독
player->OnTelemetry.Connect([](const auto& tm) {
    // UI 업데이트 등 처리
});
```

## 📂 프로젝트 구조 (Project Structure)

```
GcsCore/
├── include/
│   ├── common/         # 설정, 이벤트, 유틸리티
│   ├── data/           # 데이터 구조체 (TelemetryData 등)
│   ├── interfaces/     # 파서, 패킷, 컨버터 인터페이스
│   ├── logging/        # 로그 기록 및 재생
│   └── transport/      # 시리얼 통신 관리
├── src/                # 구현 소스 코드
└── GcsCore.vcxproj     # Visual Studio 프로젝트 파일
```

## 📝 라이선스 (License)

[MIT License](LICENSE.txt)
