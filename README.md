# ElectraBase Pro

**Professional Electronic Component Management System**

A demonstration application showcasing advanced C++, Qt, and database programming skills for ECAD/EDA systems.

## Features

- **Component Inventory Management**: Track resistors, capacitors, inductors, ICs, transistors, diodes, connectors, and more
- **Smart Search & Filtering**: Real-time search with category filtering
- **Low Stock Alerts**: Visual highlighting for items below threshold (< 10 units)
- **Full CRUD Operations**: Add, edit, and delete components with validation
- **Sample Data**: Pre-populated database with common electronic components

## Technical Highlights

### Architecture
- **MVC Pattern**: Clean separation of Model, View, and Controller logic
- **DAO Pattern**: DatabaseManager handles all data access operations
- **OOP with Polymorphism**: Abstract `Component` base class with `PassiveComponent` and `ActiveComponent` derived classes

### Technologies
- **C++17**: Smart pointers (`std::unique_ptr`), STL containers, modern language features
- **Qt 6**: Widgets, Core, and SQL modules
- **SQLite 3**: Relational database for persistent storage

### Qt Skills Demonstrated
- **Custom `QAbstractTableModel`**: Proper data-to-UI mapping with roles
- **`QStyledItemDelegate`**: Custom rendering for low stock highlighting
- **`QSortFilterProxyModel`**: Real-time filtering and sorting
- **Form Validation**: Input validation with user feedback
- **`QSplitter`**: Resizable sidebar layout

## Project Structure

```
src/
├── main.cpp                      # Application entry point
├── models/
│   ├── Category.h                # Category enum with utilities
│   ├── Component.h/cpp           # Abstract base class
│   ├── PassiveComponent.h/cpp    # Resistor, Capacitor, Inductor
│   └── ActiveComponent.h/cpp     # IC, Transistor, Diode
├── database/
│   └── DatabaseManager.h/cpp     # DAO for SQLite operations
└── ui/
    ├── MainWindow.h/cpp          # Main application window
    ├── ComponentTableModel.h/cpp # Custom table model
    ├── ComponentDialog.h/cpp     # Add/Edit dialog
    └── LowStockDelegate.h/cpp    # Custom item delegate
```

## Database Schema

```sql
CREATE TABLE inventory (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    manufacturer TEXT,
    type TEXT NOT NULL,          -- Component category
    quantity INTEGER DEFAULT 0,
    param_1 REAL,                -- Value/Voltage
    param_2 TEXT,                -- Package/Pin count
    extra_data TEXT              -- Unit/Datasheet URL
);
```

## Building

### Prerequisites
- Qt 6.x (with Widgets, Core, SQL modules)
- CMake 3.16+
- C++17 compatible compiler
- SQLite Qt driver

### Fedora/RHEL
```bash
sudo dnf install qt6-qtbase-devel qt6-qtbase-sqlite cmake gcc-c++
```

### Ubuntu/Debian
```bash
sudo apt install qt6-base-dev libqt6sql6-sqlite cmake g++
```

### Build Commands
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./ElectraBasePro
```

## Usage

1. **View Inventory**: The main table shows all components with sorting and filtering
2. **Search**: Type in the search box to filter components in real-time
3. **Filter by Category**: Use the dropdown or sidebar to filter by component type
4. **Add Component**: Click "Add" or press Ctrl+N to add a new component
5. **Edit Component**: Double-click a row or select and click "Edit"
6. **Delete Component**: Select a row and click "Delete"
7. **Low Stock View**: Click "Low Stock Items" in sidebar to see items needing restock

## Component Types

### Passive Components
- **Resistors**: Value in Ω with SI prefixes (kΩ, MΩ)
- **Capacitors**: Value in F with SI prefixes (pF, nF, μF)
- **Inductors**: Value in H with SI prefixes (μH, mH)

### Active Components
- **ICs**: Operating voltage, pin count, datasheet link
- **Transistors**: Voltage rating, package type
- **Diodes**: Forward voltage, package

## License

MIT License - Free for educational and commercial use.

---

*Built to demonstrate professional C++/Qt development skills for ECAD/EDA teams.*
