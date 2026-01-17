# Trabalho de Computação Gráfica - Espada na Pedra

Ray Caster implementado em C++ com FreeGLUT para a disciplina de Computação Gráfica.

## Tema

Uma **espada medieval cravada em uma pedra**, inspirada na lenda do Rei Arthur.

## Requisitos Atendidos

### Objetos (1.3.1) 
- **Esfera**: Gema do pomo, pedras decorativas
- **Cilindro**: Cabo e guarda da espada
- **Cone**: Decoração no topo do pomo
- **Malha**: Pedra base, lâmina da espada

### Materiais (1.3.2) 
1. Metal (espada) - brilho alto
2. Pedra (rocha) - textura procedural
3. Couro (cabo) - fosco
4. Gema (rubi) - muito brilhante

### Textura (1.3.3) 
- Textura procedural de pedra
- Textura checker no chão

### Transformações (1.4) 
- Translação: posicionamento de todos objetos
- Rotação em X/Y/Z: inclinação da espada
- Rotação em eixo arbitrário (Quatérnios): caixa decorativa
- Escala: proporções dos objetos
- Cisalhamento: caixa demonstrativa
- Espelho: esfera refletida

### Iluminação (1.5) 
- Ambiente: iluminação base
- Pontual: luz principal
- Spot: foco na espada
- Direcional: luz do "sol"

### Câmera (2) 
- Eye, At, Up configuráveis
- Distância focal e janela de visão

### Projeções (3) 
- Perspectiva com 1, 2, 3 pontos de fuga
- Ortográfica
- Oblíqua
- Zoom in/out

### Sombras (4) 
- Shadow rays implementados

### Interatividade (5) 
- Pick com mouse
- Interface com teclado

## Compilação

### Windows (MinGW)

1. Instale o FreeGLUT
2. Execute:
```bash
mingw32-make
```

### Alternativa direta:
```bash
g++ -std=c++17 -O2 -I./include -o raytracer.exe src/main.cpp -lfreeglut -lopengl32 -lglu32
```

## Controles

| Tecla | Ação |
|-------|------|
| 1, 2, 3 | Perspectiva com 1/2/3 pontos de fuga |
| O | Projeção Ortográfica |
| B | Projeção Oblíqua |
| +/- | Zoom In/Out |
| Click | Pick de objeto |
| Q/ESC | Sair |

## Estrutura do Projeto

```
Trabalho-CG/
├── include/
│   ├── vec3.h          # Vetores 3D
│   ├── vec4.h          # Vetores 4D (homogêneos)
│   ├── mat4.h          # Matrizes 4x4
│   ├── quaternion.h    # Quatérnios
│   ├── ray.h           # Raios
│   ├── color.h         # Cores RGB
│   ├── texture.h       # Texturas
│   ├── material.h      # Materiais
│   ├── hittable.h      # Interface de objetos
│   ├── hittable_list.h # Lista de objetos
│   ├── sphere.h        # Esfera
│   ├── plane.h         # Plano
│   ├── cylinder.h      # Cilindro
│   ├── cone.h          # Cone
│   ├── triangle.h      # Triângulo
│   ├── mesh.h          # Malhas
│   ├── transform.h     # Transformações
│   ├── light.h         # Iluminação
│   ├── camera.h        # Câmera
│   └── utils.h         # Utilitários
├── src/
│   └── main.cpp        # Aplicação principal
├── Makefile
└── README.md
```

## Autor

Trabalho desenvolvido para a disciplina de Computação Gráfica.
