# Light Crystals WLED Firmware

To flash your Light Crystals Kit with WLED please put it into [download mode](https://help.unexpectedmaker.com/index.php/knowledge-base/how-to-put-your-board-into-download-mode/) and then locate your serial port based on your OS and execute the command below after replacing XXX with the correct port:

`esptool.py --chip esp32s3 --port XXX --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin`

If you dont have the esptool.py tool, you can install it using pip: `pip install esptool` or download it from the [official website](https://github.com/espressif/esptool).

If you would like to build WLED from source, please follow the instructions on the [WLED Wiki](https://github.com/Aircoookie/WLED/wiki/Build-instructions).