import os
import subprocess
import time
import shutil
import zipfile

# commit/更改信息
commit_message = '降低阈值LV2'
# 芯片类型
chip_type = 'ab16'
# dcf文件名
dcf_name = "app.dcf"
# release的prd文件名 "_xxx_"（添加在日期之前，项目名之后）
release_prd_name = ""
# 放入共享盘的目录(默认share_folder + share_proj_path)
share_proj_path = r'深圳市祥云昌国际电子有限公司\ZBA-0322_L0124\固件'

#####以下若无特殊需求，无需更改#####
# 共享盘路径
share_folder = r"\\192.168.20.20\ShareFolder\AI_solutions\zbitai-delivery\Project-Delivery\项目交付"
# release_note文件名
release_note_name = 'release_note.txt'
# history文件名
history_separator = ('='*80)
history_name = 'history.txt'
# Downloader工具路径
downloader_path = r"D:\bluetrum_tools\Downloader_v3.3.3_授权版"
downloader_exe = os.path.join(downloader_path, "Downloader.exe")
# KEY文件路径
key_path = r"D:\bluetrum_tools\key\AB160-HENGSHUO(9612).key"

def get_file_type(file_path):
    return os.path.splitext(file_path)[-1]

# 得到工程名
def get_project_name(fabs_path, chip_type):
    project_path = fabs_path
    project_name = None
    while True:
        project_path = os.path.dirname(project_path)
        project_name = os.path.basename(project_path)
        
        if chip_type in project_name:
            break
    return project_path

# 提取校验码
def extract_check_code(output_text):
    """从Downloader输出中提取校验码"""
    lines = output_text.split('\n')
    check_code = ""
    program_size = ""
    auth_code = ""
    project_guid = ""
    
    for line in lines:
        line = line.strip()
        if '程序大小:' in line:
            program_size = line.split('程序大小:')[1].strip()
        elif '校验码:' in line:
            check_code = line.split('校验码:')[1].strip()
        elif '授权码[0][配置]:' in line:
            auth_code = line.split('授权码[0][配置]:')[1].strip()
        elif '项目GUID:' in line:
            project_guid = line.split('项目GUID:')[1].strip().rstrip('，')  # 移除末尾的中文逗号
    
    return {
        'program_size': program_size,
        'check_code': check_code,
        'auth_code': auth_code,
        'project_guid': project_guid
    }

# 得到文件路径
def find_file_by_name(path, name):
    for root, dirs, files in os.walk(path):
        for file in files:
            if name in file:
                return os.path.join(root, file)
    return None

# 复制文件
def copy_file(src, dst):
    return shutil.copy(src, dst)

# 运行Downloader生成PRD文件
def run_downloader(dcf_path, output_prd_path, setting_path=None):
    """运行Downloader.exe生成PRD文件"""
    cmd = [
        downloader_exe,
        "-o", output_prd_path,
        "-i", dcf_path,
        "-k", key_path
    ]
    
    # 如果有设置文件，添加-s参数
    if setting_path and os.path.exists(setting_path):
        cmd.extend(["-s", setting_path])
    
    print("执行命令:", " ".join(cmd))
    
    try:
        # 使用Popen来执行命令，这样可以与进程交互
        process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding='gbk',  # Downloader通常使用GBK编码
            cwd=downloader_path  # 在Downloader目录下运行
        )
        
        # 读取输出
        output_lines = []
        while True:
            line = process.stdout.readline()
            if line:
                print(line.strip())  # 实时显示输出
                output_lines.append(line)
            
            # 检查是否需要按键继续
            if "按任意键继续" in line or "按任意键继续 . . ." in line:
                # 发送回车键继续
                process.stdin.write('\n')
                process.stdin.flush()
            
            # 检查进程是否结束
            if process.poll() is not None:
                # 读取剩余的输出
                remaining_output = process.stdout.read()
                if remaining_output:
                    print(remaining_output)
                    output_lines.append(remaining_output)
                break
        
        # 获取错误输出
        stderr_output = process.stderr.read()
        if stderr_output:
            print("错误输出:", stderr_output)
            output_lines.append(stderr_output)
        
        # 合并所有输出
        output_text = ''.join(output_lines)
        
        # 等待进程完成
        process.wait()
        
        # 提取校验码信息
        check_info = extract_check_code(output_text)
        
        # 检查文件是否生成
        if os.path.exists(output_prd_path):
            print(f"PRD文件已生成: {output_prd_path}")
            return output_text, check_info
        else:
            print(f"PRD文件未生成: {output_prd_path}")
            return None, None
        
    except Exception as e:
        print(f"执行Downloader出错: {e}")
        return None, None

# 创建文件
def create_file(fname, info):
    with open(fname, 'w', encoding='utf-8') as f:
        f.write(info)

