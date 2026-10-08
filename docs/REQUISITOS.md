# Projeto: Tab5 Retro Arcade — Central de Jogos para M5Stack Tab5

## 1. Objetivo

Desenvolva um firmware completo chamado **Tab5 Retro Arcade**, destinado ao M5Stack Tab5, que disponibilize uma coleção de jogos clássicos em uma interface gráfica moderna, colorida e inspirada nos consoles portáteis e arcades dos anos 1980 e 1990.

O aplicativo deve funcionar nativamente no ESP32-P4, sem depender de navegador, conexão com a internet ou serviços externos.

O sistema deve ter arquitetura modular, permitindo incorporar novos jogos sem alterações significativas no núcleo da aplicação.

Os três primeiros jogos serão:

1. **Block Drop** — implementação completa do clássico jogo de blocos inspirado em Tetris.
2. **Snake** — jogo da cobrinha com diferentes mapas, dificuldades e tipos de alimentos.
3. **Retro Racer** — jogo de corrida 2D com visão superior, inspirado nos clássicos jogos de corrida de minigames e consoles portáteis.

O objetivo é produzir jogos completos, não demonstrações técnicas.

## 2. Hardware e ambiente

Hardware de destino:

- M5Stack Tab5.
- Processador ESP32-P4.
- Tela IPS de 5 polegadas, resolução 1280×720.
- Orientação obrigatória: paisagem (landscape).
- 16 MB de Flash e 32 MB de PSRAM.
- Teclado físico oficial M5Stack Tab5 Keyboard.
- Teclado conectado via I²C, endereço padrão 0x6D.
- Pinos do teclado: SDA GPIO0, SCL GPIO1 e INT GPIO50, conforme documentação oficial.
- Alto-falante integrado para músicas e efeitos sonoros.
- Tela sensível ao toque, utilizada opcionalmente nos menus.
- Cartão microSD para armazenamento complementar, quando disponível.

Priorizar ESP-IDF e C/C++, utilizando o BSP oficial da M5Stack e LVGL para menus e interfaces, desde que as versões sejam compatíveis.

Investigar previamente os exemplos oficiais da placa e do teclado.

Não presumir que o teclado funciona como um teclado USB convencional. Implementar corretamente a leitura de eventos pelo protocolo suportado pelo acessório.

Repositórios de referência:

- https://github.com/m5stack/M5Tab5-Keyboard-UserDemo
- https://github.com/m5stack/M5Unit-KEYBOARD

Consultar a documentação atual do BSP do Tab5 e verificar a variante do controlador de tela utilizada.

O firmware deve ser compatível com instalação pelo M5Launcher, gerando o arquivo BIN adequado para execução pelo launcher, além de permitir gravação convencional por USB.

Verificar o formato, particionamento e endereço de carregamento exigidos pelo M5Launcher antes de implementar a exportação. Não presumir que um app BIN comum e uma imagem completa da Flash sejam intercambiáveis.

## 3. Arquitetura

Separar a aplicação nos seguintes módulos:

- Core: gerenciamento geral da aplicação.
- Display: inicialização do display e renderização.
- Input: leitura do teclado e gerenciamento de eventos.
- Audio: música e efeitos sonoros.
- GameManager: carregamento, inicialização e encerramento dos jogos.
- Storage: arquivos de configuração, recordes e savegames.
- UI: menus, notificações e telas compartilhadas.
- Games: implementação independente de cada jogo.

Cada jogo deve implementar uma interface padronizada com métodos equivalentes a:

- init()
- start()
- update(deltaTime)
- render()
- pause()
- resume()
- save()
- load()
- reset()
- cleanup()

Evitar código monolítico.

Cada jogo deve ter seu próprio diretório e seus próprios arquivos de lógica, renderização e configuração.

Usar uma arquitetura que permita adicionar um quarto jogo simplesmente registrando seu módulo no GameManager.

