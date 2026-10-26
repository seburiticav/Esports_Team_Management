# Esports Manager

Aplicación de escritorio desarrollada en C++ y Qt para administrar jugadores
de esports y sus cuentas de League of Legends. Los datos se guardan localmente
en archivos CSV; la integración con Riot permite actualizar cuentas, rangos,
maestrías y estadísticas de partidas.

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

1. **Arreglo dinámico (`DynamicArray`)**: almacena elementos y sirve como base
   de la pila y del heap de la cola de prioridad.
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

Las estructuras procesan y presentan datos; los CSV siguen siendo el medio de
persistencia. Sus pruebas están en `tests/data_structures_tests.cpp`.

## Complejidad algorítmica

Sea `n` la cantidad de elementos almacenados y `k` la capacidad de la cola
circular. Se considera que copiar un elemento y comparar prioridades cuestan
`O(1)`.

| Estructura | Operación | Tiempo | Espacio |
|---|---|---:|---:|
| `DynamicArray` | Acceso por índice | `O(1)` | — |
| `DynamicArray` | `pushBack` | `O(1)` amortizado; `O(n)` cuando crece | `O(n)` |
| `DynamicArray` | `removeAt` | `O(n)`; quitar el último cuesta `O(1)` | — |
| `Stack` | `top`, `push` y `pop` | `O(1)` amortizado para `push`; `O(1)` para `top` y `pop` | `O(n)` |
| `Queue` enlazada | `front`, `enqueue` y `dequeue` | `O(1)` | `O(n)` |
| `Queue` enlazada | `clear` | `O(n)` | — |
| `CircularQueue` | `enqueue`, `at` y consultas de tamaño | `O(1)` | `O(k)` |
| `PriorityQueue` (heap binario) | `push` | `O(log n)` amortizado; `O(n)` si crece el arreglo | `O(n)` |
| `PriorityQueue` (heap binario) | `pop` | `O(log n)` | — |

En los informes, insertar los `n` registros en una cola de prioridad y
extraerlos para mostrarlos cuesta `O(n log n)` en total. La cola circular
mantiene un máximo de `k = 5` partidas y agregar cada una cuesta `O(1)`.
Estas cifras describen las operaciones de las estructuras, no la lectura de
CSV ni las solicitudes de red a Riot.

## Tecnologías y requisitos

- C++17.
- Qt 6 con los módulos Widgets y Network.
- CMake 3.16 o posterior.
- Un compilador C++ compatible, por ejemplo MinGW o MSVC.
- Una clave de Riot válida para las funciones que consultan la API.
- Conexión a Internet para consultar Riot y descargar recursos de Data Dragon.

## Instalación y ejecución

### Ejecutable portable de Windows

Extrae el paquete ZIP completo en una carpeta y ejecuta `Proyecto_EDD.exe`.
No muevas el ejecutable fuera de esa carpeta: necesita las bibliotecas de Qt,
los CSV y los recursos incluidos.

### Compilar desde el repositorio

Abre el `CMakeLists.txt` de la raíz del repositorio en Qt Creator, selecciona un
kit de Qt 6 para escritorio y configura el proyecto.

También puedes compilar desde una terminal. Configura `CMAKE_PREFIX_PATH` con la
carpeta de instalación de Qt:

```powershell
git clone --branch Sebastian-Buritica https://github.com/seburiticav/Esports_Team_Management.git
cd Esports_Team_Management
cmake -S . -B build -DCMAKE_PREFIX_PATH="<ruta-a-Qt>"
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Archivos de datos

Los CSV de `CSVs/` contienen jugadores, cuentas, equipos, contratos, maestrías,
partidas, estadísticas y auditoría de roles. La aplicación los lee y actualiza
durante el uso. Haz una copia de seguridad antes de editarlos manualmente y no
publiques datos personales o de cuentas sin autorización.

## Uso de inteligencia artificial

El equipo debe declarar las herramientas utilizadas y estimar el porcentaje de
código generado o modificado con IA de acuerdo con el historial real de trabajo.
