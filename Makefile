.PHONY: all clean

CC = g++ -g -std=c++11
CFLAGS = -g
INCDIR = ./include/
SRCDIR = ./src
OBJDIR = ./obj
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
TARGET = MyFileSystem

# 目标
all: $(TARGET)

# 定义依赖关系和编译规则
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(INCDIR)/%.h
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# 定义清理规则
clean:
	rm -f $(OBJS) $(TARGET)