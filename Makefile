.PHONY: all run clean

all:
	mkdir -p build
	cd build && cmake .. && make

run: all
	./build/cppsh

test: all
	./build/tests

clean:
	rm -rf build