# StatusMonitor

![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![CMake](https://img.shields.io/badge/CMake-3.10+-blue.svg)

StatusMonitor - это конфигурируемое приложение для мониторинга состояния операционной системы Linux. Оно позволяет отслеживать различные системные метрики, такие как загрузка процессора и использование памяти, с возможностью настройки периода сбора данных и способа их вывода. Архитектура приложения построена на динамических библиотеках, что позволяет легко добавлять новые метрики без перекомпиляции основного приложения.

## 🚀 Возможности

- 📊 Мониторинг загрузки процессора (всех ядер или выборочно)
- 💾 Отслеживание использования оперативной памяти
- ⚙️ Гибкая конфигурация через JSON файл
- 📝 Вывод данных в консоль и/или файл
- 🔄 Настраиваемый период сбора метрик
- 🛠️ Расширяемая архитектура для добавления новых метрик
- 📦 Поддержка Docker для простого развертывания
- 🔌 Динамическая загрузка метрик через shared libraries

## 📸 Пример использования

![StatusMonitor в действии](/docs/images/screenshot.png)

## 📋 Требования

- Linux OS
- CMake 3.10 или выше
- C++17 совместимый компилятор (GCC 7+ или Clang 5+)
- Google Test (для тестирования)
- Библиотека nlohmann/json

## 🛠️ Установка

### 📥 Стандартная установка

1. Клонируйте репозиторий:

```bash
git clone https://github.com/FFG99/statusMonitor
cd statusMonitor
```

2. Создайте директорию для сборки и выполните сборку:

```bash
mkdir build
cd build
cmake ..
make
```

## 📖 Использование

### ⚙️ Конфигурация

Программа использует JSON файл для настройки параметров мониторинга. Ниже приведено подробное описание структуры конфигурационного файла и пример его создания.

#### 📄 Структура конфигурационного файла

```json
{
    "settings": {
        "period": 5
    },
    "metrics": [
        {
            "type": "cpu",
            "library": "./cpu_metric.so",
            "config": {
                "cpu_ids": [0, 1, 2, 3]
            }
        },
        {
            "type": "memory",
            "library": "./memory_metric.so",
            "config": {
                "spec": ["MemTotal", "MemFree", "MemAvailable"]
            }
        }
    ],
    "outputs": [
        {
            "type": "console"
        },
        {
            "type": "file",
            "path": "metrics.log"
        }
    ]
}
```

#### ⚙️ Параметры конфигурации

- **settings.period**: Период сбора метрик в секундах (целое положительное число).
- **metrics**: Массив метрик для мониторинга.
  - **type**: Тип метрики ("cpu" или "memory").
  - **library**: Путь к динамической библиотеке метрики (например, "./cpu_metric.so").
  - **config**: Конфигурация конкретной метрики:
    - Для CPU:
      - **cpu_ids**: Массив идентификаторов ядер процессора для мониторинга.
    - Для памяти:
      - **spec**: Массив параметров памяти из "/proc/meminfo" для мониторинга (например, "MemTotal", "MemFree", "MemAvailable").
- **outputs**: Массив выходов для данных.
  - **type**: Тип выхода ("console" для вывода в консоль, "file" для записи в файл).
  - **path**: (только для типа "file") Путь к файлу для записи.

#### 📝 Пошаговое создание конфигурационного файла

1. **Создайте файл с расширением .json** (например, `config.json`).
2. **Настройте период сбора метрик**:

   ```json
   "settings": {
       "period": 5
   }
   ```
3. **Добавьте метрики для мониторинга**:

   - Для CPU:

     ```json
     {
         "type": "cpu",
         "library": "./cpu_metric.so",
         "config": {
             "cpu_ids": [0, 1, 2, 3]
         }
     }
     ```
   - Для памяти:

     ```json
     {
         "type": "memory",
         "library": "./memory_metric.so",
         "config": {
             "spec": ["MemTotal", "MemFree", "MemAvailable"]
         }
     }
     ```
4. **Настройте выходы для данных**:

   - Для вывода в консоль:

     ```json
     {
         "type": "console"
     }
     ```
   - Для записи в файл:

     ```json
     {
         "type": "file",
         "path": "metrics.log"
     }
     ```
5. **Объедините все части в один файл** как показано в примере структуры выше.

### 🚀 Запуск программы

```bash
./status_monitor /path/to/config.json
```

## 📊 Метрики

### 💻 CPU (cpu_metric.so)

- Отслеживание загрузки процессора
- Возможность выбора конкретных ядер
- Вывод в процентах использования

### 💾 Память (memory_metric.so)

- Общий объем памяти
- Свободная память
- Доступная память
- Все значения в МБ

## 🛠️ Разработка новых метрик

### 📝 Создание новой метрики

1. Создайте новый файл в директории `src/metrics/` (например, `NewMetric.cpp`)
2. Реализуйте интерфейс метрики
3. Добавьте сборку в CMakeLists.txt:

```cmake
add_library(new_metric SHARED
    src/metrics/NewMetric.cpp
)
target_include_directories(new_metric PUBLIC include)
target_link_libraries(new_metric PUBLIC nlohmann_json::nlohmann_json)
set_target_properties(new_metric PROPERTIES
    PREFIX ""
    OUTPUT_NAME "new_metric"
)
```

4. Скомпилируйте проект:

```bash
cd build
cmake ..
make
```

Новая метрика будет доступна как динамическая библиотека `new_metric.so`.

## 🧪 Тестирование

### 🚀 Запуск всех тестов

```bash
cd build
make metrics_test outputs_test
ctest
```

### 📊 Запуск тестов метрик

```bash
cd build
make metrics_test
./metrics_test
```

### 📝 Запуск тестов выходов

```bash
cd build
make outputs_test
./outputs_test
```

## 📁 Структура проекта

```
statusMonitor/
├── include/
│   ├── metrics/     # Заголовочные файлы метрик
│   └── output/      # Заголовочные файлы для вывода
├── src/
│   ├── metrics/     # Реализация метрик (динамические библиотеки)
│   └── output/      # Реализация выводов
├── tests/
│   ├── metrics/     # Тесты метрик
│   └── output/      # Тесты выводов
├── configs/         # Примеры конфигурационных файлов
├── CMakeLists.txt
└── README.md
```

## 📄 Лицензия

Этот проект распространяется под лицензией MIT. Подробности смотрите в файле [LICENSE](LICENSE).
