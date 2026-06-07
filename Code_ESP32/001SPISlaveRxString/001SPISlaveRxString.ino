#include <Arduino.h>
#include <driver/spi_slave.h>

// Định nghĩa các chân SPI trên ESP32 (tương ứng với STM32)
// PB15 (MOSI) ---> GPIO 23
// PB13 (SCLK) ---> GPIO 18
// PB12 (NSS)  ---> GPIO 22
#define GPIO_MOSI 23
#define GPIO_SCLK 18
#define GPIO_CS   22

#ifndef SPI3_HOST
#define SPI3_HOST VSPI_HOST
#endif

#define DMA_CHAN    2
#define MAX_BUFFER_SIZE 128

// Cấp phát bộ đệm RX căn chỉnh 32-bit cho DMA
WORD_ALIGNED_ATTR uint8_t recv_buf[MAX_BUFFER_SIZE] = {0};

void setup() {
  Serial.begin(115200);

  // 1. Cấu hình bus SPI
  spi_bus_config_t buscfg = {
      .mosi_io_num = GPIO_MOSI,
      .miso_io_num = -1,         // STM32 không dùng MISO, ESP32 cũng vô hiệu hóa
      .sclk_io_num = GPIO_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = MAX_BUFFER_SIZE
  };

  // 2. Cấu hình giao diện SPI Slave
  spi_slave_interface_config_t slvcfg = {
      .spics_io_num = GPIO_CS,
      .flags = 0,
      .queue_size = 3,
      .mode = 0,                 // Mode 0 (CPOL=0, CPHA=0) khớp hoàn toàn với STM32
      .post_setup_cb = NULL,
      .post_trans_cb = NULL
  };

  pinMode(GPIO_MOSI, INPUT_PULLUP);
  pinMode(GPIO_SCLK, INPUT_PULLUP);
  pinMode(GPIO_CS, INPUT_PULLUP);

  // 3. Khởi tạo SPI Slave
  esp_err_t ret = spi_slave_initialize(SPI3_HOST, &buscfg, &slvcfg, DMA_CHAN);
  if (ret == ESP_OK) {
    Serial.println("ESP32 SPI Slave đã sẵn sàng nhận dữ liệu từ STM32F407...");
  } else {
    Serial.println("Lỗi khởi tạo SPI Slave!");
  }
}

void loop() {
  // Reset buffer trước mỗi lần nhận
  memset(recv_buf, 0, sizeof(recv_buf));

  spi_slave_transaction_t t;
  memset(&t, 0, sizeof(t));
  
  // Khai báo sẵn chiều dài tối đa có thể nhận
  // Khi STM32 kéo CS lên HIGH, transaction sẽ tự động dừng lại
  t.length = MAX_BUFFER_SIZE * 8; 
  t.tx_buffer = NULL;
  t.rx_buffer = recv_buf;

  // Block và chờ tín hiệu NSS từ STM32
  esp_err_t ret = spi_slave_transmit(SPI3_HOST, &t, portMAX_DELAY);

  if (ret == ESP_OK) {
    // t.trans_len chứa TỔNG SỐ BIT mà Master đã truyền
    int bytes_received = t.trans_len / 8;

    if (bytes_received > 0) {
      // Bóc tách Frame dữ liệu:
      // Byte [0] là dataLen
      // Byte [1] -> [1 + dataLen] là user_data
      
      uint8_t data_len = recv_buf[0];
      
      // Tạo mảng tạm để in chuỗi an toàn
      char payload[MAX_BUFFER_SIZE];
      memset(payload, 0, sizeof(payload));

      // Lấy phần dữ liệu dựa trên byte chiều dài từ STM32 gửi sang
      // Đảm bảo không copy vượt quá số lượng byte thực tế nhận được
      int copy_size = (data_len <= (bytes_received - 1)) ? data_len : (bytes_received - 1);
      
      if (copy_size > 0) {
        memcpy(payload, &recv_buf[1], copy_size);
        
        Serial.printf("========================\n");
        Serial.printf("Tổng byte nhận: %d\n", bytes_received);
        Serial.printf("Toàn bộ dữ liệu nhận được: %s\n", recv_buf);
        Serial.printf("Kích thước payload báo trước: %d\n", data_len);
        Serial.printf("Nội dung chuỗi: %s\n", payload);
      }
    }
  }
}