RM = rm -fv

CPPFLAGS += -ggdb -Wall -std=c++0x -I=. -I=editor/ 

all: text_editor

SRC_EDITOR=editor/Editor.cpp editor/Buffer.cpp editor/EditorMain.cpp
OBJ_EDITOR = $(SRC_EDITOR:.cpp=.o)
text_editor:$(OBJ_EDITOR)
	g++ $(CFLAGS) $(OBJ_EDITOR) -lncurses -lpthread -o editor_main

editor_lib:$(OBJ_EDITOR)
	ar rcs libeditor.a $(OBJ_EDITOR)

clean:
	-$(RM) *.o encrypt_main client_main server_main
