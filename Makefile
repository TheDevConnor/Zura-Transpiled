include config.mk

all: $(EXEC)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(EXEC): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -rf build debug release

run: $(EXEC)
	@$(EXEC) build zura_files/main.zu -save $(if $(findstring debug,$(BUILD)),-debug) -name main

valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--log-file=valgrind-out.txt $(EXEC) build zura_files/main.zu -name main

install: $(EXEC)
	@echo "Installing to $(INSTALL_DIR)"
	@install -Dm755 $(EXEC) $(INSTALL_DIR)/zura

test: $(EXEC)
	@python3 run_tests.py

-include $(OBJECTS:.o=.d)

.PHONY: all clean run valgrind install test
