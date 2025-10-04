# Chat IPC con Colas de Mensajes (System V) — Extensión

Este proyecto implementa un sistema de chat entre procesos usando **colas de mensajes System V** en Linux, con **múltiples salas** y **broadcast** de mensajes.  
Se parte del **código base** del reto y se **extiende** el servidor con hilos por sala para reenviar (broadcast) los mensajes a todos los miembros.

> Requisito práctico del cliente base: **usar nombres de sala numéricos** y crearlas en **orden ascendente**: `1`, `2`, `3`, ...  
> El cliente calcula la clave de la cola de la sala con `ftok("/tmp", atoi(sala))`.  
> El servidor extendido crea la cola con esa misma `proj_id`, de modo que coinciden.

## Requisitos
- Linux (WSL con Kali funciona).
- `gcc`, `make`.

## Compilación
Coloca `servidor_ext.c`, `cliente.c` (del enunciado), y `Makefile` en la misma carpeta:
```bash
make
