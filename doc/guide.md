# 开发指南

## 1. 项目架构

```
Property_Management_System/
├── src/                        # 源代码目录
│   ├── auth/                   # 用户认证相关代码
│   │   ├── auth.c              # 认证核心功能
│   │   └── tokens.c            # 令牌管理功能
│   ├── db/                     # 数据库操作相关代码
│   │   ├── database.c          # 数据库连接管理
│   │   ├── db_init.c           # 数据库初始化
│   │   ├── db_query.c          # 数据库查询
│   │   └── db_utils.c          # 数据库工具函数
│   ├── models/                 # 数据模型相关代码
│   │   ├── apartment.c         # 房屋模型
│   │   ├── building.c          # 楼宇模型
│   │   ├── service.c           # 服务模型
│   │   ├── transaction.c       # 交易模型
│   │   └── user.c              # 用户模型
│   ├── ui/                     # 用户界面相关代码
│   │   ├── ui_admin.c          # 管理员界面
│   │   ├── ui_login.c          # 登录界面
│   │   ├── ui_main.c           # 主界面
│   │   ├── ui_owner.c          # 业主界面
│   │   └── ui_staff.c          # 服务人员界面
│   ├── utils/                  # 工具函数
│   │   ├── console.c           # 控制台操作
│   │   ├── file_ops.c          # 文件操作
│   │   └── utils.c             # 通用工具函数
│   └── main.c                  # 程序入口点
├── include/                    # 头文件目录
│   ├── auth/                   # 认证模块头文件
│   ├── db/                     # 数据库模块头文件
│   ├── models/                 # 数据模型头文件
│   ├── ui/                     # 界面模块头文件
│   └── utils/                  # 工具模块头文件
...
└── $HOME/.property_management/ # 数据库文件存放目录
```

## 2. 核心模块

### 2.1 认证模块 (auth)

认证模块负责用户登录和权限验证。

#### 主要文件:

- `auth.h/auth.c` - 定义用户认证和权限验证的函数

#### 主要功能:

- 用户登录验证
- 用户权限检查

#### 示例函数:

```c
// 用户登录
LoginResult login(Database *db, const char *username, const char *password);

// 验证权限
bool validate_permission(Database *db, const char *user_id, UserType user_type, int min_weight);
```

### 2.2 数据库模块 (db)

数据库模块提供与 SQLite 数据库交互的核心功能。

#### 主要文件:

- `database.h/database.c` - 数据库连接和基本操作
- `db_init.h/db_init.c` - 数据库初始化和表结构创建
- `db_query.h/db_query.c` - 数据库查询操作
- `db_utils.h/db_utils.c` - 数据库工具函数

#### 主要功能:

- 数据库连接管理
- 表结构初始化
- 事务处理
- 通用查询接口

#### 示例函数:

```c
// 打开数据库连接
Database *db_open(const char *db_path);

// 初始化数据库表结构
bool db_init_tables(Database *db);

// 执行事务
bool db_transaction(Database *db, const char **sqls, int count);
```

### 2.3 数据模型模块 (models)

数据模型模块定义了系统中的各种实体及其操作。

#### 主要文件:

- `user.h/user.c` - 用户模型（管理员、服务人员、业主）
- `building.h/building.c` - 楼宇模型
- `apartment.h/apartment.c` - 房屋模型
- `service.h/service.c` - 服务请求模型
- `transaction.h/transaction.c` - 财务交易模型

#### 主要功能:

- CRUD 操作（创建、读取、更新、删除）
- 关联查询
- 数据验证

#### 示例函数:

```c
// 创建房屋
bool create_apartment(Database *db, const char *token, Apartment *apartment);

// 获取楼宇信息
bool get_building_by_id(Database *db, const char *building_id, Building *building);

// 创建服务请求
bool create_service_request(Database *db, const char *token, Service *service);

// 记录交易
bool record_transaction(Database *db, const char *token, Transaction *transaction);

// 修改业主信息
bool update_owner(Database *db, const char *token, Owner *owner);
```

### 2.4 用户界面模块 (ui)

用户界面模块负责与用户交互，显示菜单和处理用户输入。

#### 主要文件:

- `ui_main.h/ui_main.c` - 主界面
- `ui_login.h/ui_login.c` - 登录界面
- `ui_admin.h/ui_admin.c` - 管理员界面
- `ui_staff.h/ui_staff.c` - 物业服务人员界面
- `ui_owner.h/ui_owner.c` - 业主界面

#### 主要功能:

- 菜单显示
- 用户输入处理
- 结果展示

#### 示例函数:

```c
// 显示主界面
void show_main_screen(Database *db, const char *token, UserType user_type);

// 显示登录界面
bool show_login_screen(Database *db, char *token);

// 显示错误信息
void show_error(const char *message);

// 显示确认对话框
bool show_confirmation(const char *message);
```

### 2.5 工具模块 (utils)

工具模块提供各种辅助功能，支持系统其他部分的运行。

#### 主要文件:

- `utils.h/utils.c` - 通用工具函数
- `file_ops.h/file_ops.c` - 文件操作函数
- `console.h/console.c` - 控制台 UI 辅助函数

#### 主要功能:

- 字符串处理
- 文件读写
- 屏幕绘制和格式化
- 错误处理

#### 示例函数:

```c
// 安全字符串复制
void safe_strcpy(char *dest, const char *src, size_t size);

// 写入文件
bool write_to_file(const char *path, const char *content);

// 清屏
void clear_screen();

// 格式化显示表格
void print_table(const char **headers, const char ***data, int rows, int cols);
```

## 3. 用户角色

系统支持三种类型的用户，每种用户具有不同的权限和功能：

1. **管理员 (USER_ADMIN)**

   - 添加/修改/删除所有用户信息
   - 管理小区、楼宇和房屋信息
   - 分配服务人员到楼宇
   - 查看系统中所有数据

2. **物业服务人员 (USER_STAFF)**

   - 查看和更新自己负责的楼宇信息
   - 修改房屋的物业费信息
   - 向业主发送通知

3. **业主 (USER_OWNER)**
   - 查看自己的房屋信息
   - 查看物业费账单
   - 更新个人信息

## 4. 数据库结构

系统使用 SQLite 数据库存储数据。主要表格包括：

### 4.1 用户相关表

#### admins 表 (管理员信息)

```sql
CREATE TABLE admins (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    password_hash TEXT NOT NULL,
    weight INTEGER DEFAULT 1
);
```

#### staff 表 (服务人员信息)

```sql
CREATE TABLE staff (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    phone TEXT,
    password_hash TEXT NOT NULL,
    service_type TEXT,
    weight INTEGER DEFAULT 2
);
```

#### owners 表 (业主信息)

```sql
CREATE TABLE owners (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    phone TEXT,
    password_hash TEXT NOT NULL,
    notification_required INTEGER DEFAULT 1,
    weight INTEGER DEFAULT 3
);
```

### 4.2 物业相关表

#### communities 表 (小区信息)

```sql
CREATE TABLE communities (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL
);
```

#### buildings 表 (楼宇信息)

```sql
CREATE TABLE buildings (
    id TEXT PRIMARY KEY,
    building_number TEXT NOT NULL,
    community_id TEXT NOT NULL,
    floors INTEGER DEFAULT 0,
    units_per_floor INTEGER DEFAULT 0,
    FOREIGN KEY (community_id) REFERENCES communities(id)
);
```

#### apartments 表 (房屋信息)

```sql
CREATE TABLE apartments (
    id TEXT PRIMARY KEY,
    building_id TEXT NOT NULL,
    floor INTEGER NOT NULL,
    unit INTEGER NOT NULL,
    area REAL NOT NULL,
    occupied INTEGER DEFAULT 0,
    property_fee_balance REAL DEFAULT 0.0,
    fee_paid INTEGER DEFAULT 0,
    owner_id TEXT,
    FOREIGN KEY (building_id) REFERENCES buildings(id),
    FOREIGN KEY (owner_id) REFERENCES owners(id)
);
```

#### parking_spaces 表 (停车位信息)

```sql
CREATE TABLE parking_spaces (
    id TEXT PRIMARY KEY,
    space_number TEXT NOT NULL,
    community_id TEXT NOT NULL,
    occupied INTEGER DEFAULT 0,
    owner_id TEXT,
    FOREIGN KEY (community_id) REFERENCES communities(id),
    FOREIGN KEY (owner_id) REFERENCES owners(id)
);
```

### 4.3 费用与服务相关表

#### transactions 表 (交易信息)

```sql
CREATE TABLE transactions (
    id TEXT PRIMARY KEY,
    owner_id TEXT NOT NULL,
    apartment_id TEXT,
    type INTEGER NOT NULL,
    amount REAL NOT NULL,
    payment_date INTEGER NOT NULL,
    description TEXT,
    FOREIGN KEY (owner_id) REFERENCES owners(id),
    FOREIGN KEY (apartment_id) REFERENCES apartments(id)
);
```

#### fee_standards 表 (费用标准信息)

```sql
CREATE TABLE fee_standards (
    id TEXT PRIMARY KEY,
    type INTEGER NOT NULL,
    rate REAL NOT NULL,
    valid_from INTEGER NOT NULL,
    valid_to INTEGER DEFAULT 0
);
```

#### service_types 表 (服务类型信息)

```sql
CREATE TABLE service_types (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT
);
```

#### service_records 表 (服务记录信息)

```sql
CREATE TABLE service_records (
    id TEXT PRIMARY KEY,
    staff_id TEXT NOT NULL,
    building_id TEXT,
    apartment_id TEXT,
    service_type_id TEXT NOT NULL,
    service_time INTEGER NOT NULL,
    description TEXT,
    FOREIGN KEY (staff_id) REFERENCES staff(id),
    FOREIGN KEY (building_id) REFERENCES buildings(id),
    FOREIGN KEY (apartment_id) REFERENCES apartments(id),
    FOREIGN KEY (service_type_id) REFERENCES service_types(id)
);
```

### 4.4 管理相关表

#### staff_assignments 表 (服务人员分配信息)

```sql
CREATE TABLE staff_assignments (
    id TEXT PRIMARY KEY,
    staff_id TEXT NOT NULL,
    building_id TEXT NOT NULL,
    assign_time INTEGER NOT NULL,
    end_time INTEGER DEFAULT 0,
    FOREIGN KEY (staff_id) REFERENCES staff(id),
    FOREIGN KEY (building_id) REFERENCES buildings(id)
);
```

#### tokens 表 (用户令牌信息)

```sql
CREATE TABLE tokens (
    token TEXT PRIMARY KEY,
    user_id TEXT NOT NULL,
    user_type INTEGER NOT NULL,
    expire_time INTEGER NOT NULL
);
```

## 5. 函数实现指南

### 5.1 基本结构

每个函数应遵循以下基本结构：

```c
bool function_name(Database *db, const char *token, /* 其他参数 */)
{
    // 1. 参数验证
    if (!db || !token /* 验证其他参数 */)
        return false;

    // 2. 权限验证（如果需要）
    int weight;
    UserType user_type;
    if (!validate_token(db, token, &weight, &user_type))
    {
        return false;
    }

    // 3. 角色权限检查（如果需要）
    if (user_type != USER_ADMIN && /* 其他条件 */)
    {
        return false;
    }

    // 4. 准备SQL语句
    const char *sql = "SQL语句";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        return false;
    }

    // 5. 绑定参数
    sqlite3_bind_text(stmt, 1, parameter1, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, parameter2);
    // ...

    // 6. 执行SQL
    rc = sqlite3_step(stmt);
    bool result = (rc == SQLITE_DONE); // 或处理查询结果

    // 7. 清理资源
    sqlite3_finalize(stmt);
    return result;
}
```

### 5.2 常见操作示例

#### 创建记录

```c
bool create_entity(Database *db, const char *token, Entity *entity)
{
    // 验证及权限检查...

    // 构造INSERT SQL
    const char *sql = "INSERT INTO entities (id, name, ...) VALUES (?, ?, ...);";

    // 准备语句、绑定参数、执行...
}
```

#### 查询记录

```c
bool get_entity_by_id(Database *db, const char *id, Entity *entity)
{
    // 验证...

    // 构造SELECT SQL
    const char *sql = "SELECT * FROM entities WHERE id = ?;";

    // 准备语句、绑定参数、执行查询、填充结构体...
}
```

#### 更新记录

```c
bool update_entity(Database *db, const char *token, Entity *entity)
{
    // 验证及权限检查...

    // 构造UPDATE SQL
    const char *sql = "UPDATE entities SET name = ?, ... WHERE id = ?;";

    // 准备语句、绑定参数、执行...
}
```

#### 删除记录

```c
bool delete_entity(Database *db, const char *token, const char *id)
{
    // 验证及权限检查...

    // 构造DELETE SQL
    const char *sql = "DELETE FROM entities WHERE id = ?;";

    // 准备语句、绑定参数、执行...
}
```
