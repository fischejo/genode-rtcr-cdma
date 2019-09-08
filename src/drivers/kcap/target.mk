# \brief  Driver for harware-based capability parsing running on the FPGA of the Zybo Board.
# \author Johannes Fischer
# \date   2019-01-09

TARGET   = kcap_drv
LIBS   += base
SRC_CC   = main.cc driver.cc

INC_DIR += $(BASE_DIR)/../base-foc/src/include
          
vpath main.cc $(PRG_DIR)

# In order to enable verbosity:
#CC_OPT += -DVERBOSE

# in order to enable debug output
#CC_OPT += -DDEBUG
