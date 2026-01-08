RGB_MATRIX_ENABLE = yes
VIA_ENABLE = yes
LTO_ENABLE = yes
CONSOLE_ENABLE = no
#MOUSEKEY_ENABLE = no
#EXTRAKEY_ENABLE = no
#NKRO_ENABLE = no

# Desabilita APDAPTIVE_NKRO_ENABLE porque NKRO está desabilitado
# Isso evita erro de compilação onde kb_report_changed não é declarado
#OPT_DEFS += -UAPDAPTIVE_NKRO_ENABLE


# Habilita "user datablock" do eeconfig no QMK para este keymap.
# 128 bytes = 124 (dados) + 4 (CRC32).
OPT_DEFS += -DEECONFIG_USER_DATA_SIZE=128
OPT_DEFS += -DEECONFIG_USER_DATA_VERSION=1

SRC += profile.c settings.c kind.c persist.c
SRC += behavior.c
SRC += pulse.c
SRC += keymod.c
SRC += event_bus.c
SRC += profiles/profiles.c
SRC += custom/custom.c
SRC += custom/hold/hold.c
SRC += custom/toggle/toggle.c
SRC += custom/unassociated/unassociated.c
SRC += persistence/persistence.c
SRC += numlock/numlock.c
SRC += modifiers/modifiers.c
SRC += disabled/disabled.c
SRC += bold/bold.c
SRC += disabled_modifiers/disabled_modifiers.c