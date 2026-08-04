# Ponte opcional para o assistente do Estêvão

Esta ponte **não faz parte do MVP atual**. Para o LilyGO pessoal, a arquitetura
em uso é:

```mermaid
flowchart LR
    A["LilyGO"] -->|OpenAI API key| B["OpenAI Responses API"]
    B -->|MCP + token| C["estevao-mcp remoto"]
    C -->|ESTEVAO_API_KEY do servidor| D["api.caminhoanglicano.com.br"]
```

Uma ponte separada continua sendo a melhor evolução quando houver mais de um
dispositivo, distribuição do firmware ou necessidade de revogar credenciais
sem reconfigurar o LilyGO:

```mermaid
flowchart LR
    A["LilyGO"] -->|API key do dispositivo| B["assistant-bridge"]
    B -->|OPENAI_API_KEY do servidor| C["OpenAI Responses API"]
    C -->|MCP + token| D["estevao-mcp remoto"]
    D -->|ESTEVAO_API_KEY do servidor| E["api.caminhoanglicano.com.br"]
```

## O que não é necessário agora

Não é necessário adicionar `POST /api/v1/assistant/ask` à `estevao-api` nem
alterar as rotas existentes. O MCP remoto já funciona como adaptador para as
fontes do Estêvão, e a OpenAI consegue chamá-lo como uma ferramenta remota na
Responses API.

## Quando criar a ponte

Crie o serviço quando a prioridade passar a ser segurança operacional, por
exemplo:

- vários LilyGOs compartilhando o serviço;
- firmware distribuído para terceiros;
- rotação/revogação centralizada da OpenAI e do MCP;
- rate limit, auditoria, custos e logs por dispositivo;
- necessidade de manter todas as credenciais fora do hardware.

Nesse cenário, a ponte pode expor um endpoint próprio, como
`POST /assistant/ask`, sem modificar a `estevao-api`. Ela deve autenticar o
dispositivo, validar texto/áudio, chamar OpenAI + MCP, normalizar a resposta e
devolver apenas texto compacto ao firmware.

## Regras da futura ponte

- somente HTTPS;
- uma API key por dispositivo, com rotação e revogação;
- `OPENAI_API_KEY`, token MCP e `ESTEVAO_API_KEY` apenas no ambiente do serviço;
- limite de tamanho e duração para áudio;
- timeout total e rate limit por dispositivo;
- não registrar chaves, tokens, áudio ou pergunta completa por padrão;
- permitir somente as ferramentas de leitura necessárias do MCP;
- resposta estável com `answer`, `request_id` e códigos de erro.

Enquanto o dispositivo for pessoal, manter a ponte fora do caminho reduz a
infraestrutura e deixa a API do Estêvão sem mudanças.
