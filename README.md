# Electronic Component Inventory Manager

A demonstration application showcasing advanced C++ 17, Qt, and database programming skills.

## Features

- **Component Inventory Management**: Track components in inventory
- **Search & Filtering**: Search with category filtering
- **Low Stock Alerts**: Visual highlighting for items below threshold
- **Full CRUD Operations**: Add, edit, and delete components with validation

## Technical Highlights

### Architecture
- **MVC Pattern**: Clean separation of Model, View, and Controller logic
- **DAO Pattern**: DatabaseManager handles all data access operations

### Technologies
- **C++17**: Smart pointers (`std::unique_ptr`), STL containers, modern language features
- **Qt 6**: Widgets, Core, and SQL modules
- **SQLite 3**: Relational database for persistent storage


## Building

### Prerequisites
- Qt 6.x (with Widgets, Core, SQL modules)
- CMake 3.16+
- C++17 compatible compiler
- SQLite Qt driver

### Build Commands
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./ECIM
```

