CCX = g++
CFLAGS = -Wall -std=c++23

NAME := inspect


all: compile link

compile:
	$(CCX) $(CFLAGS) -Isfml/include -I./imgui-sfml/ -I./imgui/ -I./implot/ -c ./*.cpp imgui/*.cpp imgui-sfml/*.cpp implot/*.cpp

link: 
	$(CCX) *.o -o $(NAME) -Lsfml/lib -Limgui-sfml/ -Limgui/ -Limplot/ -lsfml-graphics -lsfml-window -lsfml-system -lOpengl32 -lcomdlg32

