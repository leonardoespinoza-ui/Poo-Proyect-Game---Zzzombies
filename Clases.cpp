#include "Variables.h"
#include <algorithm>
#include <cmath>
#include <vector>

float distancia(Vector2 a, Vector2 b)
{
    float x = a.x - b.x;
    float y = a.y - b.y;
    return std::sqrt(x * x + y * y);
}

Vector2 direccionUnitaria(Vector2 origen, Vector2 destino)
{
    float largo = distancia(origen, destino);
    if (largo < 0.001f) return {0, 0};
    return {(destino.x - origen.x) / largo, (destino.y - origen.y) / largo};
}

float limitar(float valor, float minimo, float maximo)
{
    return std::max(minimo, std::min(valor, maximo));
}

Rectangle colisionPersonaje(Vector2 pos)
{
    return {pos.x + 6, pos.y + 8, 20, 22};
}

Rectangle rectanguloVisual(Datos::Mapa::Obstaculo objeto)
{
    Datos::Mapa::Prop datos = Datos::Mapa::props[objeto.tipo];
    if (objeto.vertical) return {float(objeto.x), float(objeto.y), float(datos.alto), float(datos.ancho)};
    return {float(objeto.x), float(objeto.y), float(datos.ancho), float(datos.alto)};
}

Rectangle colisionObstaculo(Datos::Mapa::Obstaculo objeto)
{
    Datos::Mapa::Prop datos = Datos::Mapa::props[objeto.tipo];
    Rectangle limite = datos.colision;
    if (objeto.vertical)
        return {objeto.x + datos.alto - limite.y - limite.height, objeto.y + limite.x, limite.height, limite.width};
    return {objeto.x + limite.x, objeto.y + limite.y, limite.width, limite.height};
}

bool chocaObstaculo(Rectangle zona)
{
    for (int i = 0; i < int(std::size(Datos::Mapa::obstaculos)); i++) {
        Rectangle limite = colisionObstaculo(Datos::Mapa::obstaculos[i]);
        if (limite.width > 0 && CheckCollisionRecs(zona, limite)) return true;
    }
    return false;
}

Texture2D cargarImagen(const char* ruta)
{
    if (!FileExists(ruta)) return {};
    Texture2D imagen = LoadTexture(ruta);
    if (imagen.id != 0) SetTextureFilter(imagen, TEXTURE_FILTER_POINT);
    return imagen;
}

void mostrarImagen(Texture2D textura, Rectangle zona, Color color = WHITE, float angulo = 0, Vector2 origen = {0, 0})
{
    if (textura.id == 0) return;
    DrawTexturePro(textura, {0, 0, float(textura.width), float(textura.height)}, zona, origen, angulo, color);
}

void liberarImagen(Texture2D &textura)
{
    if (textura.id != 0) UnloadTexture(textura);
    textura = {};
}

void mostrarTextoCentrado(const char* texto, int y, int tamano, Color color)
{
    DrawText(texto, (ANCHO - MeasureText(texto, tamano)) / 2, y, tamano, color);
}

class Salud
{
private:
    int actual;
    int maxima;
public:
    Salud(int cantidad = Datos::vidaJugador) : actual(cantidad), maxima(cantidad) {}
    int getActual() const { return actual; }
    int getMaxima() const { return maxima; }
    bool estaVivo() const { return actual > 0; }
    void recibirDano(int cantidad) { actual = std::max(0, actual - cantidad); }
    void curar(int cantidad) { actual = std::min(maxima, actual + cantidad); }
};

class Arma
{
private:
    int seleccion = 0;
    bool obtenidas[3] = {true, false, false};
    int balas[3] = {};
    float espera = 0;
public:
    void actualizar(float dt) { espera = std::max(0.0f, espera - dt); }
    int getSeleccion() const { return seleccion; }
    int getBalas() const { return balas[seleccion]; }
    bool tiene(int indice) const { return obtenidas[indice]; }
    bool seleccionar(int indice) {
        if (indice < 0 || indice > 2 || !obtenidas[indice]) return false;
        seleccion = indice;
        return true;
    }
    void equipar(int indice) {
        obtenidas[indice] = true;
        balas[indice] = Datos::armas[indice].municion;
        seleccion = indice;
    }
    bool recargar() {
        bool cambio = false;
        for (int i = 1; i < 3; i++) {
            if (obtenidas[i] && balas[i] < Datos::armas[i].municion) {
                balas[i] = Datos::armas[i].municion;
                cambio = true;
            }
        }
        return cambio;
    }
    bool atacar() {
        if (espera > 0 || (seleccion > 0 && balas[seleccion] == 0)) return false;
        espera = Datos::armas[seleccion].espera;
        if (seleccion > 0) balas[seleccion]--;
        return true;
    }
};

