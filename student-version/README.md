# Лабораторная работа MiniDex: студенческая версия

MiniDex — учебное консольное приложение на C++17 для анализа безопасности кода. Программа читает небольшой текстовый формат индекса документов, разбирает записи, выполняет базовую валидацию, строит отчеты и может извлекать полезную нагрузку записей в отдельные файлы.

Эта версия предназначена для самостоятельной работы студентов. В проекте есть дефекты безопасности и ошибки обработки входных данных, но их список и точные места намеренно не указаны.

## Цель работы

1. Описать поверхность атаки приложения.
2. Собрать проект и проверить штатные сценарии запуска.
3. Провести статический анализ кода.
4. Написать unit- и integration-тесты для подозрительных участков.
5. Реализовать AFL++ fuzz harnesses для функций парсинга.
6. Сопоставить результаты SAST, тестирования и фаззинга.

## Архитектура

- `src/Cli.cpp`: разбор аргументов командной строки и профиля из переменной окружения
- `src/FileReader.cpp`: чтение входного файла с ограничением по размеру
- `src/Parser.cpp`: разбор заголовка MDX, записей, полезной нагрузки и тегов
- `src/Pipeline.cpp`: валидация, преобразование данных и построение отчета
- `src/Reporter.cpp`: запись отчета и извлечение полезной нагрузки
- `tests/`: стартовая заготовка тестов
- `fuzz/`: стартовые заготовки fuzz harnesses
- `samples/`: seed-файлы для ручных запусков, тестов и фаззинга

## Формат файла

Файлы MiniDex используют построчный формат:

```text
MDX1
VERSION 1
COUNT 1
REC DOC 5 12 10 1
alpha
hello world!
TAG 3 lab
```

Строка `REC` имеет следующий вид:

```text
REC KIND NAME_LEN DATA_LEN SCORE TAG_COUNT
```

Следующая строка содержит имя записи. После нее идет одна строка полезной нагрузки, а затем `TAG_COUNT` строк с тегами.

## Сборка

Локально:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

В лабораторном Docker-образе:

```sh
docker run --rm -v /home/nva/lab4/student-version:/workspace -w /workspace deeezy/secure-cpp-lab cmake -S . -B build-docker
docker run --rm -v /home/nva/lab4/student-version:/workspace -w /workspace deeezy/secure-cpp-lab cmake --build build-docker
docker run --rm -v /home/nva/lab4/student-version:/workspace -w /workspace deeezy/secure-cpp-lab ctest --test-dir build-docker --output-on-failure
```

## Запуск

```sh
./build/minidex --input samples/valid_basic.mdx --mode summary
./build/minidex --input samples/valid_tags.mdx --mode dump --output report.txt
./build/minidex --input samples/valid_basic.mdx --mode extract --extract-dir extracted
```

## Статический анализ

Рекомендуемые инструменты:

- `clang-tidy`
- `cppcheck`
- `CodeQL`
- `semgrep`
- `flawfinder`

Пример запуска `cppcheck`:

```sh
cppcheck --enable=warning,style,performance,portability --std=c++17 \
  --suppress=missingIncludeSystem -I include src include tests fuzz
```

В Docker-образе:

```sh
docker run --rm -v /home/nva/lab4/student-version:/workspace -w /workspace deeezy/secure-cpp-lab \
  cppcheck --enable=warning,style,performance,portability --std=c++17 \
  --suppress=missingIncludeSystem -I include src include tests fuzz
```

## Unit-тесты

Файл `tests/test_parser_validation.cpp` содержит минимальный smoke-тест. Расширьте его так, чтобы проверить:

- корректные входные файлы
- некорректные заголовки и счетчики записей
- граничные значения длин
- ошибки валидации
- обработку выходных путей
- поведение при пустых или необычных payload-данных

## Фаззинг AFL++

В директории `fuzz/` есть две заготовки:

- `fuzz_parse_index.cpp`
- `fuzz_decode_record.cpp`

Заполните `LLVMFuzzerTestOneInput`, выбрав подходящие функции из публичного API парсера. Harness должен быть маленьким, воспроизводимым и не должен зависеть от полного запуска `main()`.

Пример сборки под AFL++:

```sh
CC=afl-clang-fast CXX=afl-clang-fast++ cmake -S . -B build-afl
cmake --build build-afl
mkdir -p findings
afl-fuzz -i samples -o findings -- ./build-afl/fuzz_parse_index @@
```

## Отчет

В отчете опишите:

1. Поверхность атаки приложения.
2. Найденные SAST-предупреждения и вашу оценку их достоверности.
3. Тесты, которые вы добавили, и какие дефекты они демонстрируют.
4. Конфигурацию фаззинга и найденные crash/hang/UB-сценарии.
5. Предложенные исправления и приоритеты устранения.
