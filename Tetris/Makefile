target = Tetris
source = main.c

$(target): $(source)
	i686-pc-mingw32-gcc -o $(target) $(source) -lgdi32 -luser32

debug:
	i686-pc-mingw32-gcc -DDEBUG -g -o $(target) $(source) -lgdi32 -luser32

clean:
	-@rm $(target)


