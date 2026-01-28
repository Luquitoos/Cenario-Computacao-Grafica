# Relatório Final de Conformidade - Ray Tracing "A Espada na Pedra"

## 0. Estrutura do Projeto e Guia de Modificação

Esta seção descreve a organização dos arquivos do projeto, explicando a responsabilidade de cada um e o impacto de suas modificações na imagem final.

### Arquivos Principais (`src/`)

*   **`src/scene_setup.cpp`**
    *   **O que é:** O coração criativo do projeto. Contém a função `create_scene()` onde todos os objetos, luzes e materiais são instanciados.
    *   **Mudanças Possíveis:** Adicionar novos objetos (`make_shared<sphere>`), mudar posições de translação, alterar cores de materiais, configurar o ciclo dia/noite.
    *   **Reflexo na Cena:** Qualquer alteração aqui muda diretamente o conteúdo visual do mundo (novas árvores, cor da espada, posição do sol).
    *   **Arquitetura:** Define a hierarquia e composição dos objetos complexos (ex: Animais, Tochas).

*   **`src/renderer.cpp`**
    *   **O que é:** O motor de renderização (Ray Tracer). Contém o loop principal que varre os pixels e calcula a cor (`ray_color_bvh`) e a lógica de sombras (`calculate_lighting_bvh`).
    *   **Mudanças Possíveis:** Otimizações de performance (OpenMP), alterar a lógica de sombras (suavização/bias), mudar a cor do fundo (skybox).
    *   **Reflexo na Cena:** Afeta a qualidade visual global, a performance (FPS) e como a luz interage fundamentalmente (ex: sombras duras vs suaves).

*   **`src/input_handlers.cpp`**
    *   **O que é:** Gerenciador de interações (Teclado/Mouse).
    *   **Mudanças Possíveis:** Adicionar atalhos de teclado (callback `keyboard`), mudar velocidade da câmera, alterar lógica do "Pick" (`perform_pick`).
    *   **Reflexo na Cena:** Afeta a experiência do usuário (navegação, seleção), não o visual estático.

*   **`src/globals.cpp` / `include/globals.h`**
    *   **O que é:** Variáveis globais de estado (Resolução, Câmera atual, Estado Dia/Noite, Listeners de Mouse).
    *   **Mudanças Possíveis:** Alterar `IMAGE_WIDTH`/`HEIGHT`, mudar a cor ambiente base.
    *   **Reflexo na Cena:** Configurações gerais que afetam todo o comportamento do programa.

### Bibliotecas e Classes (`include/`)

*   **`include/object/` (`sphere.h`, `cylinder.h`, `cone.h`, `mesh.h`)**
    *   **O que é:** Definição matemática e testes de intersecção das formas geométricas primitivas.
    *   **Mudanças Possíveis:** Alterar fórmulas de intersecção ou adicionar novas primitivas (ex: torus).
    *   **Reflexo na Cena:** Define *como* a geometria existe matematicamente.

*   **`include/material/material.h`**
    *   **O que é:** Definição dos materiais (Phong, Metal, Vidro, Emissivo).
    *   **Mudanças Possíveis:** Criar novos tipos de materiais (ex: PBR, Subsurface Scattering).
    *   **Reflexo na Cena:** Define a aparência física da superfície (brilho, refração, cor).

*   **`include/camera/camera.h`**
    *   **O que é:** Lógica da câmera e geração de raios para projeções.
    *   **Mudanças Possíveis:** Criar novas projeções (Ex: Olho de Peixe), alterar FOV, implementar Depth of Field.
    *   **Reflexo na Cena:** Muda *como* vemos o mundo (perspectiva, distorção, foco).

*   **`include/vectors/` (`mat4.h`, `vec3.h`)**
    *   **O que é:** Biblioteca matemática base para álgebra linear 3D.
    *   **Mudanças Possíveis:** Implementar novas operações (Inversa, Determinante).
    *   **Reflexo na Cena:** Fundamental para calcular transformações (rotação, shear) e posicionamento corretamente.

---

# Relatório Final de Conformidade - Ray Tracing "A Espada na Pedra"

