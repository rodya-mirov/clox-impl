compile:
	@echo "Compiling CLOX..."
	@gcc *.c -o clox
	@chmod +x ./clox
	@echo "CLOX compiled successfully"
	@echo

run: compile
	@echo Running CLOX...
	@./clox
	@echo

clean:
	@rm clox

.DEFAULT_GOAL := compile
