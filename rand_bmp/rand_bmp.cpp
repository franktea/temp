#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>

#pragma pack(push, 1)

struct BMPFileHeader {
    uint16_t file_type{0x4D42};
    uint32_t file_size{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offset_data{0};
};

struct BMPInfoHeader {
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bit_count{0};
    uint32_t compression{0};
    uint32_t size_image{0};
    int32_t x_pixels_per_meter{0};
    int32_t y_pixels_per_meter{0};
    uint32_t colors_used{0};
    uint32_t colors_important{0};
};
#pragma pack(pop)

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} hsv;

rgb hsv2rgb(hsv in) {
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

void generateBMPImage(int width, int height) {
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    info_header.size = sizeof(BMPInfoHeader);
    info_header.width = width;
    info_header.height = height;
    info_header.bit_count = 24;

    const int padding_amount = ((4 - (width * 3) % 4) % 4);

    file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    file_header.file_size = file_header.offset_data + (width * 3 + padding_amount) * height;

    std::vector<uint8_t> img_data(width * height * 3);

    //srand((unsigned)time(0));
    std::random_device r;
    std::mt19937 e1(r());
    std::uniform_int_distribution<int> dist(0, 255);
    /*for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            img_data[(y * width + x) * 3 + 0] = dist(e1); // Blue
            img_data[(y * width + x) * 3 + 1] = dist(e1); // Green
            img_data[(y * width + x) * 3 + 2] = dist(e1); // Red
        }
    }*/
    std::generate(img_data.begin(), img_data.end(), 
        [&dist, &e1]() { return dist(e1); });

    std::ofstream file("random.bmp", std::ios::out | std::ios::binary);
    if (file) {
        file.write((const char *)&file_header, sizeof(file_header));
        file.write((const char *)&info_header, sizeof(info_header));
        file.write((const char *)img_data.data(), img_data.size());
        file.close();
        std::cout << "BMP image created." << std::endl;
    } else {
        std::cerr << "Unable to open file." << std::endl;
    }
}

int main() {
    /*int width, height;
    std::cout << "Enter width: ";
    std::cin >> width;
    std::cout << "Enter height: ";
    std::cin >> height;*/

    generateBMPImage(1000, 1000);

    return 0;
}
