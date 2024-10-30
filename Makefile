CXX = g++
CXX_FLAGS= -std=c++20 -Wall -Iinclude -g

# LDFLAGS would have the -L<install_path>
LDFLAGS = 
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system -lfmt

TARGET = build/Microboy

SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = test

SOURCES := $(shell find $(SRCDIR) -name '*.cpp')
OBJ := $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

# Default rule
all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "Linking..."
	$(CXX) $(OBJ) -o $(TARGET) $(LDFLAGS) $(LDLIBS)
	@echo "Build complete: $(TARGET)"

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	@echo "Compiling..."
	$(CXX) $(CXX_FLAGS) -c $< -o $@

# Create build folder
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILDDIR) $(TARGET)

# Optional TESTS
test: $(TARGET)
	@echo "Running tests..."
	$(CXX) $(CXXFLAGS) $(TESTDIR)/*.cpp -o $(BUILDDIR)/test_executable

.PHONY: all clean test
