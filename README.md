# ZZZombies - avance parcial del 25 %

Juego 2D de supervivencia por oleadas para el curso de Programación Orientada a Objetos. Implementado en C++17 con raylib.

## Compilar y jugar

1. Instalar MinGW de 32 bits en `C:\msys64\mingw32`. La biblioteca incluida requiere esa arquitectura.
2. Abrir `Compilar.cmd` para generar `ZZZombies.exe`.
3. Abrir `Jugar.cmd` para jugar. Si falta el ejecutable, intenta compilar primero.
4. Después de modificar el código, volver a ejecutar `Compilar.cmd`.

El ejecutable debe permanecer junto a `assets`. No se incluye en el repositorio: se genera al compilar.

## Controles

| Acción | Control |
|---|---|
| Mover | WASD o flechas |
| Atacar hacia el cursor | Clic izquierdo |
| Atacar hacia el enemigo más cercano | Espacio |
| Seleccionar arma | 1, 2, 3 o clic en su casilla |
| Adelantar oleada | SKIP |
| Volver al menú | Escape o MENU |

## Alcance

Incluye menú, jugador animado, mapa urbano, tres tipos de zombi, espada, pistola, AK, cajas, botiquines, munición, audio, modos normal e imposible y reinicio tras perder. Los dos patrones de diseño del proyecto final aún están pendientes.

## Archivos

- `Clases.cpp`: once clases principales y funciones auxiliares.
- `Variables.h`: estadísticas, configuración, mapa y rutas.
- `assets/`: imágenes y sonidos utilizados por esta versión.
- `vendor/raylib/`: cabecera, biblioteca de 32 bits y licencia.
- [Informe parcial](INFORME_PARCIAL_ZZZOMBIES.md): arquitectura, UML, fundamentos de POO y resultados.

## Actualizar GitHub

Desde una copia clonada de este repositorio, guardar los cambios y ejecutar:

```sh
git status
git add Clases.cpp Variables.h INFORME_PARCIAL_ZZZOMBIES.md README.md
git commit -m "Actualizar avance parcial e informe"
git push origin main
```

Agregar explícitamente también los recursos nuevos cuando corresponda. Cada integrante debe registrar sus aportes reales con su propia cuenta; subir todos los archivos una vez no demuestra todo el trabajo del equipo.