Separar a lógica da simulação da taxa de atualização da tela.

Implementar temporização consistente, tratamento de eventos e limpeza dos recursos utilizados.

## 4. Interface principal

Criar uma interface de arcade com estética retrô:

- Fundo escuro.
- Elementos coloridos e contrastantes.
- Fontes pixeladas ou semelhantes às utilizadas em arcades.
- Ícones representando cada jogo.
- Animações suaves.
- Efeitos sonoros de navegação.
- Realce visual do item selecionado.
- Layout adaptado aos 1280×720 pixels.

A tela principal deve apresentar:

**TAB5 RETRO ARCADE**

Opções:

- Jogar
- Continuar partida
- Recordes
- Configurações
- Sobre

Ao selecionar "Jogar", exibir os jogos disponíveis em cartões com ícone, nome e descrição.

Ao selecionar um jogo, mostrar:

- Iniciar nova partida.
- Continuar partida salva, quando existir.
- Escolher dificuldade.
- Consultar recordes do jogo.
- Consultar controles.
- Voltar.

Os menus devem funcionar integralmente pelo teclado físico.

O touchscreen poderá complementar a navegação, mas não será obrigatório para jogar.

## 5. Controles gerais

Definir mapeamento padrão:

| Tecla | Função |
|---|---|
| Setas direcionais | Navegação e movimentação |
| Enter | Confirmar |
| Esc | Voltar ou pausar |
| Espaço | Ação principal |
| P | Pausar ou continuar |
| R | Reiniciar, mediante confirmação |
| M | Ativar/desativar som |

Permitir personalização das teclas nas configurações.

Implementar:

- Detecção de tecla pressionada.
- Detecção de tecla liberada, quando suportada.
- Repetição controlada para teclas mantidas pressionadas.
- Tratamento de múltiplas teclas simultâneas.
- Debounce.
- Evitar movimentos duplicados ou acidentais.
- Baixa latência de entrada.

Verificar quais eventos o teclado disponibiliza em cada modo de operação e selecionar o modo mais adequado aos jogos.

Não inventar suporte a eventos que o firmware do teclado não disponibilize. Quando necessário, implementar alternativas documentadas.

## 6. Jogo 1 — BLOCK DROP (Tetris)

Implementar uma versão fiel às mecânicas tradicionais de Tetris, com funcionalidades modernas opcionais.

### Campo de jogo

- Tabuleiro padrão de 10 colunas por 20 linhas visíveis.
- Linhas ocultas de nascimento das peças.
- Tabuleiro centralizado ou levemente deslocado.
- Área lateral mostrando pontuação, nível, linhas eliminadas, próxima peça e peça reservada.
- Bordas bem definidas.
- Grade opcional.
- Blocos com aparência colorida e estilo retrô.

Cada uma das sete peças tradicionais deve ter uma cor própria:

- I: ciano.
- O: amarelo.
- T: roxo.
- S: verde.
- Z: vermelho.
- J: azul.
- L: laranja.

### Mecânicas

Implementar:

- Sete tetraminós clássicos.
- Rotação correta.
- Sistema de rotação SRS (Super Rotation System), incluindo wall kicks.
- Movimentação lateral.
- Queda automática.
- Soft drop.
- Hard drop.
- Sistema de hold.
- Pré-visualização das próximas peças.
- Ghost piece mostrando a posição de aterrissagem.
- Geração equilibrada por sistema 7-bag.
- Detecção correta de colisões.
- Bloqueio e fixação das peças.
- Eliminação simultânea de até quatro linhas.
- Pontuação por linhas eliminadas.
- Pontuação por soft drop e hard drop.
- Combos.
- Back-to-back.
- T-Spin, com detecção e pontuação apropriadas.
- Progressão de níveis.
- Aumento de velocidade.
- Lock delay.
- Game over.
- Reinício de partida.

