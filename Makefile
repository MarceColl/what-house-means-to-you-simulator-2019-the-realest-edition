.PHONY: gaem

gaem:
	g++ *.cc -o gaem -lsfml-graphics -lsfml-system -lsfml-window