Este relatório descreve detalhadamente o cumprimento dos requisitos do projeto, estruturado para identificar claramente o **CONCEITO** (Teoria), o **CENÁRIO** (Manifestação Visual), e a **IMPLEMENTAÇÃO** (Código).

---

## 1. Definição do Cenário

### 1.1. Coerência temática (Obrigatório)
*   **Conceito (O que é):** A coerência temática é a unificação lógica e estética dos elementos de um mundo virtual. Em vez de objetos aleatórios, o cenário deve contar uma história visual onde todos os componentes (geometria, luz, materiais) pertencem ao mesmo universo e contribuem para a imersão.
*   **Cenário (Onde ver):** O cenário representa um local sagrado e antigo de fantasia. Visualizamos uma floresta densa circundando um lago calmo. No centro, sobre uma rocha mística, repousa a "Espada na Pedra", iluminada por uma luz divina. Ao redor, ruínas de pilares indicam uma civilização antiga. Animais (lobos, ursos, pássaros) habitam a floresta, e o ciclo dia/noite altera a atmosfera de "Aventura" (dia) para "Mistério" (noite, com vagalumes e tochas).
*   **Código (Onde está):** `src/scene_setup.cpp`, função `create_scene()`.
*   **Implementação (Como funciona):** O código orquestra a criação procedural de vegetação (cogumelos, árvores) e a disposição manual de elementos chave (espada, ruínas) usando um sistema de coordenadas unificado.

### 1.2. Coordenadas do mundo (Obrigatório)
*   **Conceito (O que é):** O sistema de coordenadas World Space define a posição global de todos os objetos. A exigência do "primeiro octante" ($x, y, z \ge 0$) simplifica a matemática espacial, evitando coordenadas negativas que podem complicar cálculos de câmera e frustum.
*   **Cenário (Onde ver):** A espada, que é o centro do mundo, está em $(900, 195, 900)$. O "chão" da floresta está no plano $y=0$. Não há objetos "enterrados" em coordenadas negativas globais.
*   **Código (Onde está):** `src/scene_setup.cpp`, constantes `CX=900`, `CZ=900`.
*   **Implementação (Como funciona):** Todas as posições de objetos são definidas somando valores positivos a essas constantes centrais, garantindo que tudo permaneça no octante positivo.

### 1.3. Objetos

#### 1.3.1. Tipos de objetos (Obrigatório)

**I. Esfera**
*   **Conceito (O que é):** Primitiva geométrica definida pelo conjunto de pontos equidistantes de um centro. É a forma mais simples para Ray Tracing devido à facilidade de cálculo de intersecção analítica.
*   **Cenário (Onde ver):** As jóias (Rubi, Safira, Esmeralda) na espada; os corpos redondos dos coelhos e vagalumes.
*   **Código (Onde está):** `src/scene_setup.cpp` (ex: `make_shared<sphere>`).
*   **Implementação:** A classe `sphere` testa a intersecção resolvendo uma equação quadrática para $t$.

**II. Cilindro**
*   **Conceito (O que é):** Superfície formada pelo deslocamento de um círculo ao longo de um eixo reto. Usado para modelar objetos tubulares ou pilares.
*   **Cenário (Onde ver):** Troncos de árvores, pilares das ruínas e cabos de tochas.
*   **Código (Onde está):** `src/scene_setup.cpp` (ex: `make_shared<cylinder>`).
*   **Implementação:** A classe `cylinder` testa intersecção com um cilindro infinito e depois recorta pelos planos de base e topo (tampas).

**III. Cone**
*   **Conceito (O que é):** Sólido que afunila suavemente de uma base circular plana até um ponto chamado ápice.
*   **Cenário (Onde ver):** A chama das tochas (formato de fogo) e a ponta pontiaguda da espada.
*   **Código (Onde está):** `include/object/cone.h`.
*   **Implementação:** Similar ao cilindro, mas o raio varia linearmente com a altura.

