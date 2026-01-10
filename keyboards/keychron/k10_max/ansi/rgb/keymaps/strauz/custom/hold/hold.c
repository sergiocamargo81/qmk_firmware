#include "hold.h"

#include "../../behavior.h"
#include "../../kind.h"
#include "../../custom_behaviors.h"  // Para custom_behaviors_t
#include "../../profile.h"
#include "../../settings.h"
#include "../custom.h"
#include "../../event_bus.h"  // Para Event Bus
#include "../../keymod.h"  // Para keymod_t
#include "../../colors.h"  // Para cores centralizadas
#include "../../pulse.h"
#include <lib/lib8tion/lib8tion.h> // Para scale8
#include <string.h>  // Para memset

// Declarações forward
bool hold_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Variáveis Globais =====

bool hold_profile_mode = true;
uint8_t hold_enabled_keys_bitfield[6] = {0};

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em HOLD_WAITING)
static keymod_t hold_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====
static bool is_key_enabled(custom_t *custom) {
    if (!custom) return false;
    // Verifica diretamente o behavior atual usando o cache
    return (custom->custom_behavior == CUSTOM_BEHAVIOR_HOLD);
}

static bool profiles_is_empty(profile_t* profile) {
    if (!profile) return true;
    for (uint8_t i = 0; i < PROFILES_KEYS_COUNT; i++) {
        if (profile->behaviors[i] != CUSTOM_BEHAVIOR_UNASSOCIATED) {
            return false;
        }
    }
    return true;
}

// Callback para notificar mudança de estado de FN
void hold_fn_state_callback(const event_t* event) {
    if (event->type != EVENT_FN_STATE_CHANGED) return;
    hold_fn_keymod = (keymod_t)event->data.fn_state_changed.keymod;
}

// Handler para notificar inclusão de tecla no controle de hold (via Event Bus)
static void hold_custom_behavior_added(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_ADDED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_HOLD) return;
    
    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    custom_t* custom = hold_get_key_by_position(row, col);
    if (custom) {
        // Inicializa estado apenas para esta tecla
        custom->state = HOLD_WAITING;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará branco pulsante (HOLD_WAITING) ou azul pulsante se FN pressionado
        // A pulsação será aplicada em update_indicators()
        // Inicializa com cor apropriada baseada no estado de FN
        uint8_t brightness = calculate_pulse_brightness();
        if (keymod_equals(hold_fn_keymod, KEYMOD_FN_ONLY)) {
            // Apenas FN está pressionada: azul pulsante
            color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
            rgb_matrix_set_color(custom->led_index, blue.r, blue.g, blue.b);
        } else {
            // FN não está pressionada: branco pulsante
            color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
            rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
        }
    }
}

// Handler para mudanças de behavior (via Event Bus)
static void hold_custom_behavior_changed_handler(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_CHANGED) return;

    uint8_t row = event->data.custom_behavior_changed.row;
    uint8_t col = event->data.custom_behavior_changed.col;
    custom_behaviors_t old_behavior = (custom_behaviors_t)event->data.custom_behavior_changed.old_behavior;
    custom_behaviors_t new_behavior = (custom_behaviors_t)event->data.custom_behavior_changed.new_behavior;

    custom_t* custom = hold_get_key_by_position(row, col);
    if (!custom) return;

    // Se passou a ser HOLD
    if (new_behavior == CUSTOM_BEHAVIOR_HOLD) {
        // Inicializa estado para esta tecla
        custom->state = HOLD_WAITING;
        // Handler já foi registrado na inicialização - não precisa registrar novamente
        // Atualiza LEDs
        update_indicators();
    }
    // Se deixou de ser HOLD
    else if (old_behavior == CUSTOM_BEHAVIOR_HOLD) {
        // Remove estado HOLD desta tecla - volta ao estado inicial
        custom->state = HOLD_WAITING;
        // Se nenhum custom tem HOLD, poderíamos desregistrar handler, mas mantemos para simplicidade
        // Atualiza LEDs
        update_indicators();
    }
}

// Handler para notificar remoção de tecla do controle de hold (via Event Bus)
static void hold_custom_behavior_removed(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_REMOVED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_HOLD) return;

    // Tecla deixou de ser hold - sai do controle do módulo hold
    // Não há estado a limpar pois ela não é mais responsabilidade deste módulo
}

// ===== Funções de Estado =====
custom_t* hold_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é HOLD usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_HOLD) return NULL;
    return custom;
}

custom_t* hold_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é HOLD usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_HOLD) return NULL;
    return custom;
}

void update_hold_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é HOLD usando o cache
        if (custom->custom_behavior != CUSTOM_BEHAVIOR_HOLD) continue;

        switch (custom->state) {
            case HOLD_PRESSING: {
                if (timer_elapsed32(custom->timer) >= HOLD_DELAY) {
                    custom->state = HOLD_FIRING; custom->timer = now; tap_code(custom->keycode);
                }
                break;
            }
            case HOLD_FIRING: {
                if (timer_elapsed32(custom->timer) >= HOLD_INTERVAL) {
                    tap_code(custom->keycode); custom->timer = now;
                }
                break;
            }
            case HOLD_RESTING: {
                if (timer_elapsed32(custom->timer) >= HOLD_LED_DEACTIVATE_TIME) custom->state = HOLD_WAITING;
                break;
            }
            default: break;
        }
    }
}

void activate_hold_key(custom_t *custom) {
    if (!custom) return;
    uint32_t now = timer_read32();
        custom->state = HOLD_PRESSING; custom->timer = now;
}

void deactivate_hold_key(custom_t *custom) {
    if (!custom) return;
    uint32_t now = timer_read32();
    if (custom->state == HOLD_FIRING) { custom->state = HOLD_RESTING; custom->timer = now; return; }
    if (custom->state == HOLD_PRESSING) { custom->state = HOLD_WAITING; return; }
}

void sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;

        bool is_enabled = is_key_enabled(custom);
        if (is_enabled) {
            // Se está habilitado como HOLD, garante que tenha um estado válido
            // Se ainda não tem estado válido, inicializa como HOLD_WAITING
            if (custom->state >= HOLD_RESTING + 1) { // Estado inválido (era DISABLED)
                custom->state = HOLD_WAITING;
            }
            // Se já estava em estado válido (HOLD_WAITING, HOLD_PRESSING, HOLD_FIRING, HOLD_RESTING), mantém
        }
        // Se não está habilitado como HOLD, não faz nada - sai do controle do módulo
    }
}


void update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a HOLD
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a HOLD usando o cache
        if (custom->custom_behavior == CUSTOM_BEHAVIOR_HOLD) {
            // Esta tecla está associada a HOLD
            // Atualiza LED baseado no estado
            switch (custom->state) {
                    case HOLD_WAITING: {
                        // Estado inicial: azul pulsante
                        uint8_t brightness = calculate_pulse_brightness();
                        color_rgb_t blue = color_apply_brightness(COLOR_BLUE, brightness);
                        rgb_matrix_set_color(custom->led_index, blue.r, blue.g, blue.b);
                        break;
                    }
                    case HOLD_PRESSING: {
                        // Tecla está pressionada: verde contínuo
                        color_rgb_t green = color_get_rgb(COLOR_GREEN);
                        rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                        break;
                    }
                    case HOLD_FIRING: {
                        // Disparo: vermelho contínuo
                        color_rgb_t red = color_get_rgb(COLOR_RED);
                        rgb_matrix_set_color(custom->led_index, red.r, red.g, red.b);
                        break;
                    }
                    case HOLD_RESTING: {
                        // Após soltar: amarelo contínuo por um tempo
                        color_rgb_t yellow = color_get_rgb(COLOR_YELLOW);
                        rgb_matrix_set_color(custom->led_index, yellow.r, yellow.g, yellow.b);
                        break;
                    }
                }
        }
    }
}

bool is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return hold_profile_mode;
}

void reset_all_enabled_keys(void) {
    memset(hold_enabled_keys_bitfield, 0, sizeof(hold_enabled_keys_bitfield));
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom) {
            custom->state = HOLD_WAITING;
        }
    }
}

// ===== Funções de Hook QMK =====

// Processa eventos de tecla para hold
bool hold_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_CUSTOM) {
        return true;
    }
    
    custom_t* custom = (custom_t*)key;
    
    // Verifica se o behavior é HOLD usando o cache
    if (custom->custom_behavior == CUSTOM_BEHAVIOR_HOLD) {
        if (pressed) {
            activate_hold_key(custom);
        } else {
            deactivate_hold_key(custom);
        }
        return true;
    }

    return true;
}

// Hook matrix_scan_user
static void hold_matrix_scan_user(const event_t* event) {
    if (event->type != EVENT_MATRIX_SCAN) return;
    update_hold_states();
}

// Hook rgb_matrix_indicators_user
static void hold_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    if (!is_profile_mode_active()) return;
    update_indicators();
}

// Hook keyboard_post_init_user
static void hold_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;

    // Registra handler para processamento de teclas
    custom_register_handler(CUSTOM_BEHAVIOR_HOLD, hold_process_key);

    // Registra handlers para eventos via Event Bus
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_ADDED, hold_custom_behavior_added);
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_REMOVED, hold_custom_behavior_removed);
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_CHANGED, hold_custom_behavior_changed_handler);

    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    hold_fn_keymod = KEYMOD_NONE;

    // Inicializa outras coisas...
    sync_enabled_states();
    
    // NOTE: A inicialização dos behaviors é feita via EVENT_CUSTOM_BEHAVIOR_CHANGED
    // disparado pelo custom_settings_loaded_handler quando settings são carregados
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void hold_init_early_hooks(void) {
    event_bus_subscribe(EVENT_MATRIX_SCAN, hold_matrix_scan_user);
    event_bus_subscribe_rgb_indicators(hold_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void hold_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, hold_keyboard_post_init_user);
}
