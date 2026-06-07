#================================================================================================================================#
# Quy trình hoạt động:
# 1. Ghi dòng lệnh: make all
# 2. Đầu tiên, make xử lý target $(BUILD_DIR) 
# 3. Nhảy vào target $(BUILD_DIR), thực hiện tạo ra thư mục build 
# 4. Tiếp đến, Make xử lý target $(TARGET).elf.
# Nhìn vào quy tắc $(TARGET).elf: $(OBJS),
# Make thấy rằng để liên kết được file .elf cuối cùng, nó cần toàn bộ các file .o 
# (object files) khai báo trong biến OBJS
# 5. Biên dịch từ .c sang .o (Quá trình lặp Core)
# Make bắt đầu lặp qua từng file .o bị thiếu.
# Đối với mỗi file (ví dụ build/main.o), 
# nó áp dụng quy tắc $(BUILD_DIR)/%.o: %.c.
# Nhờ cơ chế vpath, Make tự động lùng sục trong src/ và driver/src/ để tìm file main.c tương ứng.
# 6. Khi tìm thấy, nó thực thi lệnh compiler: arm-none-eabi-gcc -c ... -o build/main.o src/main.c.
# 7. Liên kết (Linking) tạo file thực thi
# Khi toàn bộ các file .o đã được "sản xuất", sự phụ thuộc của $(TARGET).elf đã được thỏa mãn.
# Bộ liên kết (Linker) sẽ gom tất cả các file .o lại, nhào nặn theo bản đồ bộ nhớ .ld và 
# xuất ra file chạy .elf cuối cùng.
#================================================================================================================================#

#================================================================================================================================#
# Quy trình từng bước để debug:
# 1. Mở 2 terminal: Terminal 1 - OpenOCD Sever, Terminal 2 - GDB Client
# - Khi mở terminal 1 gọi lệnh sau: openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
# - Khi mở terminal 2 gọi lệnh sau: arm-none-eabi-gdb build/<tên_file>.elf
# 2. Tiếp theo, các thao tác debug sẽ thực hiện chủ yếu tại terminal 2 - GDB Client
# - Chạy lệnh: target remote localhost:3333 : Kết nối GDB tới mạch STM32 thông qua cổng của OpenOCD.
# - monitor reset halt : Ra lệnh cho chip reset lại từ đầu và lập tức tạm dừng (halt) mọi hoạt động, chờ lệnh tiếp theo.
# - monitor arm semihosting enable (Tùy chọn): Nếu người dùng muốn sử dụng semihosting thì phải thực hiện câu lệnh này, nếu không muốn thì bỏ qua.
# - load : Nạp file .elf (đã nạp vào GDB) thẳng xuống bộ nhớ Flash của chip.
# - monitor reset halt: Reset lại từ đầu và nhảy vào đầu chương trình main
# 3. Dùng các lệnh để debug như: b, c, n, s, p, info register, ...
# NOTE: Từ khóa "monitor" để ra lệnh trực tiếp cho OpenOCD thông qua GDB Command
#================================================================================================================================#

#================================================================================================================================#
# Quy trình từng bước để biên dịch và nạp code:
# 1. make -f Makefile.mak clean		
# 2. make -f Makefile.mak all APP_NAME=<tên_file_muốn_chạy_trong_thư_mục_src> USE_SEMIHOSTING=<0 hoặc 1>
# 3. make -f Makefile.mak flash	APP_NAME=<tên_file_muốn_chạy_trong_thư_mục_src> USE_SEMIHOSTING=<0 hoặc 1>	
#================================================================================================================================#

# 1. Cấu hình Ứng dụng chính (Người dùng chỉnh sửa tại đây)
# Nhập tên file .c trong thư mục src mà muốn biên dịch và nạp xuống STM32 (không cần viết đuôi .c)
APP_NAME = 006_i2c_master_receiver
# Lựa chọn chế độ biên dịch (Mặc định: 0 - Không dùng Semihosting)
USE_SEMIHOSTING ?= 0