class Personaje
{
protected:
    Vector2 posicion;
    float velocidad;
    int orientacion = 0;
    int frame = 0;
    float animacion = 0;
    Salud salud;
    void mover(Vector2 movimiento, float dt) {
        posicion.x += movimiento.x * velocidad * dt;
        posicion.y += movimiento.y * velocidad * dt;
        posicion.x = limitar(posicion.x, 24, Datos::Mapa::ancho - 56);
        posicion.y = limitar(posicion.y, 24, Datos::Mapa::alto - 56);
        if (movimiento.x == 0 && movimiento.y == 0) { frame = 0; animacion = 0; return; }
        orientacion = 0;
        if (std::abs(movimiento.x) > std::abs(movimiento.y)) {
            orientacion = 2;
            if (movimiento.x < 0) orientacion = 1;
        }
        else if (movimiento.y < 0) orientacion = 3;
        animacion += dt;
        if (animacion >= 0.15f) {
            animacion = 0;
            if (frame == 1) frame = 2;
            else frame = 1;
        }
        if (frame == 0) frame = 1;
    }
public:
    Personaje(Vector2 inicio, float rapidez, int vida) : posicion(inicio), velocidad(rapidez), salud(vida) {}
    virtual ~Personaje() = default;
    virtual void actualizar(float dt, Vector2 objetivo) = 0;
    Vector2 getPosicion() const { return posicion; }
    Vector2 getCentro() const { return {posicion.x + 16, posicion.y + 16}; }
    int getFrame() const { return frame; }
    int getOrientacion() const { return orientacion; }
    Salud &getSalud() { return salud; }
    const Salud &getSalud() const { return salud; }
};

void actualizarPersonaje(Personaje &personaje, float dt, Vector2 objetivo)
{
    personaje.actualizar(dt, objetivo);
}

class Jugador : public Personaje
{
private:
    Arma arma;
public:
    Jugador() : Personaje({Datos::Mapa::ancho / 2.0f, Datos::Mapa::alto / 2.0f}, Datos::velocidadJugador, Datos::vidaJugador) {}
    Arma &getArma() { return arma; }
    void actualizar(float dt, Vector2) override {
        Vector2 movimiento = {0, 0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) movimiento.y--;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) movimiento.y++;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movimiento.x--;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movimiento.x++;
        movimiento = direccionUnitaria({0, 0}, movimiento);
        Vector2 anterior = posicion;
        mover(movimiento, dt);
        float nuevaY = posicion.y;
        posicion.y = anterior.y;
        if (chocaObstaculo(colisionPersonaje(posicion))) posicion.x = anterior.x;
        posicion.y = nuevaY;
        if (chocaObstaculo(colisionPersonaje(posicion))) posicion.y = anterior.y;
        arma.actualizar(dt);
    }
};

class Zombie : public Personaje
{
private:
    int tipo;
    float espera = 0;
public:
    Zombie(Vector2 inicio, int elegido, int ronda) :
        Personaje(inicio, Datos::zombies[elegido].velocidad, Datos::zombies[elegido].vida + ronda - 1), tipo(elegido) {}
    int getTipo() const { return tipo; }
    void actualizar(float dt, Vector2 objetivo) override {
        if (!salud.estaVivo()) return;
        espera = std::max(0.0f, espera - dt);
        Vector2 movimiento = direccionUnitaria(getCentro(), objetivo);
        float original = velocidad;
        if (chocaObstaculo(colisionPersonaje(posicion))) velocidad = std::max(8.0f, velocidad - 24);
        mover(movimiento, dt);
        velocidad = original;
    }
    bool puedeGolpear(Vector2 objetivo) {
        if (!salud.estaVivo() || espera > 0 || distancia(getCentro(), objetivo) > 27) return false;
        espera = Datos::zombies[tipo].espera;
        return true;
    }
};

class Proyectil
{
private:
    Vector2 posicion = {};
    Vector2 rumbo = {};
    float restante = 0;
    int dano = 0;
public:
    bool activo() const { return restante > 0; }
    Vector2 getPosicion() const { return posicion; }
    Vector2 getDireccion() const { return rumbo; }
    int getDano() const { return dano; }
    void detener() { restante = 0; }
    void lanzar(Vector2 origen, Vector2 destino, int golpe, float alcance) {
        rumbo = direccionUnitaria(origen, destino);
        posicion = origen;
        dano = golpe;
        restante = alcance;
    }
    void avanzar(float distanciaPaso) {
        posicion.x += rumbo.x * distanciaPaso;
        posicion.y += rumbo.y * distanciaPaso;
        restante = std::max(0.0f, restante - distanciaPaso);
    }
    Rectangle getCuerpo() const { return {posicion.x - 3, posicion.y - 3, 6, 6}; }
};

class Objeto
{
public:
    TipoObjeto tipo;
    Vector2 posicion;
    float tiempo = Datos::duracionObjeto;
    float fase = 0;
};

class Oleada
{
private:
    const std::vector<Zombie> &enemigos;
    int numero = 0;
    int pendientes = 0;
    float espera = 0;
    bool imposible = false;
public:
    Oleada(const std::vector<Zombie> &lista) : enemigos(lista) {}
    int getNumero() const { return numero; }
    int getPendientes() const { return pendientes; }
    int getActivos() const {
        int vivos = 0;
        for (int i = 0; i < int(enemigos.size()); i++)
            if (enemigos[i].getSalud().estaVivo()) vivos++;
        return vivos;
    }
    void iniciar(int ronda, bool modo) {
        numero = ronda;
        imposible = modo;
        pendientes = 2 + ronda * 2;
        if (imposible) pendientes += ronda * 2;
        espera = 0.15f;
    }
    int siguiente(float dt) {
        espera -= dt;
        if (pendientes == 0 || espera > 0) return -1;
        pendientes--;
        espera = std::max(0.24f, 0.8f - numero * 0.03f);
        if (imposible) espera *= 0.7f;
        int maximo = 0;
        if (numero >= 3 || imposible) maximo = 1;
        if (numero >= 5 || imposible) maximo = 2;
        return GetRandomValue(0, maximo);
    }
    bool terminada() const { return pendientes == 0 && getActivos() == 0; }
};

