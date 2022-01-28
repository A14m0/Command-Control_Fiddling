current_dir =  $(shell pwd)

build_docker_image:
	docker build -t clean/builder .
 
build_target:
	echo "[ ] Purging old build..."
	rm -rf build
	mkdir build 
	cd build 
	docker run --rm -it -v $(current_dir):/mnt clean/builder /bin/bash -c "eval 'cd /mnt/build && cmake .. && make && chgrp -R users *'"