# Arquitetura e contratos

Nome da aplicação: **Tab5 Retro Arcade**. A pasta continua sendo `Pipoca5`.

## Pesquisa antes da implementação

- O exemplo oficial do teclado, commit `5ed3b7bf6d9eba2a549e23d7cf70dbf71bd83a8f`,
  fornece componentes ESP-IDF MIT para Tab5 e teclado. Foram incorporados como
  dependências locais, preservando os avisos originais.
- `M5Unit-KEYBOARD`, commit `b58d024f52cc1b1fa4f189a857ef64468369409a`, confirma:
  no modo Normal, registro 0x20 entrega bit 7 pressionada/liberada, bits 6–4
  linha, bits 3–0 coluna. A matriz é 5×14. Diferente do modo Character, isso
  permite manter estado simultâneo e gerar repetição controlada no aplicativo.
- Teclado: I²C 0x6D, SDA0, SCL1, INT50. Leitura por polling em barramento
  dedicado I2C1; nenhum protocolo USB é empregado.
- O BSP detecta ILI9881/GT911, ST7123 ou ST7121. A placa deste projeto ainda
  precisa confirmar sua variante no log de inicialização ou etiqueta traseira.
- Display físico 720×1280, espaço lógico 1280×720 com rotação PPA na criação,
  buffers duplos e caminho DSI sem tearing. Nunca mudar rotação em execução.
- Codec ES8388, I²C 0x10 no barramento SYS do BSP. I²S: MCLK30, BCLK27,
  LRCK29, DATA26; amplificador pelo expansor 0x43, bit 1.
- Launcher: exportar app BIN separado. A imagem completa de USB inclui
  bootloader/tabela/app e não é um arquivo de atualização OTA. A partição de
  1600 KiB encontrada no projeto anterior é um limite conservador de exportação,
  não prova do tamanho de uma futura partição Pipoca5. Sem microSD, o NVS existente é usado
  com namespace exclusivo; nenhuma partição é apagada ou formatada.

