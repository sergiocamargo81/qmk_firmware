#ifndef SETTINGS_H
#define SETTINGS_H

#include QMK_KEYBOARD_H

#include "profiles.h"

// ===== Defines =====

// Versão do seu payload/settings (schema do settings_t e do blob gravado pelo persist)
#define SETTINGS_VERSION 1

// Número máximo de callbacks para mudança de profile
// Atualmente apenas custom se registra, mas permite espaço para futuros módulos
#define SETTINGS_MAX_PROFILE_CHANGED_CALLBACKS 4

// ===== Types/structs =====

// Struct em RAM (descompactada, fácil de trabalhar)
typedef struct {
    uint8_t version;
    profiles_t profiles;
} settings_t;

// Tipo de callback para notificar mudança de profile ativo
// old_profile: ponteiro para o profile anterior (pode ser NULL se não havia profile ativo)
// new_profile: ponteiro para o novo profile ativo (nunca NULL)
typedef void (*settings_profile_changed_callback_t)(profile_t* old_profile, profile_t* new_profile);

// ===== Functions =====

// Inicializa estruturas de dados e carrega da EEPROM
bool settings_init(void);

// Reseta tudo para defaults
void settings_reset_to_defaults(void);

// Acesso direto aos dados
settings_t* settings_get_working(void);
settings_t* settings_get_persisted(void);

// ===== Sistema de Callbacks para Mudança de Profile =====

// Registra um callback para ser notificado quando o profile ativo mudar
// Deve ser chamado durante {modulo}_keyboard_post_init_user
// Retorna true se sucesso, false se não há mais espaço
bool settings_register_profile_changed_callback(settings_profile_changed_callback_t callback);

// Define o profile ativo e notifica todos os callbacks registrados
// Esta função deve ser usada em vez de profiles_set_active_profile diretamente
void settings_set_active_profile(uint8_t index);

#endif // SETTINGS_H