# 在文件开头加内容
def append_file_head(fname, info):
    if not os.path.exists(fname):
        create_file(fname, info)
        return
    
    fread = open(fname, 'r', encoding='utf-8')
    fcontent = fread.read()
    fread.close()
    with open(fname, 'w+', encoding='utf-8') as f:
        f.seek(0, 0)
        f.write(info + fcontent)

# 在文件末尾加内容
def append_file_tail(fname, info):
    with open(fname, 'a', encoding='utf-8') as f:
        f.write(info)
    return

# 打包文件
def zip_release_files(zip_name, file_list):
    with zipfile.ZipFile(zip_name, 'w') as zipf:
        for file in file_list:
            # 只添加文件本身，不包含目录路径
            zipf.write(file, os.path.basename(file))

def main():
    # 获取当前时间
    current_time = time.strftime("%Y%m%d_%H%M", time.localtime())
    print(f"当前时间: {current_time}")
    
    # 获取项目路径
    project_path = get_project_name(os.path.abspath(__file__), chip_type)
    # 获取项目名
    project_name = os.path.basename(project_path)
    print(f"项目路径: {project_path}")
    print(f"项目名: {project_name}")
    
    # 获取dcf文件路径
    dcf_path = find_file_by_name(project_path, dcf_name)
    if not dcf_path:
        print(f"错误: 找不到 {dcf_name} 文件")
        return
    print(f"DCF文件路径: {dcf_path}")
    
    # 查找设置文件
    setting_file = find_file_by_name(project_path, "Boombox.setting")
    if setting_file:
        print(f"找到设置文件: {setting_file}")
    else:
        print("未找到设置文件，将使用默认设置")
    
    # 创建临时目录用于生成prd文件
    release_path = os.path.dirname(os.path.abspath(__file__))
    temp_prd_path = os.path.join(release_path, "temp_app.prd")
    
    # 运行Downloader生成PRD文件并获取输出信息
    print("\n正在生成PRD文件...")
    output_text, check_info = run_downloader(dcf_path, temp_prd_path, setting_file)
    
    if not output_text or not check_info:
        print("生成PRD文件失败")
        return
    
    print(f"\n提取的校验信息: {check_info}")
    
    # 检查PRD文件是否生成
    if not os.path.exists(temp_prd_path):
        print("错误: PRD文件未生成")
        return
    
    # 重命名PRD文件
    if release_prd_name and release_prd_name != "":
        release_name = '_'.join((project_name, release_prd_name, current_time))
    else:
        release_name = '_'.join((project_name, current_time))
    
    # 使用校验码重命名文件
    if check_info['check_code']:
        prd_new_name = f"{release_name}_{check_info['check_code']}.prd"
    else:
        prd_new_name = release_name + ".prd"
    
    prd_new_path = os.path.join(release_path, prd_new_name)
    os.rename(temp_prd_path, prd_new_path)
    print(f"PRD文件已重命名为: {prd_new_name}")
    
    # 创建release note信息
    release_note_info = f"{prd_new_name}\n\n"
    release_note_info += f"程序大小: {check_info['program_size']}\n"
    release_note_info += f"校验码: {check_info['check_code']}\n"
    release_note_info += f"授权码[0][配置]: {check_info['auth_code']}\n"
    release_note_info += f"项目GUID: {check_info['project_guid']}\n\n"
    release_note_info += f"提交信息: {commit_message}\n"
    
    # 保存release note
    release_note_path = os.path.join(release_path, release_note_name)
    create_file(release_note_path, release_note_info)
    
    # 更新history记录
    history_path = os.path.join(release_path, history_name)
    history_info = release_note_info + '\n\n' + history_separator + '\n\n'
    append_file_head(history_path, history_info)
    
    # 打包文件
    if check_info['check_code']:
        zip_name = f"{release_name}_{check_info['check_code']}.zip"
    else:
        zip_name = release_name + '.zip'
    
    zip_path = os.path.join(release_path, zip_name)
    zip_release_files(zip_path, [prd_new_path, release_note_path])
    print(f"已打包文件: {zip_name}")
    
    # 复制到共享盘
    share_proj_abspath = os.path.join(share_folder, share_proj_path)
    print(f"共享盘路径: {share_proj_abspath}")
    
    if not os.path.exists(share_proj_abspath):
        os.makedirs(share_proj_abspath)
    
    share_zip_path = os.path.join(share_proj_abspath, zip_name)
    copy_file(zip_path, share_zip_path)
    print(f"文件已复制到共享盘: {share_zip_path}")
    
    # 清理临时文件
    if os.path.exists(prd_new_path):
        os.remove(prd_new_path)
    if os.path.exists(release_note_path):
        os.remove(release_note_path)
    
    print("\n处理完成!")

if __name__ == '__main__':
    main()