# How to Flash FPGA of Zybo Board with Bitstream

1. Prepare U-Boot and Partition on SD-Card according to [this instructions](https://argos-research.github.io/documentation/u-boot.html#digilent-zybo)
2. Copy Genode image `genode.elf` to SD-Card
3. Copy `fpga/rtcr_hw.bit` to SD-Card
4. Power on Zybo Board and open U-Boot console.
5. Flash FPGA with `rtcr_hw.bit`
   ```
   fatload mmc 0:1 ${scriptaddr} rtcr_hw.bit; fpga loadb 0 ${scriptaddr} 9000000
   ```
6. Boot Genode
   ```
   fatload mmc 0:1 ${scriptaddr} genode.elf; bootelf ${scriptaddr}
   ```
