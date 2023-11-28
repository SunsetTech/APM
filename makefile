all: Binary

.PHONY: imgui
imgui:
	@$(MAKE) -C ImGuiBuild

.PHONY: tinywav
tinywav:
	@$(MAKE) -C tinywavBuild

.PHONY: Binary
Binary: imgui tinywav
	@$(MAKE) -C Binary

.PHONY: clean
clean:
	@$(MAKE) clean -C Binary
	@$(MAKE) clean -C ImGuiBuild
	@$(MAKE) clean -C tinywavBuild
