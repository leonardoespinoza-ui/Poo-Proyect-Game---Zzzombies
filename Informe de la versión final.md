# ZZZombies · El amanecer de los muertos

**Informe final de Programación Orientada a Objetos**  
Universidad Peruana Cayetano Heredia · Ciclo 2026-2  
Kenneth Samir Ramos Espinoza, Giordano Valentino Valero Bonifacio, Leonardo Pedro Espinoza Flores y Leonel Willians Sotelo Mamani.

## Introducción

Empezamos con un prototipo de supervivencia en C++17 y raylib. Tenía movimiento, combate y recogida de objetos. Al preparar la versión final cambiamos el foco. Elegir el arma antes de salir y recibir mejoras durante la partida nos permitía dedicar más atención a la horda. Retiramos las cajas, la munición y la progresión individual de armas. Conservamos el mapa de la carretera y el combate, y trabajamos en que cada enemigo se pudiera reconocer y esquivar de una manera distinta.

## De la primera elección a la última oleada

La pistola y la espada están disponibles desde el inicio. La AK-47 cuesta 30 monedas y la katana, 60. La espada añade un corazón y la katana añade dos, una ventaja que se entiende al acercarse a un enemigo para golpearlo. En Imposible, incluso con la katana, un solo golpe y morirás.

| Modo | Corazones iniciales | Bajas para la primera tarjeta | Puntos por oleada completada |
|---|---|---:|---:|
| Fácil | 3 a 5 según el arma | 5 | 10 |
| Difícil | 3 a 5 según el arma | 14 | 15 |
| Imposible | 1 | 14 | 20 |

El zombi normal persigue al jugador, el rápido acorta la distancia y el pesado cuesta más derribarlo. El Correlón se detiene un instante antes de lanzarse en la dirección que preparó. El Gordo deja una onda al morir y puede salpicar la pantalla. El Gymbro lanza rocas y tiene un indicador en el borde cuando se acerca desde fuera de la vista. Estas diferencias dan razones para cambiar de posición incluso cuando el jugador tiene ventaja.

Cada baja suma diez puntos y cuenta para la próxima tarjeta, aunque el jugador adelante la oleada. En Fácil las tres primeras tarjetas cuestan 5, 7 y 9 bajas respectivamente. Aumentamos el coste un 35 % redondeado, con un máximo de 18 bajas adicionales por elección. La tercera opción de tarjeta ofrece monedas con la salud llena y permite recuperar un corazón si falta vida. En Imposible esa opción sigue siendo de monedas.

## Decisiones técnicas que se ven al jugar

**Movimiento y profundidad.** Para comprobar si un personaje toca un obstáculo usamos rectángulos definidos en el mapa. El tronco puede bloquear el paso aunque buena parte del dibujo del árbol sea copa. Para mostrar quién queda delante, ordenamos objetos y personajes por la altura de su base en pantalla. Así el jugador puede quedar parcialmente tapado al caminar detrás del árbol. La colisión decide hasta dónde puede moverse y el orden de dibujo decide qué píxeles vemos. Véanse `Mapa::choca` en [Mapa.h](src/Mapa.h) y la lista `orden` de [Pantallas.cpp](src/Pantallas.cpp).

**Una horda con espacio entre cuerpos.** Cuando dos zombis se acercan demasiado, repartimos el desplazamiento según su masa. El ligero cede más terreno que el pesado. Para reducir comparaciones, primero los ordenamos por su posición horizontal. Al encontrar un candidato a más de 36 unidades en ese eje, pasamos al siguiente zombi. Es una optimización que se puede explicar leyendo el bucle sin atribuirle una mejora de rendimiento que no hemos medido. Véase `Partida::separarZombies` en [Partida.cpp](src/Partida.cpp).

**Conceptos del curso.** Usamos composición al incluir un `Actor` dentro de cada `Zombie`. El actor conserva posición y animación, mientras el zombi añade vida y comportamiento. Aplicamos encapsulamiento al guardar el acceso a SQLite dentro de `Guardado`. El menú pide comprar un arma mediante `comprar` y la clase se encarga de descontar las monedas y registrar el desbloqueo en una misma transacción. La responsabilidad única también se aprecia en las tarjetas. `Partida::elegir` cambia las estadísticas, `dibujarCartas` dibuja las opciones y `ejecutar` coordina el cambio de pantalla. [Partida.h](src/Partida.h) · [Guardado.cpp](src/Guardado.cpp) · [main.cpp](src/main.cpp)

## Récord local y extensión global

La versión publicada guarda en SQLite el saldo, las armas desbloqueadas y el historial. Al superar el récord, el jugador escribe su nombre y pulsa **Guardar**. Al volver al menú puede ver el puntaje, la oleada alcanzada y el modo.

Preparamos además una extensión con la API de Supabase. Cada instalación conserva un identificador aleatorio en su SQLite y el servidor mantiene una fila con su mejor puntaje, modo y oleada. Una marca superior reemplaza la anterior. El puesto se obtiene contando cuántos puntajes son mayores y sumando uno. Dos jugadores empatados comparten puesto. La consulta corre en segundo plano y se espacia durante el menú, para que el juego pueda seguir dibujando mientras espera la respuesta.

Compilamos esta extensión y probamos la actualización de marcas en PostgreSQL local. En la revisión de GitHub del 26 de noviembre, la entrega publicada llegaba al récord local. La conexión a un proyecto Supabase real y la prueba entre dos equipos quedan pendientes.

## Resultado

La versión publicada compila en Windows con MinGW de 32 bits, raylib y SQLite. Se puede seguir un recorrido completo desde la selección de arma hasta el guardado de un récord. La mejora que mejor muestra lo aprendido cabe en unos segundos de juego. Al pasar detrás de un árbol mientras nos persigue una horda, se ven a la vez la colisión del mapa, la separación entre enemigos y el orden de dibujo. Cada una de esas decisiones tiene una función que podemos abrir y explicar.
