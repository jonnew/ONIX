SBY_FILES = $(wildcard *.sby)
BUILD_FOLDERS = $(basename $(SBY_FILES))

.PHONY: formal
formal: $(SBY_FILES)
	$(foreach file, $(SBY_FILES), sby -f $(file);)

.PHONY: clean
clean:
	rm -rf $(BUILD_FOLDERS)
#	rm -f *.check
