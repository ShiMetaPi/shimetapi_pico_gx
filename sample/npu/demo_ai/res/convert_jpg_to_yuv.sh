
#!/bin/bash
input_dir="./jpg_dir/jpg_data10"
output_dir="./yuv_dir/yuv_data10"
resolution="2560x1440"
pix_fmt="nv21"

# 创建输出目录
mkdir -p "$output_dir"

# 遍历所有jpg文件
for jpg_file in "$input_dir"/*.jpg; do
    if [[ -f "$jpg_file" ]]; then
        filename=$(basename "$jpg_file" .jpg)
        ffmpeg -i "$jpg_file" -s "$resolution" -pix_fmt "$pix_fmt" -f rawvideo "$output_dir/${filename}.yuv"
        echo "已转换: $jpg_file → $output_dir/${filename}.yuv"
    fi
done

echo "转换完成！输出目录: $output_dir"
