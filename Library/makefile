PROJECT_NAME := APM

COMPILER := g++
COMPILEFLAGS := -c -fPIC -DCL_TARGET_OPENCL_VERSION=210 -MMD -std=c++20 -c -I../tinywavBuild/tinywav/ -I../OpenCL/Common/ -O3
LINKFLAGS := -shared -lm -lOpenCL -L../tinywavBuild/ -ltinywav
BUILD := Build
SOURCE := Source
SOURCES := $(shell find $(SOURCE) -name '*.cpp')
OBJECTS := $(patsubst $(SOURCE)/%,$(BUILD)/%,$(SOURCES:.cpp=.o))
NAME := lib$(PROJECT_NAME).so

all: $(NAME) seperate_headers

$(NAME): $(OBJECTS)
	$(COMPILER) $(OBJECTS) -o $@ $(LINKFLAGS)

$(BUILD)/%.o: $(SOURCE)/%.cpp
	@mkdir -p $(dir $@)
	$(COMPILER) $(COMPILEFLAGS) $< -o $@

.PHONY: seperate_headers
seperate_headers:
	-@rm -r Headers
	@rsync -avm --include='*.hpp' -f 'hide,! */' ./Source/* ./Headers

.PHONY: clean
clean:
	@echo Scrub a dub dub...
	@rm -rf Build
	@rm -rf Headers
	@rm -f $(NAME)
	@echo There! All clean! ^-^

.PHONY: print
print:
	@echo $(SOURCE)
	@echo $(SOURCES)
	@echo $(BUILD)
	@echo $(OBJECTS)

-include $(OBJECTS:.o=.d)
