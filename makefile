# Makefile para compilación automatizada

# Configuración del compilador
CXX := g++
CC := gcc
CXXFLAGS := -Wall -Wextra -pedantic -std=c++11
CFLAGS := -Wall -Wextra -std=c99

# Archivos fuente y objetos
SRCS_CXX := main.cpp generador.cpp
SRCS_C := generador_c.c
OBJS_CXX := $(SRCS_CXX:.cpp=.o)
OBJS_C := $(SRCS_C:.c=.o)
EXEC := compare

# Objetivo principal: compilar el ejecutable
all: $(EXEC)

# Enlaza todos los objetos en el ejecutable
$(EXEC): $(OBJS_CXX) $(OBJS_C)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Reglas específicas para cada objeto con sus dependencias
main.o: main.cpp generador.h persona.h generador_c.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

generador.o: generador.cpp generador.h persona.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

generador_c.o: generador_c.c generador_c.h
	$(CC) $(CFLAGS) -c $< -o $@

# Limpia archivos generados
clean:
	rm -f $(OBJS_CXX) $(OBJS_C) $(EXEC)

# Ejecuta el programa después de compilar
run: $(EXEC)
	./$(EXEC) 100000
