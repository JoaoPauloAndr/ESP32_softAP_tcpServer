# Comunicação com ESP32 via WiFi - AP

Este código usa os exemplos da ESP-IDF `softAP` e `tcp_server` como base. Ele habilita o modo AP no ESP32 e implementa um servidor TCP que processa exemplos de comandos AT.

## Como Usar

### Hardware Requerido

Este exemplo foi feito para e testado em placas ESP32-WROOM-32 e ESP32-C3.

### Configurar o projeto

```
idf.py menuconfig
```

### Build e Flash

```
idf.py build
idf.py -p PORT flash monitor
```

(Toggle do monitor serial: ``Ctrl-]``)