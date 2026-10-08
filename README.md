# Midterm Project: Implementation of ls Utility

**Học phần:** Lập Trình Hệ Thống (System Programming)  
**Sinh viên thực hiện:** Đoàn Thanh Huy  
**Mã số sinh viên:** 24IT099  
**GitHub Repository:** [https://github.com/Hermess1309/DoanThanhHuy_24IT099_midterm.git](https://github.com/Hermess1309/DoanThanhHuy_24IT099_midterm.git)  
**Môi trường thực thi:** NetBSD 11.0 (amd64) trên Oracle VM VirtualBox

---

## 1. Giới thiệu dự án (Introduction)

Dự án hiện thực lại tiện ích dòng lệnh `ls` theo đặc tả trang hướng dẫn (manual page) của hệ điều hành NetBSD. Tiện ích cho phép liệt kê nội dung của các thư mục và thông tin chi tiết của các tập tin trong hệ thống, hỗ trợ đầy đủ tập tùy chọn theo yêu cầu, xử lý các trường hợp biên (edge cases) và đảm bảo tính bền vững (robustness), không xảy ra lỗi segmentation fault hay rò rỉ bộ nhớ.

---

## 2. Cấu trúc mã nguồn (Source Code Organization)

Chương trình được thiết kế theo cấu trúc module hóa cao (modular design), tách biệt rõ ràng giữa các tệp nguồn (`.c`) và tệp tiêu đề (`.h`):

```text
DoanThanhHuy_24IT099_midterm/
├── include/            # Thư mục chứa các tệp tiêu đề (.h)
│   ├── display.h       # Khai báo hiển thị kết quả (cột đơn, -l, -n, total)
│   ├── entry.h         # Khai báo thu thập thông tin metadata của tệp/thư mục
│   ├── ls.h            # Định nghĩa cấu trúc dữ liệu chính (ls_options_t, file_entry_t)
│   ├── options.h       # Khai báo hàm khởi tạo và phân tích tùy chọn dòng lệnh
│   ├── sort.h          # Khai báo hàm sắp xếp mảng entries
│   └── traverse.h      # Khai báo duyệt operands và duyệt cây thư mục
├── src/                # Thư mục chứa mã nguồn (.c)
│   ├── display.c       # Hiện thực căn chỉnh cột, in ký tự đặc biệt (-q, -w), định dạng chuẩn
│   ├── entry.c         # Hiện thực lstat, định dạng mode, owner, size, timestamp, suffix
│   ├── main.c          # Điểm nhập của chương trình, phối hợp các module và mã thoát
│   ├── options.c       # Hiện thực xử lý cờ getopt và các quy tắc ghi đè (overrides)
│   ├── sort.c          # Hiện thực qsort theo tên, kích thước (-S), thời gian (-t), đảo ngược (-r)
│   └── traverse.c      # Hiện thực phân loại operands, mở thư mục opendir/readdir, đệ quy (-R)
├── .gitignore          # Cấu hình bỏ qua các tệp nhị phân và đối tượng biên dịch (.o)
├── Makefile            # Makefile biên dịch dự án với gcc, cờ cảnh báo nghiêm ngặt
└── README.md           # Báo cáo và tài liệu hướng dẫn dự án
```

---

## 3. Danh sách các tùy chọn đã hiện thực (Implemented Options)

Chương trình hỗ trợ đầy đủ cú pháp:

```bash
ls [-AacdFfhiklnqRrSstuw] [file ...]
```

| Tùy chọn | Mô tả chi tiết theo NetBSD Manual                                                                                                                                                      |
| :------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|  **-A**  | Liệt kê tất cả các mục ngoại trừ `.` và `..`. Tự động bật đối với tài khoản quản trị (superuser UID = 0).                                                                              |
|  **-a**  | Liệt kê tất cả các mục trong thư mục, kể cả các mục có tên bắt đầu bằng dấu chấm (`.`).                                                                                                |
|  **-c**  | Sử dụng thời gian thay đổi trạng thái tệp (`st_ctime`) thay cho thời gian sửa đổi gần nhất (`st_mtime`) để sắp xếp (`-t`) hoặc hiển thị (`-l`). Ghi đè hoặc bị ghi đè bởi `-u`.        |
|  **-d**  | Liệt kê thư mục như tệp thông thường (không duyệt đệ quy vào trong). Không truy vết symbolic link truyền qua tham số. Ghi đè hoặc bị ghi đè bởi `-R`.                                  |
|  **-F**  | Thêm ký tự phân loại vào sau tên tệp: `/` (thư mục), `*` (thực thi), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout).                                                         |
|  **-f**  | Không thực hiện sắp xếp danh sách kết quả (giữ nguyên thứ tự đọc từ đĩa). Trong chuẩn BSD, cờ này cũng bật cờ `-a`.                                                                    |
|  **-h**  | Định dạng kích thước tệp và block theo dạng đọc được cho người dùng (human-readable: B, K, M, G, ...). Áp dụng cho `-l` và `-s`. Ghi đè cờ `-k`.                                       |
|  **-i**  | In số inode (`st_ino`) của mỗi tệp ở đầu dòng.                                                                                                                                         |
|  **-k**  | Hiển thị kích thước block theo đơn vị kilobytes (1024 bytes) cho tùy chọn `-s`. Bị ghi đè bởi cờ `-h` nếu `-h` đứng sau.                                                               |
|  **-l**  | Hiển thị định dạng chi tiết (long format) gồm: file mode, số link, chủ sở hữu, nhóm, kích thước, thời gian, tên tệp (và đích của symlink). Hiển thị dòng `total <blocks>` cho thư mục. |
|  **-n**  | Tương tự `-l`, nhưng hiển thị UID và GID dạng số thay vì tra cứu tên người dùng/nhóm. Ghi đè hoặc bị ghi đè bởi `-l`.                                                                  |
|  **-q**  | Chuyển đổi các ký tự không in được trong tên tệp thành dấu hỏi `?`. Đây là hành vi mặc định khi xuất ra terminal. Ghi đè hoặc bị ghi đè bởi `-w`.                                      |
|  **-R**  | Duyệt đệ quy tất cả các thư mục con gặp phải. Ghi đè hoặc bị ghi đè bởi `-d`.                                                                                                          |
|  **-r**  | Đảo ngược thứ tự sắp xếp (ngược bảng chữ cái, nhỏ nhất trước, hoặc cũ nhất trước).                                                                                                     |
|  **-S**  | Sắp xếp tệp theo kích thước giảm dần (tệp lớn nhất đứng đầu). Nếu kích thước bằng nhau, sắp theo bảng chữ cái. Ghi đè hoặc bị ghi đè bởi `-t`.                                         |
|  **-s**  | Hiển thị số block hệ thống tập tin được sử dụng cho mỗi tệp (mặc định 512 bytes hoặc theo biến môi trường `BLOCKSIZE`, hoặc `-k` / `-h`). In tổng block cho thư mục.                   |
|  **-t**  | Sắp xếp tệp theo thời gian sửa đổi (mới nhất đứng đầu) trước khi sắp theo thứ tự bảng chữ cái. Ghi đè hoặc bị ghi đè bởi `-S`.                                                         |
|  **-u**  | Sử dụng thời gian truy cập gần nhất (`st_atime`) thay cho mtime để sắp xếp (`-t`) hoặc in (`-l`). Ghi đè hoặc bị ghi đè bởi `-c`.                                                      |
|  **-w**  | Cho phép in thô (raw bytes) các ký tự không in được. Mặc định khi output không phải là terminal. Ghi đè hoặc bị ghi đè bởi `-q`.                                                       |

