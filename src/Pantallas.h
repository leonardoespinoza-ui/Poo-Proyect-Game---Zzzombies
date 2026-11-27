#pragma once
#include "Partida.h"
#include "Mapa.h"
#include <string>
struct Arte {
    Arte();
    ~Arte();
    Arte(const Arte&) = delete;
    Arte& operator=(const Arte&) = delete;
    Texture2D jugador{}, enemigos[CantidadZombis]{}, iconos[CantidadArmas]{}, props[Mapa::Cantidad]{}, corazon{}, relleno{}, roca{}, tajo{}, bala{}, salpicadura{}, muerte{}, marcador{};
    Sound ataques[CantidadArmas]{}, herida{}, baja{}, oleada{}, derrota{}, clic{};
    Sound carga{}, lanzamiento{}, baba{}, gemido{}, carta{}, elegida{};
};
struct OpcionArma { bool abierta; };
enum class Vista { Carga, Menu, Armas, Juego, Pausa, Cartas, Derrota };
struct Accion {
    int elegir = -1;
    bool menu = false, jugar = false, guardar = false, comprar = false, dificultad = false;
};
void dibujarCarga(const Arte& arte, float tiempo);
Accion dibujarMenu(int monedas, int record, const std::string& titular, int oleadas, int modo, const std::string& global);
Accion dibujarArmas(const Arte& arte, const OpcionArma* opciones, int seleccion, Modo modo, int monedas, float tiempo);
inline constexpr float FinCarga = 3.38f;
inline constexpr Rectangle BotonSkip{758, 660, 110, 43}, BotonPausa{881, 660, 56, 43};
Accion dibujarPausa();
void dibujarPartida(const Arte& arte, const Partida& p);
Accion dibujarCartas(const Partida& p);
Accion dibujarDerrota(const Partida& p, bool record, const std::string& nombre);
void aviso(const std::string& texto);
