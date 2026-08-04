# Assistente direto da OpenAI com MCP do Estêvão

No MVP de um único dispositivo, o LilyGO usa duas integrações independentes:

```mermaid
flowchart LR
    A["LilyGO T-Embed"] -->|X-API-Key| B["api.caminhoanglicano.com.br"]
    A -->|OpenAI API key| C["api.openai.com/v1/responses"]
    C -->|MCP + token MCP| D["estevao-mcp remoto"]
    D -->|ESTEVAO_API_KEY do servidor| B
```

As telas de calendário, leituras, comemorações e explicação continuam chamando
diretamente a API do Estêvão. A tela **Pergunte** não usa o POST
`/api/v1/assistant/ask`: ela chama a Responses API da OpenAI, que pode consultar
as ferramentas somente-leitura do MCP remoto. O `estevao-mcp` usa sua própria
`ESTEVAO_API_KEY` no servidor ao consultar a API do Estêvão.

## Configuração persistida no LilyGO

O firmware grava estes campos em `/estevao/config.json`:

```json
{
  "base_url": "https://api.caminhoanglicano.com.br",
  "api_key": "<ESTEVAO_API_KEY_DO_DISPOSITIVO>",
  "openai_base_url": "https://api.openai.com",
  "openai_api_key": "<OPENAI_API_KEY_PESSOAL>",
  "openai_model": "gpt-5.6-luna",
  "mcp_server_url": "https://estevao-mcp.dodopok.dev/mcp",
  "mcp_token": "<MCP_TOKEN>"
}
```

Nenhum valor real deve ser versionado. A API key do Estêvão é usada somente
contra `api.caminhoanglicano.com.br`; a API key da OpenAI é usada somente
contra `api.openai.com`; e o token MCP é usado somente como autorização do
servidor MCP declarado na requisição da OpenAI.

Se o servidor MCP usar token no próprio caminho, por exemplo
`/mcp/<token>`, configure a URL completa e deixe `mcp_token` vazio. Se usar
Bearer, configure a URL `/mcp` e preencha `mcp_token`.

## Pergunta por texto

O firmware envia para:

```http
POST https://api.openai.com/v1/responses
Accept: application/json
Content-Type: application/json
Authorization: Bearer <OPENAI_API_KEY>
User-Agent: Estevao-LilyGO/0.1
```

O corpo contém o modelo, o contexto litúrgico, instruções curtas para a tela e
o MCP remoto:

```json
{
  "model": "gpt-5.6-luna",
  "store": false,
  "max_output_tokens": 800,
  "instructions": "Responda em português do Brasil, de forma curta...",
  "input": "Data litúrgica: 2026-08-04\nPergunta do usuário: ...",
  "tools": [
    {
      "type": "mcp",
      "server_label": "estevao",
      "server_url": "https://estevao-mcp.dodopok.dev/mcp",
      "authorization": "Bearer <MCP_TOKEN>",
      "require_approval": "never",
      "allowed_tools": [
        "get_liturgical_day",
        "get_calendar_month",
        "get_year_overview",
        "get_readings",
        "get_lectionary_cycle",
        "get_daily_office",
        "search_celebrations",
        "list_celebrations",
        "get_celebration",
        "list_prayer_books",
        "compare_prayer_books"
      ]
    }
  ]
}
```

`require_approval: "never"` é adequado aqui porque o MCP do Estêvão expõe
somente ferramentas de leitura. O modelo ainda decide quando uma ferramenta é
necessária; o firmware não chama o MCP diretamente.

O firmware procura o texto final em `output_text` ou nos itens de mensagem em
`output`. A resposta completa é salva em LittleFS para a opção **Última
resposta**.

## Pergunta por voz

O microfone do T-Embed grava WAV mono PCM signed 16-bit em 48 kHz. Para manter
o fluxo compatível com a OpenAI, o firmware faz duas chamadas:

1. `POST /v1/audio/transcriptions` como `multipart/form-data`, usando o modelo
   `gpt-4o-mini-transcribe`;
2. `POST /v1/responses` com a transcrição como pergunta e o mesmo MCP remoto.

O limite padrão é 8 segundos e o upload é limitado a 1 MB. A resposta volta
como texto para o display; esta versão não faz síntese de voz.

## TLS

O firmware valida HTTPS com um bundle embutido contendo os trust anchors
necessários para as cadeias atuais do Estêvão e da OpenAI. Um override PEM
opcional pode ser colocado em `/estevao/ca.pem` pelo WebUI quando uma futura
troca controlada de cadeia exigir isso. Não usar `setInsecure()`.

## Segurança e escopo

Esta topologia é apropriada para o dispositivo pessoal do proprietário. Uma
API key da OpenAI em firmware pode ser extraída por alguém com acesso físico e
deve ser restrita, monitorada e rotacionada quando necessário. Para distribuir
o firmware a outras pessoas ou operar vários dispositivos, use a alternativa
descrita em [`ASSISTANT_GATEWAY.md`](ASSISTANT_GATEWAY.md): uma ponte autenticada
mantém a API key da OpenAI fora do LilyGO.