### Quy tắc ưu tiên và ghi đè (Mutual Overrides)

Theo hướng dẫn của NetBSD `ls`:

- `-w` và `-q`: Cờ xuất hiện sau cùng sẽ quyết định định dạng cho ký tự không in được.
- `-l` và `-n`: Cờ xuất hiện sau cùng sẽ quyết định định dạng hiển thị chi tiết.
- `-c` và `-u`: Cờ xuất hiện sau cùng sẽ quyết định trường thời gian sử dụng.
- `-R` và `-d`: Cờ xuất hiện sau cùng sẽ quyết định hành vi duyệt thư mục.
- `-k` và `-h`: Cờ bên phải nhất (sau cùng) sẽ quyết định cách tính và hiển thị kích thước block.
- `-S` và `-t`: Cờ bên phải nhất sẽ quyết định tiêu chí sắp xếp.

---

## 4. Hướng dẫn biên dịch và thực thi (Compilation and Usage)

### 4.1. Yêu cầu môi trường

- Hệ điều hành: NetBSD (hoặc POSIX-compliant UNIX / Linux).
- Trình biên dịch: `gcc` hoặc `clang` hỗ trợ C99.
- Thư viện: libc, libutil (cho `humanize_number`).

### 4.2. Biên dịch

Để biên dịch chương trình, sử dụng lệnh `make`:

