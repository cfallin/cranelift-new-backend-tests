.PHONY: all
all: regex-rs.wasm

regex-rs.wasm: src/main.rs
	cargo build --release --target=wasm32-wasi
	cp target/wasm32-wasi/release/regex-rs.wasm .

.PHONY: clean
clean:
	rm -rf target/
	rm -f regex-rs.wasm
