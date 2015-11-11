RM = rm -fv

CPPFLAGS += -ggdb -Wall -std=c++0x -I=. -Ieditor/ -Iinclude

all: text_editor editor_lib

SRC_EDITOR=editor/Editor.cpp editor/Buffer.cpp editor/EditorMain.cpp
OBJ_EDITOR = $(SRC_EDITOR:.cpp=.o)
text_editor:$(OBJ_EDITOR) editor/helpers.h
	g++ $(CFLAGS) $(OBJ_EDITOR) -lncurses -lpthread -o editor_main

editor_lib:$(OBJ_EDITOR)
	ar rcs libeditor.a $(OBJ_EDITOR)

clean:
	-$(RM) *.o encrypt_main client_main server_main
