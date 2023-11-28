PROJECT_NAME := tinywav
COMPILER := gcc
COMPILEFLAGS := -MMD -c -Itinywav/
LINKFLAGS := 
BUILD := Build
SOURCE := tinywav
SOURCES := $(SOURCE)/tinywav.c
OBJECTS := $(patsubst $(SOURCE)/%,$(BUILD)/%,$(SOURCES:.c=.o))
OUTPUTNAME := lib$(PROJECT_NAME).a

all: $(OUTPUTNAME)

$(OUTPUTNAME): $(OBJECTS)
	ar rcs $@ $^

$(BUILD)/%.o: $(SOURCE)/%.c
	@mkdir -p $(dir $@)
	$(COMPILER) $(COMPILEFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo Scrub a dub dub...
	@rm -r $(BUILD)
	@rm -f $(OUTPUTNAME)
	@echo There! All clean! ^-^

.PHONY: print
print:
	@echo $(SOURCE)
	@echo $(SOURCES)
	@echo $(BUILD)
	@echo $(OBJECTS)
