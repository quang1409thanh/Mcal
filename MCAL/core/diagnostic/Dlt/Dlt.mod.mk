# Dlt
obj-$(USE_DLT) += Dlt.o
obj-$(USE_DLT) += Dlt_cfg.o
ifeq ($(filter Dlt_Callout_Stubs.o,$(obj-y)),)
obj-$(USE_DLT) += Dlt_Callout_Stubs.o
endif
vpath-y += $(ROOTDIR)/diagnostic/Dlt