#include "Guardado.h"
#include "RecordGlobal.h"
#include "Pantallas.h"
#include "raymath.h"
#include <algorithm>

namespace {
Entrada controles(const Partida& p) {
    return {
        {float(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - float(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)),
         float(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) - float(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))},
        GetScreenToWorld2D(GetMousePosition(), camara(p)),
        IsMouseButtonDown(MOUSE_BUTTON_LEFT) && GetMousePosition().y >= 74 &&
            !CheckCollisionPointRec(GetMousePosition(), BotonPausa) && !CheckCollisionPointRec(GetMousePosition(), BotonSkip), IsKeyDown(KEY_SPACE)
    };
}
void editarNombre(std::string& nombre) {
    for (int c = GetCharPressed(); c; c = GetCharPressed())
        if (nombre.size() < 18 && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                                   (c >= '0' && c <= '9') || c == ' '))
            if (c != ' ' || !nombre.empty()) nombre += char(c);
    if (IsKeyPressed(KEY_BACKSPACE) && !nombre.empty()) nombre.pop_back();
}
void ejecutar(const Arte& arte, Guardado& guardado) {
    Vista vista = Vista::Carga;
    Partida partida;
    int seleccion = 0;
    Modo modo=Modo::Facil;
    bool nuevoRecord=false;
    std::string nombre, mensaje;
    float tiempo = 0, esperaGemido = 0;
    int saldo = guardado.monedas(), record = guardado.record();
    std::string titular = guardado.titular();
    int oleadaRecord = guardado.oleadaRecord(), modoRecord = guardado.modoRecord();
    RecordGlobal global(guardado.idInstalacion());
    OpcionArma opciones[CantidadArmas];
    for (int i = 0; i < CantidadArmas; ++i)
        opciones[i].abierta = armas[i].precio == 0 || guardado.disponible(i);
    auto premiar = [&](int cantidad) {
        partida.monedasPartida += cantidad;
        if (guardado.premiar(cantidad)) saldo = guardado.monedas();
        else mensaje = "No se pudieron guardar las monedas";
    };
    auto iniciar = [&] {
        partida.iniciar(seleccion, modo);
        nombre.clear();
        nuevoRecord = false;
        vista = Vista::Juego;
        esperaGemido = 1.5f;
        PlaySound(arte.oleada);
    };
    while (!WindowShouldClose()) {
        float dt = std::min(GetFrameTime(), .05f);
        tiempo += dt;
        global.actualizar(dt, vista == Vista::Menu || vista == Vista::Carga || vista == Vista::Derrota,
                          record, oleadaRecord, modoRecord);
        if (vista == Vista::Carga && tiempo >= FinCarga) vista = Vista::Menu;
        const bool pausaTecla = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
        const bool pausaClic = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), BotonPausa);
        if (vista == Vista::Juego && (pausaTecla || pausaClic)) vista = Vista::Pausa;
        else if (vista == Vista::Pausa && pausaTecla) vista = Vista::Juego;
        else if (vista == Vista::Juego) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), BotonSkip)
                && partida.saltar()) PlaySound(arte.oleada);
            Evento e = partida.actualizar(dt, controles(partida));
            if (e.carga) PlaySound(arte.carga);
            if (e.roca) PlaySound(arte.lanzamiento);
            if (e.baba) PlaySound(arte.baba);
            esperaGemido -= dt;
            if (esperaGemido <= 0) {
                for (const Zombie& z : partida.zombies) {
                    float d = Vector2Distance(z.actor.pos, partida.jugador.pos);
                    if (d >= 180) continue;
                    SetSoundVolume(arte.gemido, .55f * (1 - d / 180));
                    SetSoundPitch(arte.gemido, GetRandomValue(85, 115) / 100.0f);
                    PlaySound(arte.gemido);
                    break;
                }
                esperaGemido = GetRandomValue(120, 280) / 100.0f;
            }
            if (e.ataque) PlaySound(arte.ataques[partida.arma]);
            if (e.herida) PlaySound(arte.herida);
            if (e.baja) PlaySound(arte.baja);
            if (e.oleada) {
                premiar(partida.recompensa());
                partida.avanzarOleada();
                PlaySound(arte.oleada);
            }
            if (partida.cartaLista()) {
                PlaySound(arte.carta);
                vista = Vista::Cartas;
            }
            if (partida.termino()) {
                if (!guardado.terminar(partida.puntaje(), partida.completadas, partida.arma, int(partida.modo)))
                    mensaje = "No se pudo guardar el historial";
                nuevoRecord = guardado.listo() && partida.puntaje() > record;
                PlaySound(arte.derrota);
                vista = Vista::Derrota;
            }
        }
        if (vista == Vista::Derrota) {
            partida.actualizarDerrota(dt);
            if (partida.tiempoDerrota >= 2 && nuevoRecord) editarNombre(nombre);
        }
        Accion accion;
        BeginDrawing();
        switch (vista) {
        case Vista::Carga:
            dibujarMenu(saldo, record, titular, oleadaRecord, modoRecord, global.texto());
            dibujarCarga(arte, tiempo);
            break;
        case Vista::Menu: accion = dibujarMenu(saldo, record, titular, oleadaRecord, modoRecord, global.texto()); break;
        case Vista::Armas: accion = dibujarArmas(arte, opciones, seleccion, modo, saldo, tiempo); break;
        case Vista::Juego: dibujarPartida(arte, partida); break;
        case Vista::Pausa:
            dibujarPartida(arte, partida);
            accion = dibujarPausa();
            break;
        case Vista::Cartas:
            dibujarPartida(arte, partida);
            accion = dibujarCartas(partida);
            break;
        case Vista::Derrota:
            dibujarPartida(arte,partida);
            if(partida.tiempoDerrota >= 2) accion = dibujarDerrota(partida,nuevoRecord,nombre);
            break;
        }
        if (vista != Vista::Carga) aviso(guardado.listo() ? mensaje : "Guardado local no disponible");
        EndDrawing();

        if (accion.menu || accion.jugar || accion.comprar || accion.dificultad ||
            accion.guardar || accion.elegir >= 0) {
            mensaje.clear();
            PlaySound(arte.clic);
        }
        if (accion.menu) vista = Vista::Menu;
        else if (vista == Vista::Pausa && accion.jugar) vista = Vista::Juego;
        else if (vista == Vista::Menu && accion.jugar) vista = Vista::Armas;
        else if (vista == Vista::Armas) {
            if (accion.elegir >= 0) seleccion = accion.elegir;
            if (accion.dificultad) modo = modo==Modo::Facil ? Modo::Dificil : modo==Modo::Dificil ? Modo::Imposible : Modo::Facil;
            if (accion.comprar) {
                if (guardado.comprar(seleccion, armas[seleccion].precio)) {
                    opciones[seleccion].abierta = true;
                    saldo = guardado.monedas();
                } else mensaje = "No se pudo desbloquear el arma";
            }
            if (accion.jugar) iniciar();
        } else if (vista == Vista::Cartas && accion.elegir >= 0) {
            int extra = partida.elegir(partida.cartas()[accion.elegir]);
            if (extra) premiar(extra);
            PlaySound(arte.elegida);
            if(!partida.cartaLista()) vista=Vista::Juego;
        } else if (vista == Vista::Derrota) {
            if (accion.jugar) iniciar();
            if (accion.guardar) {
                nombre.erase(nombre.find_last_not_of(' ') + 1);
                if (guardado.guardarRecord(partida.puntaje(), nombre, partida.ronda, int(partida.modo))) {
                    global.solicitar();
                    record = guardado.record();
                    titular = guardado.titular();
                    oleadaRecord = guardado.oleadaRecord();
                    modoRecord = guardado.modoRecord();
                    nuevoRecord = false;
                    mensaje = "Récord guardado";
                } else mensaje = "No se pudo guardar el récord";
            }
        }
    }
}
}
int main() {
    ChangeDirectory(GetApplicationDirectory());
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(Ancho, Alto, "ZZZombies");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice();
    {
        Arte arte;
        Guardado guardado("ZZZombies.sqlite");
        ejecutar(arte, guardado);
    }
    CloseAudioDevice();
    CloseWindow();
}
