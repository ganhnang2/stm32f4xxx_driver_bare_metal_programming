#include <stdint.h>
#include <stdio.h>

/* Định nghĩa địa chỉ các thanh ghi phần cứng của STM32F407 */
#define RCC_AHB1ENR   (*((volatile uint32_t*)0x40023830))
#define GPIOD_MODER   (*((volatile uint32_t*)0x40020C00))
#define GPIOD_ODR     (*((volatile uint32_t*)0x40020C14))

/* Hàm tạo trễ đơn giản bằng vòng lặp (vì chưa dùng Timer) */
void delay(volatile uint32_t count) {
    while(count--) {
        /* Không làm gì cả, chỉ tốn thời gian CPU */
    }
}

/* Khai báo hàm khởi tạo Semihosting */
extern void initialise_monitor_handles(void);

int main(void) {
    /* Khởi tạo Semihosting để printf in ra terminal */
    initialise_monitor_handles();
    
    /* 1. Cấp xung nhịp (Clock) cho Port D (Bit 3) */
    uint32_t count = 0;
    RCC_AHB1ENR |= (1 << 3);

    /* 2. Cấu hình chân PD11 thành chế độ Output (General purpose output mode)
       Bit 23:22 của GPIOD_MODER cần set thành 01 */
    GPIOD_MODER &= ~(3 << 22); /* Xóa sạch 2 bit 22 và 23 */
    GPIOD_MODER |= (1 << 22);  /* Ghi 01 vào 2 bit đó */

    /* 3. Vòng lặp chính */
    while(1) {
        /* Bật LED (Set bit 11 của thanh ghi Output Data Register) */
        GPIOD_ODR |= (1 << 11);
        delay(500000); /* Chờ một lát */

        /* Tắt LED (Xóa bit 12) */
        GPIOD_ODR &= ~(1 << 11);
        delay(500000); /* Chờ một lát */

        /* In ra màn hình thông qua Semihosting */
        printf("Count: %ld\n", count);
        count++;
    }

    return 0;
}