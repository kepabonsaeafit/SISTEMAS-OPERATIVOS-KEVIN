# Makefile para compilación automatizada

# Configuración del compilador
CXX := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++11

# Archivos fuente y objetos
SRCS := generador.cpp process.cpp
OBJS := $(SRCS:.cpp=.o)
EXEC := process_cpp

# Objetivo principal: compilar el ejecutable
all: $(EXEC)

# Enlaza todos los objetos en el ejecutable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Reglas específicas para cada objeto con sus dependencias
generador.o: generador.cpp generador.h persona.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

process.o: process.cpp persona.h generador.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpia archivos generados
clean:
	rm -f $(OBJS) $(EXEC)

# Ejecuta el programa después de compilar
run: $(EXEC)
	./$(EXEC)

