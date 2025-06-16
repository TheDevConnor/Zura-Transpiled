include config.mk

all: $(EXEC)

$(EXEC): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(EXEC)

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

.PHONY: all clean run valgrind install test