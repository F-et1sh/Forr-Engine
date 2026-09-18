# Forr-Engine

---

## About

**Forr-Engine** — is a framework for game development with an editor.\
It is something between [SFML](https://en.wikipedia.org/wiki/Simple_and_Fast_Multimedia_Library) and [Unreal Engine](https://en.wikipedia.org/wiki/Unreal_Engine).
I want to make this engine as flexible as possible and let the user assemble the rendering pipeline themself.

This is my second game engine after [For Engine 2D ( FE2D )](https://github.com/F-et1sh/For-Engine-2D) and my largest project.\
It is done in 3D, using **Vulkan** as the primary rendering backend and **OpenGL** as a fallback.\
Instead of *GLSL* I'm using modern **Slang** shader language here.

###### **TODO** : write more information in this file.

## Current Status

The project is very big and my core focus is currently on implementing a low-level architecture, so, there is no GUI yet.\
You can see what I'm working on right now in the [progress.md](Docs/progress.md).
The last screenshot :
<img width="2559" height="1439" alt="Screenshot 2026-08-25 154119" src="https://github.com/user-attachments/assets/258f3f04-a4f1-401b-85dc-8861575a7da2" />

Underneath this scene is a lot of work with :
- **RenderGraph** : Retained mode render graph
- **Slang Reflection API** : Slang parser and a constructor to make up pipelines like the Lego
- **Bindless textures** : The engine uses only bindless texture for now. I'm thinking about adding the default ones in some "Legacy OpenGL" backend in the future. See [not-now-but.md](Docs/not-now-but.md)
- **AZDO** : As a default method of rendering.

## Licenses

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
Licenses for third-party libraries used in this project are included in the `build` directory next to the executable ( `.exe` ).

---

Copyright (c) 2026 Farrah Fattah
