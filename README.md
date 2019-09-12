# Introduction

This is the Genode repository `rtcr_cdma` which extends the Genode core
repository `rtcr` of RTCR (Real Time Checkpoint Restore) implementation.  The
module `rtcr_cdma` inherits from `rtcr` and replaces the default software-based
dataspace copying by a FPGA implementation.


# Dependencies
This repository depends on following Genode repositories
1. `rtcr` 


# Installation

Register `rtcr_cdma` Repository in `build.conf`
```bash
# change directory to `etc` of your build directory
echo 'REPOSITORIES += $(GENODE_DIR)/repos/rtcr_cdma' >> build.conf
```
   
# Linking

Add `rtcr_cdma` library to the dependencies of the `rtcr` library.
```bash
# change directory to root of Genode directory
cd repos/rtcr/lib/mk/
echo 'LIBS += rtcr_cdma' >> rtcr.mk
```

# Configuration

```diff
 <start name="rtcr_app">
     <config>
+        <module name="cdma"/>
     </config>
 </start>
```

Read [CDMA Driver](./doc/cdma_drv/cdma_drv.md) for the CDMA driver
configuration.
	 

## Documentation
All documentation is in directory `doc`.

* Hardware Accelerated Implementation
  * [CDMA Driver](./doc/cdma_drv/cdma_drv.md)
  * [How to Create Bitstream for FPGA](./doc/rtcr_hw/create_bitstream.md)
  * [How to Flash FPGA with Bitstream](./doc/rtcr_hw/flash_bitstream.md)
