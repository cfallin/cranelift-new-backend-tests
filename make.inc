# set NAME and either C_SRC or RS_SRC, then include this file.

RUSTC_WASI := rustc --target wasm32-wasi -O
CC_WASI := /opt/wasi-sdk/bin/clang -O2

.PHONY: all
all: $(NAME).wasm

SRC := $(C_SRC) $(RS_SRC)

ifneq ($(RS_SRC),)

$(NAME).wasm: $(RS_SRC)
	$(RUSTC_WASI) -o $@ $^

else

$(NAME).wasm: $(C_SRC)
	$(CC_WASI) -o $@ $^

endif

.PHONY: clean
clean:
	rm -f $(NAME).wasm
