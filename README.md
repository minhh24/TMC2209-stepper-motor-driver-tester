# TMC2209 & Stepper Motor Driver Tester - bộ kiếm tra driver TMC2209 và motor bước

Dự án này phát triển một thiết bị cầm tay chuyên dụng để kiểm tra động cơ bước (Stepper Motor) và cấu hình Driver TMC2209 mà không cần sử dụng máy tính. Thiết bị sử dụng vi điều khiển STM32F103C8T6 (BluePill), kết hợp kiến trúc điều khiển lai (Hybrid Control) giữa UART và GPIO để khai thác tối đa tính năng của Driver.

## Video Demo
https://youtu.be/HVJBm3PD8uo

## Tính Năng Nổi Bật

1. **Hoạt động độc lập:** Thiết bị cầm tay, hiển thị thông số trực quan qua màn hình LCD 16x2.
2. **Điều khiển Hybrid (Phần mềm & Phần cứng):**
   - **Giao thức UART (Single Wire):** Cấu hình dòng điện động cơ (RMS Current) thời gian thực.
   - **Giao tiếp GPIO:** Điều khiển vi bước (Microstep) và chiều quay (Direction) để đảm bảo tốc độ đáp ứng cao nhất.
3. **5 Chế độ hoạt động chuyên biệt:**
   - **Manual Run (Chạy thủ công):** Chạy động cơ với tốc độ tùy chỉnh (RPM) sử dụng bộ tạo xung PWM Timer 1 (Advanced Timer).
   - **Set Current (Cài đặt dòng):** Cài đặt dòng điện chạy (Run Current) từ 100mA đến 2000mA qua UART.
   - **Microstep Config (Cấu hình vi bước):** Chuyển đổi nhanh giữa các chế độ vi bước: 1/8, 1/16, 1/32, 1/64.
   - **Position Test (Chế độ Handwheel):** Chế độ giả lập tay quay máy CNC. Vòng xoay Encoder được ánh xạ trực tiếp sang bước động cơ với hệ số nhân x20, giúp kiểm tra vị trí chính xác.
   - **Direction Control (Đảo chiều):** Đảo chiều quay mặc định của động cơ (CW/CCW).
4. **Kiến trúc phần mềm FSM:** Sử dụng Máy trạng thái hữu hạn (Finite State Machine) cho giao diện người dùng, đảm bảo hệ thống hoạt động mượt mà, không bị treo (Non-blocking).

## Sơ Đồ Kết Nối Phần Cứng (Pinout)

Vi điều khiển trung tâm: **STM32F103C8T6 (BluePill)**



### 1. Kết nối Driver TMC2209
| Chân STM32 | Chân TMC2209 | Chức năng | Ghi chú |
| :--- | :--- | :--- | :--- |
| **PA15** | EN | Enable | Đã Remap để tắt JTAG |
| **PB15** | STEP | Xung Bước | Sử dụng TIM1_CH3N hoặc GPIO Toggle |
| **PB14** | DIR | Hướng | Điều khiển chiều quay |
| **PA9** | PDN/UART | UART TX | **Kết nối qua điện trở 1k Ohm** |
| **PA10** | PDN/UART | UART RX | Kết nối trực tiếp |
| **PA11** | MS1 | MS1 | Cấu hình vi bước phần cứng |
| **PA12** | MS2 | MS2 | Cấu hình vi bước phần cứng |



**Lưu ý quan trọng về phần cứng:**
- **Nguồn cấp:** Cấp nguồn 12V-24V DC cho chân VM của TMC2209.
- **Giao tiếp UART:** Bắt buộc phải có **điện trở 1k Ohm** giữa chân TX của STM32 và chân PDN của TMC2209 để tạo thành giao thức Single Wire UART.

### 2. Kết nối Giao diện Người dùng (UI)
| Chân STM32 | Linh kiện | Chức năng | Ghi chú |
| :--- | :--- | :--- | :--- |
| **PB6** | Encoder | Kênh A | Timer Encoder Mode (TIM4) |
| **PB7** | Encoder | Kênh B | Timer Encoder Mode (TIM4) |
| **PB5** | Button | Nút Chọn/OK | Active Low, Input Pull-up |
| **PB10** | Button | Nút Back/Dừng | Active Low, Input Pull-up |
| **PB8** | LCD | I2C SCL | Remap I2C1 |
| **PB9** | LCD | I2C SDA | Remap I2C1 |

### 3. Tín hiệu khác
- **PA3** (Status LED): Đèn báo trạng thái động cơ đang chạy (Active High).
- **PC13** (Heartbeat LED): Đèn báo hệ thống đang hoạt động (Onboard LED).

## Cấu Trúc Thư Mục Dự Án

Dự án được tổ chức theo dạng Modular để dễ dàng bảo trì và mở rộng. Dưới đây là cây thư mục các file mã nguồn chính:

```text
Project_Root/
├── Core/
│   ├── Inc/                 # Chứa các file Header (.h)
│   └── Src/                 # Chứa các file Source (.c)
│       ├── main.c           # Vòng lặp chính, cầu nối phần cứng & logic PWM
│       ├── gpio.c           # Khởi tạo GPIO
│       ├── tim.c            # Cấu hình Timer (PWM & Encoder)
│       ├── usart.c          # Cấu hình UART
│       └── mylib/           # THƯ VIỆN TÙY CHỈNH (User Libraries)
│           ├── i2c_lcd.c    # Driver điều khiển LCD 16x2 qua I2C
│           ├── encoder.c    # Xử lý Encoder và khử rung nút nhấn
│           ├── tmc2209.c    # Giao thức UART, CRC và lệnh điều khiển Driver
│           └── interface.c  # Quản lý Menu, hiển thị và Máy trạng thái (FSM)
├── Drivers/                 # Thư viện HAL của ST
└── MDK-ARM/                 # File Project Keil C
