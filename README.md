# Forr-Engine

---

## About

**Forr-Engine** is my second game engine. This is planning to be my largest project.

---

## Solution projects

Forr.Core      - .lib ( shared structures and logic )
Forr.Platform  - .lib ( window and raw events )
Forr.Renderer  - .lib ( full renderer, uses Forr.RHI )
Forr.Resource  - .lib ( cpu resource manegement )
Forr.RHI		 - .lib ( render-hardware-interface/nvrhi )
Forr.Scene	 - .lib ( ECS scene )
Forr.Scripting - .lib ( C++ native scripting)

Forr.Engine	 - .dll ( glue code for modules )

Forr.Editor	 - .exe ( GUI editor for engine )
Forr.Game		 - .exe ( game entry point )

Game.Module - .dll ( game code/scripts ) ( NOT CREATED YET )

---

## Licenses

This project is **not open-source**.  
For detailed information about usage restrictions, see [`LICENSE.txt`](./LICENSE.txt).

Licenses for third-party libraries used in this project are included in the `build` directory next to the executable ( `.exe` ).

---

© 2026 Farrakh Fattakhov. All rights reserved.
