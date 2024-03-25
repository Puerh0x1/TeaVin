# TeaVin

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/a4c1c4c0-6f32-4240-86c3-23f6e067605b)

### Ultimate ESP32 EvilTwin

**TeaVin** - русскоязычный проект на платформе ESP32, позволяющий ИБ специалистам выполнять тестирование WiFi сетей с помощью атак вектора EvilTwin. Проект представляет собой прошивку для модуля ESP32. Исходный код открытый, некоммерческий.

> **Внимание:** Данные предоставлены сугубо для ИБ специалистов и могут быть использованы **ТОЛЬКО** на законных условиях. Автор не несет ответственности за действия разрушительного характера/прочие противоправные поступки, которые могут быть совершены с использованием данного ПО. Призываю к сознательности каждого.

#### Описание

Для использования вам необходим модуль (плата) ESP32, в том числе модифицированная.

Какой-либо инструмент для прошивки. Например, Espressif Flash Tool:
[https://www.espressif.com/en/support/download/other-tools](https://www.espressif.com/en/support/download/other-tools)

Или вы можете использовать ARDUINO IDE:
[https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

Модуль создает WiFi сеть 2.4g с вашими данными и включает на ней Captive Portal с страницей аутентификации. Все DNS и HTTP запросы, совершенные в сети с Captive Portal, переадресовываются на страницу Captive Portal, что обеспечивает моментальное открытие Captive Portal после подключения к сети и гарантию посещения страницы Captive Portal со стороны клиента этой сети.

Все введенные данные на странице Captive Portal сохраняются в логи. В версии TeaVin 2.x реализовано хранение логов в файловой системе модуля и их очистка (логи сохраняются даже если перезагрузить или выключить ESP32)

Captive Portal представляет собой фишинговую страницу с полями ввода каких-либо данных. На данный момент это данные от WiFi сети, которая является целью для тестирования на проникновение. Позже будут добавлены другие шаблоны HTML Captive Portal.

Весь контроль процессом осуществляется с помощью WiFi сетей частоты 2.4g. 5 ГГц-овый режим еще не добавлен.

#### Работа с TeaVin

После прошивки вашего модуля и подключения его к источнику питания, он создает сеть `SureWifi` с паролем `The0PassP0`. Данные могут быть изменены в исходном коде проекта.

После подключения к сети вы можете найти панель управления по адресу [http://192.168.4.1/control](http://192.168.4.1/control). Все страницы адаптированы под мобильные устройства.

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/2b563db9-51d0-4d27-8ab1-642649b88c20)

После ввода SSID (названия сети) - ждем 1 секунду, и будет создана сеть с этим названием, а текущая сеть отключится.

После подключения к новой сети с вашим названием - все DNS и HTTP запросы будут переадресованы на `192.168.4.1`, то есть на Captive Portal.

Страница с логами данных, которые введены на странице аутентификации Captive Portal, находится на вкладке:

`http://192.168.4.1/logs`

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/ca1a98e3-f682-4804-a9a4-562b7f0e60d2)

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/f8386562-4b24-47b6-b27a-160a58860b50)


Control Panel также находится на `/control`, а сам Captive Portal - на `192.168.4.1`.

---

Прошивка:

Мои настройки для прошивки модуля в ARDUINO IDE:

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/53c4d524-1b1a-4e94-a0bc-41cf61a58428)

Ссылка на менеджер плат:
`https://dl.espressif.com/dl/package_esp32_index.json`

![image](https://github.com/Puerh0x1/TeaVin/assets/162372951/f84afe0c-00c5-4293-94fa-560288e4f532)


Конфиг и команда esptool:

`./esptool.exe --chip esp32 --port COM5 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 TeaVin.ino.bootloader.bin 0x8000 TeaVin.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 TeaVin.ino.bin`

Для Python версии esptool

`python3 esptool.py --chip esp32 --port COM5 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 TeaVin.ino.bootloader.bin 0x8000 TeaVin.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 TeaVin.ino.bin`

Или просто

`esptool --chip esp32 --port COM5 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 TeaVin.ino.bootloader.bin 0x8000 TeaVin.ino.partitions.bin 0xe000 boot_app0.bin 0x10000 TeaVin.ino.bin`
