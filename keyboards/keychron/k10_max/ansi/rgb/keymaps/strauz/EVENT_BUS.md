# Event Bus - Sistema de Comunicação entre Módulos

## Visão Geral

O Event Bus é um sistema centralizado de comunicação entre módulos que permite desacoplamento através de eventos. Módulos podem se inscrever em eventos e receber notificações quando eles ocorrem, sem conhecer diretamente outros módulos.

## Arquitetura

### Estrutura de Dados

O Event Bus usa **arrays estáticos** de handlers por tipo de evento, adequado para sistemas embarcados sem alocação dinâmica.

```c
// Array de handlers por tipo de evento
// g_handlers[event_type][index] = handler
static event_handler_t g_handlers[EVENT_COUNT][EVENT_BUS_MAX_HANDLERS];

// Contador de handlers por tipo de evento
static uint8_t g_handler_counts[EVENT_COUNT];
```

**Limite**: Máximo de 16 handlers por tipo de evento (`EVENT_BUS_MAX_HANDLERS`).

## Tipos de Eventos

O Event Bus suporta 8 tipos de eventos:

### Eventos de Sistema
- `EVENT_KEYBOARD_POST_INIT` - Executado após inicialização do teclado
- `EVENT_MATRIX_SCAN` - Executado periodicamente no loop principal
- `EVENT_RGB_INDICATORS` - Executado durante renderização RGB
- `EVENT_EECONFIG_INIT` - Executado quando EEPROM é inicializada

### Eventos de Settings/Profile
- `EVENT_PROFILE_CHANGED` - Profile ativo mudou (contém `old_profile` e `new_profile`)

### Eventos de Modifiers
- `EVENT_FN_STATE_CHANGED` - Estado de FN mudou (contém `keymod_t`)

### Eventos de Custom Behaviors
- `EVENT_CUSTOM_BEHAVIOR_ADDED` - Behavior foi adicionado a uma tecla (contém `row`, `col`, `behavior`)
- `EVENT_CUSTOM_BEHAVIOR_REMOVED` - Behavior foi removido de uma tecla (contém `row`, `col`, `behavior`)

## Estrutura de Dados dos Eventos

Cada evento pode carregar dados específicos através de uma union:

```c
typedef union {
    event_profile_changed_data_t profile_changed;
    event_fn_state_changed_data_t fn_state_changed;
    event_custom_behavior_data_t custom_behavior;
} event_data_t;

typedef struct {
    event_type_t type;
    event_data_t data;
} event_t;
```

## API Principal

### Inicialização

```c
void event_bus_init(void);
```

Deve ser chamado **antes de qualquer uso**, tipicamente em `keyboard_post_init_user()`.

### Inscrição em Eventos

```c
bool event_bus_subscribe(event_type_t type, event_handler_t handler);
```

Registra um handler para um tipo de evento. Retorna `true` se sucesso, `false` se:
- Tipo de evento inválido
- Handler é `NULL`
- Handler já está registrado
- Limite de handlers atingido (16 por tipo)

### Remoção de Inscrição

```c
bool event_bus_unsubscribe(event_type_t type, event_handler_t handler);
```

Remove um handler registrado. Retorna `true` se encontrado e removido.

### Publicação de Eventos

```c
bool event_bus_publish(event_type_t type, const event_data_t* data);
```

Dispara um evento, chamando todos os handlers registrados para aquele tipo. Retorna `true` se pelo menos um handler foi chamado.

Para eventos sem dados:

```c
static inline bool event_bus_publish_void(event_type_t type);
```

## API de Conveniência

Funções auxiliares para eventos comuns:

```c
bool event_bus_subscribe_rgb_indicators(event_handler_t handler);
bool event_bus_subscribe_profile_changed(event_handler_t handler);
bool event_bus_subscribe_fn_state_changed(event_handler_t handler);
```

## Padrão de Uso

### 1. Definir Handler

Todos os handlers seguem a mesma assinatura:

```c
static void my_handler(const event_t* event) {
    // Verifica tipo de evento
    if (event->type != EVENT_XXX) return;
    
    // Extrai dados do evento (se necessário)
    // Processa...
}
```

### 2. Registrar Handler

