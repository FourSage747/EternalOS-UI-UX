Інженерський звіт щодо роботи певних клавіш, комбінацій клавіш, команд всередині eternal-ui в кооперативі з eternal-terminal

я маю три середовища розробки:

    - Моя хостова система Manjaro + compositor Hyprland - тут eternal-terminal працює прекрасно, всі команди виконуються, ніяких багів неспостерігається.
    - Моя хостова система Manjaro(Hyprland) + локально запущений мій власний композитор eternal-ui - тут все так само, термінал працює прекрасно, весь вивід з клавіатури читається, все чудово.
    - Повністю чиста і гола EternalOS яка запущена в віртуалці + мій кастомний композитор eternal-ui - от якраз в цьому середовищі мій термінал непрацює правильним чином. Є баги як із самим терміналом так із самою сесієї ui. Підозра: що проблема саме в середовищі самої EternalOS а не в коді eternal-ui чи eternal-terminal.

Проблема які спостерігаються зараз(стосується лише третього середовища, там де чиста EternalOS. В інших середовищах цих проблем немає):
- команда sudo чомусь повністю ігнорується в терміналі і невиводить вивід.
<--Було виправлено нижче--> - якшо натиснути клавішу "2" то ui сесія одразу вбивається і перезапускається.
- присутні ще певні баги з друкуванням клавіш, наприклад всередині термінала я пробував натискати "l" то вона недрукувалась. Але це не постійна проблема, якшо закрити і відкрити термінал заново то вже клавіша друкується.
- така комбінація як "Ctrl + C" повністю ігнорується в терміналі

Що було вже зроблено і перевірено:
- моя сесія ui стартує автоматично, при запуску з tty1 ось такою командою ``eternal-ui > ~/ui_debug.log 2>&1``. Тобто всі логи виводяться в файл. Але в ситуації коли натиснути "2" сесія одразу вбивається і якшо потім прочитати цей файл то там немає ніякого повідомлення про помилку. Підозра: ui невстигає нічого туди записати бо помирає раніше.
- Команди які були запущені після крашу ui:
    [eternal_engineer@eternal ~]$ coredumpctl list    

    No coredumps found.

    [eternal_engineer@eternal ~]$ journalctl -p err -b

    Oct 05 10:59:45 eternal kernel: virt/tdx: TDX not supported by the host platform

    Oct 05 10:59:45 eternal kernel: sgx: There are zero EPC sections.

    Oct 05 10:59:45 eternal systemd[1]: Failed to preset all unit: Unit dbus-org.freedesktop.ModemManager1.service is an unresolvable alias

- Команди які були виконані для перевірки sudo:
    sudo echo "Hello Eternal" > /tmp/sudo_test.txt
    cat /tmp/sudo_test.txt
    Ці команди показали результат що вивід дійсно записується у файл. Але при виклику sudo безпосередньо в терміналі, то вивід не виводится.

- Наступні команди показали таке:
    Якшо вводити команду через ssh, показує таке
    [eternal_engineer@eternal ~]$ tty
    /dev/pts/0
    Якшо просто всередині EternalOS в самому eternal-terminal, то показує
    [eternal_engineer@eternal ~]$ tty
    /dev/pts/1
    Наступні команди для перевірки прав доступу показали таке
    [eternal_engineer@eternal ~]$ ls -l /dev/pts/1
    crw------- 1 eternal_engineer tty 136, 1 Oct  9 13:41 /dev/pts/1
    [eternal_engineer@eternal ~]$ ls -l /dev/pts/0
    crw--w---- 1 eternal_engineer tty 136, 0 Oct  9 13:42 /dev/pts/0

- Наступна команда для тесту sudo:
    В віртуалці запущена EternalOS з моїм ui і терміналом. Далі я підключаюсь через ssh з хоста і виконую таку команду(далі в цей момент, я в самому eternal-terminal з графічною сесією виконую команду "sudo echo test"). Ось що ця команда показала в терміналі через ssh підключення, при цьому в eternal-terminal вивід нез'явився
    [eternal_engineer@eternal ~]$ sudo strace -p $(pidof eternal-terminal) -e trace=read,write
write(26, "s", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "s", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(26, "u", 1)                       = 1
read(26, "u", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(26, "d", 1)                       = 1
read(26, "d", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "o", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "o", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, " ", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, " ", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "e", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "e", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "c", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "c", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "h", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "h", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\3\0\0\0\0\0\0\0", 8)          = 8
write(26, "o", 1)                       = 1
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(26, "o", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, " ", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, " ", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "t", 1)                       = 1
read(6, "\3\0\0\0\0\0\0\0", 8)          = 8
read(26, "t", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "e", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "e", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "s", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "s", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "t", 1)                       = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "t", 4095)                     = 1
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
write(26, "\r", 1)                      = 1
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "\r\n\33[?2004l\r", 4095)      = 11
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(26, "\33]3008;start=312faef8f177485d8a4"..., 4095) = 231
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(26, "test\r\n", 4095)              = 6
read(26, "\33]3008;end=312faef8f177485d8a416"..., 4095) = 45
read(26, "\33]0;eternal_engineer@eternal:~\7\33"..., 4095) = 69
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
write(6, "\1\0\0\0\0\0\0\0", 8)         = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
read(6, "\1\0\0\0\0\0\0\0", 8)          = 8
read(6, "\2\0\0\0\0\0\0\0", 8)          = 8
^Cstrace: Process 7099 detached

- Наступна команда яка була проведена через підозру з попередньої команди з аномалією дивної послідовності символів:
    echo -e "\033]3008;start\a Hello Manjaro \033]3008;end\a"

    Спочатку передбачалося що текст повністю проковтнется і ми не побачимо нормального напису "Hello Manjaro" у будь якому з середовищ. Але я потестив цю команду у всіх трьох середовищах і повсюди побачив вивід нормального тексту "Hello Manjaro"

- Наступна команда яка була зроблена для тесту sudo, з метою забороти створювати ізольований PTY для sudo показала таке:
    Виконано через ssh
    [eternal_engineer@eternal ~]$ echo 'Defaults !use_pty' | sudo tee /etc/sudoers.d/01-nopty
    Defaults !use_pty

    Виконано в самому eternal-terminal всередині EternalOS
    [eternal_engineer@eternal ~]$ echo 'Defaults !use_pty' | sudo tee /etc/sudoers.d/01-nopty
    [eternal_engineer@eternal ~]$
    Тобто команда дала пустоту

- Наступна команда зроблена щоб перевірити розкладки клавіш в системі(виконано через ssh):
    [eternal_engineer@eternal ~]$ ls -l /usr/share/X11/xkb/rules
    total 790
    -rw-r--r-- 1 root root     91 Jun 21 07:24 README
    -rw-r--r-- 1 root root  35873 Jun 21 07:24 base
    -rw-r--r-- 1 root root  63231 Jun 21 07:24 base.extras.xml
    -rw-r--r-- 1 root root  47243 Jun 21 07:24 base.lst
    -rw-r--r-- 1 root root 257131 Jun 21 07:24 base.xml
    -rw-r--r-- 1 root root  31520 Jun 21 07:24 evdev
    -rw-r--r-- 1 root root  63231 Jun 21 07:24 evdev.extras.xml
    -rw-r--r-- 1 root root  47243 Jun 21 07:24 evdev.lst
    -rw-r--r-- 1 root root 257131 Jun 21 07:24 evdev.xml
    -rw-r--r-- 1 root root    873 Jun 21 07:24 xfree98
    -rw-r--r-- 1 root root   1085 Jun 21 07:24 xkb.dtd
    lrwxrwxrwx 1 root root      4 Jun 21 07:24 xorg -> base
    lrwxrwxrwx 1 root root      8 Jun 21 07:24 xorg.lst -> base.lst
    lrwxrwxrwx 1 root root      8 Jun 21 07:24 xorg.xml -> base.xml

- Наступна команда зроблена для перевірки помилки падіння ui при натискані "2"(виконано через ssh):
    sudo strace -p $(pidof eternal-ui) -o /home/eternal_engineer/ui_strace.log

    Останні рядки файлу показали таке:
    ppoll([{fd=4, events=POLLIN}, {fd=5, events=POLLIN}, {fd=6, events=POLLIN}, {fd=17, events=POLLIN}, {fd=18, events=POLLIN}, {fd=26, events=POLLIN}], 6, {tv_sec=0, tv_nsec=0}, NULL, 8) = 1 ([{fd=4, revents=POLLIN}], left {tv_sec=0, tv_nsec=0})
read(4, "\2\0\0\0\0\0\0\0", 8)          = 8
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=130708729}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=130874460}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=130985261}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131142566}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131249768}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131354703}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131460525}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131570378}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131679209}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=131848268}) = 0
ioctl(7, DRM_IOCTL_MODE_CURSOR, 0x7ffe7d9db000) = 0
ioctl(7, DRM_IOCTL_MODE_CURSOR, 0x7ffe7d9db000) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=132787542}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=132954327}) = 0
epoll_wait(26, [], 32, 0)               = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133219528}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133363401}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133496607}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133606105}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133709826}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=133889631}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134027230}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134142985}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134245444}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134347013}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134447651}) = 0
clock_gettime(CLOCK_MONOTONIC, {tv_sec=357863, tv_nsec=134551032}) = 0
ppoll([{fd=4, events=POLLIN}, {fd=5, events=POLLIN}, {fd=6, events=POLLIN}, {fd=17, events=POLLIN}, {fd=18, events=POLLIN}, {fd=26, events=POLLIN}], 6, {tv_sec=9, tv_nsec=866000000}, NULL, 8) = 1 ([{fd=18, revents=POLLIN}], left {tv_sec=9, tv_nsec=804736750})
+++ killed by SIGHUP +++

- Наступна команда це спроба пофіксити падіння графічної сесії при натискані "2", замінивши команду автозапуску графіки:
    Це зміна всередині файлу запуску графічної сесії самої ОС:
    ``sudo nano /usr/share/eternal/eternal-session/eternal-session.sh``

    nohup eternal-ui > /home/eternal_engineer/ui_debug.log 2>&1 &

    Безрезультатно!!!

- Наступна команда зі схожим акцентом для фіксу запуску графічної сесії:
    exec eternal-ui > /home/eternal_engineer/ui_debug.log 2>&1

    _|_Нарешті!!! Ця команда дала результат, після запуску графічна сесія перестала падати при натискані "2" і спокійно почала друкувати цифру 2 в термінал._|_

    Пояснення фіксу:
    -Щоб зрозуміти різницю, ось як це працює під капотом:

    -Звичайний запуск (без exec): Коли скрипт автозапуску доходить до -команди eternal-ui &, оболонка bash на tty1 створює власну копію --(через системний виклик fork), запускає в ній твій композитор, а -сама продовжує "висіти" у фоні як батьківський процес.

    -Запуск через exec: Це наказує ядру повністю замінити поточний -процес bash на eternal-ui. Жодного розгалуження процесів не -відбувається. Ядро просто вивантажує оболонку з оперативної -пам'яті і завантажує туди твій композитор, зберігаючи той самий -PID (ідентифікатор процесу).

    -Чому це вирішило проблему з крашами від клавіатури?
    -Через те, що оболонка bash перестає існувати, вона більше не може -перехоплювати сигнали від клавіатури, як-от Ctrl+C (або будь-які -інші переривання).

    -Раніше цей фоновий bash сидів на текстовій консолі tty1, ловив -натискання клавіш, які "протікали" повз Wayland, і вмирав від -них. Вмираючи, він тягнув за собою в могилу всі свої дочірні -процеси (зокрема й композитор), надсилаючи їм сигнал SIGHUP. -Тепер eternal-ui став єдиним, повноправним власником tty1, і -падати просто нічому.

- Наступна команда для діагностики і виявлення проблеми роботи команди sudo всередині eternal-terminal(виконано всередині eternal-terminal)
    ps -o pid,sess,tpgid,cmd -p $$

    Система Manjaro:
    PID    SESS   TPGID CMD
1520175 1520175 1520401 bash

    Система EternalOS:
    PID    SESS   TPGID CMD
    1088   1088    1102 bash

- Наступна команда теж для діагностики поведінки sudo(виконано всередині eternal-terminal)
    env | grep -E "XDG_SESSION_TYPE|DBUS_SESSION_BUS_ADDRESS|XDG_SEAT"

    **Система Manjaro:
    XDG_SEAT=seat0
    XDG_SESSION_TYPE=wayland
    DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus

    Система EternalOS:
    XDG_SEAT=seat0
    XDG_SESSION_TYPE=wayland
    XDG_SEAT_PATH=/org/freedesktop/DisplayManager/Seat0
    DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/1000/bus

- Наступна команда теж для дебагу sudo(виконано всередині eternal-terminal)
    loginctl session-status

    Система Manjaro:
    3 - the_eternal_engineer (1000)
  Since: Thu 2026-10-01 19:42:59 CEST; 1 day 8h ago
  State: active
 Leader: 1683 (gdm-session-wor)
   Seat: seat0; vc2
    TTY: tty2
 Remote: no
Service: gdm-password
   Type: wayland
  Class: user
   Idle: no
   Unit: session-3.scope
         ├─   1683 "gdm-session-worker [pam/gdm-password]"
         ├─   1956 /usr/lib/gdm-wayland-session --handle-registration /usr/bin/start-hyprland
         ├─   1968 /usr/bin/start-hyprland
         ├─   2021 Hyprland --watchdog-fd 4
         ├─   2063 Xwayland :0 -rootless -core -listenfd 59 -listenfd 60 -displayfd 99 -wm 96
         ├─   2276 kitty
         ├─   2281 /usr/bin/kitten __atexit__
         ├─   2285 /usr/bin/kitten __watch_conf__ 2276 100 /etc/xdg/kitty/kitty.conf /home/the_eternal_engineer/.config/kitty/kitty.conf
         ├─  25902 /usr/lib/firefox/firefox
         ├─  26207 /usr/lib/firefox/crashhelper 25902 13 /tmp/ 15
         ├─  26282 /usr/lib/firefox/firefox -contentproc -ipcHandle 0 -signalPipe 1 -initialChannelId {3ec9bb3c-5a8f-4785-acd9-475a1f0206a8} -parentPid 25902 -greomni /usr/lib/firefox/omni.ja >

         ├─  26380 "/usr/lib/firefox/firefox -contentproc -parentBuildID 20260721043307 -prefsHandle 0:40213 -prefMapHandle 1:291252 -sandboxReporter 2 -chrootClient 3 -ipcHandle 4 -initialCha>

         ├─  26406 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:40358 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  26415 "/usr/lib/firefox/firefox -contentproc -parentBuildID 20260721043307 -prefsHandle 0:40358 -prefMapHandle 1:291252 -sandboxReporter 2 -chrootClient 3 -ipcHandle 4 -initialCha>

         ├─  26541 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:34663 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  26560 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:34663 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  26678 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:40413 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  26761 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48447 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  26967 "/usr/lib/firefox/firefox -contentproc -parentBuildID 20260721043307 -sandboxingKind 0 -prefsHandle 0:54199 -prefMapHandle 1:291252 -sandboxReporter 2 -chrootClient 3 -ipcHa>

         ├─  28056 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48809 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  39615 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48809 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  40437 kitty
         ├─  40449 /usr/bin/kitten __atexit__
         ├─  40461 /usr/bin/kitten __watch_conf__ 40437 100 /etc/xdg/kitty/kitty.conf /home/the_eternal_engineer/.config/kitty/kitty.conf
         ├─  41212 kitty
         ├─  41218 /usr/bin/kitten __atexit__
         ├─  41229 /usr/bin/kitten __watch_conf__ 41212 100 /etc/xdg/kitty/kitty.conf /home/the_eternal_engineer/.config/kitty/kitty.conf
         ├─  65131 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48906 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─  96759 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48973 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 238858 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:48971 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 244792 bwrap --unshare-all --die-with-parent --chdir / --ro-bind /usr /usr --dev /dev --ro-bind-try /etc/ld.so.cache /etc/ld.so.cache --ro-bind-try /nix/store /nix/store --tmpfs /t>

         ├─ 244794 bwrap --unshare-all --die-with-parent --chdir / --ro-bind /usr /usr --dev /dev --ro-bind-try /etc/ld.so.cache /etc/ld.so.cache --ro-bind-try /nix/store /nix/store --tmpfs /t>

         ├─ 244795 /usr/lib/glycin-loaders/2+/glycin-image-rs --dbus-fd 226
         ├─ 295766 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49053 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 304685 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49053 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 694523 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49053 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 803349 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49138 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 865225 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49140 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─ 954972 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49187 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─1125184 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49188 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>

         ├─1368809 kitty
         ├─1368825 /usr/bin/kitten __atexit__
         ├─1368838 /usr/bin/kitten __watch_conf__ 1368809 100 /etc/xdg/kitty/kitty.conf /home/the_eternal_engineer/.config/kitty/kitty.conf
         └─1528959 "/usr/lib/firefox/firefox -contentproc -isForBrowser -prefsHandle 0:49482 -prefMapHandle 1:291252 -jsInitHandle 2:156236 -parentBuildID 20260721043307 -sandboxReporter 3 -ch>


