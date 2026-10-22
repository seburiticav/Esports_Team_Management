# Esports Manager
<img width="520" height="520" alt="app_icon" src="https://github.com/user-attachments/assets/4f972547-0607-40f1-b0c8-c27b0fa96059" />

Aplicación de escritorio desarrollada en C++ y Qt para administrar jugadores
de esports y sus cuentas de League of Legends. Los datos de la aplicación se
guardan localmente en archivos CSV; la integración con Riot Games permite
actualizar cuentas, rangos, maestrías y estadísticas de partidas.

## Integrantes

- Sebastian Buritica Velasco
- Andres Felipe Otero Barrera

## Funcionalidades

- Consultar, agregar, editar y eliminar jugadores.
- Registrar cuentas de Riot y asociarlas a jugadores.
- Actualizar una o todas las cuentas desde Riot, incluyendo Riot ID, rango y
  winrate.
- Administrar equipos y contratos, y consultar jugadores registrados y agentes
  libres.
- Cambiar el rol de un jugador, actualizar el contrato activo y deshacer cambios
  usando la auditoría.
- Descargar maestrías, partidas recientes y estadísticas. La vista de maestrías
  muestra hasta las cinco maestrías con más puntos por cuenta.
- Consultar informes de rendimiento y las últimas cinco partidas.
- Descargar imágenes de campeones y verlas desde las tablas.

La clave de Riot se puede introducir desde el menú **Riot API**. Se mantiene
solo en memoria durante esa sesión; también se puede proporcionar mediante la
variable de entorno `RIOT_API_KEY`. No incluyas la clave en el código, README,
CSV ni repositorio.

## Estructuras de datos

El proyecto implementa estructuras propias en `src/data_structures.h` y las
usa en operaciones de la interfaz:

1. **Arreglo dinámico (`DynamicArray`)**: almacena elementos en memoria y sirve
   como base de la pila y del heap de la cola de prioridad.
2. **Pila (`Stack`)**: mantiene los IDs de cambios de rol pendientes de
   deshacer. Al iniciar, se reconstruye leyendo
   `CSVs/state_player_changes.csv`.
3. **Cola FIFO (`Queue`)**: procesa en orden las cuentas pendientes de
   sincronización con Riot.
4. **Cola de prioridad (`PriorityQueue`)**: ordena el informe de jugadores con
   bajo rendimiento por muertes y el de agentes libres por winrate. Conserva el
   orden de entrada cuando hay empates.
5. **Cola circular (`CircularQueue`)**: conserva las cinco partidas más
   recientes que se muestran para la cuenta seleccionada.

Las estructuras se usan para procesar y presentar datos; los archivos CSV
siguen siendo el medio de persistencia. Sus pruebas se encuentran en
`tests/data_structures_tests.cpp`.

## Tecnologías y requisitos

- C++17.
- Qt 6 con los módulos Widgets y Network.
- CMake 3.16 o posterior.
- Un compilador C++ compatible, por ejemplo MinGW o MSVC.
- Una clave de Riot válida para las funciones que consultan la API.
- Conexión a Internet para consultar Riot y descargar recursos de Data Dragon.

## Instalación y ejecución

### Usar el ejecutable portable de Windows

Si tienes el paquete portable de Windows, extráelo completo en una carpeta y
ejecuta `Proyecto_EDD.exe`. No muevas el ejecutable fuera de esa carpeta:
necesita las bibliotecas de Qt, los CSV y los recursos incluidos.

### Compilar desde el repositorio

Clona la rama del proyecto y abre `CMakeLists.txt` en la raíz del repositorio en
Qt Creator.
Selecciona un kit de Qt 6 para escritorio, configura el proyecto y ejecútalo.

También se puede compilar desde una terminal con CMake. Configura
`CMAKE_PREFIX_PATH` con la carpeta de instalación de Qt:

```powershell
git clone --branch Sebastian-Buritica https://github.com/seburiticav/Esports_Team_Management.git
cd Esports_Team_Management
cmake -S . -B build -DCMAKE_PREFIX_PATH="<ruta-a-Qt>"
cmake --build build --config Release
```

Para ejecutar las pruebas de estructuras:

```powershell
ctest --test-dir build --output-on-failure
```

## Archivos de datos

Los CSV de la carpeta `CSVs/` contienen jugadores, cuentas, equipos, contratos,
maestrías, partidas, estadísticas y auditoría de roles. La aplicación los lee y
actualiza al utilizarse. Es recomendable hacer una copia de seguridad antes de modificarlos
manualmente 

## Uso de inteligencia artificial

Uso aproximado de herramientas de IA: ~75%
