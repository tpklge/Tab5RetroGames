# Tab5 Retro Arcade — Pipoca5

**Pequenos jogos, grandes partidas.** Coleção offline de jogos 2D para o
M5Stack Tab5 e o teclado oficial A164. A pasta permanece `Pipoca5`; o firmware
usa **Tab5 Retro Arcade**, conforme [a especificação](docs/REQUISITOS.md).

C++17, ESP-IDF 5.4.4 e LVGL 9.5.0. A compilação real ESP32-P4 e os 17
executáveis de testes nativos passaram. **Boot, teclado, áudio, instalação pelo
Launcher e desempenho real ainda exigem validação no Tab5.** Nenhuma gravação
ou abertura de serial foi executada.

![Menu renderizado pelo LVGL no computador](docs/previews/home.png)

## Jogos

| Jogo | Mecânicas | Modalidades |
|---|---|---|
| Block Drop | Campo 10×20 + 4 linhas ocultas; sete peças; SRS/wall kicks; 7-bag; hold; cinco próximas; ghost; soft/hard drop; lock delay 500 ms/15 resets; combos, back-to-back, T-Spins e níveis | Maratona, Sprint 40 linhas, Ultra 2 minutos, Zen |
| Snake | Células com interpolação; fila de curvas sem reversão; colisões; wrap; obstáculos/labirinto; seis alimentos configuráveis e efeitos temporários | Clássico, Sobrevivência, Desafio 2 minutos, Labirinto |
| Retro Racer | Três faixas; aceleração/freio/direção simultâneos; turbo; tráfego; ultrapassagens; colisões/integridade; cinco veículos distintos; dia/noite | Arcade, Contra o relógio (3 km/60 s), Trânsito intenso, Estrada sem fim |
| Space Invaders | Formação de 50 invasores; escudos destrutíveis; nave bônus; três vidas; tiros contínuos; ondas progressivas e sprites originais | Arcade, Campanha (5 ondas), Desafio 2 minutos, Tiro automático |
| Space Impact | Rolagem lateral; nave em quatro direções; três tipos de inimigo; três níveis de arma; bombas; melhorias; três chefes com fúria e padrões próprios | Campanha (3 setores), Sobrevivência, Desafio 3 minutos, Tiro automático |
| Quebra-blocos | Bola com rebote por ângulo; blocos de até três impactos; multibola (3); expansão e lentidão temporárias; seis desenhos de fase | Campanha (6 fases), Arcade, Desafio 3 minutos, Treino |
| Frogger | Carros, troncos que carregam o sapo, cinco abrigos, relógio por travessia e bônus por rapidez | Campanha (3 rodadas), Arcade, Desafio 3 minutos, Treino |
| Pac-Man | Três labirintos conectados; pontos, poderes, frutas, túnel lateral e quatro comportamentos de inimigos | Clássico (3 labirintos), Arcade, Desafio 3 minutos, Treino |
| Bomberman | Bombas em cruz e reação em cadeia; caixas destrutíveis; melhorias de alcance, capacidade e velocidade; inimigos e saída escondida | Campanha (5 arenas), Arcade, Desafio 3 minutos, Treino |
| Asteroids | Nave com inércia, asteroides que se dividem, bordas contínuas, discos voadores e hiperespaço | Campanha (5 ondas), Arcade, Desafio 3 minutos, Treino |
| Sokoban | 80 fases originais verificadas; 20 por dificuldade; desfazer 512 jogadas; reiniciar fase; contadores e avisos de canto | Campanha, Limite de movimentos, Limite de empurrões, Treino |

![Space Invaders renderizado pelo LVGL no computador](docs/previews/space-invaders.png)

![Space Impact renderizado pelo LVGL no computador](docs/previews/space-impact.png)

![Quebra-blocos renderizado pelo LVGL no computador](docs/previews/breakout.png)

![Frogger renderizado pelo LVGL no computador](docs/previews/frogger.png)

![Pac-Man renderizado pelo LVGL no computador](docs/previews/pacman.png)

![Bomberman renderizado pelo LVGL no computador](docs/previews/bomberman.png)

![Asteroids renderizado pelo LVGL no computador](docs/previews/asteroids.png)

![Sokoban renderizado pelo LVGL no computador](docs/previews/sokoban.png)

Cada jogo tem quatro dificuldades, pausa, reinício confirmado, save completo
e TOP10 por modalidade/dificuldade. Labirinto seleciona seu mapa no Snake;
Zen mantém a velocidade inicial. Os menus oferecem Jogar, Continuar partida,
Recordes, Configurações e Sobre, com teclado e toque complementar.

Configurações: brilho, volumes separados, música, efeitos, mudo, três temas,
nome padrão, remapeamento, grade dos blocos e FPS medido pelo renderer.

## BIN para instalar

Use **[tab5_retro_arcade-ota.bin](newversion/tab5_retro_arcade-ota.bin)** pelo
Launcher/WebUI. É somente a aplicação ESP32-P4. Tamanhos, versão, offsets e
hashes estão em [manifest.json](newversion/manifest.json) e
[SHA256SUMS.txt](newversion/SHA256SUMS.txt).

O pacote separado `tab5_retro_arcade-usb-full.bin` inclui bootloader, tabela
própria e aplicação, com base 0x0. Não é intercambiável com o app OTA.
Veja [instalação e particionamento](docs/INSTALACAO.md).

**Salvamento no microSD:** insira um cartão FAT antes de ligar. O aplicativo
cria `/pipoca5` na raiz e usa essa pasta para partidas dos onze jogos,
recordes e configurações. Não precisa de partição NVS para salvar no cartão.
A tela inicial informa o armazenamento selecionado. Sem microSD utilizável,
tenta o NVS existente, sem apagar dados ou alterar partições. Veja os
[detalhes de salvamento](docs/SALVAMENTO.md).

## Compilar sem instalar

Instale ESP-IDF 5.4.x e ferramentas ESP32-P4. As dependências estão fixadas
em `main/idf_component.yml` e `dependencies.lock`; a primeira obtenção delas
precisa de internet. O firmware funciona offline.

```bash
./tools/build_ota.sh
```

O script executa build e empacotamento, sem flash/monitor. Preserva um ambiente
ESP-IDF já ativo; neste Mac também reconhece `idf5.4_py3.9_env`, evitando o
Python global incompatível. Não requer Arduino ou M5Unified. Após adicionar
arquivos, execute `idf.py reconfigure build`.

Em outra máquina, ative o ambiente Python instalado pelo ESP-IDF e seu
`export.sh` antes de chamar o script. Ele fixa o alvo em `esp32p4`.

Flash 16 MB, PSRAM, DIO/80 MHz, stacks principal/LVGL 16 KiB, FreeRTOS 1 kHz,
refresh LVGL de 16 ms. A simulação tem passo de 10 ms em tarefa própria no core 0; a entrada é
amostrada com intervalo configurado de 4 ms. LVGL roda no core 1 e desenha
uma cópia do estado, sem bloquear regras/entrada durante a rasterização.
São configurações de temporização, não prova de FPS ou latência no aparelho.

## Controles e persistência

Setas navegam, Enter confirma, Esc volta/pausa, P pausa/continua, R pede
reinício e M alterna som. Block Drop: Cima/X gira, Z gira ao contrário,
Espaço derruba, C reserva. Space Invaders: Esquerda/Direita move, X atira (segure para repetir). Space Impact: Setas move, X atira e Z usa bomba. Quebra-blocos: Esquerda/Direita move e X lança. Frogger e Pac-Man usam as setas;
no Frogger, segurar repete saltos; no Pac-Man, a direção fica selecionada.
Bomberman: Setas movem, X coloca bomba. Sokoban: Setas movem/empurram, Z
desfaz, X reinicia a fase. Racer: Cima acelera, Baixo freia, Espaço turbo.
[Lista de controles](controles.md).

Pausa faz autosave; seu menu permite salvar manualmente e voltar salvando.
Continuar restaura estado e RNG. Nomes de até 15 caracteres são digitados no
teclado. Recordes e saves podem ser excluídos com confirmação. Preferências
são persistidas ao ajustar; Flash não é gravada por frame.

O armazenamento é escolhido ao iniciar: microSD primeiro, NVS como alternativa.
Insira o cartão antes de ligar. Se nenhum armazenamento estava disponível,
Salvar tenta montar novamente; um meio já selecionado não é trocado durante
a sessão. Falhas mostram etapa e código do erro.
`Configurações → Backup no microSD` exporta gerações do armazenamento ativo de
configuração, saves e recordes para `/pipoca5/backup.bin`, em FAT/SDMMC.
O cartão permanece montado para saves e backups. Não há formatação nem
importação de arquivos de backup no Tab5
nesta versão. Inspeção no computador:

Backups seguintes recebem sufixos numerados, preservando os anteriores.

```bash
python3 tools/read_backup.py backup.bin --extract ./extraidos
```

## Interface e fluidez

Menus com botões de 50 px e cartões de jogos de 74 px, nomes em 22 px e
descrições em 16 px. Seleção destacada, nomes e descrições separados, lista com rolagem pelo teclado.

O Snake reutiliza o fundo em RGB565; os invasores usam máscaras reutilizáveis.
Os campos animam sem redesenhar todo o painel lateral a cada quadro, e a
rotação para paisagem usa o PPA já implementado pelo BSP do ESP32-P4. No teste da cena inicial, submissões
de desenho passam de 377 para 25 no Snake e de 1634 para 235 no Space Invaders.
São contagens de desenho no computador, **não FPS medido no Tab5**.

A versão seguinte separa controles e simulação do timer LVGL: a tarefa de
regras trabalha mesmo enquanto a tela está renderizando ou esperando DSI.
Pressionar e soltar rapidamente uma direção garante pelo menos um passo,
sem deixar a tecla presa. `Mostrar FPS medido` exibe **Render FPS** e
**Regras Hz** para distinguir atualização da tela e avanço da simulação.
O BSP já usava PPA antes; a opção extra do outro port LVGL foi removida.
O ganho de resposta continua pendente de medição no Tab5.


## Estrutura e testes

```text
main/
  core/              Aplicação e navegação
  display/           Hardware, canvas LVGL e desenho
  input/             Teclado Normal e estado simultâneo
  audio/             Síntese PCM assíncrona original
  game_manager/      Contrato, adaptador e registro
  storage/           microSD principal, NVS alternativo, journal CRC, TOP10 e backup
  ui/                Menus por teclado/toque
  games/
    block_drop/      Configuração, modelo, renderização
    snake/
    retro_racer/
    space_invaders/
    space_impact/
    breakout/
    frogger/
    pacman/
    bomberman/
    sokoban/
    asteroids/
components/          BSP/teclado M5Stack com avisos preservados
tests/               Regras e integração com LVGL real
tools/               Build, pacotes e leitura de backup
docs/                Pesquisa, arquitetura, testes, limitações
newversion/          BINs e manifesto
```

```bash
python3 tests/run_tests.py
```

Requer C++17, Python, CMake, bibliotecas obtidas pelo build IDF e fontes do
ESP-IDF 5.4.x para testar o FatFs real (IDF_PATH, ou ~/esp/esp-idf). Usa
AddressSanitizer/UndefinedBehaviorSanitizer; Core/UI/renderers e LVGL são
reais, apenas periféricos são emulados. As imagens em `docs/previews` são
capturas desse renderer no computador.

Documentação: [arquitetura](docs/ARQUITETURA.md),
[adicionar jogo](docs/ADICIONAR_JOGO.md), [testes](docs/TESTES.md),
[limitações/aceitação física](docs/LIMITACOES.md), [plano](docs/PLANO.md).

Código original sob [MIT](LICENSE); terceiros mantêm suas licenças.
Consulte [THIRD_PARTY](docs/THIRD_PARTY.md).
