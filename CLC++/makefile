PROJECT_NAME := Kernels

BuildDir := Build
SourceDir := Source
SourceDirS := $(shell find $(SourceDir) -name '*.cl.cpp')
Objects := $(patsubst $(SourceDir)/%,$(BuildDir)/%,$(SourceDirS:.cl.cpp=.bc))
LLVM_Name := $(PROJECT_NAME).bc
SPIRV_Name := $(PROJECT_NAME).spv
OutputName := $(SPIRV_Name)

Compiler := clang
Linker := llvm-link
SPIRV_Generator := /opt/intel/oneapi/compiler/latest/linux/bin-llvm/llvm-spirv

CompileFlags := -MMD -c -emit-llvm -cl-std=CLC++ --target=gfx803-amd-amdhsa --include-directory=Source/Common/
LinkFlags :=

all: $(SPIRV_Name)

$(BuildDir)/%.bc: $(SourceDir)/%.cl.cpp
	@mkdir -p $(dir $@)
	$(Compiler) $(CompileFlags) $< -o $@

$(BuildDir)/$(LLVM_Name): $(Objects)
	$(Linker) $(Objects) -o $@ $(LinkFlags)

$(SPIRV_Name): $(BuildDir)/$(LLVM_Name)
	$(SPIRV_Generator) $(BuildDir)/$(LLVM_Name) -o $@

.PHONY: clean
clean:
	@echo Scrub a dub dub...
	@rm -rf Build
	@rm -f $(OutputName)
	@echo There! All clean! ^-^

.PHONY: print
print:
	@echo $(SourceDir)
	@echo $(SourceDirS)
	@echo $(BuildDir)
	@echo $(Objects)

-include $(Objects:.bc=.d)
