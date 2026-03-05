.PHONY: install test generate run clean

install:
	bundle install

test: install
	ruby src/main/ruby/main.rb test_small.txt
	@echo ""
	@echo "Output:"
	@cat output.json | jq .

generate:
	./generate.sh 100000000

run: install
	./run.sh measurements.txt

clean:
	rm -f measurements.txt output.json
