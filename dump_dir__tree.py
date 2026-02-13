import os

OUTPUT_FILE = "directory_tree.txt"

# ===== 可配置：忽略的目录名（命中即剪掉整个子树）=====
IGNORE_DIRS = {
    ".git",
    "build",
    ".idea",
    ".vscode",
    "__pycache__",
    "cmake-build-debug"
}

def dump_tree(root_path, file):
    root_path = os.path.abspath(root_path)

    for root, dirs, files in os.walk(root_path):
        # 🔴 关键：就地修改 dirs，剪掉整个子目录树
        dirs[:] = [d for d in dirs if d not in IGNORE_DIRS]

        # 计算层级（用于缩进）
        level = root.replace(root_path, "").count(os.sep)
        indent = "  " * level

        # 打印目录
        dir_name = os.path.basename(root)
        if root == root_path:
            dir_name = os.path.basename(root_path)

        file.write(f"{indent}{dir_name}/\n")

        # 打印文件
        sub_indent = "  " * (level + 1)
        for filename in sorted(files):
            file.write(f"{sub_indent}{filename}\n")

if __name__ == "__main__":
    base_dir = os.getcwd()   # 当前目录
    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        dump_tree(base_dir, f)

    print(f"[OK] Directory tree saved to {OUTPUT_FILE}")
