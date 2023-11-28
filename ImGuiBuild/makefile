PROJECT_NAME := imgui

COMPILER := g++
COMPILEFLAGS := -MMD -c -Iimgui/
LINKFLAGS := 
BUILD := Build
SOURCE := imgui
SOURCES := $(SOURCE)/imgui.cpp $(SOURCE)/imgui_demo.cpp $(SOURCE)/imgui_draw.cpp $(SOURCE)/imgui_tables.cpp $(SOURCE)/imgui_widgets.cpp $(SOURCE)/backends/imgui_impl_glfw.cpp $(SOURCE)/backends/imgui_impl_opengl3.cpp
OBJECTS := $(patsubst $(SOURCE)/%,$(BUILD)/%,$(SOURCES:.cpp=.o))
OUTPUTNAME := lib$(PROJECT_NAME).a

all: $(OUTPUTNAME)

$(OUTPUTNAME): $(OBJECTS)
	ar rcs $@ $^

$(BUILD)/%.o: $(SOURCE)/%.cpp
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

-include $(OBJECTS:.o=.d)
