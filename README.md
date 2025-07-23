# 🚌 Transport Catalogue

**Transport Catalogue** — учебный проект на C++17, реализующий консольное приложение для работы с маршрутами общественного транспорта. Программа поддерживает:

- построение маршрутов;
- отображение маршрутов на SVG-карте;
- обработку JSON-запросов;
- определение кратчайших путей с учётом ожиданий.

---

## 🚀 Возможности

- 📥 Чтение входных данных из JSON
- 🧭 Поиск кратчайшего маршрута между остановками
- 🗺 Генерация карты маршрутов в SVG-формате
- ⚙️ Разделение на режимы: `make_base` и `process_requests`
- ❌ Без сторонних библиотек — работа с JSON и SVG реализована вручную

---

## 🔧 Сборка

### 📦 Требования

- C++17
- CMake ≥ 3.10
- Поддержка std::filesystem (обычно есть в GCC ≥ 8, MSVC ≥ 2019)

### 🔨 Инструкция

```bash
git clone https://github.com/aksi321/cpp-transport-catalogue.git
cd cpp-transport-catalogue/transport-catalogue

mkdir build && cd build
cmake ..
cmake --build .
