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
  não prova do tamanho de uma futura partição Pipoca5. O NVS existente é usado
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
  duas gerações NVS para recuperar última gravação íntegra. Configurações,
  save por jogo e TOP10 por jogo/modo/dificuldade, sem escrita por frame.

Regras e persistência são testadas no computador com ASan/UBSan. A aplicação
roda offline. microSD é opcional; o funcionamento não depende do cartão.

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
O cartão é desmontado ao terminar. Não há importação nesta versão.

Core acumula tempo e usa passo de 16.667 µs separado do desenho. LVGL tem
refresh de 16 ms e FreeRTOS 1 kHz, necessário para polling de 8 ms não virar
delay zero. Block Drop filtra relógios invisíveis na decisão de redesenho;
Snake/Racer atualizam continuamente. O indicador conta LV_EVENT_RENDER_READY,
não callbacks de timer. Medição física segue pendente.
