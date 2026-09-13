CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = server
SRC = src/main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)