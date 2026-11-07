# Verificação de software

Ambiente: macOS arm64, Apple Clang/C++17, ESP-IDF **5.4.4**, toolchain
`riscv32-esp-elf` GCC **14.2.0**, LVGL **9.5.0**. Data: 08/10/2026.

`python3 tests/run_tests.py` compila código de produção com
`-Wall -Wextra -Werror -fsanitize=address,undefined` e executa seis programas.
O LVGL usa configuração nativa e framebuffer RGB565 1280×720. Apenas APIs de
periféricos físicos são substituídas no teste integrado.

| Executável | Cobertura | Resultado |
|---|---|---|
| input | Matriz oficial, pressão/liberação, duplicatas, simultaneidade, repetição e wrap do relógio | Passou |
| block | 7-bag, sete peças/ciclos de rotação, wall kick SRS, clear simultâneo/placar, hold, T-Spin/mini, B2B/combo, lock delay/reset cap, Sprint/Ultra/Zen, validação e save determinístico | Passou |
| snake | Reversão, crescimento/pontos, seis alimentos/efeitos, wrap/parede, geração segura em quatro mapas, expiração, cauda que sai versus crescimento, timer, multiplicador, save determinístico | Passou |
| racer | Direção+aceleração, freio, turbo, colisão/imunidade, ultrapassagem/placar, veículos distintos, tráfego com rota livre na entrada, timer/meta, checkpoint Endless, save/float inválido | Passou |
| storage | CRC/schema/tipo, recuperação da geração íntegra, escrita falha, preferências inválidas, TOP10/categorias, ordenação de Sprint e exclusão | Passou |
| application | Menus LVGL reais, iniciar/dropar, pausa/autosave/P, relógio pausado, continuar, cancelar reinício, nome/TOP10, exclusão do save, direção+aceleração, falha de save preservando RAM, troca de jogos e construção das telas | Passou |

Não foram reportados erros ASan/UBSan nos executáveis. O renderer LVGL nativo
gera capturas PPM em `tests/build`; versões PNG revisadas estão em
`docs/previews`. Elas não representam medição nem prova de compatibilidade
do LCD físico.

## Build e pacotes

`idf.py build` passou para ESP32-P4. A configuração de entrega é criada a
partir de `sdkconfig.defaults`, com otimização por tamanho. Warnings de formato
de log e API touch obsoleta nos componentes locais foram corrigidos de forma
pontual; o build final não desabilita `-Werror=format`.

`tools/package.py` valida chip ID, cabeçalho ESP, tamanho conservador Launcher,
offsets sem sobreposição e imagem pelo `esptool image_info`. Copia o app sem
alterar bytes e monta a imagem standalone a partir dos artefatos efetivos.
`newversion/manifest.json`, `image-info.txt` e `SHA256SUMS.txt` registram a entrega.

Nenhum flash, monitor, erase ou acesso serial foi executado. Não se considera
concluída a aceitação física. [LIMITACOES](LIMITACOES.md) contém o checklist.

Os testes verificam cenários relevantes, não são prova exaustiva de todas as
sequências de partidas. A última fase é jogar no aparelho e medir o desempenho.

A inspeção do backup no computador também foi verificada com registros
sintéticos: CRC válido, conteúdo corrompido e arquivo truncado. Isso verifica
a ferramenta de leitura; a escrita física no cartão permanece pendente.

## Space Invaders

Suíte nativa adicional em `tests/invaders_test.cpp`: limites de movimento,
pausa, disparos, colisões com invasores/escudos/nave bônus/jogador, imunidade,
rebote e descida da formação, avanço de ondas, vitória na campanha e no
modo cronometrado, derrota por invasão ou vidas, tiro automático, saves
inválidos e restauração determinística. Integração com LVGL verifica seleção
do quarto jogo, movimento/tiro simultâneos, desenho e autosave em `save3`.

Compilação ESP32-P4 e testes ASan/UBSan executados no computador.
A captura `docs/previews/space-invaders.png` vem do renderer LVGL real.
Controles, áudio e desempenho continuam pendentes de validação física.

## Space Impact e controles espaciais

`tests/impact_test.cpp` verifica movimento diagonal normalizado e limites,
pausa, disparos, colisões, três níveis de arma, itens, imunidade, derrota,
bomba consumida uma vez por pressão, progressão, três padrões de chefes,
modos de jogo, capacidade de entidades e save/load determinístico validado.
Integração LVGL verifica o quinto jogo, movimento/tiro simultâneos com X,
bomba Z, renderização de chefe e save `save4`, incluindo decodificação.
Space Invaders passa a usar X no teste de integração.

Os oito executáveis nativos passaram com ASan/UBSan e a compilação real
ESP32-P4 gerou os pacotes OTA e USB. As imagens de prévia são do LVGL
executado no computador, sem comprovar desempenho ou periféricos no Tab5.

## microSD, interface e fluidez

