#pragma once
#include <string>
struct sqlite3;
class Guardado {
public:
    explicit Guardado(const char* ruta);
    ~Guardado();
    Guardado(const Guardado&) = delete;
    Guardado& operator=(const Guardado&) = delete;
    bool listo() const { return db != nullptr; }
    int monedas() const;
    int record() const;
    std::string titular() const;
    int oleadaRecord() const;
    int modoRecord() const;
    bool disponible(int arma) const;
    bool comprar(int arma, int precio);
    bool premiar(int cantidad);
    bool terminar(int puntos, int oleadas, int arma, int modo);
    bool guardarRecord(int puntos, const std::string& nombre, int oleadas = 0, int modo = -1);
private:
    bool ejecutar(const char* sql) const;
    int entero(const char* sql, int dato = 0) const;
    sqlite3* db = nullptr;
};
