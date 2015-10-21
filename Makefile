RM = rm -fv

CFLAGS += -ggdb -Wall -std=c++0x -I=.

all: text_editor

SRC_EDITOR=editor/Editor.cpp editor/Buffer.cpp editor/EditorMain.cpp
text_editor:$(SRC_EDITOR)
	g++ $(CFLAGS) $(SRC_EDITOR) -I=editor/ -I=. $(LIB_FLAGS) -lncurses -lpthread -o editor_main

clean:
		-$(RM) *.o encrypt_main client_main server_main
