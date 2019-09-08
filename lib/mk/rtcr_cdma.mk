SRC_CC = ram_cdma_session.cc cdma_module.cc

vpath % $(REP_DIR)/src/rtcr_cdma

CC_OPT += -DVERBOSE
CC_OPT += -DDEBUG

LIBS   += config