Garantir que a ordem de geração das peças não produza sequências injustas decorrentes de sorteio ingênuo.

### Dificuldades

Disponibilizar:

- Iniciante.
- Normal.
- Difícil.
- Expert.

Cada dificuldade deve influenciar a velocidade inicial e a progressão, mantendo a jogabilidade consistente.

### Modos de jogo

- Marathon: jogar até perder.
- Sprint: eliminar 40 linhas no menor tempo possível.
- Ultra: pontuar o máximo possível em 2 minutos.
- Zen: modo casual, sem aumento automático de velocidade.

### Controles

- Esquerda/direita: mover.
- Baixo: acelerar queda.
- Cima ou X: girar no sentido horário.
- Z: girar no sentido anti-horário.
- Espaço: hard drop.
- C: reservar/trocar peça.
- P ou Esc: pausa.

### Recursos visuais

- Animação de eliminação de linhas.
- Destaque em eliminações múltiplas.
- Efeito especial para Tetris (quatro linhas).
- Feedback para combos e T-Spins.
- Indicadores de aumento de nível.
- Animação de game over.

## 7. Jogo 2 — SNAKE

Criar um Snake colorido, responsivo e com múltiplas modalidades.

### Campo de jogo

Usar uma grade adaptada à tela 1280×720.

A cobra deve ter:

- Cabeça visualmente diferenciada.
- Corpo colorido.
- Movimentação em quatro direções.
- Animação suave, sem comprometer a lógica baseada em células.
- Direção atual claramente identificável.

### Modos de mapa

**Modo clássico com paredes**

As extremidades são obstáculos. Colidir com uma parede encerra a partida.

**Modo Wrap Around**

A cobra pode ultrapassar qualquer extremidade e reaparecer no lado oposto, preservando a direção.

**Modo Obstáculos**

Adicionar obstáculos fixos ou configuráveis pelo mapa.

**Modo Labirinto**

Criar mapas diferentes com corredores e paredes internas.

Os obstáculos nunca devem tornar impossível alcançar os alimentos.

### Dificuldades

- Fácil.
- Normal.
- Difícil.
- Insano.

A dificuldade deve alterar a velocidade inicial, aceleração e quantidade de obstáculos, conforme o modo.

### Tipos de comida

Criar alimentos visualmente distintos, com cores e efeitos diferentes:

| Cor | Tipo | Efeito |
|---|---|---|
| Vermelho | Normal | +1 segmento e 10 pontos |
| Amarelo | Especial | +3 segmentos e 30 pontos |
| Roxo | Raro | +5 segmentos e 75 pontos |
| Azul | Congelante | Reduz temporariamente a velocidade |
| Verde | Bônus | Multiplica temporariamente a pontuação |
| Dourado | Super bônus | +10 segmentos e 150 pontos |

Os valores devem ser configuráveis.

### Tempo de disponibilidade

- Comida normal: permanece até ser coletada.
- Comida especial: desaparece após 10 segundos.
- Comida rara: desaparece após 7 segundos.
- Comida congelante: desaparece após 8 segundos.
- Comida bônus: desaparece após 8 segundos.
- Super bônus: desaparece após 5 segundos.

Exibir um indicador visual de tempo restante para alimentos temporários.

A geração de alimentos deve respeitar:

- Células livres.
- Posição da cobra.
- Obstáculos.
- Limites do campo.
- Distribuição aleatória equilibrada.

Não gerar alimentos sobre o corpo da cobra ou dentro de paredes.

### Mecânicas

- Impedir reversão direta de 180 graus.
- Detectar colisão com o próprio corpo.
- Detectar colisão com obstáculos.
- Implementar corretamente o wrap around.
- Aumentar progressivamente a velocidade.
- Registrar pontuação e tempo de sobrevivência.
- Adicionar efeitos visuais ao coletar alimentos.
- Permitir pausa e retomada.
- Permitir salvar e continuar partidas.

### Modos extras

