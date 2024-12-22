# Project I - IT3150

Phần mềm bẻ khoả file zip

Sinh viên: Huỳnh Đoàn Minh Đức - 20225179

# Build phần mềm

```
mkdir build
cd build
cmake ..
cmake --build .
```

Sau quá trình build sẽ xuất hiện 2 phần mềm mới `ziptool` và `benchmark`

# ziptool

Phần mềm sử dụng để xem thông tin, xem nội dung, và tấn công file zip
```
ziptool [ARGS]
[ARGS]:
  -h [help]: print this help
  -m [mode name]: one of "[b]ruteforce", "[l]ist", "[e]xtract"
  -i [index]: the index of the file to extract/bruteforce, refer to "list" mode to find which index is needed
  -p [path to password] file: specify the file containing password use to extract file, only used in "extract" mode
  -j [number]: number of thread to use, only used in password "bruteforce" mode
  -d [DICTIONARY]: dictionary, only used in password "bruteforce" mode
  -f [file_path]: path to the zip file
[DICTIONARY]:
  dictionary is a collection of file, each may be used as binary mode or text mode
  specify pattern for dictionary, use {[mode]:[file_path]}
  [mode] may be 't' or 'b'
  [file_path] is the path to the file, use \ to escape character (e.g \\, \{, \}, \:, \[, \])
  only support ascii file path, unicode path may get the program to terminate
```

# benchmark

Phần mềm sử dụng để so sánh thời gian chạy khi tấn công một file zip với từ điển cho trước, với số lượng CPU core khác nhau
```
benchmark [DICTIONARY] [PATH_TO_ZIP_FILE]
[DICTIONARY]:
  dictionary is a collection of file, each may be used as binary mode or text mode
  specify pattern for dictionary, use {[mode]:[file_path]}
  [mode] may be 't' or 'b'
  [file_path] is the path to the file, use \ to escape character (e.g \\, \{, \}, \:, \[, \])
  only support ascii file path, unicode path may get the program to terminate
```