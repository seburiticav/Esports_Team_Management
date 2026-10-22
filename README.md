# Esports_Team_Management

Proyecto final desarrollado para la asignatura de Estructura de Datos. Esta aplicación utiliza la API oficial de Riot Games para consultar, procesar y visualizar estadísticas de jugadores (Maestrias, KDAs, Rango y Winrate) vinculados a sus datos personales (ID, Nombre, Apellido, País y Fecha de Nacimiento).

# Integrantes
 - Sebastian Buritica Velasco
 - Andres Felipe Otero Barrera

# Estructura de datos utilizadas
En el siguiente proyecto se implementaron tres estructuras de datos lineales para gestionar la información obtenida de la API y de la base de datos local:

1. *Arreglos Dinámicos (Vector / ArrayList / List)*:
   * *Uso:* Almacenamiento general de la lista de jugadores y sus datos de cuenta registrados en el sistema (ID, Nombre, Apellido, País, Riot ID, Tagline, PUUID)[cite: 2, 3].
2. *Cola con Prioridad (Priority Queue)*:
   * *Uso:* Construcción del Leaderboard o ranking general. Ordena automáticamente a los jugadores con base en sus puntos de liga (LP) y su tasa de victorias (Winrate).
3. *Pila (Stack) / Cola Circular (Circular Queue)*:
   * *Uso:* Gestión del historial reciente de búsquedas y buffer de peticiones a la API para controlar el límite de tasa de solicitudes (rate-limiting).

# Uso de Inteligencia Artificial

Cumpliendo con la normativa de la entrega:

* *Porcentaje de código generado por IA:* ~75%
* *Herramientas empleadas:* Claude/GPT-6
* *Uso específico:* 

# Tecnologías y Requisitos

* *Lenguaje:* [ C++]
* *Librerías principales:* 
* *Clave de API:* Se requiere una RIOT_API_KEY válida obtenida desde el portal de [Riot Developer Portal](https://developer.riotgames.com/).


# Instalación y Ejecución

1. *Clonar el repositorio:*
   Ejecutar en elbash
   git clone [https://github.com/seburiticav/Esports_Team_Management.git]
   cd Esports_Team_Management
---
