# Ciudad Zombie — etapa 1: mapa y movimiento

Esta es solo la primera de tres partes propuestas para el avance parcial. No equivale por si sola al 25% completo ni acredita toda la rubrica.

## Probar ahora
1. Abre esta carpeta con el Explorador de Windows.
2. Haz doble clic en **Jugar.cmd**. Aparece una ventana del juego, independiente de VS Code.
3. En la portada pulsa **Iniciar** o Enter. Mueve al personaje con **W, A, S, D**; se pueden combinar para movimiento diagonal.
4. Las paredes y bordes no se pueden atravesar.
5. **Reiniciar** devuelve al jugador a su posicion inicial. **Menu** o Escape vuelven a la portada conservando la posicion. **Salir** en la portada o la X cierran el juego.
6. **Creditos** abre una pantalla con el equipo; **Volver** o Escape regresan a la portada. Edita `assets/textos/creditos.txt` (hasta 8 lineas, 60 caracteres por linea, UTF-8 sin BOM). No hacen falta cambios de codigo para los nombres.

## Dibujos de la portada
Coloca tus PNG en `assets/ui/`. Hay una guia detallada en `assets/ui/LEEME.txt`.

| Nombre | Medida del PNG | Uso |
|---|---|---|
| `button.png` | 80 x 24 | Fondo comun de todos los botones, sin letras. |
| `titulo.png` | 240 x 64 | Nombre o logo del juego; transparente. |
| `texto_iniciar.png` | 64 x 12 | Solo INICIAR; transparente. |
| `texto_creditos.png` | 64 x 12 | Solo CREDITOS; transparente. |
| `texto_salir.png` | 64 x 12 | Solo SALIR; transparente. |
| `fondo.png` | 480 x 360 | Fondo de la portada, opcional; no incluir botones ni titulo. |

Excepto button.png, todos son opcionales: se usa texto/fondo provisional mientras dibujas. Se muestran al doble de tamano. Centra el contenido dentro de cada PNG, con margenes transparentes equilibrados. Para etiquetas recomendamos letras de **5 x 7**, separadas por **1 pixel**: CREDITOS ocupa 47 x 7 dentro del lienzo 64 x 12. No necesitas pintar cada letra en un archivo separado. Una imagen opcional de dimensiones incorrectas se ignora; vuelve a exportarla con las medidas indicadas. La plantilla mantiene hover y clic aunque cambies su dibujo.

La escena ya incluye imagenes provisionales: no necesitas instalar un editor para probarla. No hay animacion de caminar; se desplaza un unico sprite fijo.

## Que crear tu
En Piskel (https://www.piskelapp.com/), crea o edita estas dos imagenes:

| Archivo | Medida exacta | Contenido |
|---|---|---|
| `assets/sprites/player.png` | 16 x 16 pixeles | Un solo personaje, fondo transparente, centrado. |
| `assets/ui/button.png` | 80 x 24 pixeles | Un fondo de boton sin texto, con borde y espacio central. |

Exporta como **PNG**, no como GIF ni como captura de pantalla. Conserva los nombres y medidas. Copia tus versiones sobre las plantillas y vuelve a abrir el juego: no es necesario compilar por cambiar imagenes.

El sprite se muestra a 32 x 32, y el boton a 160 x 48. Se usa ampliacion x2 sin suavizado. Los textos Reiniciar/Salir se dibujan con codigo sobre la misma imagen. Hover y clic cambian el tono de esa unica plantilla. No hace falta dibujar tres botones.

## Editar el mapa
Abre `assets/mapas/refugio.txt`. Debe tener exactamente **12 filas de 20 caracteres**:
- `#`: pared con colision.
- `.`: suelo transitable.
- `P`: inicio del jugador (exactamente uno).
- Todos los bordes deben ser `#`.

No agregues espacios, filas vacias ni otros simbolos. Guarda como UTF-8 sin BOM o ASCII y vuelve a abrir el juego. Si te equivocas, se muestra un mensaje en la ventana. Mantener pasillos de al menos una casilla y caminos conectados depende de tu diseño. El suelo y las paredes se dibujan con rectangulos; en esta etapa no necesitas sprites para ellos ni Tiled.

## Carpetas y responsabilidades
```
src/
  main.cpp          Punto de entrada: crea Juego y lo ejecuta.
  juego/            Bucle, portada, creditos y partida en Juego.cpp.
  mapa/             Carga del mapa, suelo, paredes y colisiones.
  personajes/       Posicion, movimiento y dibujo de Jugador.
  interfaz/         Boton: mouse, texto, clic y hover.
  recursos/         Cargar y liberar player.png y button.png.
assets/
  mapas/            Distribucion del escenario en texto.
  sprites/          player.png
  ui/               button.png
tools/              Compilacion y generador opcional de plantillas.
tests/              Comprobaciones del mapa y movimiento.
docs/               Guia para entender y explicar esta etapa.
vendor/raylib/      Biblioteca externa, cabecera y licencia.
build/              Ejecutable generado; no editar.
```

Cada clase tiene `.h` (que contiene y que puede hacer) y `.cpp` (como lo hace). A diferencia de Roblox, estas carpetas no son servicios ni ejecutan codigo por si mismas: todo se compila en un programa local. `assets` cumple el papel de una carpeta de recursos; `Juego` coordina los componentes.

## Cambiar codigo y compilar
Ya esta preparada para el compilador instalado en esta PC: `C:\msys64\mingw32\bin\g++.exe` (32 bits).

1. Cierra el juego si esta abierto.
2. Cambia los archivos de `src` y guarda.
3. Haz doble clic en **Compilar.cmd**.
4. Abre **Jugar.cmd**.

En VS Code puedes usar **Ctrl+Shift+B** para compilar, o **Terminal > Run Task > Ejecutar juego 2D** para compilar y abrir la ventana. La extension C/C++ de Microsoft ayuda con el autocompletado, pero no es necesaria para ejecutar Jugar.cmd.

Pruebas, desde PowerShell en esta carpeta:
```
powershell -NoProfile -ExecutionPolicy Bypass -File tools/compilar.ps1 -Pruebas
```

La biblioteca incluida es raylib 5.5 compilada desde el codigo oficial para MinGW de 32 bits. No la mezcles con un compilador de 64 bits; habria que recompilar la biblioteca para ese caso. El ejecutable lleva enlazados los componentes de MinGW y no necesita sus DLL para jugar. Para presentarlo en otra PC Windows, lleva la carpeta con `build/refugio.exe` y `assets`; conviene probar antes la compatibilidad de OpenGL 3.3.

## Alcance
Implementado: portada, creditos, ventana 2D, un mapa, un sprite, WASD, colisiones y botones que reutilizan una plantilla. No implementado: enemigos, combate, vida, escondite, inventario, dias, misiones ni guardado.

El anterior juego de consola queda separado en `anteriores/consola/`. No lo confundas con esta entrega ni uses su diseño como descripcion del prototipo actual.
