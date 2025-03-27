# 物业管理系统

## TODO

- [ ] 缴费状态判断函数

### Admin

- [ ] 完成你的界面逻辑！
- [ ] 对小区楼宇、房屋、物业服务人员、物业服务人员类型、收费标准、业主、停车位等信息进行管理，包括增加、修改、删除等。
- [ ] 分配或调整物业服务人员的负责区域，分配李四为小区第 8-10 栋楼的保安、王五为第 1、3 栋楼的保洁
- [ ] 查询某业主某年是否缴费、查询某业主所有缴费情况
- [ ] 对查询出来的业主按照姓名进行排序，或多属性排序
- [ ] 统计某年所有未缴费的业主数量、截止某时间已经缴费的业主数量
- [ ] 对用户的密码进行重置
- [ ] 数据备份、数据恢复

### Staff

- [ ] 查看自己负责哪些区域以及相关信息
- [ ] 给业主发送缴费提醒
- [ ] 查询某业主某年是否缴费、查询某业主所有缴费情况（同 Admin）
- [ ] 对查询出来的业主按照姓名进行排序，或多属性排序
- [ ] 统计某年所有未缴费的业主数量、截止某时间已经缴费的业主数量（同 Admin）

### Owner

- [1 ] 对自己的信息进行修改
- [ 2-2] 缴费
- [2-1] 自己的缴费记录进行各种查询
- [3-3] 查看为自己房屋服务的管家、保安、保洁人员的信息
- [3-1] 查询小区楼宇信息、物业收费标准信息
- [ 2-1改] 查询出来的缴费记录按照缴费时间进行排序
- [ 2-5] 统计几年内缴费的总金额

## 目前在做的事

大家把自己正在写以及写完了的东西写在下面，免得写重了

dch：

ptc：

- 我在下面的项目总要求里添加了数据库的表单数据结构预期实现，请按照那个作为预期的数据库结构
- user_id 是字符串！(至少需要 40 位的数组来存放)

fxl：

zzm：

## 项目指北

[项目开发指南](./doc/guide.md)

[项目总要求](./doc/require.md)

[SQLite 如何使用 C 语言调用？](https://zetcode.com/db/sqlitec)

同时，你可以在名为 practice 的 branch 里进行开发的练习（可以在 Github Desktop 中切换）

![项目时序图](./images/项目时序图.png)

![项目类图](./images/项目类图.png)

## 快速开始

### 使用 Windows 开始

最简便的方式是直接下载 Visual Studio 和 Github Desktop

先使用 github desktop 克隆(clone)下本仓库，然后使用 vs 从文件夹打开，打开本项目，VS 应当会配置好所有内容，可以直接开始

请**务必**挂上梯子或者有良好的网络环境（如校园网）

请在 VS 的扩展商店搜索并安装`force utf-8`插件，来统一项目中文编码（需要推出 VS 来完成插件安装）

### 使用 Linux 开始

如果你还没有 linux 环境，可以参照[如何安装 wsl 及 Ubuntu](#如何安装-wsl-及-ubuntu)部分进行操作后再继续本节。

#### 安装所需的必要软件

- gcc
- git
- cmake
- ninja

ubuntu 下仅需：

```bash
sudo apt-get install -y ninja-build git gcc g++ cmake pkg-config
```

#### 安装 vcpkg 并进行必要配置

选择一个你喜欢的地方拉下 vcpkg，并执行编译安装脚本

```bash
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg && ./bootstrap-vcpkg.sh
```

然后你需要将你的 vcpkg 安装位置写入环境变量

```bash
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
# 如需永久存在，请写入.bashrc或.zshrc
```

然后你应该拉下本项目并额外配置 cmake 参数来使用 vcpkg

在此项目根目录下创建 CMakeUserPresets.json 文件，内容如下：

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "引号内填入vcpkg文件夹的绝对路径"
      }
    }
  ]
}
```

#### 使用 cmake 构建

使用 CMake 配置构建，过程中需要梯子

```bash
cmake --preset=default
```

然后生成项目

```bash
cmake --build build
```

然后就能在 build 文件夹下看到构建好的文件了

#### 也可以直接使用脚本来构建和测试

运行:

```bash
./build.sh
```

然后：

```bash
./run.sh
```

## 初始化工作环境 $^{*}$

1.  根据您的操作系统及 CPU 指令架构，在仓库根目录下的 `.filter-tools` 目录中选择对应的文件，记录其文件名 $^{**}$
2.  在克隆的仓库中启动终端，使用以下命令在仓库中使用如下命令添加 Filter

        # 仅替换 `<platform>` `<arch>` 为您的平台，不要更改相对路径
        ## 除非你知道你在做什么

        git config --local filter.garbage2null.smudge "./.filter-tools/garbage2null-<platform>-<arch>"
        git config --local filter.garbage2null.clean "./.filter-tools/garbage2null-<platform>-<arch>"

## 自动化文档生成

本项目采用 Doxygen/JavaDoc 风格的注释，可以使用 doxygen 来自动化生成文档

先安装 doxygen

```bash
sudo apt install -y doxygen doxygen-gui
```

运行即可

```bash
doxywizard
```

## 如何安装 wsl 及 Ubuntu

### 启用 windows 功能

在启用或关闭 window 功能选项卡（可以搜索得到）中打开虚拟机平台（Virtual Machine Platform）和适用于 Linux 的 Windows 子系统选项，重启电脑

### 更新 wsl 内核并安装 ubuntu22.04

打开 powershell，依次运行：

```powershell
wsl --set-default-version 2

wsl --update

# 目前建议版本使用 22.04 为宜
wsl --install -d Ubuntu-22.04
```

过程中会要求给 ubuntu 取个用户名和密码（密码不会显示你输入的，输入完直接回车即可）

### 配置 vscode

下载[vscode](https://code.visualstudio.com/Download)并安装

打开，在扩展选项卡（几个小方块那个图标）中的搜索栏搜索 Chinese，选择简体中文插件安装；然后搜索 wsl，选择 WSL 插件安装。然后右下角会提示重启 vscode，点击重启即可。

然后点击 vscode 左下角的`><`状图标，在顶部选择连接到 WSL（也可能是英文的），即可进入刚安装好的 ubuntu 环境

此时按下`ctrl + ~ `即可打开终端，输入

```bash
sudo apt update
# 会提示你输入密码，密码就是创建 ubuntu 时你取的那个，直接输入、回车即可，密码不会显示在屏幕上
```

```bash
sudo apt upgrade -y
```

更新完成后，请参照[使用 Linux 开始](#使用-linux-开始)部分继续。
