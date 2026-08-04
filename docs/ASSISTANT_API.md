# Contrato do serviço `assistant/ask`

O LilyGO conversa somente com um serviço autenticado sob o domínio da API do
Estêvão. O serviço pode usar o MCP remoto e a OpenAI no servidor, mas nenhuma
dessas credenciais deve chegar ao firmware.

## Texto

```http
POST /api/v1/assistant/ask
Accept: application/json
Content-Type: application/json
X-API-Key: estevao_device_...
User-Agent: Estevao-LilyGO/0.1
```

Payload:

```json
{
  "question": "Qual é a celebração principal de hoje?",
  "date": "2026-08-04",
  "language": "pt-BR",
  "prayer_book_code": "loc_2015",
  "bible_version": "nvi",
  "reading_type": "semicontinuous"
}
```

## Voz

O mesmo endpoint recebe áudio WAV mono PCM no corpo da requisição. O contexto
litúrgico segue nos parâmetros da URL e o modo é indicado pelo header:

```http
POST /api/v1/assistant/ask?date=2026-08-04&language=pt-BR&prayer_book_code=loc_2015&bible_version=nvi&reading_type=semicontinuous
Accept: application/json
Content-Type: audio/wav
X-Question-Mode: voice
X-API-Key: estevao_device_...
User-Agent: Estevao-LilyGO/0.1
```

O formato de áudio inicial é 48 kHz, mono, PCM signed 16-bit, produzido pelo
driver de microfone do Bruce no T-Embed CC1101. O serviço deve transcrever o
WAV antes de enviar a pergunta ao modelo. O limite recomendado para o MVP é
8 segundos por pergunta.

## Resposta

O firmware espera uma resposta compacta e estável:

```json
{
  "answer": "A celebração principal é ...",
  "question": "Qual é a celebração principal de hoje?",
  "date": "2026-08-04",
  "language": "pt-BR",
  "request_id": "req_..."
}
```

O campo `answer` deve ser texto simples, curto o bastante para rolar no
display. O firmware também tolera `message` ou `text` como fallback durante a
implantação do serviço.

Erros devem seguir este formato quando possível:

```json
{
  "error": {
    "code": "rate_limited",
    "message": "Tente novamente mais tarde."
  }
}
```

O dispositivo trata `400`, `401`, `404`, `429`, `500`/`503` e timeout sem
apagar a última resposta salva localmente.

## Papel do MCP remoto

O endpoint MCP publicado pode ser usado pelo serviço como uma ferramenta do
modelo, mas não substitui este gateway: MCP não é, por si só, a API de conversa
do dispositivo. O gateway deve autenticar a API key do LilyGO, limitar a
resposta, incluir o contexto do dia e manter o token/URL do MCP no servidor.