class Sonidos
{
private:
    Sound efectos[9] = {};
    Sound voces[2][Datos::vocesZombie] = {};
    Vector2 origenes[2][Datos::vocesZombie] = {};
    double siguiente[9] = {};
    double proximoGemido = 0;
    bool listo = false;
public:
    Sonidos() = default;
    Sonidos(const Sonidos&) = delete;
    Sonidos& operator=(const Sonidos&) = delete;
    ~Sonidos() { liberar(); }
    void cargar() {
        InitAudioDevice();
        listo = IsAudioDeviceReady();
        if (!listo) return;
        for (int i = 0; i < 9; i++) {
            if (!FileExists(rutasAudio[i])) continue;
            Wave muestra = LoadWave(rutasAudio[i]);
            if (!IsWaveValid(muestra)) continue;
            WaveFormat(&muestra, 22050, 16, 1);
            efectos[i] = LoadSoundFromWave(muestra);
            UnloadWave(muestra);
        }
        for (int grupo = 0; grupo < 2; grupo++) {
            int evento = int(Audio::Gemido);
            if (grupo == 1) evento = int(Audio::Muerte);
            if (!IsSoundValid(efectos[evento])) continue;
            for (int i = 0; i < Datos::vocesZombie; i++) voces[grupo][i] = LoadSoundAlias(efectos[evento]);
        }
    }
    void emitir(Audio evento) {
        int i = int(evento);
        if (!listo || !IsSoundValid(efectos[i]) || GetTime() < siguiente[i]) return;
        siguiente[i] = GetTime() + 0.05;
        PlaySound(efectos[i]);
    }
    void zombie(bool muerte, Vector2 origen, Vector2 oyente) {
        if (!listo || distancia(origen, oyente) > Datos::radioAudio) return;
        int grupo = 0;
        if (muerte) grupo = 1;
        for (int i = 0; i < Datos::vocesZombie; i++) {
            if (!IsSoundValid(voces[grupo][i]) || IsSoundPlaying(voces[grupo][i])) continue;
            origenes[grupo][i] = origen;
            SetSoundVolume(voces[grupo][i], 1 - 0.8f * distancia(origen, oyente) / Datos::radioAudio);
            SetSoundPitch(voces[grupo][i], GetRandomValue(85, 115) / 100.0f);
            PlaySound(voces[grupo][i]);
            return;
        }
    }
    bool tocaGemido() {
        if (GetTime() < proximoGemido) return false;
        proximoGemido = GetTime() + GetRandomValue(350, 800) / 1000.0;
        return true;
    }
    void actualizar(Vector2 oyente) {
        if (!listo) return;
        for (int grupo = 0; grupo < 2; grupo++)
            for (int i = 0; i < Datos::vocesZombie; i++) {
                if (!IsSoundValid(voces[grupo][i]) || !IsSoundPlaying(voces[grupo][i])) continue;
                float d = distancia(origenes[grupo][i], oyente);
                if (d > Datos::radioAudio) StopSound(voces[grupo][i]);
                else SetSoundVolume(voces[grupo][i], 1 - 0.8f * d / Datos::radioAudio);
            }
    }
    void detener() {
        if (!listo) return;
        for (int i = 0; i < 9; i++)
            if (i != int(Audio::UI) && IsSoundValid(efectos[i])) StopSound(efectos[i]);
        for (int grupo = 0; grupo < 2; grupo++)
            for (int i = 0; i < Datos::vocesZombie; i++)
                if (IsSoundValid(voces[grupo][i])) StopSound(voces[grupo][i]);
    }
    void liberar() {
        if (!listo) return;
        for (int grupo = 0; grupo < 2; grupo++)
            for (int i = 0; i < Datos::vocesZombie; i++)
                if (IsSoundValid(voces[grupo][i])) UnloadSoundAlias(voces[grupo][i]);
        for (int i = 0; i < 9; i++)
            if (IsSoundValid(efectos[i])) UnloadSound(efectos[i]);
        CloseAudioDevice();
        listo = false;
    }
};

class Caja
{
private:
    Sonidos &sonidos;
    int vida = 0;
    Vector2 posicion = {};
public:
    Caja(Sonidos &audio) : sonidos(audio) {}
    bool activa() const { return vida > 0; }
    int getVida() const { return vida; }
    Vector2 getPosicion() const { return posicion; }
    Rectangle getCuerpo() const { return {posicion.x, posicion.y, 20, 20}; }
    void reiniciar() { vida = 0; }
    void aparecer(Vector2 jugador) {
        if (activa()) return;
        for (int intento = 0; intento < 100; intento++) {
            posicion = {float(GetRandomValue(584, 996)), float(GetRandomValue(130, Datos::Mapa::alto - 180))};
            if (!chocaObstaculo(getCuerpo()) && distancia(posicion, jugador) > 48) {
                vida = Datos::vidaCaja;
                return;
            }
        }
    }
    bool golpear(int dano) {
        if (!activa()) return false;
        vida = std::max(0, vida - dano);
        if (vida == 0) sonidos.emitir(Audio::UI);
        return vida == 0;
    }
};

