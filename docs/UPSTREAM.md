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

Para atualizar no futuro, revise os conflitos nas áreas modificadas pelo
Estêvão antes de incorporar novos commits do upstream. O Bruce é distribuído
sob AGPL-3.0; mantenha `LICENSE` e `THIRD_PARTY.md` no repositório.
