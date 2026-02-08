all export_version clean:
	$(MAKE) -C ./src/ $@

test:
	lua ./test/leregex_test.lua

.PHONY: all export_version clean test
