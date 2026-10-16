#pragma once
#include "raylib.h"

const int ANCHO = 960;
const int ALTO = 720;
const int HUD_ALTO = 80;
const int CONTROLES_Y = 630;
const int VISTA_ALTO = CONTROLES_Y - HUD_ALTO;
const Color COLOR_FONDO = {18, 29, 29, 255};
const Color COLOR_TEXTO = {232, 239, 218, 255};
const Color COLOR_APAGADO = {151, 172, 161, 255};
const Color COLOR_DORADO = {214, 183, 105, 255};

namespace Datos {
class Arma {
public:
    const char* nombre;
    int dano;
    float alcance;
    float espera;
    int municion;
    const char* imagen;
};
const Arma armas[3] = {
    {"Espada", 2, 100, 0.40f, 0, "assets/sprites/armas/espada_icon.png"},
    {"Pistola", 6, 340, 0.22f, 20, "assets/sprites/armas/pistola.png"},
    {"AK", 8, 430, 0.10f, 35, "assets/sprites/armas/ak47.png"}
};
class Zombie {
public:
    int vida;
    float velocidad;
    int dano;
    float espera;
    int frames;
    float tamano;
    const char* imagen;
};
const Zombie zombies[3] = {
    {3, 66, 1, 0.95f, 16, 32, "assets/sprites/zombies/zombie_normal_walk.png"},
    {2, 108, 1, 0.80f, 16, 32, "assets/sprites/zombies/zombie_rapido_walk.png"},
    {8, 42, 2, 1.30f, 24, 40, "assets/sprites/zombies/zombie_tanque_walk.png"}
};
const int vidaJugador = 10;
const float velocidadJugador = 192;
const int maxBalas = 48;
const float velocidadBala = 1200;
const int vidaCaja = 10;
const int curacion = 4;
const float duracionObjeto = 120;
const float radioAudio = 180;
const int vocesZombie = 5;
namespace Mapa {
    const int columnas = 50;
    const int filas = 40;
    const int tamanoLoseta = 32;
    const int ancho = columnas * tamanoLoseta;
    const int alto = filas * tamanoLoseta;
    const int grosorBorde = 24;

    const float zoomCamara = 1.5f;
    const Color borde = {34, 37, 40, 255};
    const int carreteraX = 560;
    const int carreteraAncho = 480;
    const int pistas[] = {carreteraX};
    const Color pasto = {69, 77, 53, 255};
    const Color pastoSeco = {83, 86, 58, 255};
    const Color asfalto = {62, 66, 68, 255};
    const Color grava = {106, 103, 85, 255};

    enum TipoProp { Arbusto, Barricada, Piedra, ArbolMuerto, Tumba, CantidadProps };
    class Prop {
    public:
        const char* ruta;
        int ancho;
        int alto;
        Rectangle colision;
    };
    const Prop props[] = {
        {"assets/props/arbusto.png",      48, 48, {0, 0, 0, 0}},
        {"assets/props/barricada.png",    96, 48, {4, 24, 88, 20}},
        {"assets/props/piedra.png",       48, 40, {6, 22, 36, 16}},
        {"assets/props/arbol_muerto.png", 64, 96, {22, 76, 20, 16}},
        {"assets/props/tumba.png",        32, 48, {4, 28, 24, 16}}
    };

    class Obstaculo {
    public:
        int tipo;
        int x;
        int y;
        bool vertical = false;
    };

    const Obstaculo obstaculos[] = {
        {ArbolMuerto, 210, 160}, {ArbolMuerto, 350, 630},
        {ArbolMuerto, 180, 980}, {ArbolMuerto, 1290, 220},
        {ArbolMuerto, 1170, 650}, {ArbolMuerto, 1360, 1030},
        {Arbusto, 430, 250}, {Arbusto, 460, 910},
        {Arbusto, 1090, 370}, {Arbusto, 1120, 1020},
        {Arbusto, 300, 460}, {Arbusto, 1320, 790},
        {Piedra, 500, 500}, {Piedra, 1070, 710},
        {Piedra, 390, 1100}, {Piedra, 1200, 420},
        {Barricada, 560, 32}, {Barricada, 656, 32}, {Barricada, 752, 32},
        {Barricada, 848, 32}, {Barricada, 944, 32},
        {Barricada, 560, 350}, {Barricada, 944, 880},
        {Tumba, 300, 230}, {Tumba, 345, 270},
        {Tumba, 1250, 920}, {Tumba, 1300, 950}
    };

}
}

enum class TipoObjeto { Botiquin, Municion, Pistola, AK };
enum class Audio { Espada, Pistola, AK, Dano, Muerte, Oleada, Derrota, Gemido, UI, Cantidad };
const char* rutasAudio[9] = {
    "assets/sounds/espada.wav", "assets/sounds/pistola.wav", "assets/sounds/ak.wav",
    "assets/sounds/dano_jugador.wav", "assets/sounds/muerte_zombie.wav",
    "assets/sounds/oleada.wav", "assets/sounds/derrota.wav",
    "assets/sounds/gemido_zombie.wav", "assets/sounds/ui.wav"
};

const Rectangle botonesMenu[3] = {
    {360, 343, 240, 48},
    {360, 410, 240, 48},
    {360, 477, 240, 48}
};
