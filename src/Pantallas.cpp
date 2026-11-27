#include "Pantallas.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr Color Negro{14, 10, 19, 255}, Gris{44, 40, 54, 255}, Claro{229, 222, 234, 255};
constexpr Color Rojo{212, 50, 78, 255}, Oro{255, 197, 99, 255}, Sombra{25, 16, 37, 255};
Texture2D textura(const std::string& ruta) {
    Texture2D t = LoadTexture(ruta.c_str());
    SetTextureFilter(t, TEXTURE_FILTER_POINT);
    return t;
}
unsigned trama(int x, int y) {
    unsigned n = unsigned(x) * 374761393u + unsigned(y) * 668265263u;
    return (n ^ (n >> 13)) * 1274126177u;
}
void terreno(Camera2D cam) {
    constexpr Color cesped{68,78,53,255}, asfalto{56,58,63,255}, borde{112,105,83,255};
    DrawRectangle(0,0,Mapa::ancho,Mapa::alto,cesped);
    DrawRectangle(554,0,492,Mapa::alto,{92,90,69,255});
    DrawRectangle(560,0,480,Mapa::alto,asfalto);
    DrawRectangle(569,0,4,Mapa::alto,borde);
    DrawRectangle(1027,0,4,Mapa::alto,borde);
    DrawRectangle(795,0,4,Mapa::alto,{165,133,65,255});
    DrawRectangle(803,0,4,Mapa::alto,{148,117,60,255});
    for(int y=16;y<Mapa::alto;y+=82)
        for(int x : {677,919}) DrawRectangle(x,y,4,38,{118,119,113,255});
    Vector2 desde = GetScreenToWorld2D({0,0},cam), hasta = GetScreenToWorld2D({Ancho,Alto},cam);
    for (int y = std::max(0,int(desde.y)/16*16); y < std::min(float(Mapa::alto),hasta.y+16); y += 16)
        for (int x = std::max(0,int(desde.x)/16*16); x < std::min(float(Mapa::ancho),hasta.x+16); x += 16) {
            unsigned h = trama(x,y);
            int xx = x + int((h>>8)%12), yy=y+int((h>>16)%12);
            if (xx >= 550 && xx < 1050) {
                if (h%11 == 0) DrawRectangle(xx,yy,3,2,{49,50,55,255});
                continue;
            }
            if (h%5 == 0) continue;
            Color matiz = h%3 == 0 ? Color{83,95,60,255}:Color{59,70,47,255};
            DrawRectangle(xx,yy,2,2+h%2,matiz);
            if (h%4 == 0) DrawRectangle(xx+3,yy+2,2,2,matiz);
        }
}
void imagen(Texture2D t, Rectangle r, Color color = WHITE) {
    DrawTexturePro(t, {0, 0, float(t.width), float(t.height)}, r, {}, 0, color);
}
void centrado(const char* texto, float x, float y, int tamano, Color color = Claro) {
    DrawText(texto, int(x) - MeasureText(texto, tamano) / 2, int(y), tamano, color);
}
void escalon(Rectangle r, float corte, Color c) {
    DrawRectangleRec({r.x + corte, r.y, r.width - 2 * corte, r.height}, c);
    DrawRectangleRec({r.x, r.y + corte, r.width, r.height - 2 * corte}, c);
}
Texture2D marco{}, interior{}, circulo{}, ondaGordo{}, controles[2]{};
void panel(Rectangle r, Color color, Color luz, bool activo = false) {
    Rectangle destino{r.x,r.y,r.width+4,r.height+6};
    NPatchInfo ficha{{0,0,104,54},13,12,16,16,NPATCH_NINE_PATCH};
    DrawTextureNPatch(marco,ficha,{r.x+4,r.y+6,destino.width,destino.height},{},0,Negro);
    DrawTextureNPatch(marco,ficha,destino,{},0,activo?WHITE:luz);
    DrawTextureNPatch(interior,ficha,destino,{},0,color);
}
bool encima(Rectangle r) { return CheckCollisionPointRec(GetMousePosition(), r); }
bool pulsado(Rectangle r) { return encima(r) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT); }
Texture2D portada{}, botones[3]{}, iconosCarta[4]{};
Texture2D barraVida{}, rellenoVida{};
void monedasConIcono(int cantidad,float x,float y,int tam,bool ganadas=false) {
    const char* numero=TextFormat(ganadas?"+%d":"%d",cantidad);
    float inicio=x-(tam+8+MeasureText(numero,tam))/2.0f;
    imagen(iconosCarta[int(Carta::Monedas)],{inicio,y-1,float(tam),float(tam+3)});
    DrawText(numero,int(inicio+tam+8),int(y),tam,Oro);
}
bool boton(Rectangle r, const char* texto, bool habilitado = true) {
    bool activo = habilitado && encima(r);
    Texture2D t=botones[habilitado ? (activo ? 1 : 0) : 2];
    DrawTextureNPatch(t,{{0,0,float(t.width),float(t.height)},13,12,16,16,NPATCH_NINE_PATCH},
                      {r.x,r.y,r.width+4,r.height+6},{},0,WHITE);
    centrado(texto, r.x + r.width / 2, r.y + (r.height - 23) / 2, 22, habilitado ? Claro : GRAY);
    return habilitado && pulsado(r);
}
void actor(Texture2D t, Actor a, int cuadro, float tamano, Color color = WHITE) {
    int columnas = std::max(1, t.width / cuadro), filas = std::max(1, t.height / cuadro);
    DrawTexturePro(t, {float(int(a.animacion) % columnas * cuadro), float(a.fila % filas * cuadro), float(cuadro), float(cuadro)},
                   {a.pos.x - tamano / 2, a.pos.y - tamano + 7, tamano, tamano}, {}, 0, color);
}
void dibujarOnda(const OndaGordo& onda) {
    int f=std::min(13,int(14*onda.edad/(TiempoOnda+TiempoDestello)));
    DrawTexturePro(ondaGordo,{280.f*f,0,280,192},{onda.centro.x,onda.centro.y+7,280,192},{140,96},0,WHITE);
}
void disco(int x,int y,int radio,Color c) {
    imagen(circulo,{float(x-radio),float(y-radio),float(2*radio),float(2*radio)},c);
}
}
Arte::Arte() {
    const char* rutas[]={"dano.png","pluma.png","moneda.png","vendaje.png"};
    for(int i=0;i<4;++i) iconosCarta[i]=textura(std::string("assets/ui/")+rutas[i]);
    marco=textura("assets/ui/marco.png");
    interior=textura("assets/ui/interior.png");
    circulo=textura("assets/ui/disco.png");
    ondaGordo=textura("assets/effects/onda_gordo.png");
    for(int i=0;i<2;++i) controles[i]=textura(TextFormat("assets/ui/controles%d.png",i));
    jugador = textura("assets/sprites/player/player_walk.png");
    for (int i = 0; i < Mapa::Cantidad; ++i)
        props[i] = textura(std::string("assets/props/") + Mapa::props[i].imagen);
    for (int i = 0; i < CantidadZombis; ++i) enemigos[i] = textura(std::string("assets/sprites/zombies/") + tipos[i].imagen);
    for (int i = 0; i < CantidadArmas; ++i) {
        iconos[i] = textura(std::string("assets/sprites/armas/") + armas[i].imagen);
        ataques[i] = LoadSound((std::string("assets/sounds/") + armas[i].sonido).c_str());
        SetSoundVolume(ataques[i], .45f);
    }
    herida = LoadSound("assets/sounds/dano_jugador.wav");
    baja = LoadSound("assets/sounds/muerte_zombie.wav");
    oleada = LoadSound("assets/sounds/oleada.wav");
    derrota = LoadSound("assets/sounds/derrota.wav");
    clic = LoadSound("assets/sounds/ui.wav");
    carga = LoadSound("assets/sounds/carga_correlon.wav");
    lanzamiento = LoadSound("assets/sounds/roca_gymbro.wav");
    baba = LoadSound("assets/sounds/zombie_risa.wav");
    gemido = LoadSound("assets/sounds/gemido_zombie.wav");
    carta = LoadSound("assets/sounds/carta.wav");
    elegida = LoadSound("assets/sounds/recibircarta_elegida.wav");
    roca = textura("assets/props/bouler.png");
    salpicadura = textura("assets/effects/baba.png");
    tajo = textura("assets/effects/slash_sword.png");
    bala = textura("assets/sprites/armas/bala.png");
    corazon = textura("assets/ui/corazon.png");
    relleno = textura("assets/ui/corazon_lleno.png");
    barraVida = textura("assets/ui/barra_vida.png");
    rellenoVida = textura("assets/ui/barra_vida_relleno.png");
    marcador = textura("assets/ui/gymbro.png");
    muerte = textura("assets/effects/muerte.png");
    portada = textura("assets/ui/portada.png");
    for(int i=0;i<3;++i) botones[i]=textura(TextFormat("assets/ui/boton%d.png",i));

}
Arte::~Arte() {
    for (Texture2D t : {jugador, corazon, relleno, roca, tajo, bala, salpicadura, marcador, muerte, portada}) UnloadTexture(t);
    for(Texture2D t : {marco,interior,circulo,ondaGordo,controles[0],controles[1]}) UnloadTexture(t);
    for (Texture2D t : iconosCarta) UnloadTexture(t);
    for (Texture2D t : botones) UnloadTexture(t);
    for (Texture2D t : {barraVida, rellenoVida}) UnloadTexture(t);
    for (Texture2D t : props) UnloadTexture(t);
    for (Texture2D t : enemigos) UnloadTexture(t);
    for (Texture2D t : iconos) UnloadTexture(t);
    for (Sound s : ataques) UnloadSound(s);
    for (Sound s : {herida, baja, oleada, derrota, clic, carga, lanzamiento, baba, gemido, carta, elegida}) UnloadSound(s);
}
namespace {
void controlesMenu() {
    constexpr int teclas[]={KEY_W,KEY_A,KEY_S,KEY_D,KEY_UP,KEY_LEFT,KEY_DOWN,KEY_RIGHT};
    auto indicar=[](Rectangle r,bool activo) {
        DrawTexturePro(controles[activo || encima(r)],r,r,{},0,WHITE);
    };
    for(int i=0;i<8;++i) {
        int j=i%4;
        indicar({float((i<4?83:222)+(j==0?39:(j-1)*39)),float(j==0?553:590),34,36},IsKeyDown(teclas[i]));
    }
    indicar({408,576,144,42},IsKeyDown(KEY_SPACE));
    indicar({713,548,48,66},IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    centrado("MOVER", 210, 638, 17);
    centrado("AUTOATAQUE", 480, 638, 17);
    centrado("APUNTAR", 737, 638, 17);
}
}
Accion dibujarMenu(int, int record, const std::string& titular, int oleadas, int modo, const std::string& global) {
    imagen(portada,{0,0,Ancho,Alto});
    centrado("EL AMANECER DE LOS MUERTOS", Ancho / 2, 203, 22, Oro);
    Accion a;
    a.jugar = boton({106, 338, 300, 76}, "JUGAR");
    panel({490,306,353,174},Sombra,{130,84,92,255});
    centrado("RÉCORD LOCAL",666,326,19,Oro);
    centrado(record ? TextFormat("%d PUNTOS",record) : "SIN RÉCORD",666,354,27);
    if(record) {
        centrado(titular.c_str(),666,390,21);
        centrado(modo < 0 ? "OLEADA Y MODO SIN DATOS" : TextFormat("OLEADA %d  /  %s",oleadas,nombreModo(static_cast<Modo>(modo))),
                 666,427,17,Claro);
    }
    if (!global.empty()) centrado(global.c_str(), Ancho / 2, 503, 17, Oro);
    controlesMenu();
    return a;
}
void dibujarCarga(const Arte& arte, float t) {
    if (t < 2.53f) {
        DrawRectangle(0, 0, Ancho, Alto, BLACK);
        float alfa = Clamp((2.53f-t)/.28f,0,1);
        float progreso = t < .63f ? t * .24f : t < .93f ? .1512f : Clamp(.1512f + (t - .93f) / 1.32f * .8488f, 0, 1);
        Rectangle barra{174, 369, 612, 29};
        escalon(barra, 5, Fade(GRAY, alfa));
        escalon({179,374,602,19}, 3, Fade(Sombra, alfa));
        int avance = progreso >= 1 ? 598 : int(598 * progreso / 4) * 4;
        DrawRectangle(181, 376, avance, 15, Fade(Rojo,alfa));
        actor(arte.enemigos[0], {{float(181 + avance), 346}, 2, avance / 9.0f}, 16, 70, Fade(WHITE,alfa));
        centrado("CARGANDO", Ancho / 2, 435, 22, Fade(Claro,alfa));
        return;
    }
    float progreso = (t - 2.53f) / (FinCarga - 2.53f);
    for (int y = 0; y < Alto; y += 12) for (int x = 0; x < Ancho; x += 12) {
        float nx = std::abs(x - Ancho / 2) / (Ancho / 2.0f), ny = std::abs(y - Alto / 2) / (Alto / 2.0f);
        unsigned ruido = (unsigned(x / 12) * 73856093u) ^ (unsigned(y / 12) * 19349663u);
        float umbral = (.53f * nx + .22f * ny + .16f * ((ruido % 101) / 100.0f));
        float alfa = Clamp((umbral - progreso) * 11 + 1, 0, 1);
        if (alfa > 0) DrawRectangle(x, y, 12, 12, Fade(BLACK, alfa));
    }
}
Accion dibujarArmas(const Arte& arte, const OpcionArma* opciones, int seleccion, Modo modo, int monedas, float tiempo) {
    ClearBackground({66, 15, 35, 255});
    constexpr Color sangre[] = {{109, 22, 48, 255}, {91, 40, 94, 255}, {188, 101, 137, 255}};
    for (int i = 0; i < 24; ++i) {
        int radio = 12 + (i * 5 % 35), y = 21 + i * 137 % 690;
        int x = 1080 - int(std::fmod(i * 173 + tiempo * (23 + (i % 3) * 10), 1200.0f));
        disco(x, y, radio, sangre[i % 3]);
        disco(x - radio / 3, y - radio / 3, radio / 4, Fade(WHITE, .30f));
    }
    centrado("ELIGE TU ARMA", Ancho / 2, 54, 41);
    monedasConIcono(monedas,Ancho/2,112,22);
    Accion a;
    for (int slot = -1; slot <= 1; ++slot) {
        int i = (seleccion + slot + CantidadArmas) % CantidadArmas, x = 480 + slot * 231;
        const Arma& f = armas[i];
        Rectangle r{float(x - 108), 176, 216, 333};
        bool foco = slot == 0 || encima(r);
        panel(r, slot == 0 ? Color{63, 31, 54, 255} : Gris, foco ? Oro : Rojo, encima(r));
        centrado(f.nombre, x, 203, 25);
        disco(x, 308, 62, foco ? Color{140, 63, 99, 255} : Sombra);
        disco(x, 306, 49, foco ? Color{190, 99, 129, 255} : Gris);
        imagen(arte.iconos[i], {float(x - 48), 257, 96, 96});
        centrado(TextFormat("Daño  %d", f.dano), x, 389, 20);
        centrado(TextFormat("Ataques/s  %.1f", 1 / f.intervalo), x, 416, 20);
        centrado(TextFormat("Corazones  %d", CorazonesBase + (modo==Modo::Imposible ? 1-CorazonesBase : f.corazonesExtra)), x, 443, 19);
        if(opciones[i].abierta) centrado("DISPONIBLE",x,473,17,Oro);
        else monedasConIcono(f.precio,x,473,19);
        if (pulsado(r)) a.elegir = i;
    }
    if (boton({31, 299, 66, 62}, "<") || IsKeyPressed(KEY_LEFT)) a.elegir = (seleccion + CantidadArmas - 1) % CantidadArmas;
    if (boton({863, 299, 66, 62}, ">") || IsKeyPressed(KEY_RIGHT)) a.elegir = (seleccion + 1) % CantidadArmas;
    a.menu = boton({134, 582, 206, 60}, "MENÚ");
    a.dificultad = boton({351, 582, 206, 60}, nombreModo(modo));
    if (opciones[seleccion].abierta) a.jugar = boton({568, 582, 257, 60}, "JUGAR");
    else a.comprar = boton({568, 582, 257, 60}, "DESBLOQUEAR", monedas >= armas[seleccion].precio);
    return a;
}
void dibujarPartida(const Arte& arte, const Partida& p) {
    ClearBackground({34, 35, 42, 255});
    Camera2D vista = camara(p);
    BeginMode2D(vista);
    terreno(vista);
    DrawRectangleLinesEx({0, 0, Mapa::ancho, Mapa::alto}, Mapa::borde, {53,62,47,255});
    DrawRectangleLinesEx({20, 20, Mapa::ancho-40, Mapa::alto-40}, 4, {82,87,63,255});
    auto dibujarProp = [&](Mapa::Elemento e) {
        const auto& ficha = Mapa::props[e.tipo];
        imagen(arte.props[e.tipo], {float(e.x), float(e.y), float(ficha.ancho), float(ficha.alto)});
    };
    auto dibujarBala = [&](const Bala& b) {
        DrawTexturePro(arte.bala, {0, 0, float(arte.bala.width), float(arte.bala.height)},
                       {b.pos.x, b.pos.y, 13, 5}, {6.5f, 2.5f}, std::atan2(b.rumbo.y, b.rumbo.x) * RAD2DEG, WHITE);
    };
    auto dibujarZombie = [&](const Zombie& z) {
        auto pos = z.actor.pos;
        DrawEllipse(int(pos.x), int(pos.y + 6), 13, 5, Fade(BLACK, .4f));
        if (z.tipo == Gymbro) DrawRing(pos, 18, 20, 0, 360, 24, Fade(Rojo, .7f));
        const auto& ficha = tipos[z.tipo];
        actor(arte.enemigos[z.tipo], z.actor, ficha.cuadro, ficha.tamano, z.especial > 0 ? Oro : WHITE);
        int maxVida = p.vidaZombie(z.tipo);
        if (z.vida < maxVida || z.tipo == Gymbro) {
            int ancho = z.tipo == Gymbro ? 34 : 22;
            int x = int(pos.x)-ancho/2, y = int(pos.y)-ficha.tamano;
            imagen(barraVida, {float(x-2), float(y-2), float(ancho+4), 7});
            if (z.vida > 0) {
                int lleno = std::clamp(ancho*z.vida/maxVida, 1, ancho);
                Color color = z.tipo == Gymbro ? Color{187,129,235,255} : Color{246,109,136,255};
                DrawTexturePro(rellenoVida, {0,0,float(lleno),3},
                               {float(x),float(y),float(lleno),3}, {}, 0, color);
            }
        }
    };
    auto dibujarJugador = [&] {
        if (p.termino()) return;
        DrawEllipse(int(p.jugador.pos.x), int(p.jugador.pos.y + 6), 13, 5, Fade(BLACK, .4f));
        if (!p.invulnerable || int(p.invulnerable * 16) % 2 == 0) actor(arte.jugador, p.jugador, 16, 34);
        if (p.efectoAtaque <= 0 || armas[p.arma].distancia) return;
        float progreso = Clamp(1 - p.efectoAtaque / .18f, 0, 1);
        float angulo = std::atan2(p.rumboAtaque.y, p.rumboAtaque.x) - 1.28f + progreso * 2.56f;
        float radio = armas[p.arma].alcance * .73f;
        Vector2 punto = Vector2Add(p.jugador.pos, {std::cos(angulo) * radio, std::sin(angulo) * radio});
        float frame = float(std::min(3, int(progreso * 4))) * 32;
        DrawTexturePro(arte.tajo, {frame, 0, 32, 32}, {punto.x, punto.y, 42, 22}, {21, 11},
                       angulo * RAD2DEG + 90, Fade(WHITE, Clamp((1 - progreso) * 4, 0, 1)));
    };
    enum Capa { Objeto, Enemigo, Personaje, Disparo, Piedra };
    struct Dibujo { float base; Capa capa; size_t indice; };
    static std::vector<Dibujo> orden;
    orden.clear();
    for (size_t i = 0; i < std::size(Mapa::elementos); ++i) {
        auto e = Mapa::elementos[i];
        const auto& ficha = Mapa::props[e.tipo];
        float base = ficha.cuerpo.height > 0 ? ficha.cuerpo.y + ficha.cuerpo.height : ficha.alto;
        orden.push_back({e.y + base, Objeto, i});
    }
    for (size_t i = 0; i < p.zombies.size(); ++i) orden.push_back({p.zombies[i].actor.pos.y + 7, Enemigo, i});
    for (size_t i = 0; i < p.balas.size(); ++i) orden.push_back({p.balas[i].pos.y + 2, Disparo, i});
    for (size_t i = 0; i < p.rocas.size(); ++i) orden.push_back({p.rocas[i].pos.y + 9, Piedra, i});
    orden.push_back({p.jugador.pos.y + 7, Personaje, 0});
    std::sort(orden.begin(), orden.end(), [](Dibujo a, Dibujo b) {
        if (a.base != b.base) return a.base < b.base;
        return a.capa != b.capa ? a.capa < b.capa : a.indice < b.indice;
    });
    for (const auto& onda : p.ondas) dibujarOnda(onda);
    for (auto d : orden) switch (d.capa) {
    case Objeto: dibujarProp(Mapa::elementos[d.indice]); break;
    case Enemigo: dibujarZombie(p.zombies[d.indice]); break;
    case Personaje: dibujarJugador(); break;
    case Disparo: dibujarBala(p.balas[d.indice]); break;
    case Piedra: {
        const auto& roca = p.rocas[d.indice];
        imagen(arte.roca, {roca.pos.x - 11, roca.pos.y - 11, 22, 22});
        break;
    }
    }
    for (const Destello& d:p.destellos) {
        int frame=std::min(9,int(d.edad/.52f*10));
        float ancho=d.radio*2.2f;
        DrawTexturePro(arte.muerte,{80.f*frame,0,80,80},{d.pos.x,d.pos.y,ancho,ancho},{ancho/2,ancho/2},0,WHITE);
    }
    EndMode2D();
    DrawRectangleGradientV(0,0,Ancho,Alto,{18,14,30,66},{15,12,25,24});
    if(p.termino()) return;
    if (p.baba > 0) imagen(arte.salpicadura, {0, 0, Ancho, Alto}, Fade(WHITE, .90f * std::min(p.baba, 1.0f)));
    if (p.avisoOleada > 0) {
        float alfa = std::min(1.0f, p.avisoOleada * 2);
        DrawRectangle(365, 95, 230, 51, Fade(Negro, .72f * alfa));
        centrado(TextFormat("OLEADA %d", p.ronda), Ancho/2,108,26,Fade(Claro,alfa));
    }
    for (const Zombie& z : p.zombies) {
        if (z.tipo != Gymbro) continue;
        Vector2 punto = GetWorldToScreen2D(z.actor.pos,vista);
        float tamano=tipos[Gymbro].tamano*vista.zoom;
        Rectangle sprite{punto.x-tamano/2,punto.y-tamano+7*vista.zoom,tamano,tamano};
        if (CheckCollisionRecs(sprite,{0,72,Ancho,Alto-72})) continue;
        Vector2 d = Vector2Subtract(punto,{Ancho/2.0f,Alto/2.0f});
        float factor = std::min((Ancho/2-41)/std::max(1.0f,std::abs(d.x)),
                               (Alto/2-83)/std::max(1.0f,std::abs(d.y)));
        Vector2 borde = Vector2Add({Ancho/2.0f,Alto/2.0f},Vector2Scale(d,factor));
        float lejos = std::max({0.0f,-punto.x,punto.x-Ancho,-punto.y,punto.y-Alto});
        float tam = 46 * (1 + .30f * (1-Clamp(lejos/260,0,1)));
        Rectangle placa{borde.x-tam/2-4,borde.y-tam/2-4,tam+8,tam+8};
        panel(placa,Sombra,Color{155,95,195,255});
        imagen(arte.marcador,{borde.x-tam/2,borde.y-tam/2,tam,tam});
    }
    DrawRectangle(0,0,Ancho,72,Fade(Negro,.89f));
    DrawRectangle(0,71,Ancho,2,Color{91,56,70,255});
    centrado(TextFormat("OLEADA %02d",p.ronda),Ancho/2,12,27);
    centrado(nombreModo(p.modo),Ancho/2,45,16,Rojo);
    DrawText("PUNTAJE",754,11,14,Claro);
    DrawText(TextFormat("%06d",p.puntaje()),751,29,31,Oro);
    for (int i = 0; i < p.maxVida; ++i) {
        Rectangle r{float(22+i*43),655,38,34};
        imagen(arte.corazon,r);
        if(i<p.vida) imagen(arte.relleno,r);
    }
    monedasConIcono(p.monedasPartida,682,674,20,true);
    boton(BotonSkip,"SALTAR",p.esperaSkip<=0);
    panel(BotonPausa,Gris,Rojo,encima(BotonPausa));
    DrawRectangle(901,672,4,18,Claro);
    DrawRectangle(913,672,4,18,Claro);
}
Accion dibujarPausa() {
    for(int y=0;y<Alto;y+=16) for(int x=0;x<Ancho;x+=16) {
        float borde=std::min({float(x),float(y),float(Ancho-16-x),float(Alto-16-y)});
        float variacion=float(trama(x,y)%21)-10;
        float opacidad=Clamp((202-borde+variacion)/202,0,1)*.70f;
        if(opacidad>0) DrawRectangle(x,y,16,16,Fade(BLACK,opacidad));
    }
    panel({282,225,396,261},Sombra,{137,88,110,255});
    centrado("PAUSA",Ancho/2,252,39);
    Accion a;
    a.jugar=boton({348,322,264,52},"CONTINUAR");
    a.menu=boton({348,391,264,52},"ABANDONAR");
    return a;
}
Accion dibujarCartas(const Partida& p) {
    DrawRectangle(0, 0, Ancho, Alto, Fade(Negro, .91f));
    centrado("MEJORA DISPONIBLE", Ancho / 2, 91, 42);
    centrado("ELIGE UNA CARTA",Ancho/2,158,20);
    constexpr const char* nombres[] = {"DAÑO", "VELOCIDAD", "MONEDAS", "CURACIÓN"};
    constexpr const char* detalles[] = {"+1 daño", "+12 movimiento", "+5 monedas", "+1 corazón"};
    constexpr Color colores[] = {{238, 89, 102, 255}, {109, 200, 241, 255}, Oro, {132, 224, 160, 255}};
    Accion a;
    auto cartas = p.cartas();
    for (int i = 0; i < 3; ++i) {
        int tipo = int(cartas[i]), x = 99 + i * 260;
        Rectangle r{float(x), 246, 242, 269};
        bool disponible = cartas[i] != Carta::Velocidad || p.bonosVelocidad < 8;
        bool hover = disponible && encima(r);
        Rectangle visual = r;
        if (hover) visual.y -= 6;
        panel(visual, Gris, disponible ? colores[tipo] : GRAY, hover);
        disco(x + 121, int(visual.y + 87), 44, Fade(colores[tipo], .26f));
        imagen(iconosCarta[tipo],{float(x+91),visual.y+57,60,60});
        centrado(nombres[tipo], x + 121, visual.y + 150, 27);
        centrado(disponible ? detalles[tipo] : "Velocidad al máximo", x + 121, visual.y + 206, 19);
        if (disponible && pulsado(r)) a.elegir = i;
    }
    return a;
}
Accion dibujarDerrota(const Partida& p, bool record, const std::string& nombre) {
    DrawRectangle(0,0,Ancho,Alto,Fade(Negro,.26f));
    escalon({172,27,616,119},9,Fade(Sombra,.77f));
    centrado("FIN DE LA PARTIDA", Ancho / 2, 45, 43);
    centrado(TextFormat("%d puntos  /  %d oleadas", p.puntaje(), p.completadas), Ancho / 2, 103, 26, Oro);
    Accion a;
    if (record) {
        centrado("NUEVO RÉCORD", Ancho / 2, 481, 27, Rojo);
        panel({200, 527, 323, 49}, Sombra, Rojo);
        centrado(nombre.empty() ? "Escribe tu nombre" : nombre.c_str(), 361, 542, 21);
        a.guardar = boton({545, 527, 213, 49}, "GUARDAR", !nombre.empty());
    }
    a.menu = boton({207, 600, 260, 58}, "MENÚ PRINCIPAL");
    a.jugar = boton({493, 600, 260, 58}, "VOLVER A JUGAR");
    return a;
}
void aviso(const std::string& texto) {
    if (texto.empty()) return;
    DrawRectangle(0, 665, Ancho, 55, Negro);
    centrado(texto.c_str(), Ancho / 2, 682, 20, Oro);
}
