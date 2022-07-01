compile:
	@echo "Compiling CLOX..."
	@gcc *.c -o clox
	@chmod +x ./clox
	@echo "CLOX compiled successfully"

run:
	@./clox

clean:
	@rm clox

.DEFAULT_GOAL := compile
