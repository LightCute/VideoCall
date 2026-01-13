import os

OUTPUT_FILE = "all_code.txt"
SKIP_DIRS = {"build", ".git", ".vscode"}

total_lines = 0
file_count = 0

with open(OUTPUT_FILE, "w", encoding="utf-8") as out:
    for root, dirs, files in os.walk("."):
        # 移除要跳过的目录
        dirs[:] = [d for d in dirs if d not in SKIP_DIRS]

        for name in files:
            if name.endswith(".cpp") or name.endswith(".h"):
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

    # 结尾统计
    out.write("\n\n=====================================\n")
    out.write(f"Total files: {file_count}\n")
    out.write(f"Total lines: {total_lines}\n")

print("=====================================")
print(f"Files: {file_count}")
print(f"Lines: {total_lines}")
print(f"Output written to {OUTPUT_FILE}")
