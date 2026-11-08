PLUGIN_NAME := Appeal_Default_2D_Top_Down_Mechanics
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
ifeq ($(OS),Windows_NT)
  PLATFORM := windows
  TARGET := $(PLUGIN_NAME).dll
  TEST := obj/$(PLATFORM)/Mechanics_Tests.exe
else
  PLATFORM := linux
  TARGET := $(PLUGIN_NAME).so
  TEST := obj/$(PLATFORM)/Mechanics_Tests
  CXXFLAGS += -fPIC
endif
OBJ := obj/$(PLATFORM)/main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ): src/main.cpp src/Mechanics.h src/Orientation.h $(wildcard src/*.h)
	@mkdir -p obj/$(PLATFORM)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TEST): tests/Mechanics_Tests.cpp $(OBJ) src/Mechanics.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ tests/Mechanics_Tests.cpp $(OBJ) $(LDLIBS)

test: $(TEST)
	./$(TEST)

.PHONY: all test
