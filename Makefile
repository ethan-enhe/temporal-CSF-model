
# Makefile

# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -Wall -Wextra -O2

# 链接选项
LDFLAGS = `pkg-config --cflags --libs opencv4`

# 源文件
SRCS = demo.cpp

# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 可执行文件名
TARGET = demo

# 默认目标
all: $(TARGET)

# 生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

# 生成目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

# 清理目标文件和可执行文件
clean:
	rm -f $(OBJS) $(TARGET)
