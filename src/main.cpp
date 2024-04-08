
#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <future>
#include <queue>
#include <tuple>
#include <string>
#include <sstream>
#include <typeinfo>


#ifdef OPENCV_OUTPUT
#include <opencv2/opencv.hpp>
#endif

template<typename T>
class image_2d;

template<typename T>
class image_2d_view;

int digits_sum(int x) {
    int sum = 0;
    while (x) {
        auto d = std::div(x, 10);
        x = d.quot;
        sum += d.rem;
    }
    return sum;
}

std::byte* create_pad_one(std::byte* img, size_t w, size_t h) {
    
    size_t pw = w + 2;
    size_t ph = h + 2;
    auto padded = new std::byte[pw * ph];
    std::memset(padded, 0, sizeof(std::byte) * pw * ph);
    
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            padded[j * pw + pw + i + 1] = img[j * w + i];
        }
    }

    return padded;
}

template <typename T, typename U>
std::pair<T, U> operator+(const std::pair<T, U>& lhs, const std::pair<T, U>& rhs) {
    return {lhs.first + rhs.first, lhs.second + rhs.second};
}

template <typename T, typename U>
std::pair<T, U> operator-(const std::pair<T, U>& lhs, const std::pair<T, U>& rhs) {
    return {lhs.first - rhs.first, lhs.second - rhs.second};
}

template<typename T>
size_t flood_fill_inplace(const image_2d<T>& img, int x, int y, T color, std::unique_ptr<image_2d<T>>& mask_out) {
    
    if (img.at(x, y) != color) {
        //mask_out = nullptr;
        return 0;
    }
    
    auto e = std::make_pair(1, 1);
    // lookup table
    std::deque<std::pair<int, int>> p;
    // init lookup table with starting point (consider 1px padding)
    p.push_back(std::make_pair(x, y) + e);
    // connectivity map
    std::vector<std::pair<int, int>> conn = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    // create padded image to avoid boundary checks
    auto padded = img.copy(0, 0, img.get_width(), img.get_height(), 1);
    auto mask = std::make_unique<image_2d<T>>(img.get_width(), img.get_height());
    mask->at(x, y) = T{255};
    
    size_t cnt = 1;
    while (p.size() > 0) {
        auto c = p.front();
        p.pop_front();
        auto v = padded->at(c);
        for (auto d : conn) {
            auto n = c + d;
            auto mnc = n - e;
            if (v == padded->at(n) && mask->at(mnc) != T{255}) {
                mask->at(mnc) = T{255};
                p.push_back(n);
                cnt += 1;
            }
        }
    }
    mask_out = std::move(mask);
    return cnt;
}


template<typename T>
class image_2d {
    std::vector<T> buffer;
    std::size_t width;
    std::size_t height;
    typedef std::pair<int, int> point_2d;

public:
    image_2d(std::size_t width, std::size_t height) : width(width), height(height), buffer(width * height) {}
    image_2d(const image_2d& other) : width(other.width), height(other.height), buffer(other.buffer) {}
    image_2d(image_2d&& other) noexcept : width(other.width), height(other.height), buffer(std::move(other.buffer)) {
        other.width = 0;
        other.height = 0;
    }

    T& at(int x, int y) noexcept {
        if (x >= width || y >= height) {
            // TODO: handle out of bounds access
        }
        return buffer[y * width + x];
    }

    const T& at(int x, int y) const noexcept {
        if (x >= width || y >= height) {
            // TODO: handle out of bounds access
        }
        return buffer[y * width + x];
    }

    T& at(point_2d p) {
        return at(p.first, p.second);
    }

    const T& at(point_2d p) const {
        return at(p.first, p.second);
    }

    decltype(buffer.data()) data() noexcept {
        return buffer.data();
    }

    std::unique_ptr<image_2d<T>> copy(int x0, int y0, std::size_t w, std::size_t h, std::size_t pad) const {
        if (x0 != 0 || y0 != 0 || w != 0 || h != 0) {
            if (x0 + w > width || y0 + h > height) {
                throw std::out_of_range("invalid copy rect specified");
            }
        }
        auto c = std::make_unique<image_2d<T>>(w + 2*pad, h + 2*pad);
        
        // auto rng = std::views::iota(0, h);


        for (int y = 0; y < h; y++) {
            auto line_begin = buffer.begin() + (y0 + y) * width + x0;
            auto line_end = line_begin + w;
            auto out_line_begin = c->buffer.begin() + (y + pad) * (w + 2*pad) + pad;
            std::copy(line_begin, line_end, out_line_begin);
        }

        return c;
    }

