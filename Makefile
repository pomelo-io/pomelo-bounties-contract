.PHONY: all
all:
	make build

.PHONY: build
build:
	cdt-cpp work.pomelo.cpp -I include

