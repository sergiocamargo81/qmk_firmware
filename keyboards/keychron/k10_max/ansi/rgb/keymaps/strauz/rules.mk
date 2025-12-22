RGB_MATRIX_ENABLE = yes
CONSOLE_ENABLE = no
VIA_ENABLE = yes
LTO_ENABLE = yes
# EXTRAKEY_ENABLE = yes
# MOUSEKEY_ENABLE = no

# ===== Autofire Hold System =====
SRC += \
    hold/keymap.c \
    hold/state.c \
    hold/eeprom.c