- Clássico.
- Sobrevivência.
- Desafio cronometrado.
- Labirinto.

## 8. Jogo 3 — RETRO RACER

Criar um jogo de corrida 2D com visão superior, inspirado nos clássicos jogos de corrida de consoles portáteis, minigames e arcades.

Não usar gráficos 3D.

O visual deve utilizar carros coloridos, pistas, movimento contínuo do cenário e sensação de velocidade.

### Campo de jogo

A pista deve ocupar a região central da tela.

Mostrar:

- Pista vertical com três ou mais faixas.
- Carro controlado pelo jogador.
- Carros adversários.
- Faixas de trânsito.
- Acostamentos.
- Vegetação e elementos laterais.
- Indicador de velocidade.
- Pontuação.
- Distância percorrida.
- Vidas ou integridade do veículo.

Usar sprites 2D ou elementos gráficos desenhados programaticamente.

Cada veículo deve ter cor facilmente identificável.

### Mecânicas

- Direção lateral.
- Aceleração.
- Frenagem.
- Controle progressivo de velocidade.
- Movimento do cenário.
- Tráfego com velocidades diferentes.
- Ultrapassagens.
- Detecção de colisões.
- Perda de vida ou integridade por acidente.
- Aumento gradual da dificuldade.
- Progressão por distância.
- Pontuação por distância e ultrapassagens.
- Reinício após game over.

### Veículos

Disponibilizar pelo menos cinco modelos de carros com diferentes cores e características:

- Vermelho: equilibrado.
- Azul: maior velocidade.
- Verde: melhor controle.
- Amarelo: maior aceleração.
- Roxo: maior resistência.

As diferenças devem afetar realmente a jogabilidade.

### Modos

**Arcade**

Sobreviver o máximo possível e alcançar a maior pontuação.

**Contra o relógio**

Percorrer determinada distância antes que o tempo termine.

**Trânsito intenso**

Desviar de veículos em uma pista progressivamente mais movimentada.

**Endless Road**

Percorrer a maior distância possível.

### Controles

- Esquerda/direita: direção.
- Cima: acelerar.
- Baixo: frear.
- Espaço: turbo, quando disponível.
- P ou Esc: pausa.

Permitir combinação simultânea de direção e aceleração.

### Recursos adicionais

- Efeitos visuais de colisões.
- Efeito de turbo.
- Diferentes cores de pista e cenário.
- Alternância entre cenário diurno e noturno.
- Animação de ultrapassagens.
- Sons de motor e colisão.

Manter o jogo totalmente 2D e otimizado para o ESP32-P4.

## 9. Sistema de recordes

Criar um gerenciador de recordes independente dos jogos.

Cada jogo deve poder registrar múltiplos recordes, separados por:

- Nome do jogador.
- Modo de jogo.
- Dificuldade.
- Pontuação.
- Nível atingido, quando aplicável.
- Tempo ou distância, quando aplicável.
- Data da partida, quando disponível.

Implementar TOP 10 por jogo, modalidade e dificuldade.

O nome do jogador deve ser informado pelo teclado físico.

Não armazenar somente a maior pontuação geral.

Exibir uma tela de recordes organizada por abas ou categorias.

Permitir apagar recordes com confirmação.

Os dados devem permanecer após reinicialização ou desligamento do dispositivo.

## 10. Sistema de savegame

Implementar salvamento persistente.

Cada jogo deve ser capaz de serializar e restaurar seu estado completo.

Incluir, conforme o jogo:

- Tabuleiro ou mapa.
- Posicionamento dos objetos.
- Pontuação.
- Nível e dificuldade.
- Tempo decorrido.
- Estado dos alimentos e efeitos ativos.
- Estado dos veículos.
- Progressão.
- Estado do gerador de peças ou elementos aleatórios, quando necessário.

O carregamento deve restaurar uma partida válida, sem reiniciar arbitrariamente a progressão.

