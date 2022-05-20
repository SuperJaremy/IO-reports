# Лабораторная работа 3

**Название:** "Разработка драйверов сетевых устройств"

**Цель работы:** получить знания и навыки разработки драйверов сетевых интерфейсов для операционной системы Linux.

## Описание функциональности драйвера

Созданный сетевой интерфейс перехватывает пакеты протокола UDP, содержащие конкретные данные (строку), приходящие на родительский интерфейс. Информация о перехваченных пакетах выводится в файл в /proc и в кольцевой буфер ядра.


## Инструкция по сборке

`$ make`

## Инструкция пользователя

`# insmod virt_net_if.ko link=<parent_if_name> str=<intercepted_string>`

## Примеры использования
Отправка сообщения на интерфейс

![Example 1](https://github.com/SuperJaremy/IO-reports/blob/master/lab3/examples/Example1.png)

Результат перехвата в кольцевом буфере

![Example 2](https://github.com/SuperJaremy/IO-reports/blob/master/lab3/examples/Example2.png)

Результат перехвата в файле в /proc

![Example 3](https://github.com/SuperJaremy/IO-reports/blob/master/lab3/examples/Example3.png)
