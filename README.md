## 使用 windows 开始

最简便的方式是直接下载 Visual Studio 和 Github Desktop

先使用 github desktop 克隆(clone)下本仓库，然后使用vs从文件夹打开，打开本项目，VS 应当会配置好所有内容，可以直接开始

请**务必**挂上梯子或者有良好的网络环境（如校园网）

请在VS的扩展商店搜索并安装```force utf-8```插件，来统一项目中文编码（需要推出VS来完成插件安装）

## 使用 linux 开始

### 安装所需的必要软件

- gcc
- git
- cmake
- ninja

ubuntu 下仅需：

```bash
sudo apt-get install -y ninja-build git gcc cmake
```

### 安装 vcpkg 并进行必要配置

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

### 使用 cmake 构建

使用 CMake 配置构建，过程中需要梯子

```bash
cmake --preset=default
```

然后生成项目

```bash
cmake --build build
```

然后就能在 build 文件夹下看到构建好的文件了

### 也可以直接使用脚本来构建和测试

运行:

```bash
./run.sh
```
