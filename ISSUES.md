# ISSUES - Análise de Problemas do Projeto DayZ Client

## 🔴 CRÍTICOS

### 1. Caminhos Absolutos Hardcoded
**Arquivo:** `!Start_client_parameters.ini`
**Problema:** Caminhos absolutos para `C:\Program Files\RMNZ\` impedem funcionamento em outras máquinas.
```ini
Target = C:\Program Files\RMNZ\DayZ_x64.exe
StartIn = C:\Program Files\RMNZ\
SteamClientPath = C:\Program Files\RMNZ\SmartSteamEmu\SmartSteamEmu.dll
```
**Correção:** Usar caminhos relativos ou variáveis de ambiente:
```ini
Target = .\DayZ_x64.exe
StartIn = .\
SteamClientPath = .\SmartSteamEmu\SmartSteamEmu.dll
```

### 2. Arquivos de Crash Dump Presentes
**Arquivos:** `SmartSteamEmu/sse-*.dmp` (3 arquivos)
**Problema:** Indicam instabilidade do sistema e ocupam espaço desnecessário no repositório.
**Correção:** Remover arquivos .dmp do repositório e adicionar ao .gitignore:
```
*.dmp
**/*.dmp
```

### 3. Dependência Externa de tbb.dll
**Arquivo:** `SmartSteamEmu/README_HERMES_STABILITY.txt`
**Problema:** Dependência de `tbb.dll` em caminho externo (`C:\Program Files\Epic Games\Launcher\Portal\Binaries\Win64\tbb.dll`).
**Correção:** Incluir tbb.dll no repositório ou fornecer script de download automático.

## 🟡 IMPORTANTES

### 4. Configurações Duplicadas
**Arquivos:** `!Start_client_parameters.ini` e `SmartSteamEmu.ini`
**Problema:** Configurações de SmartSteamEmu duplicadas em dois arquivos, dificultando manutenção.
**Correção:** Centralizar configurações em um único arquivo ou usar sistema de herança de configuração.

### 5. Falta de Documentação Principal
**Problema:** Não há README.md na raiz do projeto com instruções de setup e instalação.
**Correção:** Criar README.md com:
- Descrição do projeto
- Pré-requisitos
- Instruções de instalação
- Como executar
- Estrutura de diretórios
- Troubleshooting comum

### 6. Steam Emulator Exposto
**Diretório:** `SmartSteamEmu/`
**Problema:** Emulador de Steam não oficial exposto no repositório, pode violar termos de serviço.
**Correção:** 
- Documentar claramente o propósito (desenvolvimento/teste)
- Considerar mover para diretório separado ou fornecer como download opcional
- Adicionar disclaimer legal

### 7. Arquivo de Log Vazio
**Arquivo:** `SmartSteamEmu/SmartSteamEmu.log` (0 bytes)
**Problema:** Arquivo de log existe mas está vazio, pode indicar problema de configuração de logging.
**Correção:** Verificar configuração de logging e remover arquivo se não for necessário, ou configurar properly.

## 🟢 MODERADOS

### 8. BattlEye e Emulador
**Diretório:** `BattlEye/`
**Problema:** Sistema anti-cheat pode conflitar com emulador de Steam.
**Correção:** Documentar limitações e possíveis conflitos, fornecer modo de desenvolvimento sem BattlEye.

### 9. Estrutura Complexa de Arquivos .part*
**Diretório:** `Addons/` e `dta/`
**Problema:** Arquivos PBO divididos em múltiplas partes (.part0, .part1, etc.) requerem launcher para reconstrução.
**Correção:** Documentar claramente o processo de reconstrução e fornecer script automatizado.

### 10. profile_fixed.cfg com Dados Hardcoded
**Arquivo:** `scripts/profile_fixed.cfg`
**Problema:** Presets de console com itens específicos podem não ser ideais para todos os usuários.
**Correção:** Mover para arquivo de exemplo ou tornar configurável via arquivo separado.

### 11. Arquivos .lst Não Versionados
**Arquivo:** `scripts/exclude.lst` (no .gitignore)
**Problema:** Lista de exclusão não versionada pode causar inconsistências entre ambientes.
**Correção:** Versionar arquivo exclude.lst ou documentar o processo de geração.

### 12. Falta de Estrutura de Documentação
**Problema:** Não há documentação sobre arquitetura, guia de desenvolvimento ou API.
**Correção:** Criar diretório `docs/` com:
- Arquitetura do sistema
- Guia de desenvolvimento
- Referência de API
- Padrões de código

### 13. Inconsistência de EOL
**Arquivo:** `.gitattributes`
**Problema:** Diferentes finais de linha para diferentes tipos de arquivos podem causar problemas em cross-platform.
**Correção:** Padronizar EOL (recomendado: LF para todos os arquivos de texto).

### 14. Arquivos Binários Grandes no Repositório
**Diretórios:** `Addons/`, `dta/`
**Problema:** Muitos arquivos .pbo.part* grandes tornam clone lento.
**Correção:** 
- Considerar usar Git LFS para arquivos grandes
- Fornecer script de download separado para assets
- Documentar tamanho esperado do repositório

## 🔵 MENORES

### 15. Arquivos Temporários
**Arquivos:** `steam_appid.txt` com espaço em branco no final
**Problema:** Espaço em branco pode causar problemas de parsing.
**Correção:** Remover espaço em branco: `221100` (sem espaço final)

### 16. Nomes de Arquivos com Prefixo Especial
**Arquivo:** `!Start_client_parameters.ini`
**Problema:** Prefixo `!` pode causar problemas em alguns sistemas de arquivos.
**Correção:** Renomear para `Start_client_parameters.ini` ou `client_parameters.ini`.

### 17. Falta de Validação de Configuração
**Problema:** Não há validação de arquivos de configuração ao iniciar.
**Correção:** Adicionar script de validação que verifica:
- Existência de arquivos necessários
- Integridade de caminhos
- Versões compatíveis

### 18. Sem Versionamento de Dependências
**Problema:** Não há arquivo listando versões de dependências externas.
**Correção:** Criar `DEPENDENCIES.md` ou usar gerenciador de pacotes apropriado.

## 📋 RESUMO DE PRIORIDADES

**Imediato (Fazer agora):**
1. Corrigir caminhos absolutos em `!Start_client_parameters.ini`
2. Remover arquivos .dmp do repositório
3. Adicionar *.dmp ao .gitignore

**Curto Prazo (Esta semana):**
4. Criar README.md principal
5. Documentar dependência do tbb.dll
6. Centralizar configurações duplicadas

**Médio Prazo (Este mês):**
7. Criar estrutura de documentação
8. Implementar validação de configuração
9. Documentar conflitos BattlEye/Emulador

**Longo Prazo:**
10. Avaliar uso de Git LFS
11. Padronizar EOL
12. Revisar estrutura de arquivos .part*

## 🛠️ SCRIPTS SUGERIDOS

### Script de Validação (validate_setup.ps1)
```powershell
# Verifica se todos os arquivos necessários existem
# Valida caminhos de configuração
# Verifica integridade de arquivos .part*
```

### Script de Setup (setup.ps1)
```powershell
# Configura caminhos relativos
# Baixa dependências externas
# Reconstrói arquivos PBO
```

### Script de Limpeza (clean.ps1)
```powershell
# Remove arquivos temporários
# Limpa logs antigos
# Remove arquivos .dmp
```
