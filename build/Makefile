.PHONY: all clean rebuild run install

BUILD_DIR = build
EXECUTABLE = passport-cli

all: $(BUILD_DIR)/Makefile
	@$(MAKE) -C $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..

clean:
	@rm -rf $(BUILD_DIR)

rebuild: clean all

run: all
	@./$(BUILD_DIR)/$(EXECUTABLE)

install: all
	@cp $(BUILD_DIR)/$(EXECUTABLE) /usr/local/bin/$(EXECUTABLE)
	@echo "Installed to /usr/local/bin/$(EXECUTABLE)"