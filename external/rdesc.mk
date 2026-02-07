TARGET_DIR = target
export PREFIX = $(PWD)

default: include/rdesc
	@echo > /dev/null

$(TARGET_DIR)/rdesc: $(TARGET_DIR)
	git clone https://github.com/metwse/rdesc --branch v0.1.x $@

include/rdesc: $(TARGET_DIR)/rdesc
	$(MAKE) -C $(TARGET_DIR)/rdesc FEATURES='stack dump_dot dump_bnf' install

$(TARGET_DIR) $(PREFIX):
	mkdir -p $@

.PHONY: default