```bash
make
```

Lệnh trên sẽ biên dịch các tệp nguồn thành tệp thực thi `ls` với cờ tối ưu hóa `-O2` và các cờ kiểm tra cảnh báo `-Wall -Wextra -pedantic -std=c99`.

Để xóa các tệp đối tượng `.o` và binary sau khi biên dịch:

```bash
make clean
```

Để chạy bộ kiểm tra nhanh:

```bash
make test
```

### 4.3. Các ví dụ thực thi

```bash
# 1. Liệt kê cơ bản
./ls

# 2. Liệt kê chi tiết đầy đủ tệp ẩn
./ls -la

# 3. Liệt kê định dạng chi tiết kèm kích thước đọc được (human-readable)
./ls -lh

# 4. Liệt kê kèm số inode và số block
./ls -lis

# 5. Liệt kê định dạng số UID/GID
./ls -n

# 6. Liệt kê kèm ký tự phân loại loại tệp
./ls -F

# 7. Sắp xếp theo kích thước tệp giảm dần
./ls -lS

# 8. Sắp xếp theo thời gian mới nhất, đảo ngược thứ tự
./ls -ltr

# 9. Sử dụng ctime để sắp xếp và in chi tiết
./ls -ltc

# 10. Duyệt đệ quy toàn bộ thư mục
./ls -R

# 11. Xem thông tin chính thư mục mà không duyệt bên trong
./ls -ld dir1

# 12. Liệt kê nhiều tham số tệp và thư mục kết hợp
./ls -l file1 dir1 dir2
```

---

## 5. Xử lý trường hợp biên và độ tin cậy (Robustness and Edge Cases)

1. **Quản lý bộ nhớ:** Mọi vùng nhớ được cấp phát động (`malloc`, `realloc`, `strdup`) đều được giải phóng hoàn chỉnh (`free`) sau khi sử dụng, ngăn chặn hiện tượng rò rỉ bộ nhớ (memory leak).
2. **Xử lý lỗi hệ thống:** Mọi lệnh gọi hệ thống (`opendir`, `readdir`, `lstat`, `stat`, `readlink`) đều được kiểm tra mã trả về. Khi xảy ra lỗi truy cập (ví dụ tập tin không tồn tại, không đủ quyền hạn):
   - In thông báo lỗi chuẩn xác ra luồng `stderr`: `ls: <filename>: <strerror(errno)>`.
   - Cập nhật mã thoát `exit_status = 1` nhưng **không làm đổ vỡ chương trình**, tiếp tục xử lý các tham số tiếp theo theo đúng chuẩn POSIX.
3. **Phân loại tham số:** Tách biệt chính xác giữa các tham số là tệp và thư mục; hiển thị tệp trước, sau đó hiển thị các thư mục kèm tiêu đề ngăn cách rõ ràng.
4. **Hỗ trợ tệp đặc biệt:** Hiển thị đúng số major/minor cho thiết bị khối và thiết bị ký tự, đích liên kết cho symlink, định dạng quyền đặc biệt (`setuid`, `setgid`, `sticky bit`).

---