Nove executáveis nativos passaram com ASan/UBSan. O novo `file_storage`
usa arquivos temporários reais e os cinco modelos: salvar/pausar/reabrir/
carregar/retomar, TOP10 por jogo, preferências, CRC/truncamento, temporário
incompleto, falha de substituição, exclusão, limites de arquivo/chave e
armazenamento removido, sem dependência de NVS.

Integração LVGL compara pixel a pixel o fundo RGB565 e máscaras A8 com o
desenho de referência, tanto no cache frio quanto no reutilizado. Navegação
e telas passaram com botões compactos, fontes explícitas, textos que podem
crescer e descrições separadas nos cartões. Capturas em `docs/previews`.

| Cena inicial | Submissões anteriores | Submissões agrupadas |
|---|---:|---:|
| Snake | 377 | 25 |
| Space Invaders | 1634 | 235 |
| Space Impact | 131 | 131 |

O Space Impact recebe invalidação parcial de campo e, como todos os jogos,
rotação PPA habilitada. As contagens são do Canvas no computador, sem
medir tempo de quadro ou FPS físico. A comparação não representa cenas
com toda quantidade possível de entidades.

Build ESP32-P4 e pacote OTA/USB concluídos. Pendentes: gravação/leitura
física do cartão, retomada após desligar no Tab5, estabilidade do PPA e
medição do contador de renders em cada jogo. Nenhuma serial foi aberta.

## Responsividade independente da tela

Integração nativa ativa o modo independente e deixa de chamar UI/render por
120 ms enquanto fornece amostras de 4 ms: a nave continua avançando e o
relógio do modelo progride. Verifica liberação sem movimento residual, toque
curto de direção, tiro X, bomba Z consumida uma vez e pausa que congela o
modelo antes de abrir menu ou salvar. Testes extras verificam curva curta
do Snake enfileirada sem renderização e movimento/tiro curto do Space
Invaders sem repetição depois de soltar.

O teste dos cinco jogos verifica que atualizar o modelo não altera a cópia
reservada ao renderer. Saves mantêm o mesmo layout. A suíte passa com
ASan/UBSan e o firmware é compilado para ESP32-P4. Testes no computador não
medem o tempo real do escalonador, do teclado I²C ou do painel.

O caminho de produção usa o port LVGL do componente M5Stack, que já ativava
PPA por `dsi.flags.use_ppa`. A opção extra `CONFIG_LVGL_PORT_ENABLE_PPA`, do
port Espressif, não alterava esse caminho; sua remoção não desativa o PPA
do BSP. A hipótese anterior de ganho por essa opção foi corrigida.

## Correção da montagem física SD

Dez executáveis nativos. `sd_mount` compila o `sd_card.cpp` de produção com
APIs SDMMC/LDO simuladas e verifica slot 0, LDO 4 associado ao host, GPIOs
do Tab5, ausência de formatação, erro ao alimentar, erro FAT, nova tentativa,
fallback 1 linha/10 MHz por falha de comunicação e reutilização da montagem.

Testes de arquivo verificam mensagens de falha de abertura/substituição.
Teste de journal verifica que preparar um backend recém-disponível ocorre
antes da leitura dos slots, preservando a geração anterior e recuperando-a
se a nova gravação corromper. Os cinco jogos continuam com testes de
salvar/carregar em arquivos reais. Build ESP32-P4 e pacotes OTA/USB passaram.

