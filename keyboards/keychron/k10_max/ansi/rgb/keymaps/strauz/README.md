# Keychron V10 Max - Autofire Keyboard Firmware

## 📖 VISÃO GERAL

Firmware modular para Keychron V10 Max com **sistema de autofire por tecla**. Cada tecla pode ser ativada/desativada individualmente via `Fn+RControl+KEY`, com persistência em EEPROM.

### ⚙️ CARACTERÍSTICAS PRINCIPAIS

- **Autofire System**: Máquina de 5 estados (WAITING, PRESSING, FIRING, RESTING, DISABLED)
- **48 Teclas Configuráveis**: Letras (A-Z), Números (0-9), Caracteres especiais, Space
- **Feedback Visual RGB**: Cores diferentes para cada estado do autofire
- **Persistência EEPROM**: Configurações salvas automaticamente por tecla
- **Space Key Special**: Cancela todos outros autofires quando pressionado
- **Toggle per Key**: Fn+RControl+KEY para ativar/desativar autofire
- **Reset All**: Fn+Delete para resetar todas as configurações
- **Auto-Initialize**: Primeira inicialização automática no boot

---

## 🏗️ ARQUITETURA DO FIRMWARE

### Estrutura de Arquivos

```
keyboards/keychron/k10_max/ansi/rgb/keymaps/strauz/

keymap.c                      ⭐ HUB CENTRAL
│
├── hold/                     🎯 COMPORTAMENTO HOLD (Autofire)
│   ├── keymap.h             (Interface pública)
│   ├── keymap.c             (Implementação dos hooks QMK)
│   ├── state.h              (Máquina de estados - privado)
│   ├── state.c              (Implementação da máquina de estados)
│   ├── eeprom.h             (Persistência EEPROM - privado)
│   └── eeprom.c             (Implementação EEPROM)
│
├── README.md                (Este arquivo)
└── rules.mk                 (Configuração de compilação)
```

### Fluxo de Módulos

```
keymap.c (root)
└─ Hooks QMK
   ├─ process_record_user()     → hold/keymap.c → process_record_hold()
   ├─ matrix_scan_user()        → hold/keymap.c → matrix_scan_hold()
   ├─ rgb_matrix_indicators_user() → hold/keymap.c → rgb_matrix_indicators_hold()
   ├─ keyboard_post_init_user() → hold/keymap.c → keyboard_post_init_hold()
   └─ eeconfig_init_user()      → hold/keymap.c → eeconfig_init_hold()

hold/keymap.c
├─ Chama hold/state.c   (update_autofire_states, activate_autofire_key, etc)
└─ Chama hold/eeprom.c  (load_enabled_keys, toggle_key_enabled, etc)
```

---

## 📋 CONVENÇÃO DE NOMES

### Visibilidade de Funções

**hold/keymap.h** (PÚBLICA)
- Nomeação: `[função]_hold()`
- Exemplo: `process_record_hold()`, `matrix_scan_hold()`
- **Uso**: Chamadas pelos hooks QMK em keymap.c root

**hold/state.h** (PRIVADA)
- Nomeação: `[função]()` sem prefixo
- Exemplo: `update_autofire_states()`, `activate_autofire_key()`
- **Uso**: Chamadas APENAS por hold/keymap.c
- **⚠️ IMPORTANTE**: Nunca chamar de fora!

**hold/eeprom.h** (PRIVADA)
- Nomeação: `[função]()` sem prefixo
- Exemplo: `load_enabled_keys()`, `toggle_key_enabled()`
- **Uso**: Chamadas APENAS por hold/keymap.c
- **⚠️ IMPORTANTE**: Nunca chamar de fora!

### Tipos de Dados

```c
// Configuração (defines)
#define AUTOFIRE_DELAY        500      // ms para ativar autofire
#define AUTOFIRE_INTERVAL     50       // ms entre repetições
#define LED_DEACTIVATE_TIME   5000     // ms até LED voltar a WAITING
#define AUTOFIRE_KEYS_COUNT   48       // Total de teclas autofire

// Estados (enum)
typedef enum {
    DISABLED,    // Autofire desativado
    WAITING,     // Aguardando ativação
    PRESSING,    // Pressionado, aguardando delay
    FIRING,      // Disparando (repetindo)
    RESTING,     // Repouso pós-disparo
} autofire_state_t;

// Estrutura de Dados
typedef struct {
    uint16_t keycode;        // Código da tecla (KC_A, KC_1, etc)
    uint8_t row;             // Posição na matriz (linha)
    uint8_t col;             // Posição na matriz (coluna)
    uint8_t led_index;       // Índice do LED para feedback
    autofire_state_t state;  // Estado atual da máquina
    uint32_t timer;          // Timestamp para controle de tempo
    uint8_t persist_index;   // Índice no bitfield EEPROM (0-47)
} autofire_key_t;
```

