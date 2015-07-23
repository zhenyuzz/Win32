target = Tetris
source = main.c

$(target): $(source)
	gcc -o $(target) $(source) -lgdi32

debug:
	gcc -DDEBUG -g -o $(target) $(source) -lgdi32

clean:
	-@rm $(target)