Fontes primárias:
[demo teclado](https://github.com/m5stack/M5Tab5-Keyboard-UserDemo),
[M5Unit-KEYBOARD](https://github.com/m5stack/M5Unit-KEYBOARD),
[Tab5](https://docs.m5stack.com/en/core/Tab5),
[Launcher](https://github.com/bmorcelli/Launcher),
[partições](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32p4/api-guides/partition-tables.html).

## Contratos

- **Core**: relógio fixo de simulação, estado de navegação, troca segura de
  jogos, pausa, captura de nome e ações gerais.
- **Display/UI**: LVGL para menus e canvas desenhado com primitivas 2D. O
  desenho não altera a lógica. Taxa de renderização e FPS são medidos à parte.
- **Input**: eventos físicos (tecla, pressão/liberação), estado simultâneo,
  rejeição de duplicatas e repetição de navegação. Regras recebem ações lógicas.
- **Audio**: mistura PCM de música original e efeitos em tarefa separada,
  com fila não bloqueante, volumes distintos e mute.
- **GameManager**: registro de módulos com metadados, fábrica, opções e contrato
  `init/start/update/render/pause/resume/save/load/reset/cleanup`.
- **Games**: modelo independente de ESP-IDF, configuração, renderer por jogo.
  Estado finito, RNG explícito e persistido. Sem alocação por frame.
- **Storage**: envelope versionado com CRC32, payload validado antes de aplicar,
  duas gerações em microSD ou NVS para recuperar última gravação íntegra. Configurações,
  save por jogo e TOP10 por jogo/modo/dificuldade, sem escrita por frame.

Regras e persistência são testadas no computador com ASan/UBSan. A aplicação
roda offline. microSD é o armazenamento preferencial; sem cartão tenta NVS.
Sem ambos, jogos funcionam em RAM e falhas de persistência são informadas.

## Implementação de persistência e desenho

O journal usa envelope little-endian de 24 bytes (magic, schema, tipo,
geração, tamanho, CRC), alternando sufixos a/b. O CRC cobre cabeçalho e
payload; cada modelo valida campos antes de aplicar o snapshot. Gravações
ocorrem na pausa, save/saída explícitos, configuração e registro de resultado.
O layout standalone reserva 128 KiB NVS; OTA utiliza o NVS já instalado,
podendo falhar por falta de espaço em layouts menores. Falhas não apagam dados.

Backup complementar SDMMC: CLK43, CMD44, DAT0–3 em 39–42, conforme
[PinMap Tab5](https://docs.m5stack.com/en/core/Tab5). Montagem FAT sem formatação,
gravação temporária/flush/rename para nome livre; backups anteriores preservados.
A montagem permanece ativa e é compartilhada com o salvamento principal.
Não há importação de backup nesta versão.

Core acumula tempo e usa passo de 10 ms em tarefa independente do desenho. LVGL tem
refresh de 16 ms e FreeRTOS 1 kHz, necessário para polling de 8 ms não virar
delay zero. Block Drop filtra relógios invisíveis na decisão de redesenho;
Snake/Racer atualizam continuamente. O indicador conta LV_EVENT_RENDER_READY,
não callbacks de timer. Medição física segue pendente.

## Armazenamento em arquivos e desenho agrupado

`StorageBackend` seleciona `FileBackend` no cartão ou `NvsBackend` no boot.
`FileBackend` é portátil, limita tamanho/chaves, grava temporário com fsync
e renomeia o slot antigo; o journal preserva o outro slot CRC. Os jogos
continuam usando o mesmo contrato e IDs. Consulte `SALVAMENTO.md`.

Canvas oferece máscaras de sprites e fundo quadriculado com implementação
portátil de referência. `LvglCanvas` usa 12 máscaras A8 limitadas a 64×72
pixels e um fundo RGB565 de 768×528 (811008 bytes), criados sob demanda e
reutilizados. Falha de alocação usa o desenho convencional. Descritores e
pixels têm vida estática para permanecerem válidos durante tarefas LVGL.

Core invalida apenas o campo dos jogos Snake/Space Invaders/Space Impact
entre atualizações completas a cada 100 ms; entrada e fim de jogo forçam
redesenho completo. Regras e saves são preservados. O BSP já usa PPA para a rotação de paisagem. O Kconfig do outro port LVGL
não controla esse caminho e sua opção adicional foi removida.
A comparação nativa de pixels e contagem de submissões não mede FPS físico.

## Entrada e simulação independentes do renderer

Produção usa `arcade_rules` no core 0 (prioridade 5, stack 8192 bytes), que
chama `poll_game` a cada 4 ms e avança regras em passos de 10 ms. O timer
LVGL, no core 1, continua cuidando de menus, invalidação, saves e apresentação.
O polling do driver A164 foi ajustado de 8 para 4 ms, mantendo o protocolo
Normal e sem habilitar ISR ou inventar eventos.

`Application` protege entrada/estado com mutex recursivo. A thread de regras
não chama LVGL. Pressões de pausa, retorno, reinício ou mudo são preservadas
para a UI, congelando o avanço enquanto aguardam processamento. Pressão/
liberação de uma direção entre leituras não é descartada: `press` mantém
um passo de movimento mesmo quando `held` já voltou a zero.

`ModelGame<Model, Derived>::render_copy` copia o modelo para uma instância
pré-alocada, usada somente pela thread de UI. A trava dura apenas a cópia;
LVGL pode rasterizar e esperar PPA/DSI sem bloquear a tarefa de regras. Isso
é necessário porque o LVGL desta build pode despachar desenho síncrono
durante as chamadas de Canvas. Os layouts de saves não foram alterados.

O indicador distingue renders concluídos e passos de simulação por segundo.
A configuração de 100 Hz das regras não promete 100 FPS nem latência física.

A montagem SD usa slot 0 e controle de potência LDO 4, conforme referência
M5Stack. Backend sem meio pode tentar novamente no preparo de uma escrita;
a seleção acontece antes da leitura das gerações pelo Journal. Backend
ativo permanece fixo durante a sessão. Erros de montagem e arquivos são
propagados à UI pela interface `Backend::error()`.

Nomes de registros e temporários são produzidos por `file_names.h`, sob
o limite FAT 8.3 da configuração atual. `chave.tmp` é sincronizado e
renomeado para `chave.bin`; não se concatena `.tmp` a um nome que já tem
extensão. O formato dos saves finais e do envelope CRC permanece igual.

Frogger/Pac-Man usam IDs 6/7, sem alterar formatos anteriores. Pac-Man agrupa
pontos em até 23 faixas A8; Canvas oferece `dot_row` com referência portátil
e cache LVGL de até 32 faixas (até 128 KiB). Buffers/descritores têm vida
estática e só a UI os modifica. Testes verificam atualização e reutilização.

Bomberman e Sokoban usam IDs 8/9, preservando saves anteriores. O Sokoban
usa histórico circular de 512 deltas de jogada, incluído no snapshot, com
validação por reprodução inversa antes de restaurar. A UI filtra alterações
invisíveis do relógio de repetição para evitar redesenho contínuo do puzzle.
O gerenciador de recordes diferencia sua métrica (movimentos/empurrões) de
distância física. Solver/gerador de fases roda apenas como ferramenta de
autoria no computador, sem afetar a tarefa de regras do dispositivo.

Asteroids adiciona o ID 10 ao registro, com estado fixo e simulação em
subpassos de 5 ms. Canvas oferece linhas com fallback nativo e implementação
LVGL direta para os polígonos. Chaves de ranking usam um caractere base36
para o ID, mantendo os IDs 0–9 e o limite FAT 8.3 ao acrescentar A (10).
Sokoban tem 20 mapas por pacote; os quatro primeiros de cada pacote mantêm
compatibilidade com os saves anteriores.
