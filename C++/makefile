PROJECT_NAME := FDM_Synth

COMPILER := g++
COMPILEFLAGS := -MMD -c -I../tinywav/
LINKFLAGS := -lm -lOpenCL -LBuild/ -ltinywav
BUILD := Build
SOURCE := Source
SOURCES := $(shell find $(SOURCE) -name '*.cpp')
OBJECTS := $(patsubst $(SOURCE)/%,$(BUILD)/%,$(SOURCES:.cpp=.o))
NAME := $(PROJECT_NAME)

all: $(NAME)

$(BUILD)/tinywav.o: ../tinywav/tinywav.c
	$(COMPILER) -c ../tinywav/tinywav.c -o $@

$(BUILD)/libtinywav.a: $(BUILD)/tinywav.o
	ar rcs $@ $(BUILD)/tinywav.o

$(NAME): $(OBJECTS) $(BUILD)/libtinywav.a
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
