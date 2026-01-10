#include "toggle.h"

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
#include <string.h>  // Para memset

// Declarações forward
bool toggle_process_key(base_key_t* key, bool pressed, keymod_t keymod);

// ===== Variáveis Globais =====

bool toggle_profile_mode = true;
uint8_t toggle_enabled_keys_bitfield[6] = {0};

// ===== Estado de FN =====

// Rastreia keymod atual (para mudar cor das teclas em TOGGLE_WAITING)
static keymod_t toggle_fn_keymod = KEYMOD_NONE;

// ===== Funções Auxiliares =====
static bool is_key_enabled(custom_t *custom) {
    if (!custom) return false;
    // Verifica diretamente o behavior atual usando o cache
    return (custom->custom_behavior == CUSTOM_BEHAVIOR_TOGGLE);
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
void toggle_fn_state_callback(const event_t* event) {
    if (event->type != EVENT_FN_STATE_CHANGED) return;
    toggle_fn_keymod = (keymod_t)event->data.fn_state_changed.keymod;
}

// Handler para notificar inclusão de tecla no controle de toggle (via Event Bus)
static void toggle_custom_behavior_added(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_ADDED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_TOGGLE) return;
    
    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    custom_t* custom = toggle_get_key_by_position(row, col);
    if (custom) {
        // Inicializa estado apenas para esta tecla
        custom->state = TOGGLE_WAITING;
        // Atualiza o LED imediatamente após a associação
        // O LED ficará branco pulsante (TOGGLE_OFF) ou verde pulsante se FN pressionado
        // A pulsação será aplicada em toggle_update_indicators()
        // Inicializa com cor apropriada baseada no estado de FN
        uint8_t brightness = calculate_pulse_brightness();
        if (keymod_equals(toggle_fn_keymod, KEYMOD_FN_ONLY)) {
            // Apenas FN está pressionada: verde pulsante
            color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
            rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
        } else {
            // FN não está pressionada: branco pulsante
            color_rgb_t white = color_apply_brightness(COLOR_WHITE, brightness);
            rgb_matrix_set_color(custom->led_index, white.r, white.g, white.b);
        }
    }
}

// Handler para mudanças de behavior (via Event Bus)
static void toggle_custom_behavior_changed_handler(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_CHANGED) return;

    uint8_t row = event->data.custom_behavior_changed.row;
    uint8_t col = event->data.custom_behavior_changed.col;
    custom_behaviors_t old_behavior = (custom_behaviors_t)event->data.custom_behavior_changed.old_behavior;
    custom_behaviors_t new_behavior = (custom_behaviors_t)event->data.custom_behavior_changed.new_behavior;

    custom_t* custom = toggle_get_key_by_position(row, col);
    if (!custom) return;

    // Se passou a ser TOGGLE
    if (new_behavior == CUSTOM_BEHAVIOR_TOGGLE) {
        // Inicializa estado para esta tecla
        custom->state = TOGGLE_WAITING;
        // Handler já foi registrado na inicialização - não precisa registrar novamente
        // Atualiza LEDs
        toggle_update_indicators();
    }
    // Se deixou de ser TOGGLE
    else if (old_behavior == CUSTOM_BEHAVIOR_TOGGLE) {
        // Remove estado TOGGLE desta tecla
        custom->state = TOGGLE_WAITING;
        // Se nenhum custom tem TOGGLE, poderíamos desregistrar handler, mas mantemos para simplicidade
        // Atualiza LEDs
        toggle_update_indicators();
    }
}

// Handler para notificar remoção de tecla do controle de toggle (via Event Bus)
static void toggle_custom_behavior_removed(const event_t* event) {
    if (event->type != EVENT_CUSTOM_BEHAVIOR_REMOVED) return;
    if (event->data.custom_behavior.behavior != CUSTOM_BEHAVIOR_TOGGLE) return;

    uint8_t row = event->data.custom_behavior.row;
    uint8_t col = event->data.custom_behavior.col;
    custom_t* custom = toggle_get_key_by_position(row, col);
    if (custom) {
        // Limpa estado - a tecla será recolorida quando associada a outro módulo
        custom->state = TOGGLE_WAITING;
    }
}

// ===== Funções de Estado =====
custom_t* toggle_get_key_by_position(uint8_t row, uint8_t col) {
    custom_t* custom = kind_get_custom(row, col);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é TOGGLE usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_TOGGLE) return NULL;
    return custom;
}

custom_t* toggle_get_key_by_index(uint8_t persist_index) {
    custom_t* custom = kind_get_custom_by_index(persist_index);
    if (custom == NULL) return NULL;
    // Verifica se o behavior é TOGGLE usando o cache
    if (custom->custom_behavior != CUSTOM_BEHAVIOR_TOGGLE) return NULL;
    return custom;
}

void toggle_update_states(void) {
    uint32_t now = timer_read32();
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se o behavior é TOGGLE usando o cache
        if (custom->custom_behavior != CUSTOM_BEHAVIOR_TOGGLE) continue;

        if (custom->state == TOGGLE_WAITING) continue;
        if (custom->state == TOGGLE_FIRING) {
            if (timer_elapsed32(custom->timer) >= TOGGLE_INTERVAL) {
                tap_code(custom->keycode);
                custom->timer = now;
            }
        }
    }
}

static void toggle_sync_enabled_states(void) {
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        bool is_enabled = is_key_enabled(custom);
        if (!is_enabled) custom->state = TOGGLE_WAITING;
    }
}


void toggle_update_indicators(void) {
    // Atualiza apenas os LEDs das teclas que estão associadas a TOGGLE
    // Verifica diretamente o behavior atual para garantir que apenas teclas realmente associadas sejam atualizadas
    for (uint8_t i = 0; i < CUSTOM_COUNT; i++) {
        custom_t* custom = kind_get_custom_by_index(i);
        if (custom == NULL) continue;
        
        // Verifica se esta tecla está realmente associada a TOGGLE usando o cache
        if (custom->custom_behavior == CUSTOM_BEHAVIOR_TOGGLE) {
            // Esta tecla está associada a TOGGLE
            // Atualiza LED baseado no estado
            switch (custom->state) {
                case TOGGLE_WAITING: {
                    // TOGGLE_WAITING: verde pulsante
                    uint8_t brightness = calculate_pulse_brightness();
                    color_rgb_t green = color_apply_brightness(COLOR_GREEN, brightness);
                    rgb_matrix_set_color(custom->led_index, green.r, green.g, green.b);
                    break;
                }
                case TOGGLE_FIRING: {
                    // TOGGLE_FIRING: vermelho contínuo
                    color_rgb_t red = color_get_rgb(COLOR_RED);
                    rgb_matrix_set_color(custom->led_index, red.r, red.g, red.b);
                    break;
                }
            }
        }
    }
}

bool toggle_is_profile_mode_active(void) {
    settings_t* working = settings_get_working();
    profile_t* active_profile = profile_get_active_profile(&working->profiles);
    if (!active_profile || profiles_is_empty(active_profile)) {
        return false;
    }
    return toggle_profile_mode;
}


// ===== Funções de Hook QMK =====

// Processa eventos de tecla para toggle
bool toggle_process_key(base_key_t* key, bool pressed, keymod_t keymod) {
    if (key == NULL || key->kind != KIND_CUSTOM) {
        return true;
    }

    custom_t* custom = (custom_t*)key;

    // Verifica se o behavior é TOGGLE usando o cache
    if (custom->custom_behavior == CUSTOM_BEHAVIOR_TOGGLE) {
        if (pressed) {
            // Alterna entre TOGGLE_WAITING e TOGGLE_FIRING
            uint32_t now = timer_read32();
            if (custom->state == TOGGLE_WAITING) {
                custom->state = TOGGLE_FIRING;
                custom->timer = now;
                tap_code(custom->keycode);  // Primeiro envio imediato
            } else if (custom->state == TOGGLE_FIRING) {
                custom->state = TOGGLE_WAITING;
                // Para de enviar automaticamente
            }
        }
        // No release, não faz nada - o toggle é controlado apenas no press
        return true;
    }

    return true;
}

// Hook matrix_scan_user
static void toggle_matrix_scan_user(const event_t* event) {
    if (event->type != EVENT_MATRIX_SCAN) return;
    toggle_update_states();
}

// Hook rgb_matrix_indicators_user
static void toggle_rgb_matrix_indicators_user(const event_t* event) {
    if (event->type != EVENT_RGB_INDICATORS) return;
    if (!toggle_is_profile_mode_active()) return;
    toggle_update_indicators();
}

// Hook keyboard_post_init_user
static void toggle_keyboard_post_init_user(const event_t* event) {
    if (event->type != EVENT_KEYBOARD_POST_INIT) return;

    // Registra handler para processamento de teclas
    custom_register_handler(CUSTOM_BEHAVIOR_TOGGLE, toggle_process_key);

    // Registra handlers para eventos via Event Bus
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_ADDED, toggle_custom_behavior_added);
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_REMOVED, toggle_custom_behavior_removed);
    event_bus_subscribe(EVENT_CUSTOM_BEHAVIOR_CHANGED, toggle_custom_behavior_changed_handler);

    // Estado de FN será inicializado como KEYMOD_NONE
    // Será atualizado via notificação quando FN for pressionado
    toggle_fn_keymod = KEYMOD_NONE;

    // Inicializa outras coisas...
    toggle_sync_enabled_states();
    
    // NOTE: A inicialização dos behaviors é feita via EVENT_CUSTOM_BEHAVIOR_CHANGED
    // disparado pelo custom_settings_loaded_handler quando settings são carregados
}

// ===== Inicialização de Hooks =====

// Registra hooks que podem ser registrados antes da inicialização completa
// Chamado em keyboard_pre_init_user
void toggle_init_early_hooks(void) {
    event_bus_subscribe(EVENT_MATRIX_SCAN, toggle_matrix_scan_user);
    event_bus_subscribe_rgb_indicators(toggle_rgb_matrix_indicators_user);
}

// Registra hooks QMK para este módulo
// Deve ser chamado durante keyboard_post_init_user (antes de event_bus_publish_void(EVENT_KEYBOARD_POST_INIT))
void toggle_init_hooks(void) {
    event_bus_subscribe(EVENT_KEYBOARD_POST_INIT, toggle_keyboard_post_init_user);
}
