PROJECT_NAME := APM

COMPILER := g++
COMPILEFLAGS := -DCL_TARGET_OPENCL_VERSION=200 -MMD -std=c++20 -c -I../tinywavBuild/tinywav/ -I../ImGuiBuild/imgui/ -I../OpenCL/Common/
LINKFLAGS := -lm -lOpenCL -L../tinywavBuild/ -ltinywav -L../ImGuiBuild/ -limgui -lglfw -lGL
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