**IV. Malha (Mesh)**
*   **Conceito (O que é):** Objeto complexo formado por um conjunto de polígonos (geralmente triângulos) que definem sua superfície. Permite formas arbitrárias não-primitivas.
*   **Cenário (Onde ver):** A geometria facetada do "Lobo" e do "Cavalo" (estilo Low Poly) e a lâmina detalhada da espada.
*   **Código (Onde está):** `src/scene_setup.cpp` (uso de `box_mesh` e `blade_mesh`).
*   **Implementação:** `blade_mesh` cria vértices e faces para modelar o perfil específico de uma lâmina de espada.

#### 1.3.2. Materiais (Obrigatório)
*   **Conceito (O que é):** Definição matemática de como a luz interage com a superfície do objeto. O modelo de Phong separa a interação em componentes: Ambiente (luz indireta), Difusa (cor base) e Especular (brilho/reflexo direto).
*   **Cenário (Onde ver):** O metal da espada brilha (especular alto); a pedra é fosca (difusa alta); o rubi brilha e transmite luz (material dielétrico/emissivo).
*   **Código (Onde está):** `include/material/material.h`.
*   **Implementação:** Cada material possui coeficientes $K_a, K_d, K_s$ e expoente de brilho $\alpha$ usados na equação de iluminação em `renderer.cpp`.

#### 1.3.3. Textura (Obrigatório)
*   **Conceito (O que é):** Técnica para adicionar detalhe de superfície sem aumentar a complexidade geométrica, mapeando uma imagem 2D (bitmap) sobre a superfície 3D (Mapeamento UV).
*   **Cenário (Onde ver):** Riscos e manchas na superfície metálica da lâmina da espada; detalhes de musgo e terra no chão.
*   **Código (Onde está):** `image_texture::value` em `include/textures/texture.h`.
*   **Implementação:** A cor base do material em um ponto $(u,v)$ é lida diretamente do pixel correspondente na imagem JPG carregada.

### 1.4. Transformações

#### 1.4.1. Translação (Obrigatório)
*   **Conceito (O que é):** Transformação rígida que desloca todos os pontos de um objeto por um vetor constante $\vec{t} = (tx, ty, tz)$.
*   **Cenário (Onde ver):** Posiciona a espada no topo da rocha (y=195) e as árvores longe do centro.
*   **Código (Onde está):** `mat4::translate` em `src/scene_setup.cpp`.
*   **Implementação:** Multiplicação por matriz $4 \times 4$ onde a última coluna contém o vetor de translação.

#### 1.4.2. Rotação (Obrigatório)
*   **Conceito (O que é):** Transformação que gira o objeto em torno de um eixo (origem ou arbitrário) por um ângulo $\theta$.
*   **Eixos Canônicos:**
    *   **Cenário:** Lobos girados no eixo Y para olhar para o centro.
    *   **Código:** `mat4::rotate_y`.
*   **Eixo Arbitrário (Obrigatório):**
    *   **Conceito Extra:** Rotação em torno de um vetor qualquer $\vec{v}$, não alinhado com X, Y ou Z.
    *   **Cenário:** Gemas (Safira/Esmeralda) girando em órbitas inclinadas/diagonais ao redor da espada.
    *   **Código:** `rotate_axis_object` usa quatérnios para evitar *Gimbal Lock*.
    *   **Implementação:** Computação da matriz de rotação a partir de um quatérnio $q = [\cos(\theta/2), \vec{v}\sin(\theta/2)]$.

#### 1.4.3. Escala (Obrigatório)
*   **Conceito (O que é):** Transformação que altera o tamanho do objeto, podendo ser uniforme (mantém proporção) ou não-uniforme (deforma/achata).
*   **Cenário (Onde ver):** O corpo dos coelhos (esferas achatadas em elipsoides) e o tamanho variado das árvores.
*   **Código (Onde está):** `mat4::scale`.
*   **Implementação:** Matriz diagonal com fatores de escala $(S_x, S_y, S_z)$.

#### 1.4.4. Cisalhamento (Shear) (+ 0.5)
*   **Conceito (O que é):** Deformação onde o objeto é deslocado lateralmente de forma proporcional à distância de um eixo, alterando ângulos (quadrados viram paralelogramos).
*   **Cenário (Onde ver):**
    *   **Ruínas:** O Pilar 1 está "tombando" (as laterais estão inclinadas) mas o topo e a base continuam paralelos ao chão (efeito de shear puro).
    *   **Asas:** As asas das borboletas têm o formato em "V" criado cisalhando cubos.
