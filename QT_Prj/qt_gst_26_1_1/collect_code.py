import os

OUTPUT_FILE = "all_code.txt"
SKIP_DIRS = {"build", ".git", ".vscode", "cmake-build-debug"}

# 存储目录结构：key为目录路径，value为该目录下的.h/.cpp文件列表
dir_structure = {}
total_lines = 0
file_count = 0

# 第一步：遍历目录，收集目录结构和文件信息
for root, dirs, files in os.walk("."):
    # 移除要跳过的目录（不遍历这些目录）
    dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
    
    # 筛选当前目录下的.h/.cpp文件
    cpp_h_files = [f for f in files if f.endswith((".cpp", ".h"))]
    if cpp_h_files:  # 只记录有有效文件的目录
        dir_structure[root] = sorted(cpp_h_files)  # 排序让文件列表更整洁

# 第二步：写入内容到输出文件
with open(OUTPUT_FILE, "w", encoding="utf-8") as out:
    # 1. 先写入目录结构概览
    out.write("========== 工程目录结构（仅含.h/.cpp）==========\n\n")
    for dir_path, files in dir_structure.items():
        # 写入目录路径
        out.write(f"📂 {dir_path}:\n")
        # 写入该目录下的文件列表（缩进显示，更易读）
        for file_name in files:
            out.write(f"  └─ {file_name}\n")
        out.write("\n")  # 空行分隔不同目录
    
    # 分隔线，区分目录结构和代码内容
    out.write("=" * 50 + "\n")
    out.write("========== 所有.h/.cpp文件内容 ==========\n\n")

    # 2. 写入每个文件的内容和行数（保留原有逻辑）
    for root, dirs, files in os.walk("."):
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
        
        for name in files:
            if name.endswith((".cpp", ".h")):
                path = os.path.join(root, name)

                try:
                    with open(path, "r", encoding="utf-8", errors="ignore") as f:
                        lines = f.readlines()
                except Exception as e:
                    print(f"Skip {path}: {e}")
                    continue

                line_count = len(lines)
                total_lines += line_count
                file_count += 1

                out.write(f"\n========== {path} ==========\n")
                out.write(f"Lines: {line_count}\n\n")

                for line in lines:
                    out.write(line)

    # 3. 结尾统计信息
    out.write("\n\n=====================================\n")
    out.write(f"Total files: {file_count}\n")
    out.write(f"Total lines: {total_lines}\n")

# 控制台输出统计结果
print("=====================================")
print(f"Files: {file_count}")
print(f"Lines: {total_lines}")
print(f"Output written to {OUTPUT_FILE}")