---

## 🔄 MÁQUINA DE ESTADOS

```
┌─────────────────────────────────────────────────────┐
│                    DISABLED                         │
│   (Autofire desativado para esta tecla)            │
│   RGB: OFF                                          │
└────────────────┬────────────────────────────────────┘
                 │
                 │ (Fn+RControl+KEY ativa a tecla)
                 ↓
┌─────────────────────────────────────────────────────┐
│                    WAITING                          │
│   (Tecla habilitada, aguardando ativação)          │
│   RGB: GRAY (128, 128, 128)                        │
└────────┬─────────────────────────────┬──────────────┘
         │                             │
         │ (Tecla pressionada)         │ (Reset)
         ↓                             ↓ DISABLED
┌──────────────────────┐
│    PRESSING          │
│ (Aguardando delay)   │
│ RGB: GREEN           │
│ (0, 255, 0)          │
│ Delay: 500ms         │
└──────────┬───────────┘
           │
           │ (Delay expirado)
           ↓
┌──────────────────────────────────────────────────────┐
│         FIRING (Loop)                               │
│ (Disparando/repetindo)                             │
│ RGB: RED (255, 0, 0)                               │
│ Intervalo: 50ms                                     │
│ tap_code(keycode) a cada intervalo                 │
└──────────┬───────────────────────────────────────────┘
           │
           │ (Tecla solta)
           ↓
┌────────────────────────────────────────────────────┐
│           RESTING                                  │
│ (Repouso pós-disparo, LED ativo)                  │
│ RGB: YELLOW (255, 255, 0)                         │
│ Duração: 5000ms (até voltar a WAITING)            │
└────────────┬───────────────────────────────────────┘
             │
             │ (5000ms expirado)
             ↓
         WAITING
```

---

## 💾 EEPROM - PERSISTÊNCIA

### Layout

```
EEPROM Address Map:
┌─────────────────────────────────────────────────┐
│ EECONFIG_AUTOFIRE_ENABLED (EECONFIG_USER)      │
│ 6 bytes bitfield (48 bits = 48 keys)            │
│                                                  │
│ Byte 0: Keys 0-7   (A-H)                        │
│ Byte 1: Keys 8-15  (I-P)                        │
│ Byte 2: Keys 16-23 (Q-X)                        │
│ Byte 3: Keys 24-31 (Y-Z + 0-5)                  │
│ Byte 4: Keys 32-39 (6-9 + SPACE + MINUS + EQL) │
│ Byte 5: Keys 40-47 (LBRC-SLSH)                  │
└─────────────────────────────────────────────────┘
│ EEPROM_INIT_FLAG_ADDR                           │
│ 1 byte: Init flag (0xAA = inicializado)         │
└─────────────────────────────────────────────────┘
```

### Índices de Persistência

```
Índice | Tecla    | Byte | Bit
-------|----------|------|-----
0      | KC_A     | 0    | 0
1      | KC_B     | 0    | 1
2      | KC_C     | 0    | 2
...
32     | KC_7     | 4    | 0
33     | KC_8     | 4    | 1
34     | KC_9     | 4    | 2
35     | KC_0     | 4    | 3
36     | KC_SPC   | 4    | 4  ⭐
37     | KC_MINUS | 4    | 5
...
47     | KC_SLSH  | 5    | 7
```

### Inicialização

1. **Primeira Inicialização**: EEPROM virgem (init_flag ≠ 0xAA)
   - Seta todos os bytes a 0 (todas as teclas DISABLED)
   - Escreve init_flag = 0xAA
   
2. **Inicializações Posteriores**: EEPROM já inicializado
   - Lê valores do EEPROM
   - Sincroniza com states do firmware

---

## ⌨️ KEYCODES CUSTOMIZADOS

| Keycode | Função | Atalho |
|---------|--------|---------|
| `AFR_MODE` | Toggle modo visualização LED | Fn+Pause |
| `AFR_RSET` | Reset todas as teclas | Fn+Delete |
| `AFR_EECLR` | Limpar EEPROM e resetar | Fn+Delete (holding) |

---

## 🎮 ATIVAÇÃO/DESATIVAÇÃO DE TECLAS

### Por Tecla Individual

```
Keymaps[WIN_FN]:
┌─────────┬────────┬─────────┬────────────┐
│ Fn      │ RCtrl  │ Tecla   │ Ação       │
├─────────┼────────┼─────────┼────────────┤
│ Held    │ Held   │ Press A │ Toggle A   │
│ Held    │ Held   │ Press B │ Toggle B   │
│ ...     │ ...    │ ...     │ ...        │
└─────────┴────────┴─────────┴────────────┘
```

**Exemplo de uso:**
1. Segurar Fn
2. Segurar RControl
3. Pressionar A → A ficará ENABLED (ou DISABLED se já estava ENABLED)
4. LED mudará para indicar novo estado

