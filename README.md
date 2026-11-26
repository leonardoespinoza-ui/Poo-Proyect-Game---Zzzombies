ZZZombies · El amanecer de los muertos

¿Podrás sobrevivir a este juego infinito de oleadas de zombies? 

La versión final incluye cuatro armas (pistola, AK-47, espada y katana), seis tipos de zombis y un selector de dificultad. Puedes pausar la partida, volver a jugar con la misma configuración y guardar un nuevo récord con tu nombre.

## Jugar en Windows

1. Instala MinGW de MSYS2 de 32 bits en `C:\msys64\mingw32`, con el compilador C++ y SQLite para esa arquitectura (`mingw-w64-i686-gcc` y `mingw-w64-i686-sqlite3`). La biblioteca raylib está incluida en el repositorio.
2. Abre `Compilar.cmd` desde la carpeta del proyecto. Vuelve a compilar después de cada cambio de código.
3. Abre `Jugar.cmd` y pulsa **Jugar** para elegir arma y dificultad.

Al compilar se genera `ZZZombies.exe` junto a `assets/`. Mantén ambos en la misma carpeta para que el juego encuentre las imágenes y los sonidos.

| Acción | Control |
|---|---|
| Moverse | WASD o flechas |
| Atacar hacia el cursor | Mantener el clic izquierdo |
| Atacar al zombi más cercano | Mantener Espacio |
| Cambiar el arma seleccionada antes de jugar | Flechas izquierda y derecha o los botones del selector |
| Adelantar una oleada | Botón **Saltar**, disponible tras cinco segundos |
| Pausar y continuar | Esc, P o los botones en pantalla |

## Dificultad y recompensas

| Modo | Salud inicial | Bajas para la primera tarjeta | Puntos por oleada completada |
|---|---|---:|---:|
| Fácil | 3 a 5 corazones según el arma | 5 | 10 |
| Difícil | 3 a 5 corazones según el arma | 14 | 15 |
| Imposible | 1 corazón con cualquier arma | 14 | 20 |

Cada baja suma 10 puntos. Difícil e Imposible comparten hordas más numerosas y resistentes. Al elegir una tarjeta, el siguiente requisito aumenta un 35 % redondeado al entero más cercano, con un incremento máximo de 18 bajas. Puedes mejorar daño o velocidad, ganar monedas o recuperar un corazón cuando corresponda.

SQLite conserva el saldo, las armas desbloqueadas, el historial y el récord en `ZZZombies.sqlite`. El juego crea ese archivo en el equipo al iniciarse.

## Recorrido por el código

| Parte | Dónde empezar | Qué buscar |
|---|---|---|
| Partida y combate | [Partida.cpp](src/Partida.cpp) | `nuevaOleada`, `saltar` y `elegir` |
| Persistencia | [Guardado.cpp](src/Guardado.cpp) | `comprar` y `guardarRecord` |
| Interfaz | [Pantallas.cpp](src/Pantallas.cpp) | `dibujarPartida` y `dibujarCartas` |
| Integración | [main.cpp](src/main.cpp) | `ejecutar` conecta los eventos de la partida con las pantallas y el guardado |

El informe de la versión final se encuentra en: [Leer el informe](Informe%20de%20la%20versi%C3%B3n%20final.md)
