#Dem
obj-$(USE_DEM) += Dem.o
ifeq ($(filter Dem_Extension.o,$(obj-y)),)
obj-$(USE_DEM_EXTENSION) += Dem_Extension.o
endif
obj-$(USE_DEM) += Dem_Debounce.o
obj-$(USE_DEM) += Dem_LCfg.o
inc-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem
vpath-$(USE_DEM) += $(ROOTDIR)/diagnostic/Dem