Permitir:

- Salvar manualmente.
- Continuar partida.
- Reiniciar partida.
- Excluir savegame.
- Salvamento automático em pontos apropriados.

Não gravar a Flash a cada frame.

Utilizar estratégia de escrita que minimize desgaste e risco de corrupção.

Considerar NVS, LittleFS, FATFS e microSD conforme os requisitos reais de cada tipo de dado.

O sistema deve funcionar sem cartão microSD.

Implementar versionamento do formato de save, validação de integridade e recuperação segura quando um save estiver corrompido.

## 11. Sistema de áudio

Implementar efeitos sonoros usando o alto-falante integrado.

Cada jogo deve possuir sons apropriados.

Block Drop:

- Movimento.
- Rotação.
- Queda.
- Eliminação de linha.
- Combo.
- Game over.

Snake:

- Alimentação normal.
- Alimentação especial.
- Bônus.
- Colisão.
- Game over.

Retro Racer:

- Motor.
- Aceleração.
- Turbo.
- Ultrapassagem.
- Colisão.
- Game over.

Criar uma trilha simples em estilo chiptune para o menu, sem utilizar músicas protegidas.

Configurações:

- Volume geral.
- Volume da música.
- Volume dos efeitos.
- Mudo.

A reprodução de áudio não pode bloquear a atualização do jogo.

## 12. Configurações gerais

Disponibilizar:

- Brilho da tela.
- Volume.
- Música ligada/desligada.
- Efeitos sonoros ligados/desligados.
- Mapeamento de controles.
- Nome padrão do jogador.
- Tema visual.
- Mostrar FPS, opcional.
- Restaurar configurações padrão.

Salvar as preferências permanentemente.

## 13. Desempenho

Meta de renderização: 60 FPS, quando viável.

A lógica dos jogos deve utilizar tempo fixo ou delta time adequado para manter comportamento consistente.

Prioridades:

- Baixa latência do teclado.
- Animações fluidas.
- Ausência de tearing perceptível.
- Ausência de flickering.
- Uso eficiente de DMA, buffers e PSRAM.
- Uso racional da CPU e memória.
- Evitar alocações dinâmicas frequentes durante as partidas.
- Evitar redesenhar toda a tela quando não for necessário.
- Inicialização confiável dos periféricos.

Medir o desempenho real e documentar eventuais limitações.

Não alegar 60 FPS sem medição.

## 14. Organização sugerida do projeto

```text
Tab5-Retro-Arcade/
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
├── README.md
├── main/
│   └── main.cpp
├── components/
│   ├── arcade_core/
│   ├── display_manager/
│   ├── keyboard_manager/
│   ├── audio_manager/
│   ├── game_manager/
│   ├── storage_manager/
│   └── ui_manager/
├── games/
│   ├── block_drop/
│   ├── snake/
│   └── retro_racer/
├── assets/
│   ├── fonts/
│   ├── sprites/
│   └── sounds/
├── tests/
├── tools/
└── docs/
```

Ajustar a estrutura às exigências reais do CMake e do ESP-IDF.

## 15. Procedimento de desenvolvimento

Antes de escrever o código:

1. Inspecione o repositório existente.
2. Identifique o framework, versões e dependências.
3. Pesquise os exemplos oficiais do Tab5 e Tab5 Keyboard.
4. Verifique como configurar display, teclado, áudio e armazenamento.
5. Identifique as limitações do M5Launcher.
6. Defina a arquitetura e os contratos dos módulos.
7. Crie um plano incremental de implementação.

Depois, execute as seguintes etapas:

**Etapa 1 — Plataforma**

Inicialização do hardware, tela, teclado, áudio e estrutura básica.

**Etapa 2 — Menu**

Interface principal com navegação funcional pelo teclado.

**Etapa 3 — Block Drop**

Implementação completa, incluindo mecânicas, pontuação, níveis e modos.

