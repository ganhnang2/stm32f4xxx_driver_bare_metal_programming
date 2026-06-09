# Makefile 

## Quy trình hoạt động của Makefile
Dưới đây là một bức tranh toàn cảnh tuyệt đẹp về hành trình của code, từ những dòng text vô tri trong file C 
cho đến khi biến thành dòng điện chạy trong lõi con chip STM32. Dựa trên chính những file bạn đã viết, quy trình gồm 4 giai đoạn chính:

### 1. Giai đoạn Biên dịch (Compilation)
Mục tiêu: Dịch các file mã nguồn (ngôn ngữ người đọc được như C, Assembly) thành các file mã máy (Object file .o).
- Đầu vào: Các file .c trong src/, driver/src/ và file .s trong startup/.
- Công cụ thực thi: Compiler arm-none-eabi-gcc (thông qua biến \$(CC)).
- Cơ chế trong Makefile: Make sẽ nhìn vào danh sách $(OBJS) và tìm kiếm các quy tắc biên dịch. 
Đối với mỗi file, nó gọi lệnh biên dịch riêng lẻ kèm theo các cờ \$(CFLAGS)
    - Các cờ như -mcpu=cortex-m4, -mthumb giúp Compiler biết chính xác tập lệnh mã máy nào cần tạo ra.
    - Các cờ -ffunction-sections -fdata-sections đóng gói mỗi hàm/biến vào một hộp (section) riêng biệt.
- Đầu ra: Các file .o nằm rải rác nhưng được giữ nguyên cấu trúc thư mục bên trong build/ (ví dụ: build/src/main.o). 
Lúc này, các file .o chứa mã máy nhưng chưa biết địa chỉ bộ nhớ thực tế của chúng là gì.

### 2. Giai đoạn Liên kết (Linking)
Mục tiêu: Gom tất cả các file .o riêng lẻ lại, ghép nối chúng với thư viện chuẩn và sắp xếp vào đúng bản đồ bộ nhớ của con chip.
- Đầu vào: Toàn bộ danh sách file \$(OBJS) sinh ra từ bước 1.
- Công cụ thực thi: Linker (Trình liên kết, trong Makefile này vẫn dùng arm-none-eabi-gcc đóng vai trò gọi Linker ngầm phía dưới).
- Cơ chế trong Makefile: Quy tắc \$(TARGET).elf: \$(OBJS) được kích hoạt. Make ném toàn bộ file .o kèm theo $(LDFLAGS) cho Linker.
	- Dựa vào Linker Script (-T stm32f407.ld), Linker biết Flash bắt đầu từ đâu, RAM bắt đầu từ đâu để gán địa chỉ vật lý chính xác cho từng biến, từng hàm.
	- Cờ --gc-sections sẽ dọn dẹp các "hộp" (section) được tạo ra ở bước 1 mà không có phần code nào gọi đến.
- Đầu ra: File build/006_i2c_master_receiver.elf (Executable and Linkable Format). File này đã hoàn chỉnh, 
chứa toàn bộ mã máy có địa chỉ cụ thể và cả các thông tin gỡ lỗi (debug symbols).

### 3. Giai đoạn Chuyển đổi định dạng (Format Conversion)
Mục tiêu: Loại bỏ các thông tin thừa thãi (như debug symbol) để tạo ra file mã máy thuần túy, phù hợp để nạp thẳng vào Flash của chip.
- Đầu vào: File $(TARGET).elf.
- Công cụ thực thi: arm-none-eabi-objcopy (biến \$(OBJCOPY)).
- Cơ chế trong Makefile: Quy tắc \$(TARGET).bin: \$(TARGET).elf và \$(TARGET).hex: \$(TARGET).elf. Lệnh objcopy sẽ trích 
xuất đúng phần dữ liệu thô (raw data) cần thiết từ file .elf.
- Đầu ra:
	- File .bin: Chứa chuỗi byte nhị phân thô, giống hệt những gì sẽ nằm trong bộ nhớ Flash của chip.
	- File .hex: Dữ liệu nhị phân nhưng được mã hóa dưới dạng text (Intel HEX) kèm theo thông tin địa chỉ và mã kiểm tra (checksum), 
	an toàn hơn khi truyền dẫn.

### 4. Giai đoạn Runtime (Thực thi và Gỡ lỗi)
Mục tiêu: Nạp mã máy vào vi điều khiển và theo dõi quá trình hoạt động của nó.
- Đầu vào: File .elf (nếu dùng để debug/nạp qua GDB/OpenOCD) hoặc .bin/.hex (nếu dùng các phần mềm nạp của hãng như STM32CubeProgrammer).
- Cơ chế trong Makefile:
	- Khi gọi make flash: Makefile kích hoạt OpenOCD (openocd -f ...) để giao tiếp với mạch nạp ST-Link. OpenOCD đọc file .elf, lấy dữ liệu và 
	bắn qua dây cáp xuống bộ nhớ Flash của STM32, sau đó reset chip để code bắt đầu chạy.
	- Khi gọi make debug: Bật cổng kết nối. Bạn dùng Terminal thứ 2 gọi GDB (arm-none-eabi-gdb), 
	load file .elf để GDB lấy thông tin debug (biết dòng code C nào tương ứng với lệnh máy nào).
	- Đặc biệt với Semihosting: Nếu bạn set USE_SEMIHOSTING = 1, trong lúc Runtime, mỗi khi chip 
	chạy đến lệnh printf, thay vì xuất ra chân UART, chip sẽ gửi tín hiệu tạm dừng. ST-Link và OpenOCD 
	sẽ tóm lấy dữ liệu đó và in thẳng ra màn hình Terminal trên máy tính của bạn. Mạch STM32 
	lúc này "vay mượn" khả năng I/O của máy tính chủ.

## Hướng dẫn cách sử dụng Makefile

### Quy trình từng bước để debug:
1. Mở 2 terminal: Terminal 1 - OpenOCD Sever, Terminal 2 - GDB Client
- Khi mở terminal 1 gọi lệnh sau: openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
- Khi mở terminal 2 gọi lệnh sau: arm-none-eabi-gdb build/<tên_file>.elf
2. Tiếp theo, các thao tác debug sẽ thực hiện chủ yếu tại terminal 2 - GDB Client
- Chạy lệnh: target remote localhost:3333 : Kết nối GDB tới mạch STM32 thông qua cổng của OpenOCD.
- monitor reset halt : Ra lệnh cho chip reset lại từ đầu và lập tức tạm dừng (halt) mọi hoạt động, chờ lệnh tiếp theo.
- monitor arm semihosting enable (Tùy chọn): Nếu người dùng muốn sử dụng semihosting thì phải thực hiện câu lệnh này, nếu không muốn thì bỏ qua.
- load : Nạp file .elf (đã nạp vào GDB) thẳng xuống bộ nhớ Flash của chip.
- monitor reset halt: Reset lại từ đầu và nhảy vào đầu chương trình main
3. Dùng các lệnh để debug như: b, c, n, s, p, info register, ...

**NOTE: Từ khóa "monitor" để ra lệnh trực tiếp cho OpenOCD thông qua GDB Command**

### Quy trình từng bước để biên dịch và nạp code
``` bash
make -f Makefile.mak clean		
make -f Makefile.mak all APP_NAME=<tên_file_muốn_chạy_trong_thư_mục_src> USE_SEMIHOSTING=<0 hoặc 1>
make -f Makefile.mak flash APP_NAME=<tên_file_muốn_chạy_trong_thư_mục_src> USE_SEMIHOSTING=<0 hoặc 1>	
```