# Esports Manager

Aplicación de escritorio en C++ y Qt para administrar jugadores, cuentas de
League of Legends, equipos, contratos, estadísticas y reportes.

## Estructuras de datos

La interfaz utiliza implementaciones propias de `src/data_structures.h` en
operaciones reales:

- `DynamicArray`: almacenamiento dinámico utilizado como base de la pila y del
  heap de prioridad.
- `Stack`: conserva los IDs de cambios de rol pendientes de deshacer. Se
  reconstruye desde `CSVs/state_player_changes.csv` al iniciar.
- `Queue`: procesa en orden FIFO las cuentas pendientes de sincronización con
  Riot.
- `PriorityQueue`: ordena los informes de bajo rendimiento y agentes libres,
  conservando el orden original cuando hay empates.
- `CircularQueue`: mantiene las cinco partidas más recientes que se muestran
  para la cuenta seleccionada.

Las estructuras se implementan en C++ y no reemplazan los archivos CSV usados
para persistir los datos.