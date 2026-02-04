# GcsCore (Ground Control Station Core)

**GcsCore**는 Windows 환경에서 지상 관제 시스템(GCS, Ground Control Station)을 개발하기 위한 고성능 C++/WinRT 라이브러리입니다. 로켓 등의 무인 이동체와의 시리얼 통신, 데이터 파싱, 로깅 및 재생(Replay) 기능을 제공합니다.

## 🚀 주요 기능 (Features)

*   **비동기 시리얼 통신 (Serial Communication):**
    *   C++/WinRT `Windows::Devices::SerialCommunication` 기반의 고성능 비동기 I/O.
    *   포트 자동 검색 및 핫플러깅 지원 구조.
*   **이벤트 기반 아키텍처 (Event-Driven Architecture):**
    *   `Signal` 및 `ScopedConnection`을 통한 타입 안전하고 스레드 안전한 옵저버 패턴 구현.
    *   `LogPlayer`의 재생 완료(`OnEof`) 이벤트 지원.
*   **강력한 스레드 안전성 (Thread-Safety):**
    *   `BinaryLogWriter` 내 `std::recursive_mutex` 적용으로 멀티스레드 환경 및 동기적 이벤트 콜백 체인에서 데드락 방지.
    *   RAII 기반의 자원 관리 및 최적화된 소멸 순서로 안정적인 종료 보장.
*   **유연한 데이터 처리 파이프라인 (Data Pipeline):**
    *   **IParser:** 원본 바이트 스트림을 프로토콜 패킷(`IPacket`)으로 변환.
    *   **IConverter:** 패킷을 애플리케이션 레벨의 텔레메트리 데이터(`TelemetryData`)로 변환.

## 🛠️ 개발 표준 (Standards)

*   **Style Guide:** [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) 준수.
*   **Documentation:** 전 멤버 및 메서드에 대해 **Doxygen** 스타일 주석 적용.
*   **OS:** Windows 10 버전 1809 (Build 17763) 이상 또는 Windows 11.

## 💡 사용 예제 (Usage Examples)

### 1. 커스텀 프로토콜 구현 (Parser & Converter)

사용자는 자신의 프로토콜에 맞게 인터페이스를 상속받아 구현해야 합니다.

```cpp
#include "interfaces/i_parser.h"
#include "interfaces/i_converter.h"
#include "interfaces/i_packet.h"

// 1. 패킷 정의
class MyPacket : public gcs::interfaces::IPacket {
public:
    int GetId() const override { return 0x01; }
    std::vector<uint8_t> Serialize() const override { /* ... */ return {}; }
    
    float altitude;
    float velocity;
};

// 2. 파서 구현 (Byte -> Packet)
class MyParser : public gcs::interfaces::IParser {
public:
    void PushData(winrt::array_view<uint8_t const> data) override {
        // 데이터 누적 및 프로토콜 해석 로직...
        if (packet_completed) {
            auto packet = std::make_shared<MyPacket>();
            OnPacketReceived.Invoke(packet);
        }
    }
    void Reset() override { /* 버퍼 초기화 */ }
};

// 3. 컨버터 구현 (Packet -> TelemetryData)
class MyConverter : public gcs::interfaces::IConverter {
public:
    void Convert(const std::shared_ptr<gcs::interfaces::IPacket>& packet) override {
        auto my_pkt = std::dynamic_pointer_cast<MyPacket>(packet);
        if (my_pkt) {
            gcs::data::TelemetryData tm {};
            tm.pos.z() = my_pkt->altitude;
            tm.vel.x() = my_pkt->velocity;
            OnTelemetryConverted.Invoke(tm);
        }
    }
    void Reset() override {}
};
```

### 2. 전체 시스템 통합 (Full Pipeline)

시리얼 통신, 데이터 변환, 로깅을 한데 묶어 실행하는 예제입니다.

```cpp
#include "transport/serial_manager.h"
#include "logging/binary_log_writer.h"

int main() {
    // 인스턴스 생성
    auto serial = std::make_shared<gcs::transport::SerialManager>();
    auto parser = std::make_unique<MyParser>();
    auto converter = std::make_unique<MyConverter>();
    
    // 로거 설정 (파서와 컨버터의 소유권을 가짐)
    gcs::logging::BinaryLogWriter logger(
        std::make_unique<MyParser>(), 
        std::make_unique<MyConverter>(), 
        "./logs"
    );
    logger.Bind(*serial);

    // 데이터 흐름 연결 (UI 업데이트용)
    auto conn = serial->OnRawDataReceived.Connect([&](const auto& data) {
        // 필요한 경우 여기서 파서에 직접 밀어넣을 수도 있음
    });

    // 시리얼 포트 열기
    auto ports = gcs::transport::SerialManager::GetPortList();
    if (!ports.empty()) {
        serial->OpenAsync(ports[0].id).get();
        logger.StartLogging();
    }

    // ... 어플리케이션 루프 ...
    
    serial->Close();
    return 0;
}
```

### 3. 로그 재생 및 탐색 (Advanced Replay)

```cpp
#include "logging/log_player.h"

void SetupPlayer() {
    auto player = std::make_unique<gcs::logging::LogPlayer>(
        std::make_unique<MyParser>(),
        std::make_unique<MyConverter>()
    );

    // 이벤트 구독
    player->OnTelemetry.Connect([](const auto& tm) {
        printf("Current Alt: %f\n", tm.pos.z());
    });
    
    player->OnEof.Connect([]() { printf("End of Log.\n"); });

    // 재생 설정
    if (player->Load("flight_001_raw.bin", gcs::logging::LogType::kRaw)) {
        player->SetSpeed(2.0); // 2배속
        player->SeekTo(0.5);   // 50% 지점부터 시작
        player->Play();
    }
}
```

## 📂 프로젝트 구조 (Project Structure)

*   `include/common/`: 이벤트 시스템(`Signal`), 공통 설정.
*   `include/data/`: `TelemetryData` 등 표준 데이터 구조체.
*   `include/interfaces/`: `IParser`, `IConverter`, `IPacket` 등 추상 인터페이스.
*   `include/transport/`: WinRT 기반 시리얼 통신 (`SerialManager`).
*   `include/logging/`: 바이너리 로깅(`BinaryLogWriter`) 및 재생(`LogPlayer`).

## 📝 라이선스 (License)

Copyright 2026 윤원빈. All rights reserved.
이 라이브러리는 [MIT License](LICENSE.txt) 하에 배포됩니다.