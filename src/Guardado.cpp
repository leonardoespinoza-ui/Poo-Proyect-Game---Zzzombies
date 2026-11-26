#include "Guardado.h"
#include <sqlite3.h>
#include <algorithm>
#include <cctype>

namespace {
struct Consulta {
    sqlite3_stmt* stmt = nullptr;
    Consulta(sqlite3* db, const char* sql) { if (db) sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr); }
    ~Consulta() { sqlite3_finalize(stmt); }
};
}
Guardado::Guardado(const char* ruta) {
    if (sqlite3_open(ruta, &db) == SQLITE_OK) {
        sqlite3_busy_timeout(db, 1000);
        if (ejecutar(
            "CREATE TABLE IF NOT EXISTS perfil(id INTEGER PRIMARY KEY CHECK(id=1), saldo INTEGER NOT NULL CHECK(saldo>=0), puntos INTEGER NOT NULL, titular TEXT NOT NULL);"
            "INSERT OR IGNORE INTO perfil(id,saldo,puntos,titular) VALUES(1,0,0,'');"
            "CREATE TABLE IF NOT EXISTS armas(id INTEGER PRIMARY KEY);"
            "CREATE TABLE IF NOT EXISTS partidas(id INTEGER PRIMARY KEY, puntos INTEGER, oleadas INTEGER, arma INTEGER, dificil INTEGER, fecha TEXT DEFAULT CURRENT_TIMESTAMP);")
            && (entero("SELECT COUNT(*) FROM pragma_table_info('perfil') WHERE name='oleadas'")
                || ejecutar("ALTER TABLE perfil ADD COLUMN oleadas INTEGER NOT NULL DEFAULT 0"))
            && (entero("SELECT COUNT(*) FROM pragma_table_info('perfil') WHERE name='modo'")
                || ejecutar("ALTER TABLE perfil ADD COLUMN modo INTEGER NOT NULL DEFAULT -1"))) return;
    }
    sqlite3_close(db);
    db = nullptr;
}
Guardado::~Guardado() { sqlite3_close(db); }
bool Guardado::ejecutar(const char* sql) const { return db && sqlite3_exec(db, sql, nullptr, nullptr, nullptr) == SQLITE_OK; }
int Guardado::entero(const char* sql, int dato) const {
    Consulta q(db, sql);
    if (!q.stmt) return 0;
    if (sqlite3_bind_parameter_count(q.stmt)) sqlite3_bind_int(q.stmt, 1, dato);
    return sqlite3_step(q.stmt) == SQLITE_ROW ? sqlite3_column_int(q.stmt, 0) : 0;
}
int Guardado::monedas() const { return entero("SELECT saldo FROM perfil WHERE id=1"); }
int Guardado::record() const { return entero("SELECT puntos FROM perfil WHERE id=1"); }
int Guardado::oleadaRecord() const { return entero("SELECT oleadas FROM perfil WHERE id=1"); }
int Guardado::modoRecord() const { return entero("SELECT modo FROM perfil WHERE id=1"); }
std::string Guardado::titular() const {
    Consulta q(db, "SELECT titular FROM perfil WHERE id=1");
    if (!q.stmt || sqlite3_step(q.stmt) != SQLITE_ROW) return {};
    return reinterpret_cast<const char*>(sqlite3_column_text(q.stmt, 0));
}
bool Guardado::disponible(int arma) const { return entero("SELECT COUNT(*) FROM armas WHERE id=?", arma) != 0; }
bool Guardado::comprar(int arma, int precio) {
    if (precio <= 0 || !ejecutar("BEGIN IMMEDIATE")) return false;
    Consulta pago(db, "UPDATE perfil SET saldo=saldo-?1 WHERE id=1 AND saldo>=?1");
    Consulta alta(db, "INSERT INTO armas VALUES(?)");
    if (pago.stmt && alta.stmt) {
        sqlite3_bind_int(pago.stmt, 1, precio);
        sqlite3_bind_int(alta.stmt, 1, arma);
        if (sqlite3_step(alta.stmt) == SQLITE_DONE && sqlite3_step(pago.stmt) == SQLITE_DONE &&
            sqlite3_changes(db) == 1 && ejecutar("COMMIT")) return true;
    }
    ejecutar("ROLLBACK");
    return false;
}
bool Guardado::premiar(int cantidad) {
    Consulta q(db, "UPDATE perfil SET saldo=saldo+? WHERE id=1");
    if (!q.stmt || cantidad < 0) return false;
    sqlite3_bind_int(q.stmt, 1, cantidad);
    return sqlite3_step(q.stmt) == SQLITE_DONE;
}
bool Guardado::terminar(int puntos, int oleadas, int arma, int modo) {
    Consulta q(db, "INSERT INTO partidas(puntos,oleadas,arma,dificil) VALUES(?,?,?,?)");
    if (!q.stmt) return false;
    sqlite3_bind_int(q.stmt, 1, puntos);
    sqlite3_bind_int(q.stmt, 2, oleadas);
    sqlite3_bind_int(q.stmt, 3, arma);
    sqlite3_bind_int(q.stmt, 4, modo);
    return sqlite3_step(q.stmt) == SQLITE_DONE;
}
bool Guardado::guardarRecord(int puntos, const std::string& nombre, int oleadas, int modo) {
    if (nombre.size() > 18 || !std::any_of(nombre.begin(), nombre.end(), [](unsigned char c) { return std::isalnum(c) != 0; }))
        return false;
    Consulta q(db, "UPDATE perfil SET puntos=?, titular=?, oleadas=?, modo=? WHERE id=1 AND puntos<?");
    if (!q.stmt) return false;
    sqlite3_bind_int(q.stmt, 1, puntos);
    sqlite3_bind_text(q.stmt, 2, nombre.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(q.stmt, 3, oleadas);
    sqlite3_bind_int(q.stmt, 4, modo);
    sqlite3_bind_int(q.stmt, 5, puntos);
    return sqlite3_step(q.stmt) == SQLITE_DONE && sqlite3_changes(db) == 1;
}
