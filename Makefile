current_dir =  $(shell pwd)
build_dir = ./build

build_docker_image:
	@docker build -t clean/builder .
 
build_target: | $(build_dir)
	@docker run --rm -it -v $(current_dir):/mnt clean/builder /bin/bash -c "eval 'cd /mnt/build && cmake -lpthread .. && make'"

test: | build_target
	@./build/tests/tests

$(build_dir):
	@mkdir build