class Juego
{
private:
    enum class Pantalla { Menu, Combate, Derrota };
    enum class TipoDibujo { Prop, Caja, Objeto, Zombie, Jugador, Bala };
    class Elemento {
    public:
        float profundidad;
        TipoDibujo tipo;
        int indice;
    };
    Pantalla pantalla = Pantalla::Menu;
    Jugador jugador;
    std::vector<Zombie> zombies;
    std::vector<Objeto> objetos;
    std::vector<Elemento> orden;
    Oleada oleada;
    Sonidos sonidos;
    Caja caja;
    Proyectil proyectiles[Datos::maxBalas];
    Texture2D mapa = {};
    Texture2D personaje = {};
    Texture2D enemigos[3] = {};
    Texture2D armas[3] = {};
    Texture2D recogibles[4] = {};
    Texture2D props[Datos::Mapa::CantidadProps] = {};
    Texture2D interfaz[5] = {};
    Texture2D bala = {};
    Texture2D tajo = {};
    Camera2D camara = {{ANCHO / 2.0f, HUD_ALTO + VISTA_ALTO / 2.0f}, {}, 0, Datos::Mapa::zoomCamara};
    bool imposible = false;
    bool salir = false;
    bool bloquearAtaque = true;
    int eliminados = 0;
    float tiempoSkip = 0;
    float tiempoTajo = 0;
    Vector2 direccionAtaque = {1, 0};

    void cargar() {
        mapa = cargarImagen("assets/mapas/carretera.png");
        personaje = cargarImagen("assets/sprites/player/player_walk.png");
        bala = cargarImagen("assets/sprites/armas/bala.png");
        tajo = cargarImagen("assets/effects/slash_sword.png");
        for (int i = 0; i < 3; i++) {
            enemigos[i] = cargarImagen(Datos::zombies[i].imagen);
            armas[i] = cargarImagen(Datos::armas[i].imagen);
        }
        recogibles[0] = cargarImagen("assets/props/botiquin.png");
        recogibles[1] = cargarImagen("assets/props/municion.png");
        recogibles[2] = cargarImagen(Datos::armas[1].imagen);
        recogibles[3] = cargarImagen(Datos::armas[2].imagen);
        for (int i = 0; i < Datos::Mapa::CantidadProps; i++) props[i] = cargarImagen(Datos::Mapa::props[i].ruta);
        const char* rutas[5] = {"assets/ui/button.png", "assets/ui/weapon_slot.png", "assets/ui/heart.png", "assets/ui/skull.png", "assets/props/caja.png"};
        for (int i = 0; i < 5; i++) interfaz[i] = cargarImagen(rutas[i]);
        sonidos.cargar();
    }
    void liberar() {
        liberarImagen(mapa);
        liberarImagen(personaje);
        liberarImagen(bala);
        liberarImagen(tajo);
        for (int i = 0; i < 3; i++) {
            liberarImagen(enemigos[i]);
            liberarImagen(armas[i]);
        }
        for (int i = 0; i < 4; i++) liberarImagen(recogibles[i]);
        for (int i = 0; i < 5; i++) liberarImagen(interfaz[i]);
        for (int i = 0; i < Datos::Mapa::CantidadProps; i++) liberarImagen(props[i]);
        sonidos.liberar();
    }
    void actualizarCamara() {
        float mitadX = ANCHO / (2 * camara.zoom);
        float mitadY = VISTA_ALTO / (2 * camara.zoom);
        camara.target = {limitar(jugador.getCentro().x, mitadX, Datos::Mapa::ancho - mitadX),
                         limitar(jugador.getCentro().y, mitadY, Datos::Mapa::alto - mitadY)};
    }
    Rectangle vistaMundo() const {
        float ancho = ANCHO / camara.zoom;
        float alto = VISTA_ALTO / camara.zoom;
        return {camara.target.x - ancho / 2, camara.target.y - alto / 2, ancho, alto};
    }
    bool clic(Rectangle zona) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || !CheckCollisionPointRec(GetMousePosition(), zona)) return false;
        sonidos.emitir(Audio::UI);
        return true;
    }
    void nuevaOleada(int numero) {
        oleada.iniciar(numero, imposible);
        caja.aparecer(jugador.getCentro());
        sonidos.emitir(Audio::Oleada);
    }
    void nuevaPartida(bool modo) {
        sonidos.detener();
        imposible = modo;
        jugador = Jugador();
        zombies.clear();
        objetos.clear();
        for (int i = 0; i < Datos::maxBalas; i++) proyectiles[i].detener();
        caja.reiniciar();
        eliminados = 0;
        tiempoSkip = 0;
        tiempoTajo = 0;
        bloquearAtaque = true;
        pantalla = Pantalla::Combate;
        actualizarCamara();
        nuevaOleada(1);
    }
    void aparecerZombie(int tipo) {
        Vector2 sitio = {32, 32};
        for (int intento = 0; intento < 40; intento++) {
            int lado = GetRandomValue(0, 3);
            sitio = {float(GetRandomValue(32, Datos::Mapa::ancho - 64)), 30};
            if (lado == 1) sitio.y = Datos::Mapa::alto - 64;
            if (lado >= 2) {
                sitio.y = float(GetRandomValue(32, Datos::Mapa::alto - 64));
                sitio.x = 30;
                if (lado == 3) sitio.x = Datos::Mapa::ancho - 64;
            }
            if (!chocaObstaculo(colisionPersonaje(sitio))) break;
        }
        zombies.push_back(Zombie(sitio, tipo, oleada.getNumero()));
    }
    void golpearZombie(int indice, int dano) {
        if (!zombies[indice].getSalud().estaVivo()) return;
        zombies[indice].getSalud().recibirDano(dano);
        if (!zombies[indice].getSalud().estaVivo()) {
            eliminados++;
            sonidos.zombie(true, zombies[indice].getCentro(), jugador.getCentro());
        }
    }
    void golpearCaja(int dano) {
        if (!caja.golpear(dano)) return;
        std::vector<TipoObjeto> opciones = {TipoObjeto::Botiquin, TipoObjeto::Municion};
        if (!jugador.getArma().tiene(1)) opciones.push_back(TipoObjeto::Pistola);
        if (!jugador.getArma().tiene(2)) opciones.push_back(TipoObjeto::AK);
        Vector2 pos = caja.getPosicion();
        objetos.push_back({opciones[GetRandomValue(0, int(opciones.size()) - 1)], {pos.x + 10, pos.y + 10}});
    }
    void recoger(float dt) {
        for (int i = int(objetos.size()) - 1; i >= 0; i--) {
            objetos[i].tiempo -= dt;
            objetos[i].fase += dt * 2;
            bool recogido = false;
            if (distancia(jugador.getCentro(), objetos[i].posicion) <= 26) {
                TipoObjeto tipo = objetos[i].tipo;
                if (tipo == TipoObjeto::Botiquin && jugador.getSalud().getActual() < jugador.getSalud().getMaxima()) {
                    jugador.getSalud().curar(Datos::curacion);
                    recogido = true;
                }
                if (tipo == TipoObjeto::Municion) recogido = jugador.getArma().recargar();
                if (tipo == TipoObjeto::Pistola) { jugador.getArma().equipar(1); recogido = true; }
                if (tipo == TipoObjeto::AK) { jugador.getArma().equipar(2); recogido = true; }
            }
            if (recogido) sonidos.emitir(Audio::UI);
            if (recogido || objetos[i].tiempo <= 0) objetos.erase(objetos.begin() + i);
        }
    }
    void atacar(bool automatico) {
        int arma = jugador.getArma().getSeleccion();
        int libre = -1;
        for (int i = 0; i < Datos::maxBalas; i++)
            if (!proyectiles[i].activo()) { libre = i; break; }
        if (arma > 0 && libre == -1) return;
        if (!jugador.getArma().atacar()) return;
        Vector2 origen = jugador.getCentro();
        Vector2 objetivo = GetScreenToWorld2D(GetMousePosition(), camara);
        if (automatico) {
            float menor = 100000;
            for (int i = 0; i < int(zombies.size()); i++) {
                float d = distancia(origen, zombies[i].getCentro());
                if (zombies[i].getSalud().estaVivo() && d < menor) {
                    menor = d;
                    objetivo = zombies[i].getCentro();
                }
            }
        }
        direccionAtaque = direccionUnitaria(origen, objetivo);
        if (direccionAtaque.x == 0 && direccionAtaque.y == 0) direccionAtaque = {1, 0};
        if (arma > 0) {
            objetivo = {origen.x + direccionAtaque.x * Datos::armas[arma].alcance,
                        origen.y + direccionAtaque.y * Datos::armas[arma].alcance};
            proyectiles[libre].lanzar(origen, objetivo, Datos::armas[arma].dano, Datos::armas[arma].alcance);
            if (arma == 1) sonidos.emitir(Audio::Pistola);
            else sonidos.emitir(Audio::AK);
            return;
        }
        sonidos.emitir(Audio::Espada);
        tiempoTajo = 0.18f;
        for (int i = 0; i < int(zombies.size()); i++) {
            Vector2 hacia = direccionUnitaria(origen, zombies[i].getCentro());
            float frente = hacia.x * direccionAtaque.x + hacia.y * direccionAtaque.y;
            if (distancia(origen, zombies[i].getCentro()) <= Datos::armas[0].alcance && frente >= 0.28f)
                golpearZombie(i, Datos::armas[0].dano);
        }
        Vector2 centroCaja = {caja.getPosicion().x + 10, caja.getPosicion().y + 10};
        Vector2 hacia = direccionUnitaria(origen, centroCaja);
        if (distancia(origen, centroCaja) <= Datos::armas[0].alcance &&
            hacia.x * direccionAtaque.x + hacia.y * direccionAtaque.y >= 0.30f) golpearCaja(Datos::armas[0].dano);
    }
    void actualizarProyectiles(float dt) {
        for (int i = 0; i < Datos::maxBalas; i++) {
            float viaje = Datos::velocidadBala * dt;
            while (proyectiles[i].activo() && viaje > 0) {
                float paso = std::min(4.0f, viaje);
                proyectiles[i].avanzar(paso);
                viaje -= paso;
                Vector2 pos = proyectiles[i].getPosicion();
                Rectangle zona = proyectiles[i].getCuerpo();
                if (pos.x < 24 || pos.y < 24 || pos.x > Datos::Mapa::ancho - 24 || pos.y > Datos::Mapa::alto - 24 || chocaObstaculo(zona)) {
                    proyectiles[i].detener();
                    break;
                }
                if (caja.activa() && CheckCollisionRecs(zona, caja.getCuerpo())) {
                    golpearCaja(proyectiles[i].getDano());
                    proyectiles[i].detener();
                    break;
                }
                for (int j = 0; j < int(zombies.size()); j++) {
                    if (zombies[j].getSalud().estaVivo() && CheckCollisionRecs(zona, colisionPersonaje(zombies[j].getPosicion()))) {
                        golpearZombie(j, proyectiles[i].getDano());
                        proyectiles[i].detener();
                        break;
                    }
                }
            }
        }
    }
    void combate(float dt) {
        tiempoSkip = std::max(0.0f, tiempoSkip - dt);
        tiempoTajo = std::max(0.0f, tiempoTajo - dt);
        actualizarPersonaje(jugador, dt, {0, 0});
        actualizarCamara();
        for (int i = 0; i < 3; i++) {
            bool pulsado = IsKeyPressed(KEY_ONE + i);
            if (clic({24.0f + i * 44, 644, 36, 36})) pulsado = true;
            if (pulsado && jugador.getArma().seleccionar(i)) sonidos.emitir(Audio::UI);
        }
        for (int i = 0; i < int(zombies.size()); i++) {
            actualizarPersonaje(zombies[i], dt, jugador.getCentro());
            if (zombies[i].puedeGolpear(jugador.getCentro())) {
                jugador.getSalud().recibirDano(Datos::zombies[zombies[i].getTipo()].dano);
                sonidos.emitir(Audio::Dano);
            }
        }
        actualizarProyectiles(dt);
        if (!jugador.getSalud().estaVivo()) {
            pantalla = Pantalla::Derrota;
            sonidos.detener();
            sonidos.emitir(Audio::Derrota);
            return;
        }
        bool raton = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool espacio = IsKeyDown(KEY_SPACE);
        bool enMapa = CheckCollisionPointRec(GetMousePosition(), {0, HUD_ALTO, ANCHO, VISTA_ALTO});
        if (!raton && !espacio) bloquearAtaque = false;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !enMapa) bloquearAtaque = true;
        if (!bloquearAtaque && ((raton && enMapa) || espacio)) atacar(espacio);
        recoger(dt);
        sonidos.actualizar(jugador.getCentro());
        if (sonidos.tocaGemido()) {
            int cercanos = 0;
            Vector2 origen = {};
            for (int i = 0; i < int(zombies.size()); i++) {
                if (!zombies[i].getSalud().estaVivo() || distancia(zombies[i].getCentro(), jugador.getCentro()) > Datos::radioAudio) continue;
                cercanos++;
                if (GetRandomValue(1, cercanos) == 1) origen = zombies[i].getCentro();
            }
            if (cercanos > 0) sonidos.zombie(false, origen, jugador.getCentro());
        }
        int vivos = 0;
        for (int i = 0; i < int(zombies.size()); i++) {
            if (!zombies[i].getSalud().estaVivo()) continue;
            if (vivos != i) zombies[vivos] = zombies[i];
            vivos++;
        }
        zombies.erase(zombies.begin() + vivos, zombies.end());
        int tipo = oleada.siguiente(dt);
        if (tipo != -1) aparecerZombie(tipo);
        if (oleada.terminada()) nuevaOleada(oleada.getNumero() + 1);
        if (clic({535, 646, 160, 48}) && tiempoSkip == 0) {
            nuevaOleada(oleada.getNumero() + 1);
            tiempoSkip = 3;
        }
        if (clic({713, 646, 160, 48}) || IsKeyPressed(KEY_ESCAPE)) {
            pantalla = Pantalla::Menu;
            sonidos.detener();
        }
    }
    void actualizar(float dt) {
        if (pantalla == Pantalla::Menu) {
            if (clic(botonesMenu[0]) || IsKeyPressed(KEY_ENTER)) nuevaPartida(false);
            else if (clic(botonesMenu[1])) nuevaPartida(true);
            else if (clic(botonesMenu[2])) salir = true;
        }
        else if (pantalla == Pantalla::Combate) combate(dt);
        else {
            if (clic({306, 465, 160, 48}) || IsKeyPressed(KEY_ENTER)) nuevaPartida(imposible);
            else if (clic({494, 465, 160, 48}) || IsKeyPressed(KEY_ESCAPE)) {
                pantalla = Pantalla::Menu;
                sonidos.detener();
            }
        }
    }
    void mostrarBoton(Rectangle zona, const char* nombre) {
        if (interfaz[0].id == 0) return;
        Color color = {54, 79, 64, 255};
        if (CheckCollisionPointRec(GetMousePosition(), zona)) color = {98, 129, 109, 255};
        mostrarImagen(interfaz[0], zona, color);
        DrawText(nombre, int(zona.x + (zona.width - MeasureText(nombre, 20)) / 2), int(zona.y + 14), 20, COLOR_TEXTO);
    }
    void mostrarBarraVida(float x, float y, float ancho, int actual, int maximo) {
        DrawRectangle(int(x), int(y), int(ancho), 7, {9, 17, 18, 255});
        DrawRectangle(int(x + 1), int(y + 1), int((ancho - 2) * actual / maximo), 5, RED);
    }
    void dibujarPersonaje(Texture2D textura, const Personaje &actor, int cuadros, float tamano) {
        if (textura.id == 0) return;
        Vector2 pos = actor.getPosicion();
        DrawEllipse(int(pos.x + 16), int(pos.y + 31), tamano / 2, 5, {11, 20, 19, 175});
        DrawTexturePro(textura, {float(actor.getFrame() * cuadros), float(actor.getOrientacion() * cuadros), float(cuadros), float(cuadros)},
                       {pos.x + (32 - tamano) / 2, pos.y + 32 - tamano, tamano, tamano}, {0, 0}, 0, WHITE);
    }
    void dibujarObjeto(int indice) {
        Objeto objeto = objetos[indice];
        Texture2D textura = recogibles[int(objeto.tipo)];
        if (textura.id == 0) return;
        DrawEllipse(int(objeto.posicion.x), int(objeto.posicion.y + 7), 14, 4, {11, 20, 19, 165});
        float escala = std::min(32.0f / textura.width, 32.0f / textura.height);
        float ancho = textura.width * escala;
        float alto = textura.height * escala;
        mostrarImagen(textura, {objeto.posicion.x - ancho / 2, objeto.posicion.y - 22 - std::sin(objeto.fase) * 2 + (32 - alto) / 2, ancho, alto});
    }
    static bool comparar(Elemento a, Elemento b) {
        if (a.profundidad != b.profundidad) return a.profundidad < b.profundidad;
        if (a.tipo != b.tipo) return int(a.tipo) < int(b.tipo);
        return a.indice < b.indice;
    }
    void dibujarEscena() {
        orden.clear();
        for (int i = 0; i < int(std::size(Datos::Mapa::obstaculos)); i++) {
            Rectangle visual = rectanguloVisual(Datos::Mapa::obstaculos[i]);
            Rectangle limite = colisionObstaculo(Datos::Mapa::obstaculos[i]);
            float profundidad = visual.y + visual.height;
            if (limite.height > 0) profundidad = limite.y + limite.height;
            orden.push_back({profundidad, TipoDibujo::Prop, i});
        }
        if (caja.activa()) orden.push_back({caja.getPosicion().y + 20, TipoDibujo::Caja, 0});
        for (int i = 0; i < int(objetos.size()); i++) orden.push_back({objetos[i].posicion.y + 7, TipoDibujo::Objeto, i});
        for (int i = 0; i < int(zombies.size()); i++)
            if (zombies[i].getSalud().estaVivo()) orden.push_back({zombies[i].getPosicion().y + 32, TipoDibujo::Zombie, i});
        orden.push_back({jugador.getPosicion().y + 32, TipoDibujo::Jugador, 0});
        for (int i = 0; i < Datos::maxBalas; i++)
            if (proyectiles[i].activo()) orden.push_back({proyectiles[i].getPosicion().y + 3, TipoDibujo::Bala, i});
        std::sort(orden.begin(), orden.end(), comparar);
        for (int i = 0; i < int(orden.size()); i++) {
            int indice = orden[i].indice;
            switch (orden[i].tipo) {
            case TipoDibujo::Prop: {
                Datos::Mapa::Obstaculo objeto = Datos::Mapa::obstaculos[indice];
                if (props[objeto.tipo].id == 0) break;
                Rectangle zona = rectanguloVisual(objeto);
                DrawEllipse(int(zona.x + zona.width / 2), int(zona.y + zona.height - 3), zona.width / 2, 5, {20, 24, 19, 95});
                float angulo = 0;
                if (objeto.vertical) {
                    Datos::Mapa::Prop datos = Datos::Mapa::props[objeto.tipo];
                    zona = {zona.x + datos.alto, zona.y, float(datos.ancho), float(datos.alto)};
                    angulo = 90;
                }
                mostrarImagen(props[objeto.tipo], zona, WHITE, angulo);
                break;
            }
            case TipoDibujo::Caja:
                if (interfaz[4].id != 0) {
                    DrawEllipse(int(caja.getPosicion().x + 10), int(caja.getPosicion().y + 22), 12, 4, {11, 20, 19, 175});
                    mostrarImagen(interfaz[4], caja.getCuerpo());
                }
                break;
            case TipoDibujo::Objeto: dibujarObjeto(indice); break;
            case TipoDibujo::Zombie:
                dibujarPersonaje(enemigos[zombies[indice].getTipo()], zombies[indice], Datos::zombies[zombies[indice].getTipo()].frames, Datos::zombies[zombies[indice].getTipo()].tamano);
                break;
            case TipoDibujo::Jugador: dibujarPersonaje(personaje, jugador, 16, 32); break;
            case TipoDibujo::Bala: {
                Vector2 pos = proyectiles[indice].getPosicion();
                Vector2 rumbo = proyectiles[indice].getDireccion();
                mostrarImagen(bala, {pos.x, pos.y, 10, 3}, WHITE, std::atan2(rumbo.y, rumbo.x) * RAD2DEG, {5, 1.5f});
                break;
            }
            }
        }
        for (int i = 0; i < int(zombies.size()); i++)
            if (enemigos[zombies[i].getTipo()].id != 0)
                mostrarBarraVida(zombies[i].getPosicion().x, zombies[i].getPosicion().y - 8, 32, zombies[i].getSalud().getActual(), zombies[i].getSalud().getMaxima());
        if (caja.activa() && interfaz[4].id != 0) mostrarBarraVida(caja.getPosicion().x - 2, caja.getPosicion().y - 8, 24, caja.getVida(), Datos::vidaCaja);
    }
    void dibujarArena() const
    {
        mostrarImagen(mapa, {0, 0, float(Datos::Mapa::ancho), float(Datos::Mapa::alto)});
    }

    void dibujarAtaque() {
        if (personaje.id == 0) return;
        Vector2 pos = jugador.getPosicion();
        mostrarImagen(interfaz[1], {pos.x + 4, pos.y - 36, 24, 24});
        mostrarImagen(armas[jugador.getArma().getSeleccion()], {pos.x + 8, pos.y - 32, 16, 16});
        if (tiempoTajo <= 0 || tajo.id == 0 || jugador.getArma().getSeleccion() != 0) return;
        float progreso = 1 - tiempoTajo / 0.18f;
        float angulo = std::atan2(direccionAtaque.y, direccionAtaque.x) - 1.28f + progreso * 2.56f;
        Vector2 centro = jugador.getCentro();
        int cuadro = std::min(3, int(progreso * 4));
        DrawTexturePro(tajo, {float(cuadro * 32), 0, 32, 32},
                       {centro.x + std::cos(angulo) * 73, centro.y + std::sin(angulo) * 73, 42, 22},
                       {21, 11}, angulo * RAD2DEG + 90, WHITE);
    }
    void dibujarInterfaz() {
        DrawRectangle(0, 0, ANCHO, HUD_ALTO, {17, 24, 23, 255});
        DrawRectangle(0, CONTROLES_Y, ANCHO, ALTO - CONTROLES_Y, {17, 24, 23, 255});
        mostrarImagen(interfaz[2], {18, 16, 32, 32});
        mostrarImagen(interfaz[3], {158, 16, 32, 32});
        DrawText(TextFormat("%i / %i", jugador.getSalud().getActual(), jugador.getSalud().getMaxima()), 58, 20, 24, COLOR_TEXTO);
        DrawText(TextFormat("%i", eliminados), 196, 20, 24, COLOR_TEXTO);
        DrawText(TextFormat("OLEADA %i", oleada.getNumero()), 345, 18, 26, COLOR_DORADO);
        if (imposible) DrawText("IMPOSIBLE", 345, 48, 14, ORANGE);
        else DrawText("NORMAL", 345, 48, 14, COLOR_APAGADO);
        DrawText(TextFormat("ZOMBIES %i + %i", oleada.getActivos(), oleada.getPendientes()), 720, 24, 18, COLOR_TEXTO);
        for (int i = 0; i < 3; i++) {
            Rectangle zona = {24.0f + i * 44, 644, 36, 36};
            if (interfaz[1].id == 0) continue;
            mostrarImagen(interfaz[1], zona);
            if (jugador.getArma().tiene(i)) mostrarImagen(armas[i], {zona.x + 5, zona.y + 5, 26, 26});
            if (jugador.getArma().getSeleccion() == i) DrawRectangleLinesEx(zona, 2, COLOR_DORADO);
            DrawText(TextFormat("%i", i + 1), int(zona.x + 2), int(zona.y + 2), 10, COLOR_TEXTO);
        }
        int arma = jugador.getArma().getSeleccion();
        DrawText(Datos::armas[arma].nombre, 340, 640, 20, COLOR_TEXTO);
        if (arma == 0) DrawText("inf / inf", 340, 666, 18, COLOR_TEXTO);
        else DrawText(TextFormat("%i / %i", jugador.getArma().getBalas(), Datos::armas[arma].municion), 340, 666, 18, COLOR_TEXTO);
        const char* skip = "SKIP";
        if (tiempoSkip > 0) skip = TextFormat("SKIP %.0f", std::ceil(tiempoSkip));
        mostrarBoton({535, 646, 160, 48}, skip);
        mostrarBoton({713, 646, 160, 48}, "MENU");
        DrawText("WASD: MOVER   CLICK / ESPACIO: ATACAR   1-3: ARMAS", 24, 697, 12, COLOR_APAGADO);
    }
    void dibujar() {
        BeginDrawing();
        ClearBackground(COLOR_FONDO);
        if (pantalla == Pantalla::Menu) {
            DrawRectangleLines(64, 52, 832, 617, {70, 91, 78, 255});
            mostrarTextoCentrado("ZZZombies", 137, 47, COLOR_TEXTO);
            mostrarTextoCentrado("Sobrevive a las oleadas infinitas", 204, 24, COLOR_DORADO);
            mostrarTextoCentrado("Las cajas dropean items.", 262, 18, COLOR_APAGADO);
            mostrarBoton(botonesMenu[0], "JUGAR");
            mostrarBoton(botonesMenu[1], "MODO IMPOSIBLE");
            mostrarBoton(botonesMenu[2], "SALIR");
            mostrarTextoCentrado("WASD - Mover  |  Mouse o espacio - Atacar", 614, 16, COLOR_APAGADO);
        }
        else {
            BeginScissorMode(0, HUD_ALTO, ANCHO, VISTA_ALTO);
            BeginMode2D(camara);
            dibujarArena();
            dibujarEscena();
            dibujarAtaque();
            EndMode2D();
            EndScissorMode();
            dibujarInterfaz();
            if (pantalla == Pantalla::Derrota) {
                mostrarTextoCentrado("Has muerto", 161, 52, {30, 4, 4, 255});
                mostrarTextoCentrado("Has muerto", 158, 52, {240, 35, 45, 255});
                mostrarBoton({306, 465, 160, 48}, "REINICIAR");
                mostrarBoton({494, 465, 160, 48}, "MENU");
            }
        }
        EndDrawing();
    }
public:
    Juego() : oleada(zombies), caja(sonidos) {}
    void ejecutar() {
        ChangeDirectory(GetApplicationDirectory());
        InitWindow(ANCHO, ALTO, "ZZZombies");
        SetExitKey(KEY_NULL);
        SetTargetFPS(60);
        cargar();
        while (!WindowShouldClose() && !salir) {
            actualizar(std::min(GetFrameTime(), 0.05f));
            if (!salir) dibujar();
        }
        liberar();
        CloseWindow();
    }
};

int main()
{
    Juego juego;
    juego.ejecutar();
    return 0;
}
