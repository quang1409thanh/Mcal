$(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES += $(CFG_OUTPUT_DIR)/Com_Cfg.h
$(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES += $(CFG_OUTPUT_DIR)/Com_Cfg.c
$(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES += $(CFG_OUTPUT_DIR)/Com_PBCfg.h
$(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES += $(CFG_OUTPUT_DIR)/Com_PBCfg.c
$(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES += $(CFG_OUTPUT_DIR)/Com.mk

$(PROJECT_NAME)_CONFIG_FILES += $($(PROJECT_NAME)_$(COM_MODULE_NAME)_CONFIG_FILES)