*   **Código (Onde está):** `mat4::shear` em `src/scene_setup.cpp`.
*   **Implementação:** Matriz onde coordenadas dependem linearmente de outras (ex: $x' = x + ay$).

#### 1.4.5. Espelho em relação a um plano arbitrário (+ 0.5)
*   **Conceito (O que é):** Reflexão geométrica de todos os pontos do objeto em relação a um plano definido por um ponto $P$ e normal $\vec{n}$.
*   **Cenário (Onde ver):** O reflexo invertido da "Gema Vermelha" e da "Ponta da Espada" visível *dentro* da água do lago.
*   **Código (Onde está):** `reflect_object` em `src/scene_setup.cpp`.
*   **Implementação:** Cria uma cópia do objeto (instância espelhada) transformada pela matriz de reflexão de Householder generalizada para planos afins.

### 1.5. Fontes Luminosas

#### 1.5.1. Pontual (Obrigatório)
*   **Conceito (O que é):** Fonte de luz idealizada que emite raios em todas as direções a partir de um único ponto no espaço. A intensidade cai com o quadrado da distância.
*   **Cenário (Onde ver):** O brilho amarelo/laranja dentro das lanternas medievais e os corpos brilhantes dos vagalumes.
*   **Código (Onde está):** `make_shared<point_light>`.
*   **Implementação:** Cálculo da direção da luz $L = position - point$ e atenuação $1/(Kc + Kl \cdot d + Kq \cdot d^2)$.

#### 1.5.2. Spot (+ 1.0)
*   **Conceito (O que é):** Fonte de luz pontual restrita a um cone de emissão, simulando holofotes ou lanternas direcionais.
*   **Cenário (Onde ver):** O feixe de "Luz Divina" que cai do céu iluminando apenas a espada e a rocha, deixando o entorno escuro.
*   **Código (Onde está):** `make_shared<spot_light>`.
*   **Implementação:** Se o ângulo entre o vetor luz-objeto e a direção do spot for maior que o `cutoff`, a intensidade é zero.

#### 1.5.3. Direcional (+ 0.5)
*   **Conceito (O que é):** Fonte de luz no infinito que emite raios paralelos, simulando corpos celestes distantes (Sol/Lua). Não tem posição, apenas direção.
*   **Cenário (Onde ver):** A iluminação geral que cria sombras paralelas uniformes no chão durante o dia (Sol).
*   **Código (Onde está):** `make_shared<directional_light>`.
*   **Implementação:** Vetor $L$ é constante para todos os pontos da cena.

#### 1.5.4. Ambiente (Obrigatório)
*   **Conceito (O que é):** Aproximação da luz indireta (refletida por outras superfícies) que garante que áreas não atingidas por luz direta não fiquem totalmente pretas.
*   **Cenário (Onde ver):** As partes "nas costas" das árvores e pedras ainda são visíveis (cinza/azul escuro), não são silhuetas pretas.
*   **Código (Onde está):** Variável `ambient.intensity`.
*   **Implementação:** Adição de uma cor base constante na equação de iluminação final.

### 2. Câmera

#### 2.1. Especificação de Eye, At, Up (Obrigatório)
*   **Conceito (O que é):** Parâmetros que definem o sistema de coordenadas da câmera (espaço da câmera). Eye=Posição, At=Foco, Up=vetor de orientação vertical.
*   **Cenário (Onde ver):** A posição inicial da câmera mostra a espada de frente; ao mover (WASD), esses parâmetros mudam, alterando o ponto de vista na tela.
*   **Código (Onde está):** Variáveis em `globals.cpp` utilizadas em `setup_camera()`.

#### 2.2. Parâmetros Adicionais (Distância Focal, Campo de Visão) (Obrigatório)
*   **Conceito (O que é):** Determinam a "lente" da câmera. Distância Focal ($d$) define o zoom óptico; Campo de Visão (FOV) define a abertura angular visível.
*   **Cenário (Onde ver):** O enquadramento da cena. Um FOV maior mostraria mais da floresta lateralmente.
*   **Código (Onde está):** `cam.setup()` em `src/scene_setup.cpp`.

### 3. Projeções

#### 3.1. Perspectiva (Obrigatório)
*   **Conceito (O que é):** Projeção que imita a visão humana, onde objetos distantes parecem menores (escorço). Raios convergem para o centro de projeção (olho).
*   **Cenário (Onde ver):** Visão padrão. As árvores distantes parecem menores que as próximas.
*   **Código (Onde está):** `ProjectionType::PERSPECTIVE` em `camera.h`.

**Zoom In/Out (3.1.1):**
*   **Conceito:** Alterar o ângulo de visão aproximando a imagem sem mover a câmera.
*   **Cenário:** Teclas +/- aproximam a imagem da espada.
*   **Código:** `cam.zoom_in()` altera os limites da janela de projeção ($x_{min}, x_{max}, ...$).

**Pontos de Fuga (3.1.2) (+ 1.5):**
*   **Conceito:** Pontos no horizonte para onde retas paralelas convergem na perspectiva.
    *   **1 Ponto:** (Tecla 1/Preset 1) Câmera alinhada ao eixo Z. Linhas de profundidade convergem ao centro.
    *   **2 Pontos:** (Preset 2) Câmera em diagonal. Linhas X e Z convergem para dois pontos laterais.
    *   **3 Pontos:** (Preset 3) Câmera diagonal vista de cima ("olho de pássaro"). Linhas X, Y e Z convergem.
*   **Código:** `apply_vanishing_point_preset` em `src/scene_setup.cpp`.

#### 3.2. Ortográfica (+ 0.5)
*   **Conceito (O que é):** Projeção paralela onde raios são perpendiculares ao plano de projeção. Não há deformação de perspectiva; objetos mantêm tamanho real.
*   **Cenário (Onde ver):** Tecla 'O'. A cena fica "achatada". Objetos distantes têm o mesmo tamanho dos próximos. Útil para CAD/Arquitetura.
*   **Código (Onde está):** `ProjectionType::ORTHOGRAPHIC` (raios com direção constante $-w$).

#### 3.3. Oblíqua (+ 0.5)
*   **Conceito (O que é):** Projeção paralela onde os raios incidem obliquamente no plano (ângulos $\alpha \ne 90^\circ$). Preserva a escala da face frontal, mas projeta a profundidade lateralmente.
*   **Cenário (Onde ver):** Tecla 'P'. Vemos a face frontal da espada perfeita (como 2D) mas vemos também a lateral e o topo projetados diagonalmente (estilo jogos isométricos antigos).
*   **Código (Onde está):** `ProjectionType::OBLIQUE` em `camera.h`.

### 4. Sombra (Obrigatório)
*   **Conceito (O que é):** Região onde a luz é ocluída por um objeto opaco. Em Ray Tracing, é calculada verificando se o caminho até a luz está bloqueado.
*   **Cenário (Onde ver):** Sombra nítida da espada projetada na rocha atrás dela. Sombra dos pilares no chão.
*   **Código (Onde está):** `renderer.cpp`.
*   **Implementação:** Lança um raio secundário ("Shadow Ray") do ponto de intersecção em direção à luz.

### 5. Interatividade

#### 5.1. Função de Pick (Obrigatório)
*   **Conceito (O que é):** Técnica de seleção de objetos 3D através de um clique em coordenada 2D de tela (Ray Casting Inverso).
*   **Cenário (Onde ver):** Ao clicar num objeto (ex: "Tocha"), o console e a tela exibem: "Selecionado: Tocha Medieval".
*   **Código (Onde está):** `perform_pick` em `src/input_handlers.cpp`.
*   **Implementação:** Gera um raio passando pelo pixel do mouse e encontra o primeiro objeto interceptado na cena.

#### 5.2. Interface Gráfica (Bônus)
*   **Conceito (O que é):** Elementos visuais sobrepostos à cena 3D para feedback ao usuário (HUD).
*   **Cenário (Onde ver):** Texto informativo no canto superior esquerdo ("Projeção: Perspectiva", "Pick: ...").
*   **Código (Onde está):** `GUIManager::draw` em `src/gui`.

### 6. Imagem Gerada (Obrigatório)
*   **Conceito (O que é):** O resultado final do processo de renderização (síntese de imagem).
*   **Cenário (Onde ver):** A janela gráfica de 600x600 pixels exibindo a cena completa.
*   **Código (Onde está):** Loop principal de renderização pixel a pixel em `renderer.cpp`.

### 7. Bônus de Criatividade e Beleza
1.  **Ciclo Dia/Noite (Tecla N):**
    *   *Conceito:* Simulação dinâmica de tempo.
    *   *Cenário:* Mudança total de iluminação (sol para lua), cor do céu e ativação de luzes artificiais (tochas).
2.  **Modelagem Procedural:**
    *   *Conceito:* Criação de conteúdo via algoritmos em vez de modelagem manual.
    *   *Cenário:* Animais e árvores cujas formas e posições são geradas pelo código.

## 8. Guia de Apresentação e Modificação em Tempo Real

Esta seção é um **manual de emergência** para a apresentação. Ela contém instruções exatas de onde ir no código e o que digitar caso o avaliador peça modificações específicas ao vivo.

### 8.1. Provas Fundamentais

#### Pedido: "Prove que estamos no primeiro octante"
*   **Procedimento:** Abra `src/scene_setup.cpp`.
*   **Mostre:** As linhas iniciais da função `create_scene()` onde definimos `CX = 900.0` e `CZ = 900.0`.
*   **Explique:** "Todos os objetos são criados somando a essas constantes positivas. A espada está em Y=195. Não há valores negativos absolutos."

### 8.2. Manipulação de Luzes

#### Pedido: "Tire uma luz específica ou mude de lugar"
*   **Arquivo:** `src/scene_setup.cpp`, procure por `lights.push_back`.
*   **Remover:** Comente a linha `lights.push_back(...)`.
*   **Mudar de lugar:** Altere os valores `point3(x, y, z)` no construtor da luz.
    *   *Exemplo (Tocha):* Procure as luzes das tochas (perto da linha 1300 ou busca por "Torch Light") e mude `point3(tx, 145, tz)` para `point3(tx, 200, tz)` (mais alto).

#### Pedido: "Aumente a intensidade da luz"
*   **Procedimento:** No `src/scene_setup.cpp`, encontre a criação da luz.
*   **Código:** O último parâmetro (normalmente `c` ou `kc, kl, kq`) define a atenuação.
*   **Ação:** Para luzes pontuais, *diminua* os fatores de atenuação `0.00002` para `0.00001` (menor atenuação = luz vai mais longe) ou *aumente* a cor base de `color(1, 0.6, 0.2)` para `color(5, 3, 1)` (valores acima de 1 geram luzes "HDR" mais fortes).

#### Pedido: "Adicione uma fonte luminosa nova (Pontual, Spot ou Direcional)"
*   **Arquivo:** `src/scene_setup.cpp`, dentro de `create_scene()`, logo após limpar as luzes anteriores.
*   **Códigos Prontos para Copiar/Colar:**
    *   **Pontual:**
        ```cpp
        // Luz branca forte em cima da espada
        auto nova_luz = make_shared<point_light>(point3(900, 300, 900), color(1,1,1), 1.0, 0.001, 0.0001);
        lights.push_back(nova_luz);
        ```
    *   **Spot:**
        ```cpp
        // Holofote vermelho focado na espada
        auto novo_spot = make_shared<spot_light>(point3(900, 300, 900), point3(0, -1, 0), color(1,0,0), 30.0);
        lights.push_back(novo_spot);
        ```
    *   **Direcional:**
        ```cpp
        // Sol vindo do horizonte
        auto novo_sol = make_shared<directional_light>(vec3(1, -0.2, 0), color(0.5, 0.5, 0.5));
        lights.push_back(novo_sol);
        ```
    *   **Ambiente:**
        *   Mude a variável global `ambient` no início de `src/scene_setup.cpp`.
        *   `ambient = ambient_light(color(1, 1, 1), 0.8);` (Ambiente muito claro).

### 8.3. Objetos e Materiais

#### Pedido: "Adicione um objeto ao mundo (Esfera, Cilindro, ...)"
*   **Arquivo:** `src/scene_setup.cpp`, final da função `create_scene()`, antes do `setup_camera()`.
*   **Local Visível:** Perto da espada (900, 250, 900) para garantir que apareça na câmera inicial.
*   **Códigos:**
    *   **Esfera:**
        ```cpp
        auto mat_teste = make_shared<phong>(color(1,0,0), 0.1, 0.9, 0.5, 50); // Vermelho
        world.add(make_shared<sphere>(point3(920, 200, 900), 10.0, mat_teste), "Esfera Teste");
        ```
    *   **Cilindro:**
        ```cpp
        world.add(make_shared<cylinder>(point3(880, 200, 900), vec3(0,1,0), 5.0, 20.0, mat_teste), "Cilindro Teste");
        ```
    *   **Cone:**
        ```cpp
        world.add(make_shared<cone>(point3(900, 250, 900), 10.0, vec3(0,1,0), 20.0, mat_teste), "Cone Teste");
        ```
    *   **Malha (Cubo de Teste):**
        ```cpp
        auto mesh_teste = make_shared<box_mesh>(point3(900, 200, 950), 10.0, 10.0, 10.0, mat_teste);
        world.add(mesh_teste, "Cubo Teste");
        ```

#### Pedido: "Crie um material novo e aplique"
*   **Procedimento:**
    1.  Crie o material: `auto meu_mat = make_shared<phong>(color(0,1,0), 0.2, 0.8, 0.1, 10);` (Verde Plástico).
    2.  Use `meu_mat` no construtor do objeto.

#### Pedido: "Faça que um objeto específico seja uma fonte luminosa"
*   **Procedimento:** Crie um material emissivo (que emite luz própria).
    ```cpp
    // Material emissivo (Cor R, G, B) e intensidade
    auto mat_luz = make_shared<diffuse_light>(color(4, 4, 4)); 
    world.add(make_shared<sphere>(point3(900, 250, 900), 10.0, mat_luz), "Esfera de Luz");
    ```

#### Pedido: "Escala: Prove onde é feito e mude instruindo como fazer"
*   **Localizar:** Procure por `mat4::scale` em `src/scene_setup.cpp`.
*   **Exemplo Existente:** As coxas dos coelhos ou o corpo dos animais.
    ```cpp
    // Código atual:
    mat4::scale(1.0, 0.8, 1.0)
    ```
*   **Ação:** Mude os valores dentro da função `scale(x, y, z)`. Se mudar para `scale(2.0, 0.8, 1.0)`, o objeto esticará em X.

#### Pedido: "Aplique uma textura"
*   **Procedimento:**
    ```cpp
    auto tex = make_shared<image_texture>("textures/rochas.jpg"); // Use uma textura existente
    auto mat_tex = make_shared<phong>(tex, 1.0, 0.0, 10); // Material com textura
    world.add(make_shared<sphere>(point3(900, 220, 900), 15.0, mat_tex), "Esfera Texturizada");
    ```

### 8.4. Transformações (Sem GUI - Código Bruto)

#### Pedido: "Aplique transformação, rotação, escala ou cisalhamento manualmente"
*   **Arquivo:** `src/scene_setup.cpp`.
*   **Lógica:** Crie um objeto, envolva-o em um `transform`.
*   **Código Genérico:**
    ```cpp
    // 1. Criar Objeto Base
    auto objeto_base = make_shared<box_mesh>(point3(0,0,0), 10, 10, 10, mat_metal);
    
    // 2. Criar Matriz de Transformação
    mat4 M = mat4::identity();
    
    // MUDANÇA AQUI: Escolha a operação pedida:
    M = mat4::translate(900, 200, 900); // Translação
    // OU
    M = mat4::translate(900, 200, 900) * mat4::rotate_y(degrees_to_radians(45)); // Rotação
    // OU
    M = mat4::translate(900, 200, 900) * mat4::scale(1, 2, 0.5); // Escala não-uniforme
    // OU
    M = mat4::translate(900, 200, 900) * mat4::shear(0.5, 0, 0, 0, 0, 0); // Cisalhamento (Shear)
    
    // 3. Adicionar ao mundo transformado
    world.add(make_shared<transform>(objeto_base, M), "Objeto Transformado");
    ```

#### Pedido: "Como aplicar Espelho em relação a um plano arbitrário?"
*   **Arquivo:** `src/scene_setup.cpp`.
*   **Código:** Use a função auxiliar `reflect_plane` da `mat4`.
    ```cpp
    vec3 ponto_plano(900, 200, 900);
    vec3 normal_plano(1, 1, 0); // Plano diagonal a 45 graus
    mat4 M_reflete = mat4::reflect_plane(ponto_plano, unit_vector(normal_plano));
    world.add(make_shared<transform>(sword_mult, M_reflete), "Espada Refletida");
    ```

### 8.5. Câmera e Projeções

#### Pedido: "Modifique a posição, up vector ou foco da câmera (Eye, At, Up)"
*   **Arquivo:** `src/scene_setup.cpp`, função `setup_camera()`.
*   **Código:**
    ```cpp
    cam.lookat(
        point3(1000, 500, 1000), // Eye (De onde vê - Mude aqui)
        point3(900, 195, 900),   // At (Para onde olha)
        vec3(0, 1, 0)            // Up (Onde é 'cima' - Mude para (1,0,0) para deitar a câmera)
    );
    ```

#### Pedido: "Mude o Campo de Visão (FOV) ou Distância Focal"
*   **Código:** Dentro de `setup_camera()`:
    *   `cam.set_fov(90.0);` (Aumente para "Grande Angular", diminua para "Zoom").
    *   `cam.focal_distance = 50.0;` (Distância do plano de projeção).

#### Pedido: "Zoom In / Zoom Out via código (Janela de Projeção)"
*   **Teoria:** Zoom é diminuir a janela de projeção.
*   **Código:** Em `include/camera/camera.h`, as variáveis `xmin, xmax, ymin, ymax` controlam isso.
*   **Na prática agora:** Em `setup_camera()`, altere:
    ```cpp
    // Zoom IN (Janela menor)
    cam.set_window(-100, 100, -100, 100); 
    // Zoom OUT (Janela maior)
    cam.set_window(-400, 400, -400, 400); 
    ```

#### Pedido: "Demonstre Perspectivas 1, 2, 3 pontos, Ortográfica e Oblíqua"
*   **Arquivo:** `src/scene_setup.cpp`.
*   **Ação Rápida:** Chame `apply_vanishing_point_preset(N)` no final de `create_scene()` ou altere manualmente:
    *   **1 Ponto:** `cam.lookat(point3(900, 195, 1200), point3(900, 195, 900), vec3(0,1,0));` (Alinhado em Z).
    *   **2 Pontos:** `cam.lookat(point3(1200, 195, 1200), point3(900, 195, 900), vec3(0,1,0));` (Diagonal plana).
    *   **3 Pontos:** `cam.lookat(point3(1200, 500, 1200), point3(900, 195, 900), vec3(0,1,0));` (Diagonal alta).
    *   **Ortográfica:** `cam.set_projection(ProjectionType::ORTHOGRAPHIC);`
    *   **Oblíqua:** `cam.set_projection(ProjectionType::OBLIQUE); cam.set_oblique_params(degrees_to_radians(45), 0.5);`

### 8.6. Sombras

#### Pedido: "Onde calcula a sombra? Mude algo nela"
*   **Arquivo:** `src/renderer.cpp`, função `calculate_lighting_bvh`.
*   **Localizar:** O bloco `// Shadow ray`.
    ```cpp
    ray shadow_ray(rec.p + 0.001 * rec.normal, L); // O 0.001 é o BIAS
    ```
*   **Mudança para provar:**
    *   **Tirar sombra:** Comente o `if (scene_bvh.hit(shadow_ray...)) continue;`. Tudo ficará iluminado, mesmo atrás de paredes.
    *   **Shadow Acne:** Mude `0.001` para `0.0`. A imagem ficará cheia de ruído preto nos próprios objetos.
