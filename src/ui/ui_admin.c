#include "ui/ui_admin.h"
#include "ui/ui_main.h"
#include "ui/ui_login.h"
#include "models/building.h"
#include "models/apartment.h"
#include "models/user.h"
#include "models/transaction.h"
#include "models/service.h"
#include "db/db_query.h"
#include "utils/utils.h"
#include "utils/file_ops.h"
#include "utils/console.h" // 添加此行
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 显示管理员主界面
void show_admin_main_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 物业管理系统 - 管理员主界面 =====\n");
        printf("1. 信息管理\n");
        printf("2. 服务分配管理\n");
        printf("3. 信息查询\n");
        printf("4. 信息排序\n");
        printf("5. 信息统计\n");
        printf("6. 系统维护\n");
        printf("0. 退出登录\n");
        printf("请选择操作 [0-6]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 1:
            show_info_management_screen(db, token);
            break;
        case 2:
            show_service_assignment_screen(db, token);
            break;
        case 3:
            show_info_query_screen(db, token);
            break;
        case 4:
            show_info_sort_screen(db, token);
            break;
        case 5:
            show_info_statistics_screen(db, token);
            break;
        case 6:
            show_system_maintenance_screen(db, token);
            break;
        case 0:
            exit_flag = true;
            break;
        default:
            show_error("无效的选择");
        }
    }
}

// 信息管理界面
void show_info_management_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息管理 =====\n");
        printf("1. 添加小区\n");
        printf("2. 添加楼宇\n");
        printf("3. 添加房屋\n");
        printf("4. 添加业主\n");
        printf("5. 添加服务人员\n");
        printf("6. 添加管理员\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-6]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
        {
            // 添加小区
            Community community = {0};
            printf("请输入小区名称: ");
            fgets(community.name, sizeof(community.name), stdin);
            community.name[strcspn(community.name, "\n")] = '\0'; // 移除换行符

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成小区ID失败");
                break;
            }
            strncpy(community.id, uuid, sizeof(community.id) - 1);
            free(uuid);

            if (add_community(db, token, &community))
            {
                show_success("小区添加成功");
            }
            else
            {
                show_error("小区添加失败");
            }
            break;
        }
        case 2:
        {
            // 添加楼宇
            Building building = {0};
            printf("请输入楼宇编号: ");
            fgets(building.building_number, sizeof(building.building_number), stdin);
            building.building_number[strcspn(building.building_number, "\n")] = '\0'; // 移除换行符

            printf("请输入小区ID: ");
            fgets(building.community_id, sizeof(building.community_id), stdin);
            building.community_id[strcspn(building.community_id, "\n")] = '\0'; // 移除换行符

            printf("请输入楼层数: ");
            scanf("%d", &building.floors);
            getchar(); // 消耗换行符

            printf("请输入每层单元数: ");
            scanf("%d", &building.units_per_floor);
            getchar(); // 消耗换行符

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成楼宇ID失败");
                break;
            }
            strncpy(building.id, uuid, sizeof(building.id) - 1);
            free(uuid);

            if (add_building(db, token, &building))
            {
                show_success("楼宇添加成功");
            }
            else
            {
                show_error("楼宇添加失败");
            }
            break;
        }
        case 3:
        {
            // 添加房屋
            Apartment apartment = {0};
            printf("请输入楼宇ID: ");
            fgets(apartment.building_id, sizeof(apartment.building_id), stdin);
            apartment.building_id[strcspn(apartment.building_id, "\n")] = '\0'; // 移除换行符

            printf("请输入楼层: ");
            scanf("%d", &apartment.floor);
            getchar(); // 消耗换行符

            printf("请输入单元: ");
            scanf("%d", &apartment.unit);
            getchar(); // 消耗换行符

            printf("请输入面积: ");
            scanf("%f", &apartment.area);
            getchar(); // 消耗换行符

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成房屋ID失败");
                break;
            }
            strncpy(apartment.id, uuid, sizeof(apartment.id) - 1);
            free(uuid);

            if (add_apartment(db, token, &apartment))
            {
                show_success("房屋添加成功");
            }
            else
            {
                show_error("房屋添加失败");
            }
            break;
        }
        case 4:
        {
            // 添加业主
            Owner owner = {0};
            char password[64] = {0};
            printf("请输入业主姓名: ");
            fgets(owner.name, sizeof(owner.name), stdin);
            owner.name[strcspn(owner.name, "\n")] = '\0'; // 移除换行符

            printf("请输入联系电话: ");
            fgets(owner.phone, sizeof(owner.phone), stdin);
            owner.phone[strcspn(owner.phone, "\n")] = '\0'; // 移除换行符

            printf("请输入密码: ");
            read_password(password, sizeof(password));

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成业主ID失败");
                break;
            }
            strncpy(owner.id, uuid, sizeof(owner.id) - 1);
            free(uuid);

            owner.weight = 3; // 业主权重

            if (create_owner(db, &owner, password))
            {
                show_success("业主添加成功");
            }
            else
            {
                show_error("业主添加失败");
            }
            break;
        }
        case 5:
        {
            // 添加服务人员
            Staff staff = {0};
            char password[64] = {0};
            printf("请输入服务人员姓名: ");
            fgets(staff.name, sizeof(staff.name), stdin);
            staff.name[strcspn(staff.name, "\n")] = '\0'; // 移除换行符

            printf("请输入联系电话: ");
            fgets(staff.phone, sizeof(staff.phone), stdin);
            staff.phone[strcspn(staff.phone, "\n")] = '\0'; // 移除换行符

            printf("请输入服务类型: ");
            fgets(staff.service_type, sizeof(staff.service_type), stdin);
            staff.service_type[strcspn(staff.service_type, "\n")] = '\0'; // 移除换行符

            printf("请输入密码: ");
            read_password(password, sizeof(password));

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成服务人员ID失败");
                break;
            }
            strncpy(staff.id, uuid, sizeof(staff.id) - 1);
            free(uuid);

            staff.weight = 2; // 服务人员权重

            if (create_staff(db, &staff, password))
            {
                show_success("服务人员添加成功");
            }
            else
            {
                show_error("服务人员添加失败");
            }
            break;
        }
        case 6:
        {
            // 添加管理员
            Admin admin = {0};
            char password[64] = {0};
            printf("请输入管理员姓名: ");
            fgets(admin.name, sizeof(admin.name), stdin);
            admin.name[strcspn(admin.name, "\n")] = '\0'; // 移除换行符

            printf("请输入密码: ");
            read_password(password, sizeof(password));

            char *uuid = generate_uuid();
            if (!uuid)
            {
                show_error("生成管理员ID失败");
                break;
            }
            strncpy(admin.id, uuid, sizeof(admin.id) - 1);
            free(uuid);

            admin.weight = 1; // 管理员权重

            if (create_admin(db, &admin, password))
            {
                show_success("管理员添加成功");
            }
            else
            {
                show_error("管理员添加失败");
            }
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}

// 服务分配界面
void show_service_assignment_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 服务分配管理 =====\n");
        printf("1. 分配服务人员到楼宇\n");
        printf("2. 取消服务人员的楼宇分配\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-2]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
        {
            // 分配服务人员到楼宇
            char staff_id[32] = {0};
            char building_id[32] = {0};

            printf("请输入服务人员ID: ");
            fgets(staff_id, sizeof(staff_id), stdin);
            staff_id[strcspn(staff_id, "\n")] = '\0'; // 移除换行符

            printf("请输入楼宇ID: ");
            fgets(building_id, sizeof(building_id), stdin);
            building_id[strcspn(building_id, "\n")] = '\0'; // 移除换行符

            if (assign_staff_to_building(db, token, staff_id, building_id))
            {
                show_success("服务人员分配成功");
            }
            else
            {
                show_error("服务人员分配失败");
            }
            break;
        }
        case 2:
        {
            // 取消服务人员的楼宇分配
            char staff_id[32] = {0};
            char building_id[32] = {0};

            printf("请输入服务人员ID: ");
            fgets(staff_id, sizeof(staff_id), stdin);
            staff_id[strcspn(staff_id, "\n")] = '\0'; // 移除换行符

            printf("请输入楼宇ID: ");
            fgets(building_id, sizeof(building_id), stdin);
            building_id[strcspn(building_id, "\n")] = '\0'; // 移除换行符

            if (unassign_staff_from_building(db, token, staff_id, building_id))
            {
                show_success("服务人员分配取消成功");
            }
            else
            {
                show_error("服务人员分配取消失败");
            }
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}

// 信息查询界面
void show_info_query_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息查询 =====\n");
        printf("1. 查询楼宇信息\n");
        printf("2. 查询房屋信息\n");
        printf("3. 查询业主信息\n");
        printf("4. 查询服务人员信息\n");
        printf("5. 查询管理员信息\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-5]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
        {
            // 查询楼宇信息
            char building_id[32] = {0};
            Building building = {0};

            printf("请输入楼宇ID: ");
            fgets(building_id, sizeof(building_id), stdin);
            building_id[strcspn(building_id, "\n")] = '\0'; // 移除换行符

            if (get_building(db, building_id, &building))
            {
                printf("楼宇ID: %s\n", building.id);
                printf("楼宇编号: %s\n", building.building_number);
                printf("小区ID: %s\n", building.community_id);
                printf("楼层数: %d\n", building.floors);
                printf("每层单元数: %d\n", building.units_per_floor);
            }
            else
            {
                show_error("楼宇信息查询失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 2:
        {
            // 查询房屋信息
            char apartment_id[32] = {0};
            Apartment apartment = {0};

            printf("请输入房屋ID: ");
            fgets(apartment_id, sizeof(apartment_id), stdin);
            apartment_id[strcspn(apartment_id, "\n")] = '\0'; // 移除换行符

            if (get_apartment(db, apartment_id, &apartment))
            {
                printf("房屋ID: %s\n", apartment.id);
                printf("楼宇ID: %s\n", apartment.building_id);
                printf("楼层: %d\n", apartment.floor);
                printf("单元: %d\n", apartment.unit);
                printf("面积: %.2f\n", apartment.area);
                printf("是否已入住: %s\n", apartment.occupied ? "是" : "否");
                printf("物业费余额: %.2f\n", apartment.property_fee_balance);
                printf("是否已缴费: %s\n", apartment.fee_paid ? "是" : "否");
            }
            else
            {
                show_error("房屋信息查询失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 3:
        {
            // 查询业主信息
            char owner_id[32] = {0};
            Owner owner = {0};

            printf("请输入业主ID: ");
            fgets(owner_id, sizeof(owner_id), stdin);
            owner_id[strcspn(owner_id, "\n")] = '\0'; // 移除换行符

            if (get_owner_by_id(db, owner_id, &owner))
            {
                printf("业主ID: %s\n", owner.id);
                printf("姓名: %s\n", owner.name);
                printf("联系电话: %s\n", owner.phone);
                printf("楼宇ID: %s\n", owner.building_id);
                printf("房屋ID: %s\n", owner.apartment_id);
                printf("是否需要缴费提醒: %s\n", owner.notification_required ? "是" : "否");
            }
            else
            {
                show_error("业主信息查询失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 4:
        {
            // 查询服务人员信息
            char staff_id[32] = {0};
            Staff staff = {0};

            printf("请输入服务人员ID: ");
            fgets(staff_id, sizeof(staff_id), stdin);
            staff_id[strcspn(staff_id, "\n")] = '\0'; // 移除换行符

            if (get_staff_by_id(db, staff_id, &staff))
            {
                printf("服务人员ID: %s\n", staff.id);
                printf("姓名: %s\n", staff.name);
                printf("联系电话: %s\n", staff.phone);
                printf("服务类型: %s\n", staff.service_type);
            }
            else
            {
                show_error("服务人员信息查询失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 5:
        {
            // 查询管理员信息
            char admin_id[32] = {0};
            Admin admin = {0};

            printf("请输入管理员ID: ");
            fgets(admin_id, sizeof(admin_id), stdin);
            admin_id[strcspn(admin_id, "\n")] = '\0'; // 移除换行符

            if (get_admin_by_id(db, admin_id, &admin))
            {
                printf("管理员ID: %s\n", admin.id);
                printf("姓名: %s\n", admin.name);
            }
            else
            {
                show_error("管理员信息查询失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}

// 信息排序界面
void show_info_sort_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息排序 =====\n");
        printf("1. 按楼宇编号排序\n");
        printf("2. 按房屋面积排序\n");
        printf("3. 按业主姓名排序\n");
        printf("4. 按服务人员姓名排序\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-4]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
        {
            // 按楼宇编号排序
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT * FROM buildings ORDER BY building_number;", &result) == SQLITE_OK)
            {
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("楼宇ID: %s\n", result.rows[i].values[0]);
                    printf("楼宇编号: %s\n", result.rows[i].values[1]);
                    printf("小区ID: %s\n", result.rows[i].values[2]);
                    printf("楼层数: %s\n", result.rows[i].values[3]);
                    printf("每层单元数: %s\n", result.rows[i].values[4]);
                    printf("\n");
                }
                free_query_result(&result);
            }
            else
            {
                show_error("楼宇信息排序失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 2:
        {
            // 按房屋面积排序
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT * FROM apartments ORDER BY area;", &result) == SQLITE_OK)
            {
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("房屋ID: %s\n", result.rows[i].values[0]);
                    printf("楼宇ID: %s\n", result.rows[i].values[1]);
                    printf("楼层: %s\n", result.rows[i].values[2]);
                    printf("单元: %s\n", result.rows[i].values[3]);
                    printf("面积: %s\n", result.rows[i].values[4]);
                    printf("是否已入住: %s\n", result.rows[i].values[5]);
                    printf("物业费余额: %s\n", result.rows[i].values[6]);
                    printf("是否已缴费: %s\n", result.rows[i].values[7]);
                    printf("\n");
                }
                free_query_result(&result);
            }
            else
            {
                show_error("房屋信息排序失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 3:
        {
            // 按业主姓名排序
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT * FROM owners ORDER BY name;", &result) == SQLITE_OK)
            {
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("业主ID: %s\n", result.rows[i].values[0]);
                    printf("姓名: %s\n", result.rows[i].values[1]);
                    printf("联系电话: %s\n", result.rows[i].values[2]);
                    printf("楼宇ID: %s\n", result.rows[i].values[3]);
                    printf("房屋ID: %s\n", result.rows[i].values[4]);
                    printf("是否需要缴费提醒: %s\n", result.rows[i].values[5]);
                    printf("\n");
                }
                free_query_result(&result);
            }
            else
            {
                show_error("业主信息排序失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 4:
        {
            // 按服务人员姓名排序
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT * FROM staff ORDER BY name;", &result) == SQLITE_OK)
            {
                for (int i = 0; i < result.row_count; i++)
                {
                    printf("服务人员ID: %s\n", result.rows[i].values[0]);
                    printf("姓名: %s\n", result.rows[i].values[1]);
                    printf("联系电话: %s\n", result.rows[i].values[2]);
                    printf("服务类型: %s\n", result.rows[i].values[3]);
                    printf("\n");
                }
                free_query_result(&result);
            }
            else
            {
                show_error("服务人员信息排序失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}

// 信息统计界面
void show_info_statistics_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 信息统计 =====\n");
        printf("1. 统计楼宇数量\n");
        printf("2. 统计房屋数量\n");
        printf("3. 统计业主数量\n");
        printf("4. 统计服务人员数量\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-4]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
        {
            // 统计楼宇数量
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT COUNT(*) FROM buildings;", &result) == SQLITE_OK)
            {
                printf("楼宇数量: %s\n", result.rows[0].values[0]);
                free_query_result(&result);
            }
            else
            {
                show_error("楼宇数量统计失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 2:
        {
            // 统计房屋数量
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT COUNT(*) FROM apartments;", &result) == SQLITE_OK)
            {
                printf("房屋数量: %s\n", result.rows[0].values[0]);
                free_query_result(&result);
            }
            else
            {
                show_error("房屋数量统计失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 3:
        {
            // 统计业主数量
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT COUNT(*) FROM owners;", &result) == SQLITE_OK)
            {
                printf("业主数量: %s\n", result.rows[0].values[0]);
                free_query_result(&result);
            }
            else
            {
                show_error("业主数量统计失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        case 4:
        {
            // 统计服务人员数量
            QueryResult result = {0};
            if (db_simple_query(db, "SELECT COUNT(*) FROM staff;", &result) == SQLITE_OK)
            {
                printf("服务人员数量: %s\n", result.rows[0].values[0]);
                free_query_result(&result);
            }
            else
            {
                show_error("服务人员数量统计失败");
            }
            printf("按任意键继续...");
            getch();
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}

// 系统维护界面
void show_system_maintenance_screen(Database *db, const char *token)
{
    int choice;
    bool exit_flag = false;

    while (!exit_flag)
    {
        system("clear || cls");
        printf("===== 系统维护 =====\n");
        printf("1. 修改密码\n");
        printf("2. 重置用户密码\n");
        printf("3. 数据备份\n");
        printf("4. 数据恢复\n");
        printf("0. 返回上级菜单\n");
        printf("请选择操作 [0-4]: ");

        scanf("%d", &choice);
        getchar(); // 消耗换行符

        switch (choice)
        {
        case 0:
            exit_flag = true;
            break;
        case 1:
            show_change_password_screen(db, token, USER_ADMIN);
            break;
        case 2:
            show_reset_password_screen(db, token);
            break;
        case 3:
        {
            // 数据备份功能
            char backup_path[512];

            system("clear || cls");
            printf("===== 数据备份 =====\n");
            printf("请输入备份文件路径: ");
            fgets(backup_path, sizeof(backup_path), stdin);
            backup_path[strcspn(backup_path, "\n")] = '\0'; // 移除换行符

            if (db_backup(db, backup_path) == SQLITE_OK)
            {
                char success_message[600];
                sprintf(success_message, "数据备份成功，备份路径：%s", backup_path);
                show_success(success_message);
            }
            else
            {
                show_error("数据备份失败");
            }
            break;
        }
        case 4:
        {
            // 数据恢复功能
            char backup_path[512];

            system("clear || cls");
            printf("===== 数据恢复 =====\n");
            printf("请输入备份文件路径: ");
            fgets(backup_path, sizeof(backup_path), stdin);
            backup_path[strcspn(backup_path, "\n")] = '\0'; // 移除换行符

            if (!file_exists(backup_path))
            {
                show_error("备份文件不存在");
                break;
            }

            if (show_confirmation("确定要恢复数据吗？当前数据将被覆盖"))
            {
                if (db_restore(db, backup_path) == SQLITE_OK)
                {
                    show_success("数据恢复成功");
                }
                else
                {
                    show_error("数据恢复失败");
                }
            }
            break;
        }
        default:
            show_error("无效的选择");
        }
    }
}