**Etapa 4 — Snake**

Implementação completa, incluindo diferentes mapas, alimentos e dificuldades.

**Etapa 5 — Retro Racer**

Implementação completa, incluindo tráfego, colisões, veículos e modos.

**Etapa 6 — Persistência**

Savegames, recordes e configurações.

**Etapa 7 — Polimento**

Efeitos visuais, áudio, otimizações e acabamento.

**Etapa 8 — Empacotamento**

Compilação final e criação dos arquivos adequados para o M5Launcher e gravação direta.

Cada etapa deve produzir código funcional e compilável.

Não implemente os três jogos de maneira superficial apenas para declarar o projeto concluído.

## 16. Testes e validação

Criar testes automatizados para a lógica independente do hardware.

Testar especialmente:

Block Drop:
- Rotação de peças.
- Wall kicks.
- Colisões.
- Eliminação de linhas.
- Pontuação.
- Geração de peças.
- T-Spins.
- Save e load.

Snake:
- Colisão com paredes.
- Wrap around.
- Colisão com o corpo.
- Geração de alimentos.
- Expiração de alimentos.
- Crescimento.
- Efeitos temporários.
- Save e load.

Retro Racer:
- Movimento.
- Colisões.
- Progressão de velocidade.
- Geração de tráfego.
- Pontuação.
- Save e load.

Sistema:
- Entrada de teclado.
- Persistência.
- Integridade de arquivos.
- Gerenciamento de memória.
- Troca entre jogos.
- Retorno seguro ao menu.

Executar compilação real.

Corrigir erros encontrados antes de avançar para a etapa seguinte.

Se o hardware físico não estiver conectado, diferenciar claramente aquilo que foi validado por testes de software daquilo que ainda depende de testes no Tab5.

## 17. Documentação e entrega

Fornecer:

- Código-fonte completo.
- README detalhado.
- Instruções de compilação.
- Instruções de gravação.
- Instruções de instalação pelo M5Launcher.
- Descrição da arquitetura.
- Lista de controles.
- Documentação para adicionar novos jogos.
- Resultado dos testes realizados.
- Limitações conhecidas.
- Arquivos BIN gerados, quando o ambiente permitir.

## 18. Requisitos obrigatórios de qualidade

- Não usar funções vazias como implementação definitiva.
- Não apresentar pseudocódigo como código concluído.
- Não substituir funcionalidades solicitadas por simples telas demonstrativas.
- Não eliminar silenciosamente funcionalidades difíceis.
- Não modificar código funcional de outros módulos sem necessidade.
- Evitar dependências desnecessárias.
- Não utilizar assets proprietários sem licença apropriada.
- Manter a separação entre lógica dos jogos e camada de hardware.
- Permitir expansão com novos jogos.
- Manter toda a interface em português brasileiro.

O projeto deverá utilizar licença MIT para o código original, respeitando as licenças das bibliotecas e componentes de terceiros.

## 19. Critérios de aceitação

Considerar o projeto concluído somente quando:

1. O aplicativo inicializar corretamente no Tab5.
2. O menu responder ao teclado físico.
3. Os três jogos estiverem jogáveis do início ao fim.
4. As regras, colisões e pontuações funcionarem corretamente.
5. Existirem diferentes dificuldades e modos de jogo.
6. O sistema de recordes funcionar.
7. Os savegames forem persistentes.
8. Os efeitos sonoros funcionarem.
9. A interface estiver adequada à orientação paisagem.
10. A compilação estiver limpa e a instalação documentada.

**Instrução final:** implemente este projeto de forma incremental, com código real, modular, testável e pronto para execução no hardware. Priorize a qualidade e a fidelidade das mecânicas dos jogos, não apenas a aparência. Quando encontrar incompatibilidades ou limitações de hardware, investigue e apresente soluções tecnicamente justificadas, mantendo as funcionalidades especificadas sempre que viável.