# Instalação e arquivos BIN

Pacotes em `newversion`, montados pelos offsets de `build/flasher_args.json`.
O exportador valida ESP32-P4, cabeçalho, tamanho e ausência de sobreposição.
Não acessa dispositivos.

## Launcher / OTA

Use `tab5_retro_arcade-ota.bin`, apenas aplicação. O destino é o slot app
escolhido pelo Launcher. **Não fixe 0x30000 ou 0x1a0000 para uma atualização
pelo Launcher.** O app não leva a tabela standalone nem amplia NVS.

A [documentação oficial do Launcher](https://github.com/bmorcelli/Launcher/wiki/Obtaining-binaries-to-launch)
descreve instalação por `SD → arquivo → Install`, ou pelo `WUI`: acessar o
endereço exibido, enviar BIN e iniciar atualização. Menus podem variar por versão.

O projeto Tab5Meshtastic registra um slot anterior de 1600 KiB em 0x1a0000.
O exportador usa **1.638.400 bytes** como teto conservador. Isso não confirma
um novo slot criado para este arcade. Confira espaço no Launcher instalado.

NVS existente `nvs`, namespace `pipoca5`; namespaces de outros projetos são
preservados. Ela precisa estar disponível e ter espaço. Falhas são informadas;
sair sem salvar requer confirmação. Não há erase automático na inicialização.

Standalone reserva 128 KiB de NVS para 48 categorias TOP10, três saves e
configurações com duas gerações. Os 24 KiB do layout anterior podem esgotar.
Manter todas as categorias simultaneamente exige espaço adequado; OTA
sozinha não amplia essa partição. A tabela do seu Tab5 não foi modificada.

## Instalação standalone por USB

Este caminho, se escolhido pelo usuário, **substitui a tabela do Launcher e
dados existentes na área escrita**. `tab5_retro_arcade-usb-full.bin` é imagem
mesclada com base 0x0, preenchimento FF, bootloader e tabela próprios.
Não usar como app OTA.

| Região | Offset | Tamanho |
|---|---|---|
| Bootloader P4 | 0x2000 | Gerado pelo build |
| Tabela | 0x8000 | 0x1000 |
| NVS | 0x9000 | 0x20000 (128 KiB) |
| PHY init | 0x29000 | 0x1000 |
| App factory | 0x30000 | 0x600000 (6 MiB) |

Com IDF ativo, pode usar `idf.py -p PORT flash` no projeto, ou a imagem mesclada:

```bash
python -m esptool --chip esp32p4 --port PORT write_flash \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x0 newversion/tab5_retro_arcade-usb-full.bin
```

São instruções: esses comandos não foram executados. Compatibilidade física
permanece pendente para o LCD presente no aparelho.

Referência: [partições ESP-IDF 5.4/P4](https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32p4/api-guides/partition-tables.html).
