# Примеры

Перед тем, как запускать примеры необходимо скомпелировать проект и поместить исполняемые файлы в папку `~/examples/build/`.\
Для этого перейдите в папку `~/Src` и выполните:
```bash
make examples
```
После перейдите в папку `~/examples/`.

Если вы хотите посмотреть как работают примеры, просто пропишите в консоли следующие команды для каждого примера соответственно:

```bash
bash run/run_example_1.bash
bash run/run_example_2.bash
bash run/run_example_3.bash
bash run/run_example_4.bash
bash run/run_example_5.bash
```

# Ожидаемый результат

**Если вывод программы на вашем компьютере будет отличаться, то вероятно программа работает некоректно.**\
Вероятные причины:\
-несовместимость с вашим устройствой и/или ОС.\
-изменение исходных файлов (попробуйте переустановить программу).

## example1
![Example1 result](../assets/example1_result.png)

**вывод в консоль**:
```bash
Hello, World!
Hello, World!
```
<br>

## example2
![Example1 result](../assets/example2_result.png)

**вывод в консоль:**
```bash
5
```
<br>

## example3
![Example1 result](../assets/example3_result.png)

**вывод в консоль:**
```bash
0 1 2 3 4 5 6 7 8 9 10 
10 9 8 7 6 5 4 3 2 1 0
```
<br>


## example4
![Example1 result](../assets/example4_result.png)

**вывод в консоль:**
```bash
5040
```
<br>


## example5
**Создастся окно размером 600x800 и будет выведена картинка:**

![Example1 result](../assets/example5_result1.png)
<br>

**После нажатия на `Space` окно закроется и в консоль будет выведен `0`, как знак успешного завершения программы.**

<br>
![Example1 result](../assets/example5_result2.png)

**вывод в консоль:**
```bash
0
```
