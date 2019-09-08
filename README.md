# Introduction

This is the Genode repository `rtcr_fpga` which extends the Genode core
repository `rtcr` of RTCR (Real Time Checkpoint Restore) implementation. It
provides FPGA implementation for hardware accelerated copying of dataspaces and
parsing of capbilities.

# Genode Repository Dependencies
This repository depends on following Genode repositories
1. `rtcr` 
2. `profiler`

## Modules

| **Module** | **Description** | **Optional** | **Dependencies** |
| --- | --- | --- | --- |
| `core_kcap` | FPGA-based Capability parsing | | `ds` |
| `ds_cdma` | FPGA-based memory copying acceleration. This module replaces the module `ds`. | | |


# Installation
2. Register `rtcr_fpga` Repository in `build.conf`
   ```bash
   # change directory to `etc` of your build directory
   echo 'REPOSITORIES += $(GENODE_DIR)/repos/rtcr_fpga' >> build.conf
   ```
   
# Module Configuration

## rtcr_ds_cdma
The module `rtcr_ds_cdma` inherits from `rtcr_ds` and replaces the default
software-based dataspace copying by a FPGA implementation. This module is optional.

Add `rtcr_cdma` library to the dependencies of the `rtcr` library.
```bash
# change directory to root of Genode directory
cd repos/rtcr/lib/mk/
echo 'LIBS += rtcr_ds_cdma' >> rtcr.mk
```

```diff
<start name="rtcr_app">
	<resource name="RAM" quantum="100M"/>
	<config>
-		<module name="ds"  />
+		<module name="ds_cdma" />
	</config>
</start>
```

**Read [CDMA Driver](./doc/cdma_drv/cdma_drv.md) for the CDMA driver configuration**

## rtcr_core_fpga
The module `rtcr_core_fpga` inherits from `rtcr_core` and repalces the
cabability parsing in `core_module_pd.cc` by a FPGA implementation. This module
is optional.

Add `rtcr_kcap` library to the dependencies of the `rtcr` library.
```bash
# change directory to root of Genode directory
cd repos/rtcr/lib/mk/
echo 'LIBS += rtcr_core_kcap >> rtcr.mk
```

```diff
<start name="rtcr_app">
	<resource name="RAM" quantum="100M"/>
	<config>
-		<module name="core" />	
+		<module name="core_kcap" />
	</config>
</start>
```
**Read [KCAP Driver](./doc/kcap_drv/kcap_drv.md) for the KCAP driver configuration**


## Documentation
All documentation is in directory `doc`.

* Hardware Accelerated Implementation
  * [CDMA Driver](./doc/cdma_drv/cdma_drv.md)
  * [KCAP Driver](./doc/kcap_drv/kcap_drv.md)
  * [How to Create Bitstream for FPGA](./doc/rtcr_hw/create_bitstream.md)
  * [How to Flash FPGA with Bitstream](./doc/rtcr_hw/flash_bitstream.md)
