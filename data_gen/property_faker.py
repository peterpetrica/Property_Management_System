import sqlite3
import random
import time
import uuid
import hashlib
from datetime import datetime, timedelta
from faker import Faker
import os

# 初始化Faker库
fake = Faker('zh_CN')

# 确保输出目录存在
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DB_DIR = os.path.join(SCRIPT_DIR, "Database")
os.makedirs(DB_DIR, exist_ok=True)
DB_PATH = os.path.join(DB_DIR, "property_management.db")

# 连接数据库
conn = sqlite3.connect(DB_PATH)
cursor = conn.cursor()

# 生成ID的函数
def gen_id():
    return str(uuid.uuid4())

# 密码哈希函数(从utils.c中的实现转换为Python)
def hash_password(password):
    salt = "pms"
    salted_password = password + salt
    return hashlib.sha256(salted_password.encode('utf-8')).hexdigest()

# 时间转换函数(Unix时间戳)
def to_timestamp(dt):
    return int(time.mktime(dt.timetuple()))

def from_timestamp(ts):
    return datetime.fromtimestamp(ts)

# 获取当前时间戳
current_time = to_timestamp(datetime.now())

# 创建数据库表
def create_tables():
    # 角色表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS roles (
        role_id TEXT PRIMARY KEY,
        role_name TEXT NOT NULL,
        permission_level INTEGER NOT NULL
    )
    ''')

    # 用户表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS users (
        user_id TEXT PRIMARY KEY,
        username TEXT NOT NULL UNIQUE,
        password_hash TEXT NOT NULL,
        name TEXT NOT NULL,
        phone_number TEXT,
        email TEXT,
        role_id TEXT NOT NULL,
        status INTEGER DEFAULT 1,
        registration_date INTEGER NOT NULL,
        FOREIGN KEY (role_id) REFERENCES roles(role_id)
    )
    ''')

    # 楼宇表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS buildings (
        building_id TEXT PRIMARY KEY,
        building_name TEXT NOT NULL,
        address TEXT NOT NULL,
        floors_count INTEGER NOT NULL
    )
    ''')

    # 房屋表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS rooms (
        room_id TEXT PRIMARY KEY,
        building_id TEXT NOT NULL,
        room_number TEXT NOT NULL,
        floor INTEGER NOT NULL,
        area_sqm REAL NOT NULL,
        owner_id TEXT,
        status INTEGER DEFAULT 0,
        FOREIGN KEY (building_id) REFERENCES buildings(building_id),
        FOREIGN KEY (owner_id) REFERENCES users(user_id)
    )
    ''')

    # 停车位表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS parking_spaces (
        parking_id TEXT PRIMARY KEY,
        parking_number TEXT NOT NULL,
        owner_id TEXT,
        status INTEGER DEFAULT 0,
        FOREIGN KEY (owner_id) REFERENCES users(user_id)
    )
    ''')

    # 人员类型表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS staff_types (
        staff_type_id TEXT PRIMARY KEY,
        type_name TEXT NOT NULL,
        description TEXT
    )
    ''')

    # 物业人员表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS staff (
        staff_id TEXT PRIMARY KEY,
        user_id TEXT NOT NULL,
        staff_type_id TEXT NOT NULL,
        hire_date INTEGER NOT NULL,
        status INTEGER DEFAULT 1,
        FOREIGN KEY (user_id) REFERENCES users(user_id),
        FOREIGN KEY (staff_type_id) REFERENCES staff_types(staff_type_id)
    )
    ''')

    # 服务区域表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS service_areas (
        area_id TEXT PRIMARY KEY,
        staff_id TEXT NOT NULL,
        building_id TEXT NOT NULL,
        assignment_date INTEGER NOT NULL,
        FOREIGN KEY (staff_id) REFERENCES staff(staff_id),
        FOREIGN KEY (building_id) REFERENCES buildings(building_id)
    )
    ''')

    # 服务记录表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS service_records (
        record_id TEXT PRIMARY KEY,
        staff_id TEXT NOT NULL,
        service_type TEXT NOT NULL,
        service_date INTEGER NOT NULL,
        description TEXT,
        status INTEGER DEFAULT 0,
        target_id TEXT NOT NULL,
        FOREIGN KEY (staff_id) REFERENCES staff(staff_id)
    )
    ''')

    # 费用标准表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS fee_standards (
        standard_id TEXT PRIMARY KEY,
        fee_type INTEGER NOT NULL,
        price_per_unit REAL NOT NULL,
        unit TEXT NOT NULL,
        effective_date INTEGER NOT NULL,
        end_date INTEGER DEFAULT 0
    )
    ''')

    # 交易表
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS transactions (
        transaction_id TEXT PRIMARY KEY,
        user_id TEXT NOT NULL,
        room_id TEXT,
        parking_id TEXT,
        fee_type INTEGER NOT NULL,
        amount REAL NOT NULL,
        payment_date INTEGER NOT NULL,
        due_date INTEGER NOT NULL,
        payment_method INTEGER DEFAULT 0,
        status INTEGER DEFAULT 0,
        period_start INTEGER NOT NULL,
        period_end INTEGER NOT NULL,
        FOREIGN KEY (user_id) REFERENCES users(user_id),
        FOREIGN KEY (room_id) REFERENCES rooms(room_id),
        FOREIGN KEY (parking_id) REFERENCES parking_spaces(parking_id)
    )
    ''')

# 初始化角色数据
def init_roles():
    roles = [
        ('role_admin', '管理员', 1),
        ('role_staff', '物业服务人员', 2),
        ('role_owner', '业主', 3)
    ]
    
    cursor.executemany(
        "INSERT OR IGNORE INTO roles (role_id, role_name, permission_level) VALUES (?, ?, ?)",
        roles
    )

# 初始化费用标准
def init_fee_standards():
    standards = [
        ('PF01', 1, 3.5, '元/㎡/月', to_timestamp(datetime(2024, 1, 1))),  # 物业费
        ('CF01', 2, 300.0, '元/月', to_timestamp(datetime(2024, 1, 1))),   # 地上停车费
        ('CF02', 2, 400.0, '元/月', to_timestamp(datetime(2024, 1, 1))),   # 地下停车费
        ('WF01', 3, 4.9, '元/m³', to_timestamp(datetime(2024, 1, 1))),     # 水费
        ('EF01', 4, 0.98, '元/kWh', to_timestamp(datetime(2024, 1, 1))),   # 电费
        ('GF01', 5, 3.2, '元/m³', to_timestamp(datetime(2024, 1, 1)))      # 燃气费
    ]
    
    cursor.executemany(
        "INSERT OR IGNORE INTO fee_standards (standard_id, fee_type, price_per_unit, unit, effective_date) VALUES (?, ?, ?, ?, ?)",
        standards
    )

# 初始化默认用户
def init_default_users():
    # 管理员
    admin_id = gen_id()
    cursor.execute(
        "INSERT OR IGNORE INTO users (user_id, username, password_hash, name, role_id, status, registration_date) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (admin_id, 'admin', hash_password('admin123'), '系统管理员', 'role_admin', 1, current_time)
    )
    
    # 物业服务人员
    staff_id = gen_id()
    cursor.execute(
        "INSERT OR IGNORE INTO users (user_id, username, password_hash, name, role_id, status, registration_date) VALUES (?, ?, ?, ?, ?, ?, ?)",
        (staff_id, 'staff', hash_password('staff123'), '物业服务员', 'role_staff', 1, current_time)
    )
    
    # 添加物业服务人员类型
    staff_type_id = gen_id()
    cursor.execute(
        "INSERT OR IGNORE INTO staff_types (staff_type_id, type_name, description) VALUES (?, ?, ?)",
        (staff_type_id, '普通物业人员', '默认物业服务人员类型')
    )
    
    # 添加到物业人员表
    cursor.execute(
        "INSERT OR IGNORE INTO staff (staff_id, user_id, staff_type_id, hire_date, status) VALUES (?, ?, ?, ?, ?)",
        (gen_id(), staff_id, staff_type_id, current_time, 1)
    )

# 生成楼宇数据
def generate_buildings(count=5):
    buildings = []
    for i in range(1, count + 1):
        building_id = gen_id()
        buildings.append((
            building_id,
            f"A{i}",
            f"示范小区{i}号楼",
            random.randint(18, 33)  # 楼层数
        ))
    
    cursor.executemany(
        "INSERT INTO buildings (building_id, building_name, address, floors_count) VALUES (?, ?, ?, ?)",
        buildings
    )
    return [b[0] for b in buildings]  # 返回所有生成的building_id

# 生成业主用户
def generate_owners(count=100):
    owners = []
    for i in range(count):
        user_id = gen_id()
        username = fake.user_name() + str(random.randint(1, 9999))
        password = hash_password('password123')
        name = fake.name()
        phone = fake.phone_number()
        email = fake.email()
        reg_date = to_timestamp(fake.date_time_between(
            start_date='-2y', end_date='now'
        ))
        
        owners.append((
            user_id,
            username,
            password,
            name,
            phone,
            email,
            'role_owner',
            1,  # 状态-正常
            reg_date
        ))
    
    cursor.executemany(
        "INSERT INTO users (user_id, username, password_hash, name, phone_number, email, role_id, status, registration_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
        owners
    )
    return [o[0] for o in owners]  # 返回所有生成的user_id

# 生成房屋数据
def generate_rooms(building_ids, owner_ids, count_per_building=None):
    rooms = []
    all_rooms = []  # 存储所有生成的room_id
    
    for building_id in building_ids:
        # 获取该楼栋的楼层数
        cursor.execute("SELECT floors_count FROM buildings WHERE building_id = ?", (building_id,))
        floors_count = cursor.fetchone()[0]
        
        # 为每个楼层生成房间
        for floor in range(1, floors_count + 1):
            for unit in range(1, 9):  # 每层8个单元，更符合实际情况
                room_id = gen_id()
                room_number = f"{floor}{unit:02d}"  # 例如 2楼1单元 = 201
                area = random.uniform(70, 150)  # 面积70-150平方米
                
                # 有75%的房间有业主
                owner_id = random.choice(owner_ids) if random.random() < 0.75 and owner_ids else None
                status = 1 if owner_id else 0  # 有业主状态为1，无业主为0
                
                rooms.append((
                    room_id,
                    building_id,
                    room_number,
                    floor,
                    round(area, 2),
                    owner_id,
                    status
                ))
                all_rooms.append((room_id, owner_id))
                
                # 批量插入，避免内存占用过大
                if len(rooms) >= 500:
                    cursor.executemany(
                        "INSERT INTO rooms (room_id, building_id, room_number, floor, area_sqm, owner_id, status) VALUES (?, ?, ?, ?, ?, ?, ?)",
                        rooms
                    )
                    rooms = []
    
    # 插入剩余房间记录
    if rooms:
        cursor.executemany(
            "INSERT INTO rooms (room_id, building_id, room_number, floor, area_sqm, owner_id, status) VALUES (?, ?, ?, ?, ?, ?, ?)",
            rooms
        )
    return all_rooms  # 返回(room_id, owner_id)的元组列表

# 生成停车位数据
def generate_parking_spaces(owner_ids, count=50):
    parkings = []
    all_parkings = []  # 存储所有生成的parking_id
    
    for i in range(1, count + 1):
        parking_id = gen_id()
        parking_number = f"P{i:03d}"
        
        # 随机决定是否有车位主人
        owner_id = random.choice(owner_ids) if random.random() < 0.6 else None
        status = 1 if owner_id else 0
        
        parkings.append((
            parking_id,
            parking_number,
            owner_id,
            status
        ))
        all_parkings.append((parking_id, owner_id))
    
    cursor.executemany(
        "INSERT INTO parking_spaces (parking_id, parking_number, owner_id, status) VALUES (?, ?, ?, ?)",
        parkings
    )
    return all_parkings  # 返回(parking_id, owner_id)的元组列表

# 生成物业人员类型数据
def generate_staff_types():
    types = [
        (gen_id(), '管家', '负责业主日常服务'),
        (gen_id(), '保安', '负责小区安全'),
        (gen_id(), '清洁工', '负责小区清洁'),
        (gen_id(), '维修工', '负责设施维修'),
        (gen_id(), '绿化工', '负责园艺管理')
    ]
    
    cursor.executemany(
        "INSERT INTO staff_types (staff_type_id, type_name, description) VALUES (?, ?, ?)",
        types
    )
    return [t[0] for t in types]  # 返回所有生成的staff_type_id

# 生成物业人员数据
def generate_staff(staff_type_ids, count=20):
    staff_members = []
    staff_ids = []
    
    # 先生成用户
    for i in range(count):
        user_id = gen_id()
        username = f"staff{i+1}"
        password = hash_password('staffpass')
        name = fake.name()
        phone = fake.phone_number()
        email = fake.email()
        reg_date = to_timestamp(fake.date_time_between(
            start_date='-3y', end_date='-1y'
        ))
        
        cursor.execute(
            "INSERT INTO users (user_id, username, password_hash, name, phone_number, email, role_id, status, registration_date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (user_id, username, password, name, phone, email, 'role_staff', 1, reg_date)
        )
        
        # 随机分配员工类型
        staff_type_id = random.choice(staff_type_ids)
        hire_date = to_timestamp(fake.date_time_between(
            start_date='-3y', end_date='-6m'
        ))
        
        staff_id = gen_id()
        staff_members.append((
            staff_id,
            user_id,
            staff_type_id,
            hire_date,
            1  # 状态-在职
        ))
        staff_ids.append(staff_id)
        
        # 批量插入，避免一次插入过多数据
        if len(staff_members) >= 50:
            cursor.executemany(
                "INSERT INTO staff (staff_id, user_id, staff_type_id, hire_date, status) VALUES (?, ?, ?, ?, ?)",
                staff_members
            )
            staff_members = []
    
    # 插入剩余员工记录
    if staff_members:
        cursor.executemany(
            "INSERT INTO staff (staff_id, user_id, staff_type_id, hire_date, status) VALUES (?, ?, ?, ?, ?)",
            staff_members
        )
    
    return staff_ids  # 返回所有生成的staff_id

# 生成服务区域数据
def generate_service_areas(staff_ids, building_ids):
    areas = []
    
    # 每个员工负责1-2个楼栋
    for staff_id in staff_ids:
        assigned_buildings = random.sample(
            building_ids, 
            min(random.randint(1, 2), len(building_ids))
        )
        
        for building_id in assigned_buildings:
            area_id = gen_id()
            assignment_date = to_timestamp(fake.date_time_between(
                start_date='-2y', end_date='-1m'
            ))
            
            areas.append((
                area_id,
                staff_id,
                building_id,
                assignment_date
            ))
    
    cursor.executemany(
        "INSERT INTO service_areas (area_id, staff_id, building_id, assignment_date) VALUES (?, ?, ?, ?)",
        areas
    )

# 生成服务记录数据
def generate_service_records(staff_ids, building_ids, room_data, count=500):
    service_types = [
        '日常巡检', '设备维修', '清洁服务', '安全检查', 
        '投诉处理', '访客登记', '快递接收', '紧急救援'
    ]
    
    service_status = [0, 1, 2]  # 0-待处理, 1-处理中, 2-已完成
    records = []
    
    for _ in range(count):
        record_id = gen_id()
        staff_id = random.choice(staff_ids)
        service_type = random.choice(service_types)
        
        # 服务日期在过去两年内
        service_date = to_timestamp(fake.date_time_between(
            start_date='-2y', end_date='now'
        ))
        
        description = fake.paragraph(nb_sentences=2)
        status = random.choices(service_status, weights=[0.1, 0.2, 0.7])[0]
        
        # 随机决定目标是楼栋还是房间
        if random.random() < 0.3:  # 30%概率为楼栋
            target_id = random.choice(building_ids)
        else:  # 70%概率为房间
            target_id = random.choice([r[0] for r in room_data])
        
        records.append((
            record_id,
            staff_id,
            service_type,
            service_date,
            description,
            status,
            target_id
        ))
        
        # 批量插入服务记录
        if len(records) >= 500:
            cursor.executemany(
                "INSERT INTO service_records (record_id, staff_id, service_type, service_date, description, status, target_id) VALUES (?, ?, ?, ?, ?, ?, ?)",
                records
            )
            records = []
    
    # 插入剩余服务记录
    if records:
        cursor.executemany(
            "INSERT INTO service_records (record_id, staff_id, service_type, service_date, description, status, target_id) VALUES (?, ?, ?, ?, ?, ?, ?)",
            records
        )

# 新增函数 - 按指定月份生成交易数据
def generate_transactions_with_months(room_data, parking_data, months):
    transactions = []
    
    # 获取费用标准
    cursor.execute("SELECT fee_type, price_per_unit FROM fee_standards")
    fee_standards = {row[0]: row[1] for row in cursor.fetchall()}
    
    payment_methods = [0, 1, 2, 3]  # 0-现金, 1-微信, 2-支付宝, 3-银行转账
    payment_status = [0, 1, 2]  # 0-未付, 1-已付, 2-逾期
    
    # 生成物业费账单
    for room_id, owner_id in room_data:
        if not owner_id:
            continue  # 跳过无主房间
        
        # 获取房间面积
        cursor.execute("SELECT area_sqm FROM rooms WHERE room_id = ?", (room_id,))
        area = cursor.fetchone()[0]
        
        # 为指定月份生成账单
        for month in months:
            period_start = to_timestamp(month.replace(day=1))
            if month.month == 12:
                period_end = to_timestamp(datetime(month.year+1, 1, 1) - timedelta(days=1))
            else:
                period_end = to_timestamp(datetime(month.year, month.month+1, 1) - timedelta(days=1))
            
            # 计算物业费
            amount = round(area * fee_standards[1], 2)  # 面积 * 单价
            
            # 设置支付和到期日期
            due_date = period_end
            
            # 决定支付状态
            status = random.choices(payment_status, weights=[0.1, 0.8, 0.1])[0]
            
            # 如果已付款，设置支付日期在账单开始和到期日之间
            if status == 1:  # 已付
                payment_date = to_timestamp(fake.date_time_between(
                    start_date=from_timestamp(period_start),
                    end_date=from_timestamp(due_date)
                ))
            elif status == 2:  # 逾期
                payment_date = 0  # 未支付
            else:  # 未付
                payment_date = 0
            
            payment_method = random.choice(payment_methods) if status == 1 else 0
            
            transactions.append((
                gen_id(),
                owner_id,
                room_id,
                None,  # parking_id为空
                1,     # fee_type=1 物业费
                amount,
                payment_date,
                due_date,
                payment_method,
                status,
                period_start,
                period_end
            ))
            
            # 批量插入交易记录，避免内存占用过大
            if len(transactions) >= 500:
                cursor.executemany(
                    "INSERT INTO transactions (transaction_id, user_id, room_id, parking_id, fee_type, amount, payment_date, due_date, payment_method, status, period_start, period_end) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    transactions
                )
                transactions = []
    
    # 生成停车费账单
    for parking_id, owner_id in parking_data:
        if not owner_id:
            continue  # 跳过无主停车位
        
        # 为指定月份生成账单
        for month in months:
            period_start = to_timestamp(month.replace(day=1))
            if month.month == 12:
                period_end = to_timestamp(datetime(month.year+1, 1, 1) - timedelta(days=1))
            else:
                period_end = to_timestamp(datetime(month.year, month.month+1, 1) - timedelta(days=1))
            
            # 计算停车费
            parking_fee_type = random.choice([2])  # 固定为2=停车费
            amount = fee_standards[parking_fee_type]
            
            # 设置支付和到期日期
            due_date = period_end
            
            # 决定支付状态
            status = random.choices(payment_status, weights=[0.1, 0.8, 0.1])[0]
            
            # 如果已付款，设置支付日期在账单开始和到期日之间
            if status == 1:  # 已付
                payment_date = to_timestamp(fake.date_time_between(
                    start_date=from_timestamp(period_start),
                    end_date=from_timestamp(due_date)
                ))
            elif status == 2:  # 逾期
                payment_date = 0  # 未支付
            else:  # 未付
                payment_date = 0
            
            payment_method = random.choice(payment_methods) if status == 1 else 0
            
            transactions.append((
                gen_id(),
                owner_id,
                None,  # room_id为空
                parking_id,
                2,     # fee_type=2 停车费
                amount,
                payment_date,
                due_date,
                payment_method,
                status,
                period_start,
                period_end
            ))
            
            # 批量插入交易记录
            if len(transactions) >= 500:
                cursor.executemany(
                    "INSERT INTO transactions (transaction_id, user_id, room_id, parking_id, fee_type, amount, payment_date, due_date, payment_method, status, period_start, period_end) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                    transactions
                )
                transactions = []
    
    # 插入剩余交易记录
    if transactions:
        cursor.executemany(
            "INSERT INTO transactions (transaction_id, user_id, room_id, parking_id, fee_type, amount, payment_date, due_date, payment_method, status, period_start, period_end) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            transactions
        )

# 生成测试数据的主函数
def generate_test_data():
    print("开始生成物业管理系统测试数据...")
    
    # 创建数据库表
    create_tables()
    print("数据库表创建完成")
    
    # 初始化基础数据
    init_roles()
    init_fee_standards()
    init_default_users()
    print("基础数据初始化完成")
    
    # 生成楼宇数据 - 减少到6栋
    building_ids = generate_buildings(count=6)
    print(f"生成了 {len(building_ids)} 栋楼宇")
    
    # 生成业主用户 - 设置为200个
    owner_ids = generate_owners(count=200)
    print(f"生成了 {len(owner_ids)} 个业主账户")
    
    # 生成房屋数据 - 不限制每栋楼房间数，自然生成
    room_data = generate_rooms(building_ids, owner_ids)
    print(f"生成了 {len(room_data)} 个房间")
    
    # 生成停车位数据 - 减少到房间数量的60%左右
    parking_count = int(len(room_data) * 0.6)
    parking_data = generate_parking_spaces(owner_ids, count=parking_count)
    print(f"生成了 {len(parking_data)} 个停车位")
    
    # 生成物业人员类型数据
    staff_type_ids = generate_staff_types()
    print(f"生成了 {len(staff_type_ids)} 种物业人员类型")
    
    # 生成物业人员数据 - 调整为每60个房间配1名物业人员
    staff_count = max(15, int(len(room_data) / 60))
    staff_ids = generate_staff(staff_type_ids, count=staff_count)
    print(f"生成了 {len(staff_ids)} 名物业人员")
    
    # 生成服务区域数据
    generate_service_areas(staff_ids, building_ids)
    print("生成了服务区域分配数据")
    
    # 生成服务记录数据 - 减少到500条
    generate_service_records(staff_ids, building_ids, room_data, count=500)
    print("生成了500条服务记录")
    
    # 生成交易数据 - 采用部分生成策略，每个拥有业主的房间和停车位只为最近6个月生成账单
    months_to_generate = 6  # 只生成最近6个月的账单
    now = datetime.now()
    recent_months = []
    
    for i in range(months_to_generate):
        month = now.replace(day=1) - timedelta(days=i*30)
        recent_months.append(month)
    
    # 采用自定义月份列表生成交易数据
    generate_transactions_with_months(room_data, parking_data, recent_months)
    
    # 计算实际生成的交易记录数量
    cursor.execute("SELECT COUNT(*) FROM transactions")
    transaction_count = cursor.fetchone()[0]
    print(f"生成了{transaction_count}条交易记录")
    
    # 提交所有更改
    conn.commit()
    print("所有数据已提交到数据库")
    
    # 显示数据库统计信息
    print("\n数据库统计:")
    tables = [
        "users", "roles", "buildings", "rooms", "parking_spaces",
        "staff_types", "staff", "service_areas", "service_records",
        "fee_standards", "transactions"
    ]
    
    for table in tables:
        cursor.execute(f"SELECT COUNT(*) FROM {table}")
        count = cursor.fetchone()[0]
        print(f"表 {table}: {count} 条记录")

# 执行主函数
if __name__ == "__main__":
    try:
        generate_test_data()
        print(f"\n测试数据生成成功！数据库保存在: {DB_PATH}")
    except Exception as e:
        print(f"生成测试数据时出错: {e}")
    finally:
        # 关闭数据库连接
        conn.close()
