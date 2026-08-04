#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

UPSTREAM_REMOTE="${UPSTREAM_REMOTE:-upstream}"
UPSTREAM_BRANCH="${UPSTREAM_BRANCH:-main}"
ALLOW_NON_MAIN=0

usage() {
    cat <<'EOF'
Uso: ./scripts/update-bruce.sh [opções]

Busca o upstream do Bruce e faz merge no branch atual.

Opções:
  --remote NOME       Remote do Bruce (padrão: upstream)
  --branch NOME       Branch do Bruce (padrão: main)
  --allow-non-main    Permite executar fora do branch main
  -h, --help          Mostra esta ajuda

O script exige uma árvore de trabalho limpa e não faz push automaticamente.
Depois do merge, valide o firmware e use `git push origin main` quando estiver
pronto.
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --remote)
            [[ $# -ge 2 ]] || { echo "Falta o nome do remote após --remote" >&2; exit 2; }
            UPSTREAM_REMOTE="$2"
            shift 2
            ;;
        --branch)
            [[ $# -ge 2 ]] || { echo "Falta o nome da branch após --branch" >&2; exit 2; }
            UPSTREAM_BRANCH="$2"
            shift 2
            ;;
        --allow-non-main)
            ALLOW_NON_MAIN=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Opção desconhecida: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

CURRENT_BRANCH="$(git branch --show-current)"
if [[ -z "$CURRENT_BRANCH" ]]; then
    echo "Erro: o repositório está em detached HEAD." >&2
    exit 1
fi

if [[ "$CURRENT_BRANCH" != "main" && "$ALLOW_NON_MAIN" != "1" ]]; then
    echo "Erro: sincronização bloqueada fora de main (branch atual: $CURRENT_BRANCH)." >&2
    echo "Use --allow-non-main somente se esse merge for intencional." >&2
    exit 1
fi

if ! git remote get-url "$UPSTREAM_REMOTE" >/dev/null 2>&1; then
    echo "Erro: remote '$UPSTREAM_REMOTE' não está configurado." >&2
    exit 1
fi

if [[ -n "$(git status --porcelain=v1)" ]]; then
    echo "Erro: a árvore de trabalho não está limpa." >&2
    echo "Faça commit ou stash das alterações antes de sincronizar." >&2
    git status --short >&2
    exit 1
fi

UPSTREAM_REF="$UPSTREAM_REMOTE/$UPSTREAM_BRANCH"
echo "Buscando $UPSTREAM_REF..."
git fetch --prune "$UPSTREAM_REMOTE" "$UPSTREAM_BRANCH"

if ! git show-ref --verify --quiet "refs/remotes/$UPSTREAM_REF"; then
    echo "Erro: não encontrei a referência remota $UPSTREAM_REF após o fetch." >&2
    exit 1
fi

BRUCE_COMMITS="$(git rev-list --count "HEAD..$UPSTREAM_REF")"
LOCAL_COMMITS="$(git rev-list --count "$UPSTREAM_REF..HEAD")"
echo "Commits novos do Bruce: $BRUCE_COMMITS"
echo "Commits locais preservados: $LOCAL_COMMITS"

if [[ "$BRUCE_COMMITS" == "0" ]]; then
    echo "Nenhuma atualização do Bruce para incorporar."
    exit 0
fi

echo
echo "Atualizações que serão incorporadas:"
git log --oneline --no-decorate "HEAD..$UPSTREAM_REF"
echo

if ! git merge --no-edit "$UPSTREAM_REF"; then
    echo >&2
    echo "Merge interrompido por conflitos. Resolva-os e conclua o merge manualmente." >&2
    echo "Use 'git merge --abort' para cancelar a operação." >&2
    exit 1
fi

echo
echo "Bruce atualizado com sucesso em $CURRENT_BRANCH."
echo "Próximos passos: validar o firmware e, se estiver tudo certo, executar:"
echo "  git push origin $CURRENT_BRANCH"
