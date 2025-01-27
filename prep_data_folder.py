Import('env', 'projenv')

import os
import gzip
import shutil
import glob

# HELPER TO GZIP A FILE
def gzip_file(src_path, dst_path):
    print(f"Gzipping {src_path} to {dst_path}")
    try:
        with open(src_path, 'rb') as src, gzip.open(dst_path, 'wb') as dst:
            for chunk in iter(lambda: src.read(4096), b""):
                dst.write(chunk)
    except FileNotFoundError:
        print(f"File not found: {src_path}")
    except PermissionError:
        print(f"Permission denied for: {src_path} or {dst_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

def prepare_www_files(source, target, env):
    #WARNING -  this script will DELETE your 'data' dir and recreate an empty one to copy/gzip files from 'data_src'
    #           so make sure to edit your files in 'data_src' folder as changes madt to files in 'data' woll be LOST
    #           
    #           If 'data_src' dir doesn't exist, and 'data' dir is found, the script will autimatically
    #           rename 'data' to 'data_src


    #add filetypes (extensions only) to be gzipped before uploading. Everything else will be copied directly
    filetypes_to_gzip = ['js', 'html', 'css']

    
    print('[COPY/GZIP DATA FILES]')

    data_dir = os.path.join(env.get('PROJECT_DIR'), 'data')
    data_src_dir = os.path.join(env.get('PROJECT_DIR'), 'data_src')

    print(f"Data dir: {data_dir}")
    print(f"Data src dir: {data_src_dir}")
    

    if(os.path.exists(data_dir) and not os.path.exists(data_src_dir) ):
        print('  "data" dir exists, "data_src" not found.')
        print('  renaming "' + data_dir + '" to "' + data_src_dir + '"')
        os.rename(data_dir, data_src_dir)

    if(os.path.exists(data_dir)):
        print('  Deleting data dir ' + data_dir)
        shutil.rmtree(data_dir)

    print('  Re-creating empty data dir ' + data_dir)
    os.mkdir(data_dir)

    files_to_gzip = []
    for extension in filetypes_to_gzip:
        files_to_gzip.extend(glob.glob(os.path.join(data_src_dir, '*.' + extension)))
    
    print('  files to gzip: ' + str(files_to_gzip))

    all_files = glob.glob(os.path.join(data_src_dir, '*.*'))
    files_to_copy = list(set(all_files) - set(files_to_gzip))

    print('  files to copy: ' + str(files_to_copy))

    for file in files_to_copy:
        print('  Copying file: ' + file + ' to data dir')
        shutil.copy(file, data_dir)
    
    for file in files_to_gzip:
        print('  GZipping file: ' + file + ' to data dir')
        gzip_file(file,os.path.join(data_dir, os.path.basename(file) + '.gz'))
        #with open(file) as src, gzip.open(os.path.join(data_dir, os.path.basename(file) + '.gz'), 'wb') as dst:        
        #    dst.writelines(src)

    print('[/COPY/GZIP DATA FILES]')
    
env.AddPreAction('$BUILD_DIR/littlefs.bin', prepare_www_files)