Oct 09 14:24:40 theeternalengineer /usr/lib/gdm-wayland-session[1108129]: >                   Using 0, ignoring 0
Oct 09 14:24:40 theeternalengineer /usr/lib/gdm-wayland-session[1108129]: Errors from xkbcomp are not fatal to the X server
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: The XKEYBOARD keymap compiler (xkbcomp) reports:
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: > Warning:          Unsupported maximum keycode 709, clipping.
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: >                   X11 cannot support keycodes above 255.
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: > Warning:          Virtual modifier Hyper multiply defined
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: >                   Using 0, ignoring 0
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: > Warning:          Virtual modifier ScrollLock multiply defined
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: >                   Using 0, ignoring 0
Oct 12 13:57:07 theeternalengineer /usr/lib/gdm-wayland-session[1318136]: Errors from xkbcomp are not fatal to the X server


    **Система EternalOS:
    2 - eternal_engineer (1000)
   Since: Mon 2026-10-12 16:25:06 UTC; 18min ago
   State: active
  Leader: 964 (sddm-helper)
    Seat: seat0; vc1
     TTY: tty1
  Remote: no
 Service: sddm
    Type: wayland
   Class: user
    Idle: no
    Unit: session-2.scope
          ├─ 964 /usr/lib/sddm/sddm-helper --socket /tmp/sddm-auth-4050efee-ba20-4d83-93a8-b71a31acc4f4 --id 1 --s
tart /usr/share/eternal/eternal-session/eternal-session.sh --user eternal_engineer
          ├─ 989 /bin/bash /usr/share/eternal/eternal-session/eternal-session.sh
          ├─1008 eternal-ui
          ├─1216 eternal-terminal
          ├─1221 bash
          ├─1232 loginctl session-status
          └─1233 more

Oct 12 16:43:49 eternal eternal-terminal[1216]: CHAR: 't' CURSOR: 0 50
Oct 12 16:43:49 eternal eternal-terminal[1216]: CHAR: 'u' CURSOR: 0 51
Oct 12 16:43:49 eternal eternal-terminal[1216]: CHAR: 's' CURSOR: 0 52
Oct 12 16:43:49 eternal eternal-terminal[1216]: RENDER UPDATE
Oct 12 16:43:51 eternal eternal-terminal[1216]: SEND: "\r"
Oct 12 16:43:51 eternal eternal-terminal[1216]: PTY WRITE: "0d"
Oct 12 16:43:51 eternal eternal-terminal[1216]: CHAR: '\xd' CURSOR: 0 53
Oct 12 16:43:51 eternal eternal-terminal[1216]: CHAR: '\xa' CURSOR: 0 0
Oct 12 16:43:51 eternal eternal-terminal[1216]: CSI: "?2004l"
Oct 12 16:43:51 eternal eternal-terminal[1216]: CHAR: '\xd' CURSOR: 1 0


- Наступна команда для діагностики параметрів монтування PTY теж стосується sudo(виконано всередині eternal-terminal)
    mount | grep devpts

    Система Manjaro:
    devpts on /dev/pts type devpts (rw,nosuid,noexec,relatime,gid=5,mode=620,ptmxmode=000)

    Система EternalOS:
    devpts on /dev/pts type devpts (rw,nosuid,noexec,relatime,gid=5,mode=600,ptmxmode=000)

- Наступна команда по змінам прав доступу в надіях що це виправить поведінку sudo(виконано через ssh)
    Виконане ручне перемонтування системи, щоб кожен новий термінал створювався з правильним  правами:
    sudo mount -o remount,mode=620,gid=5 /dev/pts

    Система EternalOS(всередині eternal-terminal):
    [eternal_engineer@eternal ~]$ ls -l /dev/pts/
    total 0
    crw--w---- 1 eternal_engineer tty 136, 0 Oct 12 17:14 0
    crw--w---- 1 eternal_engineer tty 136, 1 Oct 12 17:12 1
    c--------- 1 root             root  5, 2 Oct 12 16:17 ptmx
    [eternal_engineer@eternal ~]$ sudo echo "test"
    [eternal_engineer@eternal ~]$

    Врезультаті sudo так і нічого невивів!!!

- НОВЕ ВІДКРИТТЯ: виявилось що якшо я на системі Manjaro відкрию eternal-terminal і підключуся з нього через ssh всередину EternalOS(підключення пройшло успішно), то в ньому взагалі відмовляються виводитись будь які команди. Попри те що при звичайному користувані eternal-terminal на системі Manjaro всі команди працюють відмінно.

- НОВЕ ВІДКРИТТЯ: для того щоб переконатись що виключно мій термінал(eternal-terminal) має проблеми з відпрацюванням команди sudo, в систему було встановлено інший термінал(foot), виявилось що він прекрасно відпрацьовує команди з sudo. Тобто проблема скоріш за все в коді мого термінала