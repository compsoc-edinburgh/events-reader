BUILD_DIR := build
BOARD := pico2_w

.PHONY: build configure clean flash

configure:
	cmake -S . -B $(BUILD_DIR) -DPICO_BOARD=$(BOARD) -G Ninja

build: configure
	cmake --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

flash: build
	cp $(BUILD_DIR)/*.uf2 /run/media/$$USER/RPI-RP2 2>/dev/null || \
	echo "Mount your Pico in BOOTSEL mode first."
