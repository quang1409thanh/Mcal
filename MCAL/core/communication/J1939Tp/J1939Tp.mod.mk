#J1939Tp
obj-$(USE_J1939TP) += J1939Tp.o
#obj-$(USE_J1939TP) += J1939Tp_LCfg.o
obj-$(USE_J1939TP) += J1939Tp_PBcfg.o
inc-$(USE_J1939TP) += $(ROOTDIR)/communication/J1939Tp
vpath-$(USE_J1939TP) += $(ROOTDIR)/communication/J1939Tp