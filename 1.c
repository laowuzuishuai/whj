#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <dirent.h>
#include <unistd.h>
#include <sched.h>

void set_cpu_affinity() {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask); // 将进程绑定到CPU 0上

    if (sched_setaffinity(0, sizeof(mask), &mask) < 0) {
        perror("Could not set CPU affinity");
    }
}

void decode_jpeg(const char* input_path) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    JSAMPARRAY buffer;
    int row_stride;

    if ((infile = fopen(input_path, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", input_path);
        return;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
}

void print_progress(int progress, int total) {
    // 创建一个简单的文本进度条
    int width = 50; // 进度条的宽度
    int pos = (width * progress) / total; // 计算进度条的位置
    char bar[width + 1]; // 进度条字符串
    int i;

    memset(bar, 0, sizeof(bar)); // 初始化进度条字符串

    for (i = 0; i < pos; i++) {
        bar[i] = '=';
    }
    bar[pos] = '\0'; // 确保字符串以null结尾

    printf("\rProgress: [%-50s] %d%%", bar, (progress * 100) / total);
    fflush(stdout);
}

int main() {
    DIR *dir;
    struct dirent *ent;
    char input_dir[] = "/zstd/jpeg-test/photo";
    int file_count = 0;
    int processed_count = 0;
    int i;

    set_cpu_affinity();

    // 第一次遍历目录以计算文件数量
    if ((dir = opendir(input_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".jpg") != NULL || strstr(ent->d_name, ".jpeg") != NULL) {
                file_count++;
            }
        }
        closedir(dir);
    } else {
        perror("Unable to read directory");
        return EXIT_FAILURE;
    }

    // 第二次遍历目录以处理文件并显示进度
    if ((dir = opendir(input_dir)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".jpg") != NULL || strstr(ent->d_name, ".jpeg") != NULL) {
                char input_path[1024];
                sprintf(input_path, "%s/%s", input_dir, ent->d_name);
                decode_jpeg(input_path);
                processed_count++;

                // 更新进度条
                print_progress(processed_count, file_count);
            }
        }
        printf("\n"); // 在进度条完成后换行
        closedir(dir);
    } else {
        perror("Unable to read directory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

