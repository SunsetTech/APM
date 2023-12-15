PROJECT_NAME := Tests

COMPILER := g++
COMPILEFLAGS := -DCL_TARGET_OPENCL_VERSION=210 -MMD -std=c++20 -c -I../Library/Headers/ -I../tinywavBuild/tinywav/ -I../OpenCL/Common/ -O3
LINKFLAGS := -lm -lOpenCL -L../tinywavBuild/ -ltinywav -L../Library/ -lAPM
BUILD := Build
SOURCE := Source
SOURCES := $(shell find $(SOURCE) -name '*.cpp')
OBJECTS := $(patsubst $(SOURCE)/%,$(BUILD)/%,$(SOURCES:.cpp=.o))
NAME := $(PROJECT_NAME)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(COMPILER) $(OBJECTS) -o $@ $(LINKFLAGS)

$(BUILD)/%.o: $(SOURCE)/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(COMPILEFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo Scrub a dub dub...
	@rm -rf Build
	@rm -f $(NAME)
	@echo There! All clean! ^-^

.PHONY: print
print:
	@echo $(SOURCE)
	@echo $(SOURCES)
	@echo $(BUILD)
	@echo $(OBJECTS)

-include $(OBJECTS:.o=.d)
