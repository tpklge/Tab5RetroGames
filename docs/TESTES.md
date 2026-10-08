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
