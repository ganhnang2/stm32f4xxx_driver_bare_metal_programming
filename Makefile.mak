#=======================================USER CONFIGURATION===========================================#
# Nhập tên file .c trong thư mục src mà muốn biên dịch và nạp xuống STM32 (không cần viết đuôi .c)
APP_NAME = 006_i2c_master_receiver
# Lựa chọn chế độ biên dịch (Mặc định: 0 - Không dùng Semihosting)
USE_SEMIHOSTING ?= 0

#===============================================DIRECTORIES=========================================#
SRC_DIR = src
INC_DIR = inc
DRIVER_SRC_DIR = driver/src
STARTUP_DIR = startup
DRIVER_INC_DIR = driver/inc
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(APP_NAME)

#=================================================TOOLCHAIN========================================#
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

#===============================================MCU================================================#
# 
CPU = cortex-m4
FPU = fpv4-sp-d16
MCU_FLAGS = -mcpu=$(CPU) -mthumb -mfloat-abi=hard -mfpu=$(FPU)

#========================================COMPILER AND LINKER FLAGS=================================#
# Complier
# -std=gnu11: Sử dụng ngôn ngữ C11
# -Wall: hiển thị toàn bộ cảnh báo (nếu có)
# -g: Nhúng các thông tin debug (debug symbols) vào các file .o, .elf để giúp cho việc debug
# -O0: Không tối ưu hóa code 
# -ffunction-sections -fdata-sections: Tách các hàm và biến ra các section riêng biệt để Linker có 
# thể loại bỏ code không được sử dụng (Code chết).
# -I: Thông báo cho compiler khi thấy các file .h trong các file .c thì hãy tìm file .h trong thư mục src, driver/src
CFLAGS = -c $(MCU_FLAGS) 
CFLAGS += -std=gnu11 
CFLAGS += -Wall 
CFLAGS += -g -O0 
CFLAGS += -ffunction-sections -fdata-sections 
CFLAGS += -I$(INC_DIR) -I$(DRIVER_INC_DIR) 

# Linker
# -Wl,-Map=...: Tạo ra file map để xem chi tiết địa chỉ của các section (không bắt buộc)
# -T: Cung cấp Linker Script
# --gc-sections: Cờ chỉ định cho Linker loại bỏ code chết.
# -lc -lm: Thư viện tiêu chuẩn C và math
LD_SCRIPTS = stm32f407.ld
LDFLAGS = $(MCU_FLAGS) 
LDFLAGS += -T $(LD_SCRIPTS)
LDFLAGS += -Wl,-Map=$(TARGET).map,--gc-sections
LDFLAGS += -lc -lm

#========================================SEMIHOSTING================================================#
ifeq ($(USE_SEMIHOSTING), 1)
LDFLAGS += --specs=nano.specs --specs=rdimon.specs -lrdimon
APP_SRC = $(SRC_DIR)/$(APP_NAME).c
else
LDFLAGS += --specs=nano.specs
APP_SRC = $(SRC_DIR)/$(APP_NAME).c $(SRC_DIR)/syscalls.c
endif

#=====================================SOURCE FILES===================================================#
# Gom file nguồn: Quét file .c ở cả src/ và driver/src/
SUPPORT_SRCS = $(wildcard $(STARTUP_DIR)/*.c) $(wildcard $(DRIVER_SRC_DIR)/*.c)
SRCS = $(APP_SRC) $(SUPPORT_SRCS)

#====================================OBJECT FILES====================================================#
# C_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(filter %.c, $(SRCS)))
# Cú pháp hoạt động như sau:
# 1. Chạy hàm lọc: Lọc ra tất cả các file có .c trong $(SRCS) 
# -> Khi này, chỉ lấy những file .c mà ko vô tình lấy các file khác như .s, .txt, ...
# 2. Chạy hàm thay thế: Ký tự % đóng vai trò là "chất kết dính" (stem) để lưu trữ phần tên file.
# Make lấy từ đầu tiên: src/main.c. Nó so khớp với %.c -> Make lưu lại phần thân % = src/main
# Ráp phần % vừa tìm được vào build/%.o -> Kết quả tạo ra: build/src/main.o
# 3. Cuối cùng gán vào C_OBJS
C_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(filter %.c, $(SRCS)))
OBJS = $(C_OBJS)

#========================================BUILD TARGETS==============================================#
all: $(BUILD_DIR) $(TARGET).elf $(TARGET).bin $(TARGET).hex size

# Tạo ra thư mục build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Quy tắc biên dịch file .c sang .o
# mkdir -p $(dir $@): Tạo ra thư mục có đường dẫn như target $(BUILD_DIR)/%.o
# Ví dụ: build/src/main.o thì tạo ra thư mục build/src
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

# Liên kết thành file .elf
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	
# Tạo ra file .bin (Mã máy nhị phân thuần túy)
# Cú pháp: objcopy [các_cờ_tùy_chọn] <file_đầu_vào> <file_đầu_ra>
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	
# Tạo ra file .hex (Định dạng intel hex)
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@
	
# Kích thước bộ nhớ
size: $(TARGET).elf
	@echo "------Size------"
	$(SIZE) $<

# Dọn dẹp
clean:
	rm -rf $(BUILD_DIR)

# Nạp code
flash: 
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(TARGET).elf verify reset exit"
	
# Gỡ lỗi
debug:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
	
# Khai báo các mục tiêu ảo (tránh lỗi trùng tên file vật lý)
.PHONY: all clean flash size debug