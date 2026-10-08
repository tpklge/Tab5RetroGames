# Testes nativos

Execute python3 tests/run_tests.py na raiz. Requer C++17, CMake e as bibliotecas
obtidas pelo build ESP-IDF. Seis executáveis usam ASan/UBSan; a integração utiliza
LVGL, menus, Core e renderers reais com periféricos emulados. Saídas em build/.
Cobertura e resultados: ../docs/TESTES.md. Hardware físico permanece pendente.
