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
## Использование

1. Скомпилируйте проект:
```bash
make
```
Будут созданы папки bin и build. Так же будет создан и помещен в папку build исполняемый файл processor.

2. Создайте рабочий файл с раширением .asm: <your_file_name>.asm
3. Скомпилируйте ваш файл:
```bash
./build/processor -compile <your_file_name>.asm bin.bin
```
где bin.bin - файл куда будет помещен машинный код (файл будет создан автоматически).
4. Запустите бинарник:
```bash
./buid/processor -run bin.bin
```

Так же имеется возможность использования с помощью предлоденного в проекте Makefile:
1. 