# Khai báo thư mục và công cụ
SRC_DIR = src
INC_DIR = inc
DRIVER_SRC_DIR = driver/src
STARTUP_DIR = startup
DRIVER_INC_DIR = driver/inc
BUILD_DIR = build
TARGET = $(BUILD_DIR)/$(APP_NAME)

# Khai báo các bộ toolchain
CC = arm-none-eabi-gcc
MACH = cortex-m4
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# 2. Cờ biên dịch: Cấp đường dẫn tìm file .h cho cả inc/ và driver/inc/
# CFLAGS: Khai báo các cờ cho trình biên dịch 
# -mfloat-abi=soft: Thực hiện tính toán số float bằng các thư viện phần mềm thay vì phần cứng
# -g: Nhúng các thông tin debug (debug symbols) vào các file .o, .elf để giúp cho việc debug
# -O0: Không tối ưu hóa code 
# -I: Thông báo cho compiler khi thấy các file .h trong các file .c thì hãy tìm file .h trong thư mục src, driver/src
# -ffunction-sections -fdata-sections: Tách các hàm và biến ra các section riêng biệt 
# để Linker có thể loại bỏ code không được sử dụng (Code chết).
# LDFLAGS: Khai báo các cờ cho Linker Script
# --specs=nano.specs: Sử dụng thư viện newlib-nano được tối ưu cho vi điều khiển
# --specs=rdimon.specs: Chỉ định cho Linker sử dụng môi trường semihosting để xử lý các thao tác I/O và kết nối với host PC
# -lc -lm: Thư viện tiêu chuẩn C và math
# --gc-sections: Cờ chỉ định cho Linker loại bỏ code chết.
# -Wl,-Map=...: Tạo ra file map để xem chi tiết địa chỉ của các section (không bắt buộc)
CFLAGS = -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -Wall -g -O0 -ffunction-sections -fdata-sections -I$(INC_DIR) -I$(DRIVER_INC_DIR)

# Lựa chọn chế độ dựa trên biến USE_SEMIHOSTING
ifeq ($(USE_SEMIHOSTING), 1)
# Chế độ Semihosting
LDFLAGS = -mcpu=$(MACH) -mthumb -mfloat-abi=soft -T stm32f407.ld -Wl,-Map=$(TARGET).map,--gc-sections --specs=nano.specs --specs=rdimon.specs -lc -lm -lrdimon
APP_SRC = $(SRC_DIR)/$(APP_NAME).c
else
# Chế độ bình thường
LDFLAGS = -mcpu=$(MACH) -mthumb -mfloat-abi=soft -T stm32f407.ld -Wl,-Map=$(TARGET).map,--gc-sections --specs=nano.specs -lc -lm
APP_SRC = $(SRC_DIR)/$(APP_NAME).c $(SRC_DIR)/syscalls.c
endif

# 3. Gom file nguồn: Quét file .c ở cả src/ và driver/src/
SUPPORT_SRCS = $(wildcard $(STARTUP_DIR)/*.c) $(wildcard $(DRIVER_SRC_DIR)/*.c)

# Ghép lại thành file hoàn chỉnh
SRCS = $(APP_SRC) $(SUPPORT_SRCS)

# 4. Tạo danh sách file .o và ép lưu vào build/
# Khi này, OBJS = build/main.o build/gpio.o
# Cú pháp: notdir dùng để xóa đường dẫn đến file .o
# addprefix: Thêm đường dẫn build để file .o lưu vào
OBJS = $(addprefix $(BUILD_DIR)/, $(notdir $(SRCS:.c=.o)))

# 5. VPATH: Cấp radar dò tìm file .c ở cả 2 thư mục source
# Bất cứ khi nào tìm file .c thì Make sẽ nhảy vào các thư mục này để tìm kiếm
vpath %.c $(SRC_DIR) $(STARTUP_DIR) $(DRIVER_SRC_DIR)

# Các lệnh thực thi
all: $(BUILD_DIR) $(TARGET).elf $(TARGET).bin $(TARGET).hex size

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Quy tắc biên dịch file .o
$(BUILD_DIR)/%.o: %.c
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