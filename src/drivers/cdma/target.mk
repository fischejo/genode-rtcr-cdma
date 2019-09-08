# \brief  Driver for CDMA implementation running on FPGA of Zybo Board.
# \author Johannes Fischer
# \date   2018-12-16

TARGET   = cdma_drv

SRC_CC   = main.cc driver.cc
LIBS     = base
INC_DIR += $(PRG_DIR)

vpath main.cc $(PRG_DIR)

# In order to enable verbosity:
#CC_OPT += -DVERBOSE

# in order to enable debug output
#CC_OPT += -DDEBUG


CC_CXX_WARN_STRICT =
