#pragma once
#include <future>
#include <string>

class RecordGlobal {
public:
    explicit RecordGlobal(std::string instalacion);
    void actualizar(float dt, bool menu, int puntos, int oleada, int modo);
    void solicitar() { espera = 0; }
    std::string texto() const;

private:
    struct Resultado { int lider = -1, puesto = 0; };
    static Resultado sincronizar(std::wstring host, std::wstring apiKey, std::string cuerpo);
    std::wstring host, apiKey;
    std::string instalacion;
    std::future<Resultado> consulta;
    Resultado resultado;
    float espera = 0;
};
