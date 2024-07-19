# Makefile

# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -Wall -Wextra -O2

# 链接选项
LDFLAGS = `pkg-config --cflags --libs opencv4`

# 源文件目录
SRC_DIR = src

# 目标文件目录
BUILD_DIR = build

# 源文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# 目标文件
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# 可执行文件名
TARGET = $(BUILD_DIR)/demo

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

# 生成目标文件目录
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)
