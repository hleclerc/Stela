all: clean run

run:
	nsmake run --verbose src/stela_ast.cpp

clean:
	cd ~/nsmake; tsc
	nsmake stop
	rm -f ~/.nsmake/server.log
	rm -f ~/.nsmake/build/*
	rm -f dist/*

sumrf:
	nsmake mocha --target-testing-env Firefox,Chrome,Nodejs ex.js
	nsmake html --ext-lib "react https://unpkg.com/react@15/dist/react.js React" ex.jsx
	# nsmake mocha --target-testing-env Chrome,Nodejs '*.jsx'