```c
// Durante inicialização (keyboard_post_init_user ou keyboard_pre_init_user)
event_bus_subscribe(EVENT_XXX, my_handler);

// Ou usando API de conveniência
event_bus_subscribe_rgb_indicators(my_handler);
```

### 3. Publicar Evento

```c
// Evento sem dados
event_bus_publish_void(EVENT_KEYBOARD_POST_INIT);

// Evento com dados
event_data_t data = {
    .profile_changed = {
        .old_profile = old_profile,
        .new_profile = new_profile
    }
};
event_bus_publish(EVENT_PROFILE_CHANGED, &data);
```

## Exemplos Práticos

### Exemplo 1: Handler RGB Indicators

```c
static void module_rgb_indicators(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    
    // Atualiza LEDs do módulo
    update_module_leds();
}

// Registro
event_bus_subscribe_rgb_indicators(module_rgb_indicators);
```

### Exemplo 2: Handler Profile Changed

```c
static void module_profile_changed(const event_t* event) {
    if (event->type != EVENT_PROFILE_CHANGED) return;
    
    profile_t* old_profile = (profile_t*)event->data.profile_changed.old_profile;
    profile_t* new_profile = (profile_t*)event->data.profile_changed.new_profile;
    
    // Atualiza estado do módulo para o novo profile
    sync_module_to_profile(new_profile);
}

// Registro
event_bus_subscribe_profile_changed(module_profile_changed);
```

### Exemplo 3: Handler Custom Behavior Added

```c
static void module_custom_behavior_added(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_ADDED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_XXX) return;
    
    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    
    // Inicializa tecla no módulo
    initialize_key(row, col);
}

// Registro
event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_ADDED, module_custom_behavior_added);
```

## Integração com Hooks QMK

O Event Bus é integrado diretamente aos hooks QMK em `keymap.c`:

```c
void eeconfig_init_user(void) {
    event_bus_publish_void(EVENT_EECONFIG_INIT);
}

void keyboard_post_init_user(void) {
    // ... inicialização dos módulos ...
    event_bus_publish_void(EVENT_KEYBOARD_POST_INIT);
}

void matrix_scan_user(void) {
    event_bus_publish_void(EVENT_MATRIX_SCAN);
}

bool rgb_matrix_indicators_user(void) {
    // ... lógica de renderização ...
    event_bus_publish_void(EVENT_RGB_INDICATORS);
    return true;
}
```

Os hooks QMK chamam diretamente `event_bus_publish_void()` para disparar os eventos correspondentes.

## Características Técnicas

### Alocação Estática
- Arrays pré-alocados (sem malloc/free)
- Adequado para sistemas embarcados
- Limite máximo: 16 handlers por tipo × 8 tipos = 128 handlers totais

### Performance
- Inscrição: O(n) onde n é o número de handlers já registrados (verifica duplicatas)
- Publicação: O(n) onde n é o número de handlers registrados para o tipo (iteração direta no array)
- Remoção: O(n) onde n é o número de handlers registrados para o tipo (move elementos para compactar)
- **Vantagem**: Arrays têm melhor cache locality que listas encadeadas

### Thread Safety
- **Não thread-safe** (assume execução em thread única, típico de firmware embarcado)

## Funções de Debug

```c
// Retorna número de handlers registrados para um tipo
uint8_t event_bus_get_handler_count(event_type_t type);

// Retorna nome do tipo de evento (para debug)
const char* event_bus_get_event_name(event_type_t type);
```

## Boas Práticas

1. **Sempre verifique o tipo de evento** no início do handler:
   ```c
   if (event->type != EVENT_XXX) return;
   ```

2. **Use tipos corretos ao extrair dados**:
   ```c
   profile_t* profile = (profile_t*)event->data.profile_changed.new_profile;
   keymod_t keymod = (keymod_t)event->data.fn_state_changed.keymod;
   ```

3. **Registre handlers durante inicialização** (antes de `event_bus_publish_void()` ser chamado nos hooks QMK)

4. **Não bloqueie handlers** - mantenha processamento rápido para não afetar outros módulos

5. **Use API de conveniência quando disponível** para código mais legível

## Limitações

- Máximo de 16 handlers por tipo de evento
- Arrays pré-alocados (128 handlers totais)
- Não thread-safe
- Handlers são chamados na ordem de registro (sem prioridade)
- Remoção de handlers move elementos no array (O(n))