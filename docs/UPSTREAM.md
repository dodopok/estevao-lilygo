# Upstream do firmware

O firmware foi importado do repositório oficial do Bruce na raiz deste
projeto, pois o build do Bruce espera `platformio.ini`, `boards/`, `lib/` e
`src/` no diretório principal. Um submódulo em uma pasta aninhada não atenderia
esse layout sem uma camada adicional de build.

Remote configurado:

```text
upstream https://github.com/BruceDevices/firmware.git
```

Commit usado como baseline da importação:

```text
59e83bfbd8a63a6b67ea23498e15c710a1ed9657
```

## Sincronização

Use o script [`scripts/update-bruce.sh`](../scripts/update-bruce.sh) a partir
da raiz do repositório:

```bash
./scripts/update-bruce.sh
```

Ele exige uma árvore limpa, busca `upstream/main`, mostra os commits novos e
faz um merge no `main`, preservando os commits do Estêvão. O script não faz
push e não executa o build automaticamente. Depois do merge:

```bash
pio run -e lilygo-t-embed-cc1101
git push origin main
```

Para um remote ou branch diferente, use `--remote` e `--branch`. O script
recusa branches diferentes de `main` por padrão; `--allow-non-main` existe
somente para um merge deliberado em outra branch. Revise os conflitos nas áreas
modificadas pelo Estêvão antes de incorporar novos commits do upstream. O
Bruce é distribuído sob AGPL-3.0; mantenha `LICENSE` e `THIRD_PARTY.md` no
repositório.
