<!--
Título: Conventional Commits em inglês, como os commits: `feat(estevao): …`,
`fix: …`.
Escreva para quem vai revisar sem ter acompanhado a conversa. Seção que não se
aplica fica com "Nada." em vez de sumir: assim fica claro que ela foi pensada.
Estes comentários não aparecem no PR.
Este arquivo é do Estêvão, não do Bruce: numa sincronização com o upstream,
mantenha esta versão.
-->

## Por quê

<!-- O problema ou a necessidade, do ponto de vista de quem usa o T-Embed com o
Estêvão. Se é uma sincronização com o Bruce, o intervalo de commits do
upstream. Link para a issue ou a conversa. -->

## O que muda

<!-- O que muda no aparelho: menu do Estêvão, calendário, leituras,
comemorações, Pergunte (texto ou voz), configuração. Diga se toca código do
Bruce fora de `src/modules/estevao/` e `src/core/menu_items/EstevaoMenu.*`.
Lista curta; o diff já mostra o código. -->

## Como foi feito

<!-- O que o diff não explica: a abordagem, o que foi descartado e por quê, os
arquivos que valem ser lidos primeiro. -->

## Riscos

<!--
Uma linha por risco: o que pode dar errado, quem sente, e o que segura. Pense
em:
- o upstream: arquivo do Bruce mudado aqui conflita no próximo
  `scripts/update-bruce.sh`; num merge do upstream, o que ele mexeu nas áreas
  do Estêvão;
- o aparelho gravado: só volta pelo cabo; a configuração em
  `/estevao/config.json` e o cache em `/estevao/cache` (LittleFS) lidos pelo
  firmware novo;
- segredos: a API key do Estêvão, a API key da OpenAI e o token do MCP moram
  no aparelho; nada disso em código, log, tela, captura ou exemplo;
- TLS: certificado validado, raiz que muda;
- custo: cada pergunta chama a OpenAI com a chave de quem configurou (modelo,
  tamanho da resposta, voz);
- contratos: a API do Estêvão (`X-API-Key`), a Responses API da OpenAI e as
  ferramentas do estevao-mcp listadas em `allowed_tools`
  (`docs/ASSISTANT_API.md`);
- memória: heap e PSRAM do ESP32-S3, a partição `custom_16Mb.csv`, o tamanho
  do binário;
- as outras placas: o código comum do Bruce compila para dezenas de ambientes;
- licença: AGPL-3.0, `LICENSE` e `THIRD_PARTY.md`.
Se não há risco que valha nomear, diga por quê.
-->

## Publicação

<!--
O merge não grava nenhum aparelho. Com o GitHub Actions ativo, um push no
`main` que toca código roda `buil_parallel.yml`, que compila os ambientes e
troca os binários do pré-lançamento `betaRelease`; uma tag atualiza o
`lastRelease`. Escreva o que vai além disso, em ordem:
- gravar no T-Embed (`pio run -e lilygo-t-embed-cc1101 -t upload`), com o
  backup do firmware atual antes;
- campo novo ou mudado em `/estevao/config.json`, e como preenchê-lo no
  aparelho;
- ferramenta do estevao-mcp ou rota da API que precisa estar em produção
  antes, com o link;
- tag a criar, se for uma versão.
-->

- [ ] Nada além do merge.

## Como desfazer

<!-- Reverter o merge não regrava nenhum aparelho: diga como voltar o que já foi
gravado (backup do firmware, binário anterior em Releases) e o que fica no
LittleFS (configuração, cache) e continua sendo lido. Merge do upstream
revertido volta a divergir do Bruce: diga como sincronizar depois. -->

## Como testar

<!-- O que foi rodado, com o resultado (`pio run -e lilygo-t-embed-cc1101`), e
o que foi visto no aparelho: menu, data, livro de oração, Pergunte por texto e
por voz, sem rede. Sem o aparelho à mão, escreva: "compilou, não foi gravado
nem testado no T-Embed". -->

## Telas

<!-- Fotos da tela do T-Embed antes e depois. "Nada." se não muda tela. -->

## Relacionados

<!-- PRs em outros repositórios (`dodopok/estevao-mcp#123`,
`dodopok/estevao-api#123`), commits do upstream (`BruceDevices/firmware`), a
ordem em que entram, e issues. -->

## Checklist

- [ ] `pio run -e lilygo-t-embed-cc1101` compila
- [ ] O que é do Estêvão fica em `src/modules/estevao/` e `src/core/menu_items/EstevaoMenu.*`; o que toca código do Bruce é o mínimo e está dito aqui
- [ ] Sincronização com o Bruce feita por `scripts/update-bruce.sh`, com os conflitos nas áreas do Estêvão revisados
- [ ] Nenhuma chave ou token (Estêvão, OpenAI, MCP) em código, log, tela ou exemplo; `docs/ESTEVAO_CONFIG.example.json` só com valores de exemplo
- [ ] Configuração antiga em `/estevao/config.json` continua sendo lida, ou o PR diz como refazê-la
- [ ] Conexões continuam validando o certificado
- [ ] Integração com a OpenAI ou o MCP mudou: `docs/ASSISTANT_API.md` (e `docs/ASSISTANT_GATEWAY.md`) atualizado
- [ ] `LICENSE` e `THIRD_PARTY.md` mantidos
- [ ] C++ formatado conforme o `.clang-format`
- [ ] Gravado e testado no T-Embed, ou o PR diz que não
