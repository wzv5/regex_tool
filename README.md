# Regex Tool

一个跨平台的正则表达式调试工具。

正则引擎来自 rust 官方库，界面使用 Qt，通过 cxx bridge 把 rust 库静态链接进主程序。

## 特性

* 实时解析正则语法树
* 支持 匹配、替换、分割 3 种模式
* 支持高亮语法树中选中的部分
* 支持高亮匹配项
* 跨平台，已测试 Windows 和 Arch Linux

## 截图

![screenshot_1](screenshot_1.jpg)

![screenshot_2](screenshot_2.jpg)

## 编译

安装 rust，并安装依赖：

``` bash
cargo install cxxbridge-cmd
```

之后直接使用 cmake 编译即可。

Windows 平台或许需要设置 `CMAKE_PREFIX_PATH`，添加 Qt 库的相关路径，如 `-DCMAKE_PREFIX_PATH=D:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5`

## 已知问题

* rust 正则引擎不支持前向、后向匹配