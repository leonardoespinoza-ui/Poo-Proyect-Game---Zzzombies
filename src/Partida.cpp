#include "Partida.h"
#include "Mapa.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace {
constexpr float MundoX = Mapa::ancho, MundoY = Mapa::alto;
void mover(Actor& actor, Vector2 rumbo, float velocidad, float dt) {
    actor.pos = Vector2Add(actor.pos, Vector2Scale(rumbo, velocidad * dt));
    actor.pos.x = Clamp(actor.pos.x, 26, MundoX - 26);
    actor.pos.y = Clamp(actor.pos.y, 26, MundoY - 26);
    if (Vector2LengthSqr(rumbo) < .001f) return;
    actor.fila = std::abs(rumbo.x) > std::abs(rumbo.y) ? (rumbo.x < 0 ? 1 : 2) : (rumbo.y < 0 ? 3 : 0);
    actor.animacion += dt * 9;
}
}
void Partida::iniciar(int elegida, Modo dificultad) {
    *this = Partida{};
    arma = elegida;
    modo = dificultad;
    vida = maxVida = modo==Modo::Imposible ? 1 : std::min(CorazonesTope, CorazonesBase + armas[arma].corazonesExtra);
    jugador.pos = {MundoX / 2, MundoY / 2};
    nuevaOleada();
    bajasPorCarta=std::accumulate(pendientes.begin(),pendientes.end(),0);
}
void Partida::nuevaOleada() {
    constexpr int iniciales[][3] = {{4,0,0}, {6,0,0}, {6,2,0}, {8,3,0},
                                    {8,4,1}, {10,5,2}, {12,6,2}, {14,7,3}};
    constexpr float pausas[] = {.80f,.78f,.74f,.70f,.64f,.60f,.56f,.52f};
    int extra = ronda - 8;
    if (extra <= 0) {
        const auto& grupo = iniciales[ronda - 1];
        pendientes = {grupo[0], grupo[1], grupo[2], 0, 0, 0};
        intervalo = pausas[ronda - 1];
    } else {
        pendientes = {14 + 2 * extra, 7 + extra, 3 + extra / 3, 0, 0, 0};
        intervalo = std::max(.24f, .48f - extra * .01f);
    }
    if (modo != Modo::Facil) {
        pendientes[Rapido] += 1 + ronda / 3;
        ++pendientes[Pesado];
        pendientes[Correlon] = 1 + (ronda - 1) / 3;
        pendientes[Gordo] = 1 + (ronda - 1) / 4;
        int total = std::accumulate(pendientes.begin(), pendientes.end(), 0);
        int refuerzos = (total * 8 + 5) / 10;
        pendientes[Rapido] += refuerzos / 3;
        pendientes[Correlon] += refuerzos / 4;
        pendientes[Pesado] += refuerzos - refuerzos / 3 - refuerzos / 4;
        if (ronda % 5 == 0) pendientes[Gymbro] = 1 + ronda / 20;
        intervalo = std::max(.14f, intervalo * .50f);
    } else {
        pendientes[Correlon] = ronda % 2 == 1 ? 1 : 0;
        pendientes[Gordo] = ronda % 2 == 0 ? 1 : 0;
        if (ronda >= 8) { pendientes[Correlon] += ronda / 8; pendientes[Gordo] += ronda / 10; }
        if (ronda >= 15 && ronda % 5 == 0) pendientes[Gymbro] = 1 + (ronda - 15) / 10;
    }
    espera = .25f;
    avisoOleada = 1.6f;
    esperaSkip = 5;
}
int Partida::vidaZombie(int tipo) const {
    int base = tipos[tipo].vida;
    return modo != Modo::Facil ? base + (base + 1) / 2 : base;
}
void Partida::aparecer() {
    float angulo = GetRandomValue(0, 359) * DEG2RAD;
    Vector2 pos = Vector2Add(jugador.pos, {std::cos(angulo) * 310, std::sin(angulo) * 310});
    pos = {Clamp(pos.x, 28, MundoX - 28), Clamp(pos.y, 28, MundoY - 28)};
    if (Vector2Distance(pos, jugador.pos) < 160) {
        pos = {jugador.pos.x < MundoX / 2 ? jugador.pos.x + 300 : jugador.pos.x - 300, jugador.pos.y};
    }
    int ticket = GetRandomValue(1, std::accumulate(pendientes.begin(), pendientes.end(), 0));
    int tipo = 0;
    while (ticket > pendientes[tipo]) ticket -= pendientes[tipo++];
    if (tipo == Gymbro) {
        const Camera2D vista = camara(*this);
        float mejor = -1;
        for (int i = 0; i < 16; ++i) {
            float a = angulo + i * (2 * PI / 16);
            Vector2 candidato = {Clamp(jugador.pos.x + std::cos(a) * 405, 28, MundoX - 28),
                                 Clamp(jugador.pos.y + std::sin(a) * 405, 28, MundoY - 28)};
            Vector2 punto = GetWorldToScreen2D(candidato, vista);
            if (CheckCollisionPointRec(punto, {-20, -20, Ancho + 40, Alto + 40})) continue;
            float distancia = Vector2DistanceSqr(candidato, jugador.pos);
            if (distancia > mejor) { mejor = distancia; pos = candidato; }
        }
    }
    zombies.push_back({{pos}, tipo, vidaZombie(tipo)});
    --pendientes[tipo];
}
void Partida::atacar(Entrada entrada) {
    const Arma& ficha = armas[arma];
    Vector2 objetivo = entrada.objetivo;
    if (entrada.automatico) {
        auto cerca = std::min_element(zombies.begin(), zombies.end(), [&](const Zombie& a, const Zombie& b) {
            return Vector2DistanceSqr(jugador.pos, a.actor.pos) < Vector2DistanceSqr(jugador.pos, b.actor.pos);
        });
        if (cerca == zombies.end()) return;
        objetivo = cerca->actor.pos;
    }
    Vector2 rumbo = Vector2Normalize(Vector2Subtract(objetivo, jugador.pos));
    if (Vector2LengthSqr(rumbo) == 0) return;
    esperaAtaque = ficha.intervalo;
    efectoAtaque = .18f;
    rumboAtaque = rumbo;
    if (ficha.distancia) {
        balas.push_back({jugador.pos, rumbo, ficha.alcance});
        return;
    }
    for (Zombie& z : zombies) {
        Vector2 diferencia = Vector2Subtract(z.actor.pos, jugador.pos);
        float distancia = Vector2Length(diferencia);
        if (distancia <= ficha.alcance && (distancia < 18 || Vector2DotProduct(rumbo, diferencia) > distancia * .63f))
            z.vida -= ficha.dano + bonoDano;
    }
}
void Partida::moverBalas(float dt) {
    for (Bala& bala : balas) {
        float recorrido = std::min(bala.restante, dt * 950);
        while (recorrido > 0) {
            float paso = std::min(4.0f, recorrido);
            recorrido -= paso;
            bala.restante -= paso;
            bala.pos = Vector2Add(bala.pos, Vector2Scale(bala.rumbo, paso));
            if (Mapa::fuera(bala.pos) || Mapa::choca({bala.pos.x - 2, bala.pos.y - 2, 4, 4})) {
                bala.restante = 0;
                break;
            }
            for (Zombie& z : zombies) {
                if (z.vida <= 0 || Vector2DistanceSqr(z.actor.pos, bala.pos) > 13 * 13) continue;
                z.vida -= armas[arma].dano + bonoDano;
                bala.restante = 0;
                recorrido = 0;
                break;
            }
        }
    }
    balas.erase(std::remove_if(balas.begin(), balas.end(), [](const Bala& b) { return b.restante <= 0; }), balas.end());
}
bool Partida::saltar() {
    if (esperaSkip > 0 || termino()) return false;
    ++ronda;
    nuevaOleada();
    return true;
}
void Partida::recibirGolpe(Evento& e) {
    if (invulnerable > 0 || termino()) return;
    --vida;
    invulnerable = 1;
    e.herida = true;
}
void Partida::actualizarZombie(Zombie& z, float dt, Evento& e) {
    const Vector2 delta = Vector2Subtract(jugador.pos, z.actor.pos);
    const float distancia = Vector2Length(delta);
    const Vector2 rumbo = Vector2Normalize(delta);
    z.recarga = std::max(0.0f, z.recarga - dt);
    if (z.tipo == Gymbro && distancia <= 170 && z.recarga == 0) {
        rocas.push_back({z.actor.pos, rumbo});
        z.recarga = 2;
        e.roca = true;
    }
    if (z.tipo == Correlon) {
        if (z.especial > 0) {
            z.especial -= dt;
            if (z.especial <= 0) z.especial = -.43f;
            return;
        }
        if (z.especial < 0) {
            Vector2 anterior = z.actor.pos;
            mover(z.actor, z.rumbo, 480, dt);
            z.especial = std::min(0.0f, z.especial + dt);
            if (Mapa::choca(Mapa::cuerpoActor(z.actor.pos))) {
                z.actor.pos = anterior;
                z.especial = 0;
            } else if (Vector2DistanceSqr(z.actor.pos, jugador.pos) < 25 * 25) {
                recibirGolpe(e);
                z.especial = 0;
            }
            if (z.especial == 0) z.recarga = 2;
            return;
        }
        if (z.recarga == 0 && distancia > 24 && distancia <= 220) {
            z.especial = .5f;
            z.rumbo = rumbo;
            e.carga = true;
            return;
        }
    }
    if (distancia <= 17) recibirGolpe(e);
    else {
        float velocidad = tipos[z.tipo].velocidad;
        if (Mapa::choca(Mapa::cuerpoActor(z.actor.pos))) velocidad = std::max(8.0f, velocidad - 24);
        mover(z.actor, rumbo, velocidad, dt);
    }
}
void Partida::moverRocas(float dt, Evento& e) {
    for (Roca& r : rocas) {
        r.tiempo -= dt;
        r.pos = Vector2Add(r.pos, Vector2Scale(r.rumbo, 360 * dt));
        if (Mapa::fuera(r.pos) || Mapa::choca({r.pos.x - 8, r.pos.y - 8, 16, 16})) r.tiempo = 0;
        else if (Vector2DistanceSqr(r.pos, jugador.pos) < 21 * 21) {
            r.tiempo = 0;
            recibirGolpe(e);
        }
    }
    rocas.erase(std::remove_if(rocas.begin(), rocas.end(), [](const Roca& r) { return r.tiempo <= 0; }), rocas.end());
}
void Partida::actualizarOndas(float dt, Evento& e) {
    for (OndaGordo& onda : ondas) {
        float anterior = onda.edad;
        onda.edad += dt;
        if (anterior >= TiempoOnda || onda.edad < TiempoOnda || termino()) continue;
        Vector2 diferencia = Vector2Subtract(jugador.pos, onda.centro);
        diferencia.y /= AspectoOnda;
        if (Vector2LengthSqr(diferencia) <= RadioOnda * RadioOnda) {
            baba = 4;
            e.baba = true;
        }
    }
    ondas.erase(std::remove_if(ondas.begin(), ondas.end(), [](const OndaGordo& onda) {
        return onda.edad >= TiempoOnda + TiempoDestello;
    }), ondas.end());
}
void Partida::avanzarDestellos(float dt) {
    for (auto& d : destellos) d.edad += dt;
    destellos.erase(std::remove_if(destellos.begin(),destellos.end(),[](const Destello& d){return d.edad >= .52f;}),destellos.end());
}
void Partida::separarZombies() {
    constexpr float masas[] = {1, .65f, 2.6f, 1.2f, 3.4f, 5};
    std::vector<size_t> orden(zombies.size());
    std::iota(orden.begin(),orden.end(),0);
    for (int vuelta=0; vuelta<2; ++vuelta) {
        std::sort(orden.begin(),orden.end(),[&](size_t a,size_t b){ return zombies[a].actor.pos.x < zombies[b].actor.pos.x; });
        for (size_t i=0; i<orden.size(); ++i) for (size_t j=i+1; j<orden.size(); ++j) {
            Zombie& a=zombies[orden[i]];
            Zombie& b=zombies[orden[j]];
            Vector2 delta=Vector2Subtract(b.actor.pos,a.actor.pos);
            if (delta.x>36) break;
            float minimo=(tipos[a.tipo].tamano+tipos[b.tipo].tamano)*.24f;
            float distancia=Vector2Length(delta);
            if(distancia>=minimo) continue;
            float angulo=float((orden[i]*37+orden[j]*137)%360)*DEG2RAD;
            Vector2 normal=distancia>.001f?Vector2Scale(delta,1/distancia):Vector2{std::cos(angulo),std::sin(angulo)};
            Vector2 ajuste=Vector2Scale(normal,(minimo-distancia)*.88f);
            float total=masas[a.tipo]+masas[b.tipo];
            a.actor.pos=Vector2Subtract(a.actor.pos,Vector2Scale(ajuste,masas[b.tipo]/total));
            b.actor.pos=Vector2Add(b.actor.pos,Vector2Scale(ajuste,masas[a.tipo]/total));
        }
        for(auto& z:zombies) {
            z.actor.pos.x=Clamp(z.actor.pos.x,26,MundoX-26);
            z.actor.pos.y=Clamp(z.actor.pos.y,26,MundoY-26);
        }
    }
}
void Partida::actualizarDerrota(float dt) {
    tiempoDerrota += dt;
    avanzarDestellos(dt);
    baba = std::max(0.f,baba-dt);
    Evento sinDano;
    actualizarOndas(dt,sinDano);
    for(auto& z:zombies) {
        Vector2 hacia=Vector2Subtract(jugador.pos,z.actor.pos);
        if(Vector2Length(hacia)>18 && tiempoDerrota>.55f)
            mover(z.actor,Vector2Normalize(hacia),tipos[z.tipo].velocidad*.73f,dt);
        z.actor.pos=Vector2Add(z.actor.pos,Vector2Scale(z.impulso,dt));
        z.impulso=Vector2Scale(z.impulso,std::exp(-4.3f*dt));
    }
    separarZombies();
}
Evento Partida::actualizar(float dt, Entrada entrada) {
    avanzarDestellos(dt);
    Evento e;
    avisoOleada = std::max(0.0f, avisoOleada - dt);
    esperaSkip = std::max(0.0f, esperaSkip - dt);
    baba = std::max(0.0f, baba - dt);
    esperaAtaque = std::max(0.0f, esperaAtaque - dt);
    invulnerable = std::max(0.0f, invulnerable - dt);
    efectoAtaque = std::max(0.0f, efectoAtaque - dt);
    Vector2 anterior = jugador.pos;
    mover(jugador, Vector2Normalize(entrada.movimiento), 193 + 12 * bonosVelocidad, dt);
    if (Mapa::choca(Mapa::cuerpoActor({jugador.pos.x, anterior.y}))) jugador.pos.x = anterior.x;
    if (Mapa::choca(Mapa::cuerpoActor(jugador.pos))) jugador.pos.y = anterior.y;
    espera -= dt;
    if (espera <= 0 && std::accumulate(pendientes.begin(), pendientes.end(), 0) > 0) {
        aparecer();
        espera = intervalo;
    }
    if (esperaAtaque == 0 && (entrada.atacar || entrada.automatico)) {
        atacar(entrada);
        e.ataque = esperaAtaque > 0;
    }
    moverBalas(dt);
    for (Zombie& z : zombies) {
        if (z.vida > 0) actualizarZombie(z, dt, e);
        else {
            destellos.push_back({z.actor.pos,0,28});
            if (z.tipo == Gordo) ondas.push_back({z.actor.pos});
        }
    }
    moverRocas(dt, e);
    actualizarOndas(dt, e);
    auto muertos = std::remove_if(zombies.begin(), zombies.end(), [](const Zombie& z) { return z.vida <= 0; });
    int cantidad = int(zombies.end() - muertos);
    zombies.erase(muertos, zombies.end());
    separarZombies();
    if (termino()) {
        constexpr float radio = 160;
        destellos.push_back({jugador.pos,0,radio});
        for(size_t i=0;i<zombies.size();++i) {
            Zombie& z=zombies[i];
            Vector2 delta=Vector2Subtract(z.actor.pos,jugador.pos);
            float distancia=Vector2Length(delta);
            if(distancia>=radio) continue;
            float angulo=float(i*137)*DEG2RAD;
            Vector2 rumbo=distancia>.01f?Vector2Scale(delta,1/distancia):Vector2{std::cos(angulo),std::sin(angulo)};
            z.impulso=Vector2Scale(rumbo,640*(1-distancia/radio));
        }
        balas.clear();
        rocas.clear();
    }
    bajas += cantidad;
    e.baja = cantidad > 0;
    e.oleada = vida > 0 && std::accumulate(pendientes.begin(), pendientes.end(), 0) == 0 && zombies.empty() && ondas.empty() && destellos.empty();
    if (e.oleada) ++completadas;
    return e;
}
std::array<Carta, 3> Partida::cartas() const {
    return {Carta::Dano, Carta::Velocidad, modo != Modo::Imposible && vida < maxVida ? Carta::Curar : Carta::Monedas};
}
int Partida::elegir(Carta carta) {
    if(!cartaLista()) return 0;
    bajasCanjeadas += bajasPorCarta;
    int aumento = static_cast<int>(std::lround(bajasPorCarta * 0.35));
    bajasPorCarta += std::clamp(aumento, 1, 18);
    switch (carta) {
    case Carta::Dano: ++bonoDano; break;
    case Carta::Velocidad: bonosVelocidad = std::min(bonosVelocidad + 1, 8); break;
    case Carta::Curar: if(modo!=Modo::Imposible) vida=std::min(vida+1,maxVida); break;
    default: break;
    }
    return carta == Carta::Monedas ? 5 : 0;
}
void Partida::avanzarOleada() {
    ++ronda;
    balas.clear();
    rocas.clear();
    nuevaOleada();
}
Camera2D camara(const Partida& p) {
    constexpr float zoom = 1.65f, margenX = Ancho / (2 * zoom), margenY = (Alto - 72) / (2 * zoom);
    return {{Ancho / 2.0f, (Alto + 72) / 2.0f},
            {Clamp(p.jugador.pos.x, margenX, MundoX - margenX), Clamp(p.jugador.pos.y, margenY, MundoY - margenY)}, 0, zoom};
}
