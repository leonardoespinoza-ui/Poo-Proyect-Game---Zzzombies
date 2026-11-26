#pragma once
#include "raylib.h"
#include <array>
#include <vector>

constexpr int Ancho = 960, Alto = 720;
constexpr int CorazonesBase = 3, CorazonesTope = 5;
struct Arma {
    const char* nombre;
    const char* imagen;
    const char* sonido;
    int dano, corazonesExtra, precio;
    float alcance, intervalo;
    bool distancia;
};
inline constexpr Arma armas[] = {
    {"Pistol", "pistola.png", "pistola.wav", 3, 0, 0, 380, .32f, true},
    {"AK-47", "ak47.png", "ak.wav", 2, 0, 30, 410, .18f, true},
    {"Espada", "espada_icon.png", "espada.wav", 5, 1, 0, 100, .44f, false},
    {"Katana", "katana.png", "espada.wav", 6, 2, 60, 88, .55f, false}
};
constexpr int CantidadArmas = sizeof armas / sizeof armas[0];
enum Tipo { Normal, Rapido, Pesado, Correlon, Gordo, Gymbro, CantidadZombis };
struct TipoZombie { const char* imagen; int vida, cuadro, tamano; float velocidad; };
inline constexpr TipoZombie tipos[] = {
    {"zombie_normal_walk.png", 3, 16, 32, 66},
    {"zombie_rapido_walk.png", 2, 16, 32, 108},
    {"zombie_tanque_walk.png", 8, 24, 40, 42},
    {"zombie_correlon_walk.png", 6, 16, 32, 66},
    {"zombie_gordo_walk.png", 10, 24, 40, 54},
    {"zombie_gymbro_walk.png", 75, 24, 48, 174}
};
enum class Modo { Facil=0, Imposible=1, Dificil=2 };
inline const char* nombreModo(Modo modo) {
    if(modo==Modo::Imposible) return "IMPOSIBLE";
    return modo==Modo::Dificil ? "DIFÍCIL" : "FÁCIL";
}
enum class Carta { Dano, Velocidad, Monedas, Curar, Cantidad };
struct Actor { Vector2 pos{}; int fila = 0; float animacion = 0; };
struct Zombie { Actor actor; int tipo, vida; float especial = 0, recarga = 0; Vector2 rumbo{}, impulso{}; };
inline constexpr float RadioOnda = 120, AspectoOnda = .68f, TiempoOnda = .30f, TiempoDestello = .16f;
struct OndaGordo { Vector2 centro; float edad = 0; };
struct Destello { Vector2 pos; float edad, radio; };
struct Roca { Vector2 pos, rumbo; float tiempo = 2; };
struct Bala { Vector2 pos, rumbo; float restante; };
struct Evento { bool ataque = false, herida = false, baja = false, oleada = false, carga = false, roca = false, baba = false; };
struct Entrada { Vector2 movimiento, objetivo; bool atacar, automatico; };
class Partida {
public:
    void iniciar(int arma, Modo modo);
    Evento actualizar(float dt, Entrada entrada);
    std::array<Carta, 3> cartas() const;
    int elegir(Carta carta);
    int vidaZombie(int tipo) const;
    bool saltar();
    void avanzarOleada();
    bool cartaLista() const { return !termino() && bajas-bajasCanjeadas>=bajasPorCarta; }
    void actualizarDerrota(float dt);
    int recompensa() const { return modo != Modo::Facil ? 15 : 5; }
    int puntaje() const { return bajas*10 + completadas*(modo==Modo::Facil ? 10 : modo==Modo::Dificil ? 15 : 20); }
    bool termino() const { return vida <= 0; }

    Actor jugador;
    std::vector<Zombie> zombies;
    std::vector<Bala> balas;
    std::vector<Roca> rocas;
    std::vector<Destello> destellos;
    float tiempoDerrota = 0;
    std::vector<OndaGordo> ondas;
    int arma = 0, vida = 3, maxVida = 3, ronda = 1, completadas = 0, bajas = 0;
    int bonoDano = 0, bonosVelocidad = 0, monedasPartida = 0;
    Modo modo = Modo::Facil;
    float invulnerable = 0, efectoAtaque = 0, avisoOleada = 0, esperaSkip = 0, baba = 0;
    Vector2 rumboAtaque{};
private:
    int bajasCanjeadas = 0, bajasPorCarta = 1;
    void nuevaOleada();
    void aparecer();
    void atacar(Entrada entrada);
    void moverBalas(float dt);
    void actualizarZombie(Zombie& z, float dt, Evento& e);
    void separarZombies();
    void avanzarDestellos(float dt);
    void moverRocas(float dt, Evento& e);
    void actualizarOndas(float dt, Evento& e);
    void recibirGolpe(Evento& e);
    std::array<int, CantidadZombis> pendientes{};
    float intervalo = .8f;
    float espera = 0, esperaAtaque = 0;
};
Camera2D camara(const Partida& p);
