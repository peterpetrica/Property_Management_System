## 开始的必要软件

- gcc（对于 windows 用户：MSVC）
- git
- cmake

windows 下请去官网下[CMAKE](https://cmake.org/download)和[GIT](https://git-scm.com/downloads)，MSVC 可通过 Visual Studio 2022 安装或者在此下载单独的[build-tools](https://visualstudio.microsoft.com/zh-hans/visual-cpp-build-tools)

linux 下可以直接使用包管理器安装

## 安装 Ninja

ubuntu 下仅需：

```bash
sudo apt-get install ninja-build
```

对于 windows 用户：

在此下载 ninja 的预编译版本[NINJA](https://github.com/ninja-build/ninja/releases)

解压

然后你需要通过 Windows 系统环境变量面板将当前包含 ninja.exe 的目录添加到环境变量

## 安装 vcpkg 并进行必要配置

选择一个你喜欢的地方拉下 vcpkg，并执行编译安装脚本

```bash
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg && ./bootstrap-vcpkg.sh
```

对于 windows 用户：

请尽量使用 powershell 而非 cmd

```powershell
git clone https://github.com/microsoft/vcpkg.git

cd vcpkg; .\bootstrap-vcpkg.bat
```

现在你需要将你的 vcpkg 安装位置写入环境变量

```bash
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
# 如需永久存在，请写入.bashrc或.zshrc
```

对于 windows 用户：

```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"
# 如需永久存在，请通过 Windows 系统环境变量面板进行设置
```

然后你应该拉下本项目

```bash
https://github.com/PeterPtroc/Property_Management_System.git

cd Property_Management_System
```

然后你应该额外配置 cmake 参数来使用 vcpkg

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

## 使用 cmake 构建

使用 CMake 配置构建，过程中需要梯子

```bash
cmake --preset=default
```

然后生成项目

```bash
cmake --build build
```

然后就能在 build 文件夹下看到构建好的文件了

## 也可以直接使用脚本来构建和测试

运行:

```bash
./run.sh
```

或者对于 windows 用户，运行：

```powershell
./run.ps1
```
