#ifndef SETTINGS_H
#define SETTINGS_H

#include QMK_KEYBOARD_H

#include "profile.h"

// ===== Defines =====

// Versão do seu payload/settings (schema do settings_t e do blob gravado pelo persist)
#define SETTINGS_VERSION 1

// Número máximo de callbacks para mudança de profile
// Registros atuais: custom, profiles (2)
#define SETTINGS_MAX_PROFILE_CHANGED_CALLBACKS 2

// ===== Types/structs =====

// Struct em RAM (descompactada, fácil de trabalhar)
typedef struct {
    uint8_t version;
    profiles_t profiles;
} settings_t;

// ===== Functions =====

// Inicializa estruturas de dados e carrega da EEPROM
bool settings_init(void);

// Reseta tudo para defaults
void settings_reset_to_defaults(void);

// Acesso direto aos dados
settings_t* settings_get_working(void);
settings_t* settings_get_persisted(void);

// ===== Sistema de Callbacks para Mudança de Profile =====
// Migrado para Event Bus - use event_bus_subscribe_profile_changed() em vez de settings_register_profile_changed_callback()

// Define o profile ativo e notifica todos os callbacks registrados
// Esta função deve ser usada em vez de profile_set_active_profile diretamente
void settings_set_active_profile(uint8_t index);

#endif // SETTINGS_H

