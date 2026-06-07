#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08  
#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_FREQ 100000      

// Các biến dùng chung giữa hàm ngắt (ISR) và loop() BẮT BUỘC phải có từ khóa 'volatile'
// Điều này báo cho trình biên dịch biết biến có thể thay đổi đột ngột ngoài luồng chương trình chính.
volatile bool isDataReceived = false;
volatile int dataLength = 0;

// Mảng lưu dữ liệu (buffer nhận của ESP32 thường khoảng 128 bytes)
volatile uint8_t dataBuffer[128]; 

void setup() {
  Serial.begin(115200);
  Serial.println("Khởi động ESP32 ở chế độ I2C Slave Receiver (Tối ưu Ngắt)...");

  // Đăng ký hàm ngắt
  Wire.onReceive(receiveEvent);

  // Khởi tạo I2C
  Wire.begin((uint8_t)I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN, I2C_FREQ); 

  Serial.println("Đã cấu hình xong. Chờ dữ liệu từ STM32...");
}

void loop() {
  // Kiểm tra cờ báo hiệu (Flag)
  if (isDataReceived) {
    
    // In số lượng byte nhận được
    Serial.print("Nhận được ");
    Serial.print(dataLength);
    Serial.print(" bytes: ");

    // Xử lý và in dữ liệu ra màn hình
    for (int i = 0; i < dataLength; i++) {
      Serial.print((char)dataBuffer[i]);
    }
    Serial.println();
    
    // Đặt lại cờ về false để chờ gói dữ liệu tiếp theo
    isDataReceived = false; 
  }
  
  // Bạn có thể thoải mái viết thêm các hàm xử lý khác ở đây, 
  // ví dụ chớp nháy LED, đọc cảm biến, kết nối WiFi...
  // delay(10); 
}

// ---------------------------------------------------------
// HÀM NGẮT (ISR): Chỉ đọc dữ liệu vào buffer và bật cờ
// KHÔNG dùng Serial.print, KHÔNG dùng delay() ở đây
// ---------------------------------------------------------
void receiveEvent(int howMany) {
  int i = 0;
  
  // Rút hết dữ liệu từ bộ đệm của phần cứng I2C ra mảng toàn cục
  while (Wire.available() && i < sizeof(dataBuffer)) {
    dataBuffer[i] = Wire.read();
    i++;
  }
  
  // Lưu lại độ dài và bật cờ báo hiệu
  dataLength = i;
  isDataReceived = true; 
}