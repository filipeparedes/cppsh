.PHONY: all run clean

all:
	mkdir -p build
	cd build && cmake .. && make

run: all
	./build/cppsh

clean:
	rm -rf build