### Reset Global

```
Fn+Delete → Reset de TODAS as teclas para DISABLED
```

---

## 🎨 FEEDBACK RGB

### Estados e Cores

| Estado | RGB | Descrição |
|--------|-----|-----------|
| **DISABLED** | OFF (0,0,0) | Autofire desativado |
| **WAITING** | GRAY (128,128,128) | Aguardando ativação |
| **PRESSING** | GREEN (0,255,0) | Pressionado, delay ativo |
| **FIRING** | RED (255,0,0) | Disparando/repetindo |
| **RESTING** | YELLOW (255,255,0) | Repouso pós-disparo |

### Modo Visualização

- **AFR_MODE ON**: LED feedback ativado (mostra estado de cada tecla)
- **AFR_MODE OFF**: LED feedback desativado (todos LEDs OFF)

---

## 🛠️ COMPILAÇÃO E INSTALAÇÃO

### Pré-requisitos

```bash
# Instalar QMK CLI
pip install qmk

# Clonar repositório Keychron
git clone https://github.com/Keychron/qmk_firmware.git
cd qmk_firmware
```

### Copiar Arquivos

```bash
# Copiar para pasta do keymap
cp -r strauz/ keyboards/keychron/k10_max/ansi/rgb/keymaps/
```

### Compilar

```bash
# Compilação padrão
qmk compile -kb keychron/k10_max/ansi/rgb -km strauz

# Com verbose
qmk compile -kb keychron/k10_max/ansi/rgb -km strauz -v

# Flash automático (se suportado)
qmk flash -kb keychron/k10_max/ansi/rgb -km strauz
```

### Arquivo .hex

O arquivo compilado será:
```
keychron_k10_max_ansi_rgb_strauz.hex
```

---

## 🐛 TROUBLESHOOTING

### Problema: Teclas aparecem ativadas após flash

**Causa**: EEPROM com lixo no byte de inicialização
**Solução**: 
1. Pressionar Fn+Delete para resetar
2. Ou limpar EEPROM via QMK CLI:
   ```bash
   qmk util --reset
   ```

### Problema: LED não muda de cor

**Verificar:**
1. RGB_MATRIX_ENABLE = yes em rules.mk
2. Profile mode ativado (Fn+Pause)
3. Tecla realmente ativada (Fn+RControl+KEY)

### Problema: Autofire não funciona

**Verificar:**
1. Tecla está ENABLED (não DISABLED)
2. Não está em modo PRESSING (precisa 500ms)
3. EEPROM inicializada corretamente

---

## 📁 ESTRUTURA MODULAR

### Como Adicionar Novo Comportamento

Se quiser adicionar `toggle/` (comportamento de toggle):

```
toggle/
├── keymap.h/c         (Interface pública)
├── state.h/c          (Máquina de estados - privado)
└── eeprom.h/c         (Persistência - privado)
```

1. Criar pasta `toggle/`
2. Implementar mesma estrutura que `hold/`
3. Em `keymap.c` (root), adicionar:
   ```c
   #include "toggle/keymap.h"
   
   bool process_record_user(...) {
       // ... hold processing ...
       if (!process_record_toggle(keycode, record)) {
           return false;
       }
       return true;
   }
   
   // Similar para outros hooks
   ```

---

## 📝 NOTAS DE DESENVOLVIMENTO

### Visibilidade de Módulos

- **hold/keymap.h**: PÚBLICA (inclusa em keymap.c root)
- **hold/state.h**: PRIVADA (inclusa APENAS em hold/keymap.c)
- **hold/eeprom.h**: PRIVADA (inclusa APENAS em hold/keymap.c)

### Regra de Ouro

> Funções em `state.h` e `eeprom.h` NUNCA são chamadas de fora do módulo hold/.

---

## 📚 REFERÊNCIAS

- [QMK Docs](https://docs.qmk.fm/)
- [Keychron K10 Max](https://www.keychron.com/)
- [Matrix Configuration](https://docs.qmk.fm/using-qmk/guides/keyboard-matrix)
- [RGB Matrix](https://docs.qmk.fm/features/rgb_matrix)

---

## ✅ CHANGELOG

### v4 (ATUAL - MODULAR)
- ✅ Arquitetura totalmente modular
- ✅ Cada comportamento em sua pasta
- ✅ Separação clara de públic/privado
- ✅ Escalável para novos comportamentos
- ✅ Documentação completa

### v3
- Estado machine otimizada
- Binômio search implementado
- RGB feedback aprimorado

### v2
- EEPROM bitfield
- Per-key persistence

### v1
- Autofire básico
- RGB feedback

---

## 👤 AUTOR

**Strauz** - Mauá, São Paulo, BR
Linux enthusiast | Keyboard customizer | Gaming

---

**Última atualização**: 22 Dez 2024
**Status**: Production Ready ✅
