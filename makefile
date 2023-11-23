all: CPP CLCPP

.PHONY: CPP
CPP:
	@$(MAKE) -C C++

.PHONY: CLCPP
CLCPP:
	@$(MAKE) -C CLC++

.PHONY: clean
clean:
	@$(MAKE) clean -C C++
	@$(MAKE) clean -C CLC++