    std::size_t get_width() const { return width; }
    std::size_t get_height() const { return height; }
};

template<typename T>
class image_2d_view {
    image_2d<T>& parent;
    std::size_t x0, y0, width, height;

public:
    image_2d_view(image_2d<T>& parent, std::size_t x0, std::size_t y0, std::size_t width, std::size_t height)
        : parent(parent), x0(x0), y0(y0), width(width), height(height) {
        if (x0 + width > parent.get_width() || y0 + height > parent.get_height()) {
            throw std::out_of_range("Subimage coordinates out of range");
        }
    }

    T& at(std::size_t x, std::size_t y) {
        if (x >= width || y >= height) {
            throw std::out_of_range("Pixel coordinates out of range");
        }
        return parent.at(x0 + x, y0 + y);
    }

    const T& at(std::size_t x, std::size_t y) const {
        if (x >= width || y >= height) {
            throw std::out_of_range("Pixel coordinates out of range");
        }
        return parent.at(x0 + x, y0 + y);
    }

    std::size_t get_width() const { return width; }
    std::size_t get_height() const { return height; }

    // Delete copy constructor and copy assignment operator
    image_2d_view(const image_2d_view&) = delete;
    image_2d_view& operator=(const image_2d_view&) = delete;
};

template<typename... Args, std::size_t... Is>
std::tuple<Args...> parse_arguments_impl(char** argv, std::index_sequence<Is...>) {
    std::tuple<Args...> args;
    std::istringstream iss;
    // TODO: add is_convertible from string predicate, so we can detect invalid arguments
    ((iss.str(argv[Is + 1]), iss >> std::get<Is>(args), iss.clear()), ...);
    return args;
}

template<typename... Args>
std::tuple<Args...> parse_arguments(int argc, char** argv) {
    if (argc - 1 != sizeof...(Args)) {
        throw std::invalid_argument("Incorrect number of arguments");
    }
    return parse_arguments_impl<Args...>(argv, std::index_sequence_for<Args...>{});
}

template<typename... Args>
std::string type_names() {
    std::stringstream ss;
    ((ss << typeid(Args).name() << ", "), ...);
    std::string result = ss.str();
    // remove trailing comma and space
    result.pop_back();
    result.pop_back();
    return result;
}

int main(int argc, char **argv) {

    #ifndef EXPERIMENTS

    try {
        auto [bound, width, height, px, py] = parse_arguments<int, int, int, int, int>(argc, argv);
        std::cout << "bound: " << bound << "\n";
        std::cout << "width: " << width << "\n";
        std::cout << "height: " << height << "\n";
        std::cout << "px: " << px << "\n";
        std::cout << "py: " << py << "\n";

        // grain size should be such that L3 == num_threads * grain_size * grain_size  
        int grain_size = 512;
        int num_threads = std::thread::hardware_concurrency();
        std::queue<std::future<void>> futures;
        image_2d<std::byte> img(width, height);

        auto x_view = std::views::iota(0, width) | std::views::chunk(grain_size);
        auto y_view = std::views::iota(0, height) | std::views::chunk(grain_size);

        for (auto x_chunk : x_view) {
            for (auto y_chunk : y_view) {
                
                if (futures.size() >= num_threads) {
                    futures.front().get();
                    futures.pop();
                }

                futures.push(std::async(std::launch::async, [&, x_chunk, y_chunk]() {
                    for (auto x : x_chunk) {
                        for (auto y : y_chunk) {
                            if (digits_sum(x) + digits_sum(y) < bound) {
                                img.at(x, y) = std::byte{255};
                            }
                        }
                    }
                }));

            }
        }

        // wait for all futures to complete
        while (!futures.empty()) {
            futures.front().get();
            futures.pop();
        }

        #ifdef OPENCV_OUTPUT
        {
            cv::Mat out(height, width, CV_8UC1, img.data());
            cv::imwrite("map.bmp", out);
        }
        #endif

        std::unique_ptr<image_2d<std::byte>> mask;
        auto cnt = flood_fill_inplace<std::byte>(img, px, py, std::byte{255}, mask);
        std::cout << "number of connected pixels: " << cnt << "\n";

        #ifdef OPENCV_OUTPUT
        {
            cv::Mat out(height, width, CV_8UC1, mask->data());
            cv::imwrite("mask.bmp", out);
        }
        #endif
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }


    #else
    #endif
 
    return 0;
}