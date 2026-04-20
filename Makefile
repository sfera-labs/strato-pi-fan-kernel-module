MODULE_MAIN_OBJ := module.o
UDEV_RULES := 99-stratopifan.rules

SOURCE_DIR := $(if $(src),$(src),$(CURDIR))
include $(SOURCE_DIR)/commons/scripts/kmod-common.mk
