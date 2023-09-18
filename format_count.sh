#!/bin/bash

# 格式化
find include/ -type f -name "*.hpp" -exec clang-format -i {} \;
find src/ example/ test/ -type f -name "*.cpp" -exec clang-format -i {} \;

# 统计代码行数
cloc --git `git branch --show-current`
