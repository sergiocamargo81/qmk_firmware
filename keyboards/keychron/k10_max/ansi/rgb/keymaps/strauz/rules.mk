RGB_MATRIX_ENABLE = yes
CONSOLE_ENABLE = no
VIA_ENABLE = yes
LTO_ENABLE = yes
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

SRC += profiles.c settings.c kind.c persist.c
SRC += behavior.c
SRC += hooks.c
SRC += pulse.c
SRC += profiles/profiles.c
SRC += custom/custom.c
SRC += custom/hold/hold.c
SRC += custom/toggle/toggle.c
SRC += persistence/persistence.c
SRC += numlock/numlock.c
SRC += modifiers/modifiers.c