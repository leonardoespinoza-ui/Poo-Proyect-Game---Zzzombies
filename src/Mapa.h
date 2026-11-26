#pragma once
#include "raylib.h"

namespace Mapa {
inline constexpr int ancho = 1600, alto = 1280, borde = 24;
enum Tipo { Arbusto, Barricada, Piedra, Arbol, Tumba, Cantidad };
struct Prop { const char* imagen; int ancho, alto; Rectangle cuerpo; };
inline constexpr Prop props[] = {
    {"arbusto.png", 48, 48, {}},
    {"barricada.png", 96, 48, {4, 24, 88, 20}},
    {"piedra.png", 48, 40, {6, 22, 36, 16}},
    {"arbol_muerto.png", 64, 96, {22, 76, 20, 16}},
    {"tumba.png", 32, 48, {4, 28, 24, 16}}
};
struct Elemento { Tipo tipo; int x, y; };
inline constexpr Elemento elementos[] = {
    {Arbol, 210, 160}, {Arbol, 350, 630}, {Arbol, 180, 980},
    {Arbol, 1290, 220}, {Arbol, 1170, 650}, {Arbol, 1360, 1030},
    {Arbusto, 430, 250}, {Arbusto, 460, 910}, {Arbusto, 1090, 370},
    {Arbusto, 1120, 1020}, {Arbusto, 300, 460}, {Arbusto, 1320, 790},
    {Piedra, 500, 500}, {Piedra, 1070, 710}, {Piedra, 390, 1100}, {Piedra, 1200, 420},
    {Barricada, 560, 32}, {Barricada, 656, 32}, {Barricada, 752, 32},
    {Barricada, 848, 32}, {Barricada, 944, 32}, {Barricada, 560, 350}, {Barricada, 944, 880},
    {Tumba, 300, 230}, {Tumba, 345, 270}, {Tumba, 1250, 920}, {Tumba, 1300, 950}
};
inline bool choca(Rectangle zona) {
    for (auto e : elementos) {
        Rectangle cuerpo = props[e.tipo].cuerpo;
        if (cuerpo.width == 0) continue;
        cuerpo.x += e.x;
        cuerpo.y += e.y;
        if (CheckCollisionRecs(zona, cuerpo)) return true;
    }
    return false;
}
inline Rectangle cuerpoActor(Vector2 pos) { return {pos.x - 10, pos.y - 8, 20, 22}; }
inline bool fuera(Vector2 pos) {
    return pos.x < borde || pos.y < borde || pos.x > ancho - borde || pos.y > alto - borde;
}
}
