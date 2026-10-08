# Licenças e procedência

`main/`, testes e ferramentas do arcade são código original sob MIT (`LICENSE`).
Músicas e efeitos são síntese original; carros, mapas, alimentos e blocos são
primitivas desenhadas no código. Não foram utilizados sprites, gravações ou
implementações GPL do Tab5Meshtastic.

| Dependência | Procedência / licença |
|---|---|
| Componentes locais Tab5 e teclado | [M5Tab5-Keyboard-UserDemo](https://github.com/m5stack/M5Tab5-Keyboard-UserDemo), commit `5ed3b7bf6d9eba2a549e23d7cf70dbf71bd83a8f`; repositório MIT, avisos SPDX preservados nos arquivos |
| Referência de protocolo | [M5Unit-KEYBOARD](https://github.com/m5stack/M5Unit-KEYBOARD), commit `b58d024f52cc1b1fa4f189a857ef64468369409a`; não incorporado como biblioteca |
| ESP-IDF 5.4.4 | Apache-2.0 e licenças dos componentes do SDK; arquivo principal em `licenses/ESP-IDF.txt` |
| LVGL 9.5.0 | MIT, `licenses/lvgl__lvgl-LICENCE.txt`; componentes internos mantêm seus avisos |
| Dependências Espressif | Versões fixadas em `dependencies.lock`; licenças de cada pacote copiadas para `licenses/` |
| Montserrat, fontes internas LVGL | SIL Open Font License; [fonte oficial](https://github.com/JulietaUla/Montserrat), texto em `licenses/Montserrat-OFL.txt` |
| Unscii, fonte pixelada interna LVGL | [Unscii](https://github.com/viznut/unscii); origem `unscii-8.ttf`, ASCII 0x20–0x7F, declarada Public Domain/CC0; aviso original em `licenses/Unscii-README.md`. Não usamos `unscii-16-full`/Unifont GPL |

O BSP possui alguns arquivos com avisos Apache-2.0 além do MIT do repositório;
foram preservados, sem atribuir a licença original do arcade a esses arquivos.
Os componentes de Wi-Fi aparecem por dependência do BSP, mas a aplicação não
inicializa conectividade.

Alterações locais ao BSP: cast explícito dos valores de resolução em um log
para GCC14 e uso da API touch `esp_lcd_touch_get_data`. Drivers e avisos originais
foram mantidos. O teclado oficial não foi reimplementado nem convertido em USB.

Ao redistribuir os BINs, acompanhe `LICENSE`, este documento e `docs/licenses`,
além das licenças dos componentes locais. As bibliotecas gerenciadas conservam
seus avisos nas pastas recuperadas pelo Component Manager.
