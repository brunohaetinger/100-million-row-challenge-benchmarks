.PHONY: test generate run clean

test:
	bun run src/main.js test/test_small.txt
	@echo ""
	@echo "Output:"
	@cat output.json | python3 -m json.tool

generate:
	./generate.sh 100000000

run:
	./run.sh measurements.txt

clean:
	rm -f measurements.txt output.json
