esptool --chip esp32s3 merge_bin --output cplt.bin \
0x0 .pio/build/default/bootloader.bin \
0x8000 .pio/build/default/partitions.bin \
0x9000 scripts/UiFlow2_nvs.bin \
0x10000 .pio/build/default/firmware.bin
