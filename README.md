# Processor
в данном проекте реализован простейший виртуальный процессор на языке С с элементами С++. Для данного процессора был реализован собственный ассемблер стандарта DED32.

![Processor Logo](https://github.com/Maksim-Sebelev/Processor/blob/main/assets/asm_code.png)

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
Будут созданы папки bin и build. Так же будет создан и помещен в папку build исполняемый файл processor.

## Использование
## Без make
1. Создайте рабочий файл с раширением .asm: <your_file_name>.asm
2. Скомпилируйте ваш файл:
```bash
./build/processor -compile <your_asm_file_name>.asm <your_bin_file_name>.bin
```
где <your_bin_file_name>.bin - файл куда будет помещен машинный код (файл будет создан автоматически).
3. Запустите бинарник:
```bash
./buid/processor -run <your_bin_file_name>.bin
```
## С помощью make
Также имеется возможность использования с помощью предложенного в проекте Makefile:
1) С изменением Makefile
Для удобства предлагается изменить некоторые переменные в Makefile:
1. Папка для ваших .asm файлов - `ASM_DIR`. Присвойте ей желаемое значение.
2. Папка для ваших бинарных файлов - `CODE_SIR`.
3. Если вы планируете работать с одним ассемблерным файлов, то рекомендуетс присвоить желаемые значения переменным `ASM_FILE` и `BIN_FILE`, для ваших ассемблерного и бинарного файла.
4. 
В таком случае работа с процессором сводится к 3 командам:
  
```bash
make proc
```
- компилириует ассемблерный файл и запускает полученный бинарник.

```bash
make compile
```
- компилирует ассембленый файл, но не запускает бинарник.

```bash
make run
```
- запускает бинарник.

2) Без изменения Makefile
Так же используйте 3 прошлые команды, но если вам нужны ваши собственная именая директорий и файлов при вызове make укажите конкретные значения для соотвествующих переменных:
```bash
make ASM_DIR=<your_rep> BIN_DIR=<your_rep> ASM_FILE=<your_file> BIN_FILE=<your_file> proc
```

## С чем может рабоать ассемблер?
1) Стек на целых числах
2) ОЗУ
3) 4 Регистра по 4 байта (ax, bx, cx, dx)
.## Команды ассемблера:
1) операции со стеком:
  1.1) `push <arg>` - кладет arg на верхушку стека.
  arg = {int; reg; RAM addr}

  примеры:

  `push 1` - кладет на стек 1.

  `push ax` - кладет на стек значения регистра ax.
  
  `push [100]`  - кладет на стек значения в сотой ячейке оперативной памяти.

  `push [ax]`   - кладет на стек значения в ячейке памяти под номером, численно равным значению ax.

  `push [ax+1]` - кладет на сте значения в ячейке памяти под номером, численно равным значию (ax+1).

  1.2) 
  `pop <arg>` - присваивает аргументу значения верхнего элемента стека, удаляя верхушку стека.
  arg = {reg, RAM addr}

  примеры:
  `pop ax`     - присваивает ax значение верхнего элемента стека и удаляет его из стека.

  `pop [ax+1]` - присваивает ячейки ОЗУ с номером (ax+1) значение верхнего элемента стека и удаляет его из стека.
   
3) арифметические команы: 
  `add`
  - складывает 2 верхних числа на стеке и результат кладет на стек.
  `sub`
  - вычитает из самого верхнего числа не стеке, второе верхее число и результат кладет на стек.
  `mul`
  -  умножает 2 верхних числа на стеке и результат кладет на стек.
  `div`
  - делит самое верхнее число не стеке, на второе верхее число и результат кладет на стек.
5) прыжки:
  совершает "прыжок" в место куда указывает аргумент (метка), при некоторых условиях.

  `jmp` - при любых условиях.

  `ja` - если верхний элемент стека больше второго сверху.

  `jae` - если верхний элемент стека больше либо равен второго сверху.

  `jb` - если верхний элемент стека меньше второго сверху.

  `jbe` - если верхний элемент стека меньше либо равен второго сверху.

  `je` - если верхний элемент стека равен второму сверху.

  `jne` - если верхний элемент стека не равен второму сверху.


  примеры:
  `jmp loop:
   push 1
   loop:`
  - пропустит команду push 1.
   `push 1
   push 2
   ja loop:
   push 3
   loop:`
  - пропустит команду push 3.
   `push 2
   push 1
   ja loop:
   push 3
   loop:`
  - не пропустит команду push 3.
   


