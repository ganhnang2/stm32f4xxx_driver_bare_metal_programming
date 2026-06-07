#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08  
#define SDA_PIN 21
#define SCL_PIN 22
#define I2C_FREQ 100000      

volatile uint8_t dataToSend[] = "Hello. This is an message from ESP32!";
volatile uint8_t len = sizeof(dataToSend);
volatile uint8_t isDataSent = 0;
volatile uint8_t command = 0xFF;

void setup() {
  Serial.begin(115200);
  Serial.println("Khởi động ESP32 ở chế độ I2C Slave Transmitter...");

  Wire.onRequest(requestEvent);   
  Wire.onReceive(receiveEvent);   

  Wire.begin((uint8_t)I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN, I2C_FREQ); 

  Serial.println("Đã cấu hình xong. Sẵn sàng gửi dữ liệu khi STM32 gọi...");
}

void loop() {
  if (isDataSent) {
    Serial.printf("Đã gửi %d bytes dữ liệu: %s\n", len, (char*)dataToSend);
    isDataSent = 0;
  }
}

void requestEvent() {
  // Không thực hiện Wire.write ở đây nữa để tránh độ trễ.
  // Dữ liệu đã được nạp sẵn từ receiveEvent.
  command = 0xFF; // Reset lệnh
}

void receiveEvent(int bytes) {
  if (Wire.available()) {
    command = Wire.read();    
    
    // NẠP SẴN DỮ LIỆU VÀO TX FIFO NGAY LÚC NÀY
    // Lưu ý: Tùy phiên bản ESP32 Core, bạn có thể dùng Wire.slaveWrite() 
    // Nếu Core báo lỗi không có slaveWrite, hãy giữ nguyên Wire.write()
    if (command == 0x51) {
      Wire.slaveWrite((uint8_t*)&len, 1); 
    } 
    else if (command == 0x52) {
      Wire.slaveWrite((uint8_t*)dataToSend, len);
      isDataSent = 1;
    }
  }
  
  // Xóa các byte thừa (nếu có) trong buffer để tránh kẹt
  while(Wire.available()) {
    Wire.read();
  }
}