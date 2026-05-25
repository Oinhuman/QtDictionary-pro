# QtDictionary Pro

基于 [bugcat9/QtDictionary](https://github.com/bugcat9/QtDictionary.git) 二次优化的 Qt Widgets 英语电子词典。项目保留红黑树词典检索结构，并补充了更直接的页面导航、键盘优先的背词流程和持久化错题本。

## 功能亮点

- 单词查询：支持英文查询、中文释义、音标显示、朗读和查询历史。
- 背单词：支持中文提示、回车提交、自动进入下一题，答错后自动加入错题本。
- 错题本：错题会保存到本机应用数据目录的 `wrong_words.json`，可复习、朗读、标记掌握或清空。
- 词库管理：支持添加、删除、更新单词，也支持从文本文件批量导入。
- 词库路径：统一使用 `txt/output_utf8_file.txt`，并附带 `txt/phonetics.txt` 音标数据。

## 界面预览

以下图片由当前程序运行时导出，展示的是实际 Qt 界面效果。

<table>
  <tr>
    <td width="50%">
      <strong>单词查询</strong><br>
      <img src="./image/optimized-search.png" alt="单词查询界面" width="100%">
    </td>
    <td width="50%">
      <strong>背单词</strong><br>
      <img src="./image/optimized-remember.png" alt="背单词界面" width="100%">
    </td>
  </tr>
  <tr>
    <td colspan="2">
      <strong>错题本</strong><br>
      <img src="./image/optimized-wrongbook.png" alt="错题本界面" width="100%">
    </td>
  </tr>
</table>

## 项目结构

```text
QtDictionary.pro          qmake 项目文件
main.cpp                  程序入口
mainwindow.*              主窗口、导航、查询、背词和错题本逻辑
dictionary.*              词典读写和查询逻辑
rbtree.*                  红黑树实现
showword.*                单词详情窗口
txt/                      词库和音标数据
image/                    README 展示图片
transfer.py               词库转换脚本
```

## 编译运行

安装 Qt 后，在项目根目录执行：

```powershell
qmake QtDictionary.pro
mingw32-make
```

编译后运行可执行文件。若运行时词库为空，请确认可执行文件同级目录存在：

```text
txt/output_utf8_file.txt
txt/phonetics.txt
```

在本机 Qt 6.11 + MinGW 环境中，建议把 Qt 自带 MinGW 放到 `PATH` 前面，避免系统中旧版 GCC 头文件干扰编译。

```powershell
$env:PATH="C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.0\mingw_64\bin;" + $env:PATH
qmake QtDictionary.pro
mingw32-make
```

## 设计说明

本项目面向英语四级词汇电子辞典场景，使用红黑树维护动态索引，以支持查询、插入、删除和更新等操作。相比原始版本，本优化版重点改善日常使用流程：减少菜单层级、强化键盘操作、保留错题数据，并统一词库生成、构建复制和运行时读取路径。
