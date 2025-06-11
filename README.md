# Processor
В данном проекте реализован простейший виртуальный процессор на языке С с элементами С++. Для данного процессора был реализован собственный ассемблер стандарта DED32.

```asm
draw_circle:
    push cx
    push cx
    mul
    pop  cx   ; in cx square of radius 

    push ax
    push bx
    mul
    pop ex

    push 0
    pop dx

    cycle:

    call calc_dist_to_centre:
    push fx
    push cx
    ja notInCircle:
    jmp inCircle:

    back_in_cycle:

    pp   dx
    push dx
    push ex
    jb cycle:

    draw ax bx

    ret
```

**Пример кода на виртуальном ассемблере**

## Установка

1. Клонируйте репозиторий:
```bash
git clone https://github.com/Maksim-Sebelev/Processor.git
```
2. Перейдите в папку проекта:
```bash
cd Processor
```

## Компиляция проекта:
Выполните:
```bash
make
```
Будут созданы папки bin и build. Так же будут созданы и помещены в папку build исполняемые файлы `asm` и `.exe`.

## Использование
## Без make
1. Создайте рабочий файл с раширением .asm: <your_file_name>.asm
2. Скомпилируйте ваш файл:
```bash
./build/asm --src <your_asm_file_name>.asm --bin <your_bin_file_name>.bin
```
<your_bin_file_name>.bin - файл куда будет помещен машинный код (файл будет создан автоматически).

3. Запустите бинарник:
```bash
./buid/.exe - <your_bin_file_name>.binсобирается 
```
## С помощью make
Также имеется возможность работать с проектом с помощью предложенного в проекте Makefile:


Создайте папку для работы с ассемблерными и бинарными файлами (по умолчанию файлы будут браться из корней папки проекта). В файлах `make/make-asm.mk` и `make/make-proc.mk` присвойте переменным `ASM_DIR` и `BIN_DIR`соотвествующие значения.\
По умолчанию имена файлов это `programm.asm` и `programm.bin`. Если хотите свои имена, то поменяйте значени переменных `ASM_FILE` и `BIN_FILE` в файлах `make/make-asm.mk` и `make/make-proc.mk`.\
Когда проделаете эти шаги, работа с проектом сведется к простым командам:

```bash
make asm 
```
- компилирует ассемблерный файл и создает на его основ бинарный.

```bash
make proc
```
- запускает бинарник.

```bash
make run
```
- объединяет 2 предыдущие команды.

<br>
Так же есть удобные команды для работы с кодом проекта:

```Makefile
make rerun
make rebuild
make clean

make run_asm
make rerun_asm
make rebuild_asm
make clean_asm

make run_proc
make rerun_proc
make rebuild_proc
make clean_proc
```
Команды `rerun` выполняют 
```Makefile
make
make rerun
```

Команды `rebuild` выполняют
```Makefile
make clean
make
```

## С чем может работать ассемблер?
1) Стек на целых числах
2) ОЗУ
3) 6 регистров по 4 байта (ax, bx, cx, dx, ex, fx)

<br><br>

# Команды ассемблера:
## Основные команды для работы со стеком
## push

`push <arg>` - кладет arg на стек.\
arg = {int; reg; [RAM addr]}

  *Примеры использования команды с пояснением:*
```assembler

push 1       ; кладет на стек 1.

push '\n'    ; кладет на стек ASCII код '\n' (= 10).

push '\_'    ; кладет на стек ASCII код ' '. (!) В связи с особенностями токенизатора разработчики были вынуждены вместо ' ' использовать '\_'.

push ax      ; кладет на стек значения регистра ax.

push [100]   ; кладет на стек значения в сотой ячейке оперативной памяти.

push [ax]    ; кладет на стек значения в ячейке памяти под номером, численно равным значению ax.

push [ax+1]  ; кладет на стек значения в ячейке памяти под номером, численно равным значию (ax+1).
```
<br>

## pop

`pop <arg>` - присваивает аргументу значения верхнего элемента стека, удаляя верхушку стека.

arg = {reg, RAM addr}

  Примеры использования с пояснением:

```asm
  pop ax     ; присваивает ax значение верхнего элемента стека и удаляет его из стека.

  pop [ax+1] ; присваивает ячейки ОЗУ с номером (ax+1) значение верхнего элемента стека и удаляет его из стека.
```

## Арифметические команды (не имеют аргументов)
Пусть `a` - верхнее число на стеке, `b` - следующее за ним.\
Тогда все следующие команды удаляют `a` и `b` с вершины стека и взамен кладут:\
<br>
  `add` - a+b\
  `sub` - a-b\
  `mul` - a*b\
  `div` - a:b

## Около-арифметические команды
  ## pp/mm
  `pp <arg>` - прибавляет к аргументу 1.\
  `mm <arg>` - вычитает 1 из аргумента.

  arg = {reg}

## Прыжки:
  Все следующие команды имеют в качестве аргумента метку, указывающую место в коде, куда должен перейти процессор.

  Пусть `a` - второй сверху стека элемент, `b` - верхний. Тогда эти элементы будут удалены с вершины стека (кроме случае с `jmp`) и процессор совершит прыжок, если:

```asm
jmp ; при любых условиях
ja  ; a >  b
jae ; a >= b
jb  ; a <  b
jbe ; a <= b
je  ; a == b
jne ; a != b
```
<br>
<br>

*Пример использования с пояснением*
```asm
push 1
push 2

ja loop: ; процессор НЕ совершит прыжок по метке, так как 1 < 2

loop:
  <some code>
```
```asm
push 1
push 2

jb loop: ; процессор совершит прыжок по метке, так как 1 < 2

loop:
  <some code>
```

## Вызов и возврат функций
## call

`call <loop:>` - переходит по метке `loop` и кладет на стек адрес возврата (индекс в массиве кода, где находится сам `call`).

## ret

`ret` - делает прыжок по массиву кода на индекс, численно равный элементу на верхушке стека + 2 и удаляет верхушку стека.


<br>

**Примечание 1:**  Команды `call` и `ret` логично использовать только в связке друг с другом.\
**Примечание 2:** Следите за тем, чтобы перед `ret` внутри функции на стек **ничего не пушилось**, в противном случае случае `ret` воспримет это как и должен в связи с чем практически гарантируется **неопределенное поведение**.

## Консольный вывод
Следующие команды не имеют аргументов.
Если `a` - верхнее число на стеке, то 

`out`   - выведет в консоль `a`\
`outr`  - выведет в консоль `a` и удалит `a` с верхушки стека\
`outc`  - выведет в консоль char, ASCII код которого равен по модулю 256 значению `a`\
`outrc` - выведет в консоль char, ASCII код которого равен по модулю 256 значению `a` и удалит `a` с верхушки стека

# Графические команды
## rgba

`rgba <arg1> <arg2> <arg3> <arg4>` - кладет на верхушку стека целое число, кодирующее RGBA расцветку, заданную аргументами. Аргументы берутся по модулю `256`.

arg = {int, reg}

## draw

`draw <arg1> <arg2>` - создает окно рамером arg1 x arg2, в котором i-ый пиксель красится в цвет по RGBA, где RGBA-кодировка задана i-ым числом в виртуальной памяти процессора. Чтобы закрыть окно, нажмите `Space`.

arg = {int, reg},

<br>


# Структура проекта
```bash
.
├── assembler
│   ├── include
│   │   ├── assembler
│   │   │   └── assembler.hpp
│   │   ├── fileread
│   │   │   └── fileread.hpp
│   │   └── flags
│   │       └── flags.hpp
│   ├── main.cpp
│   └── src
│       ├── assembler
│       │   └── assembler.cpp
│       ├── fileread
│       │   └── fileread.cpp
│       └── flags
│           └── flags.cpp
│
├── processor
│   ├── include
│   │   ├── flags
│   │   │   └── flags.hpp
│   │   └── processor
│   │       └── processor.hpp
│   ├── main.cpp
│   └── src
│       ├── flags
│       │   └── flags.cpp
│       └── processor
│           └── processor.cpp
│
├── common
│   ├── include
│   │   ├── global
│   │   │   └── global_include.hpp
│   │   ├── lib
│   │   │   └── lib.hpp
│   │   ├── logger
│   │   │   └── log.hpp
│   │   └── stack
│   │       ├── hash.hpp
│   │       └── stack.hpp
│   └── src
│       ├── global
│       │   └── global.cpp
│       ├── lib
│       │   └── lib.cpp
│       ├── logger
│       │   ├── backgrounds
│       │   │   ├── anime_tyan_1.webp
│       │   │   ├── anime_tyan_2.webp
│       │   │   ├── anime_tyan_3.png
│       │   │   └── anime_tyan_main.jpg
│       │   └── log.cpp
│       └── stack
│           ├── hash.cpp
│           └── stack.cpp
│
├── make
│   ├── make-asm.mk
│   └── make-proc.mk
├── Makefile
│
├── examples
│   ├── example1
│   │   ├── HelloWorld.asm
│   │   └── HelloWorld.bin
│   ├── example2
│   │   ├── Math.asm
│   │   └── Math.bin
│   ├── example3
│   │   ├── Cycle.asm
│   │   └── Cycle.bin
│   ├── example4
│   │   ├── Factorial.asm
│   │   └── Factorial.bin
│   ├── example5
│   │   ├── circle.asm
│   │   └── circle.bin
│   └── Readme.md
│
├── assets
│   ├── example1_result.png
│   ├── example2_result.png
│   ├── example3_result.png
│   ├── example4_result.png
│   └── example5_result.png
│
└── README.md

37 directories, 46 files

```

<br>

# Примеры простейших программ реализованных на данном ассемблере:

В папке `/examples/` лежат 5 папок с примерами простейших программ. 
Для их запуска прочитайте файл `/examples/Readme.md`. 