A correção segue a [inicialização oficial M5Stack](https://github.com/m5stack/M5Tab5-UserDemo/blob/main/platforms/tab5/components/m5stack_tab5/m5stack_tab5.c),
que associa LDO 4 e SDMMC_HOST_SLOT_0. Escrita física após a correção ainda
precisa ser confirmada no cartão do usuário; serial não foi aberta.

## Regressão do erro EINVAL (22) em FatFs real

Onze executáveis nativos. O novo `fatfs` compila `ff.c` diretamente das
fontes ESP-IDF 5.4.x, sem alterar a biblioteca, com `FF_USE_LFN=0`. O disco
é um array RAM de 8 MiB; só ele é formatado no teste. O dispositivo físico
não é acessado. Dependência localizada por IDF_PATH ou ~/esp/esp-idf.

Reproduz `FR_INVALID_NAME` ao criar `save0a.bin.tmp` e `backup-0001.bin`.
Os nomes de produção são compartilhados via `file_names.h`, evitando um
teste com nomes diferentes dos usados pelo firmware. Verifica todos os
pares de saves dos cinco jogos, 160 slots de rankings (5×4×4×2),
configurações e backups nos limites da numeração. Executa criar/escrever/
sincronizar/fechar/substituir/renomear/ler e compara o conteúdo.

Testes POSIX de FileBackend continuam verificando falha de substituição,
geração anterior preservada, truncamento, temporário incompleto e os cinco
modelos. Compilação ESP32-P4 e pacotes OTA/USB atualizados. Correção física
ainda depende de instalar e testar o BIN no Tab5.

## Quebra-blocos

Doze executáveis nativos. `breakout` verifica lançamento e acompanhamento
da plataforma, limites, rebote por ângulo, paredes, faces dos blocos,
resistência/pontuação, geração/coleta de bônus, expansão/lentidão e
expiração, limite de multibola, perda de bola versus vida, treino, avanço
e vitória da campanha, desafio cronometrado, pausa e restauração determinística.
Saves truncados, NaN, resistência inválida e contagem inconsistente são rejeitados.
Cem segundos simulados na velocidade máxima fazem restaurações a cada 250 ms.

Integração LVGL testa seleção do sexto jogo, X e movimento na tarefa de
regras sem atualizar a UI, coleta/representação de bônus, pausa e `save5`.
Teste de arquivos reabre os seis modelos; FatFs real cobre 192 slots de
rankings e pares de saves dos seis jogos, com nomes longos desativados.

A compilação real ESP32-P4 e os testes ASan/UBSan passaram. Preview em
`docs/previews/breakout.png` vem do renderer real no computador. Jogabilidade,
áudio e desempenho físicos do novo jogo seguem pendentes de validação.

## Frogger e Pac-Man

14 executáveis nativos. Frogger verifica fila de saltos, carros, água, troncos
e transporte, bordas, abrigos ocupados, rodada completa, vitória, relógio, modos
e saves determinísticos. Pac-Man verifica mapas conectados e diferentes,
paredes, curva antecipada, interpolação, túnel, pontos/poder, alvos distintos,
combo/captura/dano, frutas, modos e vitória. Ambos fazem 200 s simulados com
restauração periódica de saves.

Integração LVGL seleciona jogos 7/8 pelo menu, aplica pressões curtas na
tarefa de regras sem desenhar, verifica salto/coleta, captura prévias e
valida pausa/save6/save7. FileBackend reabre oito modelos; FatFs real testa
saves e 256 slots de rankings (8×4×4×2), além de backups.

Comparação de pixels inclui faixas A8 de pontos com alteração de conteúdo.
Cena inicial Pac-Man: 192 submissões agrupadas versus 517 de referência.
Frogger: 168. Contagens não medem FPS ou latência física.

Suíte ASan/UBSan e build ESP32-P4 passaram; OTA/USB gerados. Controles, áudio
e desempenho dos novos jogos ainda precisam ser experimentados no Tab5.
Nenhum flash ou acesso serial foi executado.

## Bomberman e Sokoban

16 executáveis nativos. Bomberman cobre movimento/bomba própria, pavio,
paredes, caixas, propagação interrompida, reação em cadeia, inimigo atingido,
dano/vidas, melhorias/limites, áreas ameaçadas, saída/progressão, relógio,
modos e validação/determinismo do save. Há 200 s simulados com bombas e
restaurações periódicas no Treino.

Sokoban executa as 16 soluções originais nos quatro modos: caixa/parede,
empurrar/não puxar, cantos, retorno completo de rota, 512 passos de histórico,
reinício, limites de movimentos e empurrões, avanço e pontuação. Saves com
histórico inválido são rejeitados. Recordes priorizam campanhas concluídas
e menos movimentos, ou empurrões no modo correspondente.

Integração LVGL seleciona jogos 9/10 pelo teclado, coloca bomba com X e
verifica fuga; restaura save8 com chamas ativas. No Sokoban usa setas/Z/X e
restaura save9, incluindo desfazer após carregar. Prévia dos dois renderers
em `docs/previews`. FileBackend reabre os dez modelos; FatFs real cobre
seus pares de saves e 320 slots de rankings (10×4×4×2), com nomes 8.3.

A suíte ASan/UBSan e o build ESP32-P4 passaram. BINs OTA/USB gerados sem
flash ou abertura de serial. Os novos jogos requerem validação física de
controles, áudio, balanceamento e desempenho no Tab5.

## Asteroids e expansão do Sokoban

A suíte atual executa 17 binários nativos com ASan/UBSan. Sokoban verifica
80 mapas únicos nos quatro modos, as 16 fases anteriores sem alterações e
migração de campanhas antigas concluídas para a fase 5. São 20 fases por
pacote, com histórico e limites preservados.

Asteroids verifica rotação, aceleração/inércia/freio, velocidades, bordas,
colisões, disparos, divisão até 36 rochas pequenas, discos voadores, vidas,
hiperespaço, modos, passos determinísticos e rejeição de saves inválidos.
A integração LVGL cobre entrada independente, navegação e save10/restauração.
FileBackend cobre os onze jogos; FatFs real verifica 352 slots de rankings
(11×4×4×2), incluindo `rankA00a.bin`, sem exceder FAT 8.3.
`tests/backup_reader_test.py` valida nomes antigos e novos e CRC dos arquivos
aceitos pelo leitor de backup. As prévias são geradas pelo LVGL no computador;
não medem FPS nem validam controles no dispositivo físico.
