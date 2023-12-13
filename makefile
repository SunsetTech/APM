all: Binary

.PHONY: imgui
imgui:
	@$(MAKE) -C ImGuiBuild

.PHONY: tinywav
tinywav:
	@$(MAKE) -C tinywavBuild

.PHONY: Library
Library:
	@$(MAKE) -C Library

.PHONY: Binary
Binary: tinywav Library
	@$(MAKE) -C Binary

.PHONY: clean
clean:
	-@$(MAKE) clean -C ImGuiBuild
	-@$(MAKE) clean -C tinywavBuild
	-@$(MAKE) clean -C Library
	-@$(MAKE) clean -C Binary
