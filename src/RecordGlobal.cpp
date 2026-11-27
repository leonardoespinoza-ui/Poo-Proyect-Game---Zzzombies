#include "RecordGlobal.h"
#include <windows.h>
#include <winhttp.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <utility>

namespace {
struct Http {
    HINTERNET valor;
    ~Http() { if (valor) WinHttpCloseHandle(valor); }
};
}

RecordGlobal::RecordGlobal(std::string id) : instalacion(std::move(id)) {
    const wchar_t* url = _wgetenv(L"ZZZOMBIES_SUPABASE_URL");
    const wchar_t* clave = _wgetenv(L"ZZZOMBIES_SUPABASE_KEY");
    if (instalacion.empty()) return;
    std::wstring proyecto;
    if (url && clave) {
        proyecto = url;
        apiKey = clave;
    } else {
        std::wifstream config("supabase/config.txt");
        if (!std::getline(config, proyecto) || !std::getline(config, apiKey)) return;
    }
    if (!proyecto.empty() && proyecto.back() == L'/') proyecto.pop_back();
    constexpr wchar_t prefijo[] = L"https://";
    if (proyecto.rfind(prefijo, 0) != 0) return;
    proyecto.erase(0, 8);
    if (proyecto.empty() || proyecto.find_first_not_of(L"abcdefghijklmnopqrstuvwxyz0123456789-.") != std::wstring::npos)
        return;
    if (apiKey.rfind(L"sb_publishable_", 0) != 0 ||
        apiKey.find_first_not_of(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-") != std::wstring::npos) return;
    host = std::move(proyecto);
}

RecordGlobal::Resultado RecordGlobal::sincronizar(std::wstring host, std::wstring apiKey, std::string cuerpo) {
    Http sesion{WinHttpOpen(L"ZZZombies/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0)};
    if (!sesion.valor || !WinHttpSetTimeouts(sesion.valor, 2000, 2000, 2500, 2500)) return {};
    Http conexion{WinHttpConnect(sesion.valor, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0)};
    if (!conexion.valor) return {};
    Http peticion{WinHttpOpenRequest(conexion.valor, L"POST", L"/rest/v1/rpc/zzzombies_record",
                                   nullptr, nullptr, nullptr, WINHTTP_FLAG_SECURE)};
    if (!peticion.valor) return {};
    DWORD redireccion = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    if (!WinHttpSetOption(peticion.valor, WINHTTP_OPTION_REDIRECT_POLICY, &redireccion, sizeof(redireccion))) return {};
    std::wstring encabezados = L"Content-Type: application/json\r\napikey: " + apiKey + L"\r\n";
    DWORD tam = static_cast<DWORD>(cuerpo.size());
    if (!WinHttpSendRequest(peticion.valor, encabezados.c_str(), DWORD(-1), cuerpo.data(), tam, tam, 0) ||
        !WinHttpReceiveResponse(peticion.valor, nullptr)) return {};
    DWORD status = 0, bytes = sizeof(status);
    if (!WinHttpQueryHeaders(peticion.valor, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &bytes, WINHTTP_NO_HEADER_INDEX) || status != 200)
        return {};
    std::string respuesta;
    char bloque[128];
    do {
        if (!WinHttpReadData(peticion.valor, bloque, sizeof(bloque), &bytes)) return {};
        respuesta.append(bloque, bytes);
        if (respuesta.size() > sizeof(bloque)) return {};
    } while (bytes);
    Resultado dato;
    char cierre = 0;
    if (std::sscanf(respuesta.c_str(), "\"%d:%d%c", &dato.lider, &dato.puesto, &cierre) != 3 ||
        cierre != '"' || dato.lider < 0 || dato.puesto < 0) return {};
    return dato;
}

void RecordGlobal::actualizar(float dt, bool menu, int puntos, int oleada, int modo) {
    if (host.empty()) return;
    if (consulta.valid() && consulta.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        resultado = consulta.get();
    espera -= dt;
    if (consulta.valid() || espera > 0 || !menu) return;
    espera = 30;
    std::string cuerpo = "{\"p_clave\":\"" + instalacion + "\",\"p_puntos\":" + std::to_string(puntos) +
                         ",\"p_oleada\":" + std::to_string(oleada) + ",\"p_modo\":" + std::to_string(modo) + "}";
    consulta = std::async(std::launch::async, sincronizar, host, apiKey, std::move(cuerpo));
}

std::string RecordGlobal::texto() const {
    if (host.empty()) return {};
    if (resultado.lider < 0) return consulta.valid() ? "CONSULTANDO RÉCORD GLOBAL..." : "RÉCORD GLOBAL NO DISPONIBLE";
    if (resultado.lider == 0) return "RÉCORD GLOBAL SIN PUNTAJES";
    std::string linea = "RÉCORD GLOBAL  " + std::to_string(resultado.lider);
    if (resultado.puesto > 0) linea += "   /   TU PUESTO  " + std::to_string(resultado.puesto);
    return linea;
}
