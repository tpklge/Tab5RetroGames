# Arquivos de entrega

Use tab5_retro_arcade-ota.bin para instalar a aplicação pelo Launcher/WebUI.
O arquivo tab5_retro_arcade-usb-full.bin é separado: bootloader + tabela própria
+ aplicação, gravado em 0x0 em instalação standalone. Não é um app OTA.

manifest.json registra formatos, tamanho, offsets e validação física pendente.
SHA256SUMS.txt contém os hashes; image-info.txt registra a inspeção esptool.
Instruções e impacto do layout em ../docs/INSTALACAO.md.

Somente foram gerados arquivos. Não houve flash ou abertura de serial.
