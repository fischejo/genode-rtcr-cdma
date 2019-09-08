LIBS += profiler

include $(REP_DIR)/lib/mk/rtcr_cdma.mk

# disable any debugging log to improve logging performance
CC_OPT += -UVERBOSE
CC_OPT += -UDEBUG

# enable profiling log
CC_OPT += -